#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <array>
#include <cstring>
#include <cstdint>
#include <iostream>

using Microsoft::WRL::ComPtr;

namespace
{
LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM w, LPARAM l)
{ return DefWindowProc(window, message, w, l); }

bool Check(bool condition, const char* message)
{
    if (condition) return true;
    std::cerr << "D3D11 integration test failed: " << message << '\n';
    return false;
}
}

int main()
{
    bool ok = true;
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.lpszClassName = L"wxSDKEditorD3D11TestWindow";
    RegisterClassW(&windowClass);
    HWND window = CreateWindowW(windowClass.lpszClassName, L"D3D11 test",
        WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, nullptr, nullptr,
        windowClass.hInstance, nullptr);
    ok &= Check(window != nullptr, "hidden test window creation");

    DXGI_SWAP_CHAIN_DESC swap{};
    swap.BufferDesc.Width = 64; swap.BufferDesc.Height = 64;
    swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap.SampleDesc.Count = 1; swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap.BufferCount = 2; swap.OutputWindow = window; swap.Windowed = TRUE;
    swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    ComPtr<ID3D11Device> device; ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain; D3D_FEATURE_LEVEL feature{};
    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP,
        nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
        D3D11_SDK_VERSION, &swap, &swapChain, &device, &feature, &context);
    ok &= Check(SUCCEEDED(result), "WARP device and swap chain creation");

    const char* shader =
        "struct O{float4 p:SV_POSITION;};"
        "O VS(float3 p:POSITION){O o;o.p=float4(p,1);return o;}"
        "float4 PS(O o):SV_TARGET{return o.p.z<0.5?float4(0.1,0.8,0.2,1):float4(0.9,0.1,0.1,1);}";
    ComPtr<ID3DBlob> vsCode, psCode, errors;
    result = D3DCompile(shader, std::strlen(shader), "test", nullptr, nullptr,
        "VS", "vs_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsCode, &errors);
    ok &= Check(SUCCEEDED(result), "vertex shader compilation");
    result = D3DCompile(shader, std::strlen(shader), "test", nullptr, nullptr,
        "PS", "ps_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &psCode, &errors);
    ok &= Check(SUCCEEDED(result), "pixel shader compilation");
    ComPtr<ID3D11VertexShader> vertexShader; ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> layout;
    if (device && vsCode && psCode)
    {
        ok &= Check(SUCCEEDED(device->CreateVertexShader(vsCode->GetBufferPointer(),
            vsCode->GetBufferSize(), nullptr, &vertexShader)), "vertex shader creation");
        ok &= Check(SUCCEEDED(device->CreatePixelShader(psCode->GetBufferPointer(),
            psCode->GetBufferSize(), nullptr, &pixelShader)), "pixel shader creation");
        D3D11_INPUT_ELEMENT_DESC input{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,
            0,0,D3D11_INPUT_PER_VERTEX_DATA,0};
        ok &= Check(SUCCEEDED(device->CreateInputLayout(&input,1,
            vsCode->GetBufferPointer(),vsCode->GetBufferSize(),&layout)),
            "input layout creation");
    }

    const std::array<float,18> vertices{
        -0.7f,-0.7f,0.2f, 0.0f,0.7f,0.2f, 0.7f,-0.7f,0.2f,
        -0.7f,-0.7f,0.8f, 0.0f,0.7f,0.8f, 0.7f,-0.7f,0.8f};
    const std::array<std::uint32_t,6> indices{0,1,2,3,4,5};
    ComPtr<ID3D11Buffer> vertexBuffer, indexBuffer;
    if (device)
    {
        D3D11_BUFFER_DESC description{}; D3D11_SUBRESOURCE_DATA data{};
        description.ByteWidth=sizeof(vertices);description.Usage=D3D11_USAGE_IMMUTABLE;
        description.BindFlags=D3D11_BIND_VERTEX_BUFFER;data.pSysMem=vertices.data();
        ok &= Check(SUCCEEDED(device->CreateBuffer(&description,&data,&vertexBuffer)),
            "immutable vertex buffer creation");
        description.ByteWidth=sizeof(indices);description.BindFlags=D3D11_BIND_INDEX_BUFFER;
        data.pSysMem=indices.data();
        ok &= Check(SUCCEEDED(device->CreateBuffer(&description,&data,&indexBuffer)),
            "immutable 32-bit index buffer creation");
    }

    if (context && swapChain && vertexBuffer && indexBuffer)
    {
        ComPtr<ID3D11Texture2D> backBuffer, depth;
        ComPtr<ID3D11RenderTargetView> target; ComPtr<ID3D11DepthStencilView> depthView;
        ComPtr<ID3D11DepthStencilState> depthState;
        ok &= Check(SUCCEEDED(swapChain->GetBuffer(0,IID_PPV_ARGS(&backBuffer))) &&
            SUCCEEDED(device->CreateRenderTargetView(backBuffer.Get(),nullptr,&target)),
            "render target creation");
        D3D11_TEXTURE2D_DESC depthDesc{};depthDesc.Width=64;depthDesc.Height=64;
        depthDesc.MipLevels=1;depthDesc.ArraySize=1;depthDesc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count=1;depthDesc.BindFlags=D3D11_BIND_DEPTH_STENCIL;
        ok &= Check(SUCCEEDED(device->CreateTexture2D(&depthDesc,nullptr,&depth)) &&
            SUCCEEDED(device->CreateDepthStencilView(depth.Get(),nullptr,&depthView)),
            "depth target creation");
        D3D11_DEPTH_STENCIL_DESC depthStateDesc{};depthStateDesc.DepthEnable=TRUE;
        depthStateDesc.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;
        depthStateDesc.DepthFunc=D3D11_COMPARISON_LESS;
        ok &= Check(SUCCEEDED(device->CreateDepthStencilState(&depthStateDesc,&depthState)),
            "depth state creation");
        context->OMSetRenderTargets(1,target.GetAddressOf(),depthView.Get());
        context->OMSetDepthStencilState(depthState.Get(),0);
        const float clear[4]{0,0,0,1};context->ClearRenderTargetView(target.Get(),clear);
        context->ClearDepthStencilView(depthView.Get(),D3D11_CLEAR_DEPTH,1,0);
        D3D11_VIEWPORT viewport{0,0,64,64,0,1};context->RSSetViewports(1,&viewport);
        UINT stride=12,offset=0;ID3D11Buffer* vb=vertexBuffer.Get();
        context->IASetVertexBuffers(0,1,&vb,&stride,&offset);
        context->IASetIndexBuffer(indexBuffer.Get(),DXGI_FORMAT_R32_UINT,0);
        context->IASetInputLayout(layout.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(vertexShader.Get(),nullptr,0);
        context->PSSetShader(pixelShader.Get(),nullptr,0);
        context->DrawIndexed(6,0,0);
        D3D11_TEXTURE2D_DESC stagingDesc{};backBuffer->GetDesc(&stagingDesc);
        stagingDesc.BindFlags=0;stagingDesc.MiscFlags=0;
        stagingDesc.Usage=D3D11_USAGE_STAGING;stagingDesc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
        ComPtr<ID3D11Texture2D> staging;
        ok &= Check(SUCCEEDED(device->CreateTexture2D(&stagingDesc,nullptr,&staging)),
            "staging readback creation");
        context->CopyResource(staging.Get(),backBuffer.Get());
        D3D11_MAPPED_SUBRESOURCE pixels{};
        if (SUCCEEDED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&pixels)))
        {
            const auto* center=static_cast<const std::uint8_t*>(pixels.pData)+
                32*pixels.RowPitch+32*4;
            ok &= Check(center[1] > center[0],
                "near green triangle occludes later far red triangle");
            context->Unmap(staging.Get(),0);
        }
        else ok &= Check(false,"render-target readback");
        ok &= Check(SUCCEEDED(swapChain->Present(0,0)), "draw and present");
        context->OMSetRenderTargets(0,nullptr,nullptr);target.Reset();depthView.Reset();
        backBuffer.Reset();depth.Reset();
        ok &= Check(SUCCEEDED(swapChain->ResizeBuffers(0,96,72,DXGI_FORMAT_UNKNOWN,0)),
            "swap-chain resize");
    }
    if (context) context->ClearState();
    swapChain.Reset(); context.Reset(); device.Reset();
    if (window) DestroyWindow(window);
    UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
    if (!ok) return 1;
    std::cout << "PASS: wxSDKEditor D3D11 device, shader, buffer, depth, present, and resize smoke\n";
    return 0;
}
