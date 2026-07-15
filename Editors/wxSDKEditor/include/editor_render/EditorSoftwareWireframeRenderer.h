#ifndef XR_WX_SDK_EDITOR_EDITOR_SOFTWARE_WIREFRAME_RENDERER_H
#define XR_WX_SDK_EDITOR_EDITOR_SOFTWARE_WIREFRAME_RENDERER_H

#include "editor_render/EditorGeometryBuffer.h"
#include "editor_render/EditorRenderAsset.h"
#include "editor_model/EditorTransform.h"

#include <cstddef>
#include <string>
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

struct EditorWireframeVector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct EditorWireframeWorldBounds
{
    EditorWireframeVector3 minimum;
    EditorWireframeVector3 maximum;
    EditorWireframeVector3 center;
    float radius = 0.0f;
    bool valid = false;
};

struct EditorWireframeCameraBasis
{
    EditorWireframeVector3 right;
    EditorWireframeVector3 up;
    EditorWireframeVector3 forward;
};

enum class EditorWireframeCullReason
{
    None,
    Hidden,
    InvalidBounds,
    InvalidTransform,
    BehindNearPlane,
    BeyondFarPlane,
    OutsideHorizontalFov,
    OutsideVerticalFov
};

const char* ToString(EditorWireframeCullReason reason);

struct EditorWireframeSelectedDiagnostic
{
    bool present = false;
    bool assetResolved = false;
    std::string logicalPath;
    std::string assetId;
    EditorRenderAssetReadiness readiness =
        EditorRenderAssetReadiness::Unsupported;
    EditorTransform transform;
    EditorRenderBounds objectBounds;
    EditorWireframeWorldBounds worldBounds;
    EditorWireframeCameraBasis cameraBasis;
    EditorWireframeVector3 cameraSpaceCenter;
    EditorWireframeCullReason cullReason = EditorWireframeCullReason::None;
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
    EditorWireframeSelectedDiagnostic selectedDiagnostic;
};

EditorWireframeCamera MakeEditorWireframeCamera(
    const EditorViewportState& state);
EditorWireframeWorldBounds ComputeEditorWireframeWorldBounds(
    const EditorRenderInstance& instance);
EditorWireframeCameraBasis ComputeEditorWireframeCameraBasis(
    const EditorWireframeCamera& camera);

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
