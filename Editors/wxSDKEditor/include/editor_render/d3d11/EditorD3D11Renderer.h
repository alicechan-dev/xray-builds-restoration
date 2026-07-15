#ifndef XR_WX_SDK_EDITOR_EDITOR_D3D11_RENDERER_H
#define XR_WX_SDK_EDITOR_EDITOR_D3D11_RENDERER_H

#include "editor_render/EditorRenderAssetWorkingSet.h"

#include <cstddef>
#include <memory>
#include <string>

class EditorRenderAssetRegistry;
class EditorRenderGeometryCache;
class EditorRenderScene;
struct EditorViewportState;

struct EditorD3D11RenderOptions
{
    bool filledMeshes = true;
    bool wireframeOverlay = false;
    bool backfaceCulling = true;
    bool isolateSelected = false;
};

struct EditorD3D11Diagnostics
{
    bool initialized = false;
    bool usingWarp = false;
    bool presentSucceeded = false;
    std::size_t workingSetAssets = 0;
    std::size_t residentAssets = 0;
    std::size_t uploadsThisFrame = 0;
    std::size_t cacheHits = 0;
    std::size_t cacheMisses = 0;
    std::size_t instancesDrawn = 0;
    std::size_t trianglesSubmitted = 0;
    std::size_t drawCalls = 0;
    std::size_t fallbackBounds = 0;
    std::size_t culledInstances = 0;
    std::size_t gpuBytes = 0;
    double cpuFrameMilliseconds = 0.0;
    std::string status;
};

class EditorD3D11Renderer
{
public:
    EditorD3D11Renderer();
    ~EditorD3D11Renderer();
    EditorD3D11Renderer(const EditorD3D11Renderer&) = delete;
    EditorD3D11Renderer& operator=(const EditorD3D11Renderer&) = delete;

    bool Initialize(void* nativeWindow, int width, int height,
        std::string* reason = nullptr);
    void Shutdown();
    bool Resize(int width, int height, std::string* reason = nullptr);
    void Bind(EditorRenderAssetRegistry* assets,
        EditorRenderGeometryCache* geometryCache);
    void SetScene(const EditorRenderScene& scene);
    void ClearGeometryCache();
    bool Render(const EditorViewportState& state,
        const EditorD3D11RenderOptions& options);

    bool IsInitialized() const;
    const EditorD3D11Diagnostics& Diagnostics() const;
    const EditorRenderAssetWorkingSet& WorkingSet() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif
