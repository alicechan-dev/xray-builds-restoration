#include "editor_render/d3d11/EditorD3D11Renderer.h"

#include "editor_render/EditorRenderAssetRegistry.h"
#include "editor_render/EditorRenderGeometryCache.h"
#include "editor_render/EditorRenderScene.h"
#include "editor_render/EditorRenderOverlay.h"
#include "editor_render/EditorRenderProjection.h"
#include "editor_render/EditorSoftwareWireframeRenderer.h"
#include "editor_render/EditorStaticMeshGeometry.h"
#include "editor_view/EditorViewportState.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace
{
constexpr std::size_t MaximumGpuBytes = 512ull * 1024ull * 1024ull;
constexpr std::size_t MaximumAssetBytes = 128ull * 1024ull * 1024ull;
constexpr std::size_t MaximumUploadsPerFrame = 4;
constexpr std::size_t MaximumDrawCalls = 8192;
constexpr std::size_t MaximumOverlayVertices = 131072;
constexpr float Pi = 3.14159265358979323846f;

const char* ShaderSource = R"(
cbuffer FrameConstants : register(b0) { row_major float4x4 viewProjection; };
cbuffer ObjectConstants : register(b1) { row_major float4x4 world; float4 color; };
struct VSInput { float3 position : POSITION; };
struct VSOutput { float4 position : SV_POSITION; };
struct OverlayInput { float3 position : POSITION; float4 color : COLOR; };
struct OverlayOutput { float4 position : SV_POSITION; float4 color : COLOR; };
struct ScreenInput { float2 position : POSITION; float2 uv : TEXCOORD; };
struct ScreenOutput { float4 position : SV_POSITION; float2 uv : TEXCOORD; };
VSOutput VSMain(VSInput value) {
    VSOutput output;
    output.position = mul(mul(float4(value.position, 1.0), world), viewProjection);
    return output;
}
float4 PSMain(VSOutput value) : SV_TARGET { return color; }
OverlayOutput OverlayVS(OverlayInput value) {
    OverlayOutput output;
    output.position = mul(float4(value.position, 1.0), viewProjection);
    output.color = value.color;
    return output;
}
float4 OverlayPS(OverlayOutput value) : SV_TARGET { return value.color; }
ScreenOutput ScreenVS(ScreenInput value) {
    ScreenOutput output; output.position=float4(value.position,0.0,1.0);
    output.uv=value.uv; return output;
}
Texture2D labelTexture : register(t0);
SamplerState labelSampler : register(s0);
float4 ScreenPS(ScreenOutput value) : SV_TARGET {
    return labelTexture.Sample(labelSampler,value.uv);
}
)";

struct FrameConstants { XMFLOAT4X4 viewProjection; };
struct ObjectConstants { XMFLOAT4X4 world; XMFLOAT4 color; };
struct OverlayVertex { XMFLOAT3 position; XMFLOAT4 color; };
struct ScreenVertex { XMFLOAT2 position; XMFLOAT2 uv; };

XMFLOAT4 OverlayColor(EditorRenderOverlayStyle style)
{
    switch(style)
    {
    case EditorRenderOverlayStyle::Runtime: return {0.25f,0.9f,0.55f,1.0f};
    case EditorRenderOverlayStyle::Glow: return {0.3f,0.85f,0.9f,1.0f};
    case EditorRenderOverlayStyle::Light: return {1.0f,0.82f,0.25f,1.0f};
    case EditorRenderOverlayStyle::Unsupported: return {0.72f,0.72f,0.75f,1.0f};
    case EditorRenderOverlayStyle::Selected: return {1.0f,0.42f,0.08f,1.0f};
    case EditorRenderOverlayStyle::GizmoX: return {0.95f,0.2f,0.2f,1.0f};
    case EditorRenderOverlayStyle::GizmoZ: return {0.2f,0.45f,1.0f,1.0f};
    case EditorRenderOverlayStyle::Grid: return {0.18f,0.2f,0.22f,1.0f};
    default: return {0.35f,0.78f,0.85f,1.0f};
    }
}

XMMATRIX WorldMatrix(const EditorTransform& transform)
{
    return XMMatrixScaling(transform.sx, transform.sy, transform.sz) *
        XMMatrixRotationZ(transform.roll) *
        XMMatrixRotationX(transform.pitch) *
        XMMatrixRotationY(transform.yaw) *
        XMMatrixTranslation(transform.x, transform.y, transform.z);
}

XMMATRIX ViewMatrix(const EditorViewportCamera& camera)
{
    const float yaw = camera.yaw * Pi / 180.0f;
    const float pitch = -camera.pitch * Pi / 180.0f;
    const XMVECTOR direction = XMVectorSet(std::cos(pitch) * std::sin(yaw),
        std::sin(pitch), std::cos(pitch) * std::cos(yaw), 0.0f);
    return XMMatrixLookToLH(XMVectorSet(camera.x, camera.y, camera.z, 1.0f),
        direction, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

bool Compile(const char* entry, const char* target, ComPtr<ID3DBlob>& bytecode,
    std::string& reason)
{
    ComPtr<ID3DBlob> errors;
    const HRESULT result = D3DCompile(ShaderSource, std::strlen(ShaderSource),
        "wxSDKEditor-flat.hlsl", nullptr, nullptr, entry, target,
        D3DCOMPILE_ENABLE_STRICTNESS, 0, &bytecode, &errors);
    if (SUCCEEDED(result)) return true;
    reason = errors ? static_cast<const char*>(errors->GetBufferPointer()) :
        "D3DCompile failed.";
    return false;
}
}

class EditorD3D11Renderer::Impl
{
public:
    struct Geometry
    {
        ComPtr<ID3D11Buffer> vertices;
        ComPtr<ID3D11Buffer> indices;
        UINT indexCount = 0;
        std::size_t bytes = 0;
    };

    bool Initialize(void* window, int width, int height, std::string* reason)
    {
        Shutdown();
        if (!window) return Fail("Viewport HWND is unavailable.", reason);
        hwnd = static_cast<HWND>(window);
        DXGI_SWAP_CHAIN_DESC descriptor{};
        descriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        descriptor.SampleDesc.Count = 1;
        descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        descriptor.BufferCount = 2;
        descriptor.OutputWindow = hwnd;
        descriptor.Windowed = TRUE;
        descriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
        D3D_FEATURE_LEVEL level{};
        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr,
            D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels,
            static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION,
            &descriptor, &swapChain, &device, &level, &context);
#if defined(_DEBUG)
        if (FAILED(result))
        {
            flags &= ~D3D11_CREATE_DEVICE_DEBUG;
            result = D3D11CreateDeviceAndSwapChain(nullptr,
                D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels,
                static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION,
                &descriptor, &swapChain, &device, &level, &context);
        }
#endif
        if (FAILED(result))
        {
            diagnostics.usingWarp = true;
            result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP,
                nullptr, flags, levels, static_cast<UINT>(std::size(levels)),
                D3D11_SDK_VERSION, &descriptor, &swapChain, &device, &level,
                &context);
        }
        if (FAILED(result)) return Fail("D3D11 device creation failed.", reason);
        if (!CreatePipeline(reason) || !Resize(width, height, reason))
        { Shutdown(); return false; }
        diagnostics.initialized = true;
        diagnostics.status = diagnostics.usingWarp ? "D3D11 WARP" : "D3D11 hardware";
        return true;
    }

    bool CreatePipeline(std::string* reason)
    {
        std::string compileReason;
        ComPtr<ID3DBlob> vs, ps, overlayVs, overlayPs, screenVs, screenPs;
        if (!Compile("VSMain", "vs_4_0", vs, compileReason) ||
            !Compile("PSMain", "ps_4_0", ps, compileReason) ||
            !Compile("OverlayVS", "vs_4_0", overlayVs, compileReason) ||
            !Compile("OverlayPS", "ps_4_0", overlayPs, compileReason) ||
            !Compile("ScreenVS", "vs_4_0", screenVs, compileReason) ||
            !Compile("ScreenPS", "ps_4_0", screenPs, compileReason))
            return Fail(compileReason, reason);
        if (FAILED(device->CreateVertexShader(vs->GetBufferPointer(),
                vs->GetBufferSize(), nullptr, &vertexShader)) ||
            FAILED(device->CreatePixelShader(ps->GetBufferPointer(),
                ps->GetBufferSize(), nullptr, &pixelShader)))
            return Fail("D3D11 shader creation failed.", reason);
        if (FAILED(device->CreateVertexShader(overlayVs->GetBufferPointer(),
                overlayVs->GetBufferSize(), nullptr, &overlayVertexShader)) ||
            FAILED(device->CreatePixelShader(overlayPs->GetBufferPointer(),
                overlayPs->GetBufferSize(), nullptr, &overlayPixelShader)))
            return Fail("D3D11 overlay shader creation failed.", reason);
        if (FAILED(device->CreateVertexShader(screenVs->GetBufferPointer(),
                screenVs->GetBufferSize(),nullptr,&screenVertexShader)) ||
            FAILED(device->CreatePixelShader(screenPs->GetBufferPointer(),
                screenPs->GetBufferSize(),nullptr,&screenPixelShader)))
            return Fail("D3D11 label shader creation failed.",reason);
        const D3D11_INPUT_ELEMENT_DESC input[] = {{"POSITION", 0,
            DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D11_INPUT_PER_VERTEX_DATA, 0}};
        if (FAILED(device->CreateInputLayout(input, 1, vs->GetBufferPointer(),
                vs->GetBufferSize(), &inputLayout)))
            return Fail("D3D11 input-layout creation failed.", reason);
        const D3D11_INPUT_ELEMENT_DESC overlayInput[] = {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
                D3D11_INPUT_PER_VERTEX_DATA,0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,
                D3D11_INPUT_PER_VERTEX_DATA,0}};
        if (FAILED(device->CreateInputLayout(overlayInput,2,
                overlayVs->GetBufferPointer(),overlayVs->GetBufferSize(),
                &overlayInputLayout)))
            return Fail("D3D11 overlay input-layout creation failed.",reason);
        const D3D11_INPUT_ELEMENT_DESC screenInput[]={{"POSITION",0,
            DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,8,
                D3D11_INPUT_PER_VERTEX_DATA,0}};
        if(FAILED(device->CreateInputLayout(screenInput,2,
                screenVs->GetBufferPointer(),screenVs->GetBufferSize(),
                &screenInputLayout)))
            return Fail("D3D11 label input-layout creation failed.",reason);
        D3D11_BUFFER_DESC buffer{};
        buffer.ByteWidth = sizeof(FrameConstants);
        buffer.Usage = D3D11_USAGE_DYNAMIC;
        buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(device->CreateBuffer(&buffer, nullptr, &frameConstants)))
            return Fail("D3D11 frame constant-buffer creation failed.", reason);
        buffer.ByteWidth = sizeof(ObjectConstants);
        if (FAILED(device->CreateBuffer(&buffer, nullptr, &objectConstants)))
            return Fail("D3D11 object constant-buffer creation failed.", reason);
        buffer.ByteWidth=static_cast<UINT>(MaximumOverlayVertices*sizeof(OverlayVertex));
        buffer.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        if(FAILED(device->CreateBuffer(&buffer,nullptr,&overlayVertices)))
            return Fail("D3D11 overlay vertex-buffer creation failed.",reason);
        const ScreenVertex quad[]={{{-1,1},{0,0}},{{1,1},{1,0}},{{-1,-1},{0,1}},
            {{-1,-1},{0,1}},{{1,1},{1,0}},{{1,-1},{1,1}}};
        buffer.Usage=D3D11_USAGE_IMMUTABLE;
        buffer.CPUAccessFlags=0;
        buffer.ByteWidth=sizeof(quad);
        D3D11_SUBRESOURCE_DATA quadData{quad,0,0};
        if(FAILED(device->CreateBuffer(&buffer,&quadData,&screenVertices)))
            return Fail("D3D11 label quad creation failed.",reason);
        D3D11_DEPTH_STENCIL_DESC depth{};
        depth.DepthEnable = TRUE; depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        if (FAILED(device->CreateDepthStencilState(&depth, &depthState)))
            return Fail("D3D11 depth-state creation failed.", reason);
        depth.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
        if(FAILED(device->CreateDepthStencilState(&depth,&depthReadState)))
            return Fail("D3D11 overlay depth-state creation failed.",reason);
        depth.DepthEnable=FALSE;
        if(FAILED(device->CreateDepthStencilState(&depth,&depthOffState)))
            return Fail("D3D11 overlay no-depth state creation failed.",reason);
        D3D11_BLEND_DESC blend{};
        blend.RenderTarget[0].BlendEnable=TRUE;
        blend.RenderTarget[0].SrcBlend=D3D11_BLEND_SRC_ALPHA;
        blend.RenderTarget[0].DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOp=D3D11_BLEND_OP_ADD;
        blend.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
        blend.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
        blend.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
        if(FAILED(device->CreateBlendState(&blend,&alphaBlend)))
            return Fail("D3D11 label blend-state creation failed.",reason);
        D3D11_SAMPLER_DESC sampler{};
        sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler.MaxLOD=D3D11_FLOAT32_MAX;
        if(FAILED(device->CreateSamplerState(&sampler,&labelSampler)))
            return Fail("D3D11 label sampler creation failed.",reason);
        if (!CreateRasterizer(D3D11_FILL_SOLID, D3D11_CULL_BACK, solidCull) ||
            !CreateRasterizer(D3D11_FILL_SOLID, D3D11_CULL_NONE, solidNoCull) ||
            !CreateRasterizer(D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, wireNoCull))
            return Fail("D3D11 rasterizer-state creation failed.", reason);
        return true;
    }

    bool CreateRasterizer(D3D11_FILL_MODE fill, D3D11_CULL_MODE cull,
        ComPtr<ID3D11RasterizerState>& state)
    {
        D3D11_RASTERIZER_DESC descriptor{};
        descriptor.FillMode = fill; descriptor.CullMode = cull;
        descriptor.FrontCounterClockwise = FALSE;
        descriptor.DepthClipEnable = TRUE;
        return SUCCEEDED(device->CreateRasterizerState(&descriptor, &state));
    }

    bool Resize(int width, int height, std::string* reason)
    {
        viewportWidth = (std::max)(0, width);
        viewportHeight = (std::max)(0, height);
        renderTarget.Reset(); depthView.Reset(); depthTexture.Reset();
        labelView.Reset(); labelTexture.Reset();
        if (!swapChain || viewportWidth == 0 || viewportHeight == 0) return true;
        context->OMSetRenderTargets(0, nullptr, nullptr);
        if (FAILED(swapChain->ResizeBuffers(0, static_cast<UINT>(viewportWidth),
                static_cast<UINT>(viewportHeight), DXGI_FORMAT_UNKNOWN, 0)))
            return Fail("D3D11 swap-chain resize failed.", reason);
        ComPtr<ID3D11Texture2D> backBuffer;
        if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))) ||
            FAILED(device->CreateRenderTargetView(backBuffer.Get(), nullptr,
                &renderTarget)))
            return Fail("D3D11 back-buffer view creation failed.", reason);
        D3D11_TEXTURE2D_DESC depth{};
        depth.Width = static_cast<UINT>(viewportWidth);
        depth.Height = static_cast<UINT>(viewportHeight);
        depth.MipLevels = 1; depth.ArraySize = 1;
        depth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth.SampleDesc.Count = 1; depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (FAILED(device->CreateTexture2D(&depth, nullptr, &depthTexture)) ||
            FAILED(device->CreateDepthStencilView(depthTexture.Get(), nullptr,
                &depthView)))
            return Fail("D3D11 depth-buffer creation failed.", reason);
        D3D11_TEXTURE2D_DESC labels{};
        labels.Width=static_cast<UINT>(viewportWidth);
        labels.Height=static_cast<UINT>(viewportHeight);
        labels.MipLevels=labels.ArraySize=1;
        labels.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
        labels.SampleDesc.Count=1;
        labels.Usage=D3D11_USAGE_DYNAMIC;
        labels.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        labels.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
        if(FAILED(device->CreateTexture2D(&labels,nullptr,&labelTexture)) ||
            FAILED(device->CreateShaderResourceView(labelTexture.Get(),nullptr,
                &labelView)))
            return Fail("D3D11 label texture creation failed.",reason);
        return true;
    }

    void Shutdown()
    {
        geometry.clear(); gpuBytes = 0;
        if (context) context->ClearState();
        wireNoCull.Reset(); solidNoCull.Reset(); solidCull.Reset();
        labelView.Reset(); labelTexture.Reset(); labelSampler.Reset();
        alphaBlend.Reset(); depthOffState.Reset(); depthReadState.Reset(); depthState.Reset();
        screenVertices.Reset(); overlayVertices.Reset(); objectConstants.Reset(); frameConstants.Reset();
        screenInputLayout.Reset(); overlayInputLayout.Reset(); inputLayout.Reset();
        screenPixelShader.Reset();screenVertexShader.Reset();
        overlayPixelShader.Reset(); overlayVertexShader.Reset();
        pixelShader.Reset(); vertexShader.Reset();
        depthView.Reset(); depthTexture.Reset(); renderTarget.Reset();
        swapChain.Reset(); context.Reset(); device.Reset();
        hwnd = nullptr; viewportWidth = viewportHeight = 0;
        diagnostics = {};
    }

    void SetScene(const EditorRenderScene& value)
    {
        scene = value;
        if (assets) workingSet.Rebuild(scene, *assets); else workingSet.Clear();
    }

    void Bind(EditorRenderAssetRegistry* valueAssets,
        EditorRenderGeometryCache* valueCache)
    {
        if (assets != valueAssets || cpuCache != valueCache) ClearGeometry();
        assets = valueAssets; cpuCache = valueCache;
        if (assets) workingSet.Rebuild(scene, *assets); else workingSet.Clear();
    }

    void ClearGeometry() { geometry.clear(); gpuBytes = 0; }

    bool Upload(const std::string& assetId, const EditorStaticAssetGeometry& source)
    {
        std::vector<EditorGeometryPosition> vertices;
        std::vector<std::uint32_t> indices;
        vertices.reserve(source.totalVertices);
        indices.reserve(source.totalTriangles * 3);
        for (const auto& mesh : source.meshes)
        {
            const std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
            vertices.insert(vertices.end(), mesh.buffer.positions.begin(),
                mesh.buffer.positions.end());
            for (const auto& triangle : mesh.buffer.triangles)
            {
                indices.push_back(base + triangle.a);
                indices.push_back(base + triangle.b);
                indices.push_back(base + triangle.c);
            }
        }
        const std::size_t bytes = vertices.size() * sizeof(vertices[0]) +
            indices.size() * sizeof(indices[0]);
        if (vertices.empty() || indices.empty() || bytes > MaximumAssetBytes ||
            bytes > MaximumGpuBytes - gpuBytes) return false;
        Geometry resource; resource.bytes = bytes;
        resource.indexCount = static_cast<UINT>(indices.size());
        D3D11_BUFFER_DESC descriptor{};
        descriptor.Usage = D3D11_USAGE_IMMUTABLE;
        descriptor.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(vertices[0]));
        descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA data{vertices.data(), 0, 0};
        if (FAILED(device->CreateBuffer(&descriptor, &data, &resource.vertices)))
            return false;
        descriptor.ByteWidth = static_cast<UINT>(indices.size() * sizeof(indices[0]));
        descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
        data.pSysMem = indices.data();
        if (FAILED(device->CreateBuffer(&descriptor, &data, &resource.indices)))
            return false;
        gpuBytes += bytes;
        geometry.emplace(assetId, std::move(resource));
        return true;
    }

    bool IsVisible(const EditorRenderInstance& instance,
        const EditorViewportState& state, float& distance) const
    {
        const EditorWireframeWorldBounds bounds =
            ComputeEditorWireframeWorldBounds(instance);
        if (!bounds.valid) return false;
        EditorWireframeCamera camera = MakeEditorWireframeCamera(state);
        const EditorWireframeCameraBasis basis = ComputeEditorWireframeCameraBasis(camera);
        const float dx = bounds.center.x - camera.x;
        const float dy = bounds.center.y - camera.y;
        const float dz = bounds.center.z - camera.z;
        const float x = dx*basis.right.x + dy*basis.right.y + dz*basis.right.z;
        const float y = dx*basis.up.x + dy*basis.up.y + dz*basis.up.z;
        const float z = dx*basis.forward.x + dy*basis.forward.y + dz*basis.forward.z;
        distance = z;
        if (z + bounds.radius < camera.nearPlane ||
            z - bounds.radius > camera.farPlane) return false;
        const float tanY = std::tan(camera.verticalFovDegrees * Pi / 360.0f);
        const float tanX = tanY * static_cast<float>(state.width) /
            static_cast<float>((std::max)(1, state.height));
        const float depth = (std::max)(z, camera.nearPlane);
        return std::fabs(x) <= depth*tanX + bounds.radius &&
            std::fabs(y) <= depth*tanY + bounds.radius;
    }

    void DrawOverlayLines(const EditorRenderOverlayBatch& overlay,
        bool depthTest)
    {
        std::vector<OverlayVertex> vertices;
        vertices.reserve((std::min)(overlay.lines.size()*2,
            MaximumOverlayVertices));
        for(const auto& line:overlay.lines)
        {
            if(line.depthTest!=depthTest || vertices.size()+2>MaximumOverlayVertices)
                continue;
            const XMFLOAT4 color=OverlayColor(line.style);
            vertices.push_back({{line.first.x,line.first.y,line.first.z},color});
            vertices.push_back({{line.second.x,line.second.y,line.second.z},color});
        }
        if(vertices.empty()) return;
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if(FAILED(context->Map(overlayVertices.Get(),0,D3D11_MAP_WRITE_DISCARD,
                0,&mapped))) return;
        std::memcpy(mapped.pData,vertices.data(),vertices.size()*sizeof(vertices[0]));
        context->Unmap(overlayVertices.Get(),0);
        const UINT stride=sizeof(OverlayVertex),offset=0;
        ID3D11Buffer* buffer=overlayVertices.Get();
        context->IASetInputLayout(overlayInputLayout.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->IASetVertexBuffers(0,1,&buffer,&stride,&offset);
        context->IASetIndexBuffer(nullptr,DXGI_FORMAT_UNKNOWN,0);
        context->VSSetShader(overlayVertexShader.Get(),nullptr,0);
        context->PSSetShader(overlayPixelShader.Get(),nullptr,0);
        context->OMSetDepthStencilState(depthTest?depthReadState.Get():
            depthOffState.Get(),0);
        context->RSSetState(solidNoCull.Get());
        context->Draw(static_cast<UINT>(vertices.size()),0);
        ++diagnostics.overlayDrawCalls;
        diagnostics.overlayPrimitives+=vertices.size()/2;
    }

    void DrawLabels(const EditorRenderOverlayBatch& overlay)
    {
        if(overlay.labels.empty() || !labelTexture || viewportWidth<=0 ||
            viewportHeight<=0) return;
        BITMAPINFO info{};
        info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth=viewportWidth;
        info.bmiHeader.biHeight=-viewportHeight;
        info.bmiHeader.biPlanes=1;
        info.bmiHeader.biBitCount=32;
        info.bmiHeader.biCompression=BI_RGB;
        void* bits=nullptr;
        HDC dc=CreateCompatibleDC(nullptr);
        HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);
        if(!dc||!bitmap||!bits)
        { if(bitmap) DeleteObject(bitmap); if(dc) DeleteDC(dc); return; }
        HGDIOBJ oldBitmap=SelectObject(dc,bitmap);
        std::memset(bits,0,static_cast<std::size_t>(viewportWidth)*viewportHeight*4);
        SetBkMode(dc,TRANSPARENT);
        HFONT font=static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        HGDIOBJ oldFont=SelectObject(dc,font);
        for(const auto& label:overlay.labels)
        {
            SetTextColor(dc,label.selected?RGB(255,145,55):RGB(220,225,230));
            TextOutA(dc,static_cast<int>(label.screenX),
                static_cast<int>(label.screenY),label.text.c_str(),
                static_cast<int>(label.text.size()));
        }
        auto* pixels=static_cast<std::uint32_t*>(bits);
        const std::size_t count=static_cast<std::size_t>(viewportWidth)*viewportHeight;
        for(std::size_t i=0;i<count;++i)
        {
            const std::uint32_t rgb=pixels[i]&0x00ffffffu;
            const std::uint32_t b=rgb&255u,g=(rgb>>8)&255u,r=(rgb>>16)&255u;
            pixels[i]=rgb|((std::max)({r,g,b})<<24);
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if(SUCCEEDED(context->Map(labelTexture.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))
        {
            const auto* source=static_cast<const std::uint8_t*>(bits);
            auto* target=static_cast<std::uint8_t*>(mapped.pData);
            for(int y=0;y<viewportHeight;++y)
                std::memcpy(target+static_cast<std::size_t>(y)*mapped.RowPitch,
                    source+static_cast<std::size_t>(y)*viewportWidth*4,
                    static_cast<std::size_t>(viewportWidth)*4);
            context->Unmap(labelTexture.Get(),0);
            const UINT stride=sizeof(ScreenVertex),offset=0;
            ID3D11Buffer* buffer=screenVertices.Get();
            context->IASetInputLayout(screenInputLayout.Get());
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->IASetVertexBuffers(0,1,&buffer,&stride,&offset);
            context->VSSetShader(screenVertexShader.Get(),nullptr,0);
            context->PSSetShader(screenPixelShader.Get(),nullptr,0);
            ID3D11ShaderResourceView* view=labelView.Get();
            ID3D11SamplerState* sampler=labelSampler.Get();
            context->PSSetShaderResources(0,1,&view);
            context->PSSetSamplers(0,1,&sampler);
            context->OMSetDepthStencilState(depthOffState.Get(),0);
            const float blendFactor[4]{};
            context->OMSetBlendState(alphaBlend.Get(),blendFactor,0xffffffffu);
            context->Draw(6,0);
            context->OMSetBlendState(nullptr,blendFactor,0xffffffffu);
            ID3D11ShaderResourceView* nullView=nullptr;
            context->PSSetShaderResources(0,1,&nullView);
            ++diagnostics.overlayDrawCalls;
        }
        SelectObject(dc,oldFont);
        SelectObject(dc,oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(dc);
    }

    bool Render(const EditorViewportState& state,
        const EditorD3D11RenderOptions& options,
        const EditorRenderOverlayBatch& overlay)
    {
        const auto start = std::chrono::steady_clock::now();
        const bool wasWarp = diagnostics.usingWarp;
        const std::string deviceStatus = diagnostics.status;
        diagnostics = {}; diagnostics.initialized = device != nullptr;
        diagnostics.usingWarp = wasWarp; diagnostics.status = deviceStatus;
        diagnostics.workingSetAssets = workingSet.Statistics().uniqueAssets;
        diagnostics.residentAssets = geometry.size(); diagnostics.gpuBytes = gpuBytes;
        if (!device || !renderTarget || state.width <= 0 || state.height <= 0)
            return false;
        const float clear[] = {0.055f, 0.065f, 0.075f, 1.0f};
        context->OMSetRenderTargets(1, renderTarget.GetAddressOf(), depthView.Get());
        context->ClearRenderTargetView(renderTarget.Get(), clear);
        context->ClearDepthStencilView(depthView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        D3D11_VIEWPORT viewport{0, 0, static_cast<float>(state.width),
            static_cast<float>(state.height), 0.0f, 1.0f};
        context->RSSetViewports(1, &viewport);
        context->OMSetDepthStencilState(depthState.Get(), 0);
        context->IASetInputLayout(inputLayout.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(vertexShader.Get(), nullptr, 0);
        context->PSSetShader(pixelShader.Get(), nullptr, 0);
        ID3D11Buffer* frameBuffers[] = {frameConstants.Get()};
        ID3D11Buffer* objectBuffers[] = {objectConstants.Get()};
        context->VSSetConstantBuffers(0, 1, frameBuffers);
        context->VSSetConstantBuffers(1, 1, objectBuffers);
        context->PSSetConstantBuffers(1, 1, objectBuffers);
        FrameConstants frameData;
        const auto matrix=BuildEditorViewProjectionMatrix(
            MakeEditorRenderFrameContext(state));
        std::memcpy(&frameData.viewProjection,matrix.data(),sizeof(matrix));
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context->Map(frameConstants.Get(), 0,
                D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return false;
        *static_cast<FrameConstants*>(mapped.pData) = frameData;
        context->Unmap(frameConstants.Get(), 0);

        struct Item { const EditorRenderInstance* instance; float distance; };
        std::vector<Item> visible;
        for (const auto& instance : scene.Instances())
        {
            if (!instance.visible || (options.isolateSelected && !instance.selected))
                continue;
            float distance = 0.0f;
            if (!instance.selected && !IsVisible(instance, state, distance))
            { ++diagnostics.culledInstances; continue; }
            visible.push_back({&instance, distance});
        }
        std::stable_sort(visible.begin(), visible.end(), [](const Item& a, const Item& b)
        { if (a.instance->selected != b.instance->selected) return a.instance->selected;
          return a.distance < b.distance; });

        auto drawPass = [&](bool wireframe)
        {
            context->RSSetState(wireframe ? wireNoCull.Get() :
                (options.backfaceCulling ? solidCull.Get() : solidNoCull.Get()));
            for (const Item& item : visible)
            {
                if (diagnostics.drawCalls >= MaximumDrawCalls) break;
                const auto* asset = assets ? assets->Find(item.instance->assetId) : nullptr;
                const auto* cpu = cpuCache ? cpuCache->Find(item.instance->assetId) : nullptr;
                if (!asset || !cpu) { if (!wireframe) ++diagnostics.fallbackBounds; continue; }
                auto found = geometry.find(item.instance->assetId);
                if (found == geometry.end())
                {
                    ++diagnostics.cacheMisses;
                    if (wireframe || diagnostics.uploadsThisFrame >= MaximumUploadsPerFrame ||
                        !Upload(item.instance->assetId, *cpu))
                    { if (!wireframe) ++diagnostics.fallbackBounds; continue; }
                    ++diagnostics.uploadsThisFrame;
                    found = geometry.find(item.instance->assetId);
                }
                else ++diagnostics.cacheHits;
                const Geometry& resource = found->second;
                const UINT stride = sizeof(EditorGeometryPosition), offset = 0;
                ID3D11Buffer* vertex = resource.vertices.Get();
                context->IASetVertexBuffers(0, 1, &vertex, &stride, &offset);
                context->IASetIndexBuffer(resource.indices.Get(),
                    DXGI_FORMAT_R32_UINT, 0);
                ObjectConstants objectData;
                XMStoreFloat4x4(&objectData.world, WorldMatrix(item.instance->transform));
                objectData.color = item.instance->selected
                    ? XMFLOAT4(1.0f, 0.42f, 0.08f, 1.0f)
                    : (wireframe ? XMFLOAT4(0.18f,0.75f,0.82f,1.0f)
                        : XMFLOAT4(0.38f,0.48f,0.52f,1.0f));
                if (FAILED(context->Map(objectConstants.Get(), 0,
                        D3D11_MAP_WRITE_DISCARD, 0, &mapped))) continue;
                *static_cast<ObjectConstants*>(mapped.pData) = objectData;
                context->Unmap(objectConstants.Get(), 0);
                context->DrawIndexed(resource.indexCount, 0, 0);
                ++diagnostics.drawCalls;
                if (!wireframe)
                {
                    ++diagnostics.instancesDrawn;
                    diagnostics.trianglesSubmitted += resource.indexCount / 3;
                }
            }
        };
        if (options.filledMeshes) drawPass(false);
        if (options.wireframeOverlay) drawPass(true);
        DrawOverlayLines(overlay,true);
        DrawOverlayLines(overlay,false);
        DrawLabels(overlay);
        diagnostics.labelsDrawn=overlay.labels.size();
        const HRESULT present = swapChain->Present(1, 0);
        diagnostics.presentSucceeded = SUCCEEDED(present);
        diagnostics.residentAssets = geometry.size(); diagnostics.gpuBytes = gpuBytes;
        diagnostics.cpuFrameMilliseconds = std::chrono::duration<double,std::milli>(
            std::chrono::steady_clock::now()-start).count();
        if (FAILED(present)) diagnostics.status = "D3D11 present/device failure";
        return SUCCEEDED(present);
    }

    bool Fail(const std::string& message, std::string* reason)
    { diagnostics.status = message; if (reason) *reason = message; return false; }

    HWND hwnd = nullptr; int viewportWidth = 0, viewportHeight = 0;
    ComPtr<ID3D11Device> device; ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain; ComPtr<ID3D11RenderTargetView> renderTarget;
    ComPtr<ID3D11Texture2D> depthTexture; ComPtr<ID3D11DepthStencilView> depthView;
    ComPtr<ID3D11VertexShader> vertexShader,overlayVertexShader,screenVertexShader;
    ComPtr<ID3D11PixelShader> pixelShader,overlayPixelShader,screenPixelShader;
    ComPtr<ID3D11InputLayout> inputLayout,overlayInputLayout,screenInputLayout;
    ComPtr<ID3D11Buffer> frameConstants, objectConstants,overlayVertices,screenVertices;
    ComPtr<ID3D11DepthStencilState> depthState,depthReadState,depthOffState;
    ComPtr<ID3D11BlendState> alphaBlend;
    ComPtr<ID3D11SamplerState> labelSampler;
    ComPtr<ID3D11Texture2D> labelTexture;
    ComPtr<ID3D11ShaderResourceView> labelView;
    ComPtr<ID3D11RasterizerState> solidCull, solidNoCull, wireNoCull;
    EditorRenderAssetRegistry* assets = nullptr;
    EditorRenderGeometryCache* cpuCache = nullptr;
    EditorRenderScene scene; EditorRenderAssetWorkingSet workingSet;
    std::unordered_map<std::string, Geometry> geometry;
    std::size_t gpuBytes = 0; EditorD3D11Diagnostics diagnostics;
};

EditorD3D11Renderer::EditorD3D11Renderer() : impl_(std::make_unique<Impl>()) {}
EditorD3D11Renderer::~EditorD3D11Renderer() = default;
bool EditorD3D11Renderer::Initialize(void* h,int w,int v,std::string* r){return impl_->Initialize(h,w,v,r);}
void EditorD3D11Renderer::Shutdown(){impl_->Shutdown();}
bool EditorD3D11Renderer::Resize(int w,int h,std::string* r){return impl_->Resize(w,h,r);}
void EditorD3D11Renderer::Bind(EditorRenderAssetRegistry* a,EditorRenderGeometryCache* c){impl_->Bind(a,c);}
void EditorD3D11Renderer::SetScene(const EditorRenderScene& s){impl_->SetScene(s);}
void EditorD3D11Renderer::ClearGeometryCache(){impl_->ClearGeometry();}
bool EditorD3D11Renderer::Render(const EditorViewportState& s,const EditorD3D11RenderOptions& o,const EditorRenderOverlayBatch& v){return impl_->Render(s,o,v);}
bool EditorD3D11Renderer::IsInitialized() const{return impl_->device!=nullptr;}
const EditorD3D11Diagnostics& EditorD3D11Renderer::Diagnostics() const{return impl_->diagnostics;}
const EditorRenderAssetWorkingSet& EditorD3D11Renderer::WorkingSet() const{return impl_->workingSet;}
