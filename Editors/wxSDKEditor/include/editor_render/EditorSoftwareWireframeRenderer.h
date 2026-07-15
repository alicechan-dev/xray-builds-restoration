#ifndef XR_WX_SDK_EDITOR_EDITOR_SOFTWARE_WIREFRAME_RENDERER_H
#define XR_WX_SDK_EDITOR_EDITOR_SOFTWARE_WIREFRAME_RENDERER_H

#include "editor_render/EditorGeometryBuffer.h"

#include <cstddef>
#include <vector>

class EditorRenderAssetRegistry;
class EditorRenderGeometryCache;
class EditorRenderScene;
struct EditorRenderInstance;
struct EditorViewportState;

struct EditorWireframeCamera
{
    float x = 0.0f;
    float y = 1.0f;
    float z = -5.0f;
    float yawDegrees = 0.0f;
    float pitchDegrees = 0.0f;
    float verticalFovDegrees = 60.0f;
    float nearPlane = 0.05f;
    float farPlane = 5000.0f;
    int viewportWidth = 0;
    int viewportHeight = 0;

    bool IsValid() const;
};

struct EditorWireframeBudget
{
    std::size_t maximumInstances = 512;
    std::size_t maximumTriangles = 150000;
    std::size_t maximumLines = 450000;
};

struct EditorWireframeLine
{
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    bool selected = false;
};

struct EditorWireframeStatistics
{
    std::size_t consideredInstances = 0;
    std::size_t visibleInstances = 0;
    std::size_t culledInstances = 0;
    std::size_t decodedAssets = 0;
    std::size_t fallbackBounds = 0;
    std::size_t budgetSkippedObjects = 0;
    std::size_t decodeFailures = 0;
    std::size_t trianglesSubmitted = 0;
    std::size_t linesDrawn = 0;
};

struct EditorWireframeFrame
{
    std::vector<EditorWireframeLine> lines;
    EditorWireframeStatistics statistics;
    bool validCamera = false;
    bool budgetExceeded = false;
};

EditorWireframeCamera MakeEditorWireframeCamera(
    const EditorViewportState& state);

class EditorSoftwareWireframeRenderer
{
public:
    EditorWireframeFrame Render(const EditorRenderScene& scene,
        EditorRenderAssetRegistry& assets,
        EditorRenderGeometryCache& cache,
        const EditorWireframeCamera& camera,
        bool backfaceCulling,
        const EditorWireframeBudget& budget = {}) const;
};

#endif
