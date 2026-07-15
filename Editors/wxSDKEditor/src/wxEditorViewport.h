#ifndef XR_WX_SDK_EDITOR_WX_EDITOR_VIEWPORT_H
#define XR_WX_SDK_EDITOR_WX_EDITOR_VIEWPORT_H

#include "editor_view/EditorViewportController.h"
#include "editor_view/EditorPreviewRenderer.h"
#include "editor_view/EditorPreviewScene.h"
#include "editor_view/EditorMoveGizmo.h"
#include "editor_app/EditorToolMode.h"
#include "editor_assets/EditorAssetDescriptor.h"
#include "editor_render/EditorRenderScene.h"
#include "editor_render/EditorRenderOverlay.h"
#include "editor_render/EditorSoftwareWireframeRenderer.h"
#include "editor_render/d3d11/EditorD3D11Renderer.h"

#include <memory>
#include <functional>
#include <string>
#include <wx/panel.h>
#include <wx/timer.h>

class EditorTreeModel;
class EditorRenderAssetRegistry;
class EditorRenderGeometryCache;

enum class EditorViewportBackend
{
    SoftwareDiagnostic,
    Direct3D11
};

class wxEditorViewport final : public wxPanel
{
public:
    explicit wxEditorViewport(wxWindow* parent);
    ~wxEditorViewport() override;

    void ToggleGrid();
    void ResetCamera();
    void FocusViewport();
    bool IsGridVisible() const;
    void RebuildPreview(const EditorTreeModel& model,
        const std::string& selectedPath);
    void SetPreviewScene(EditorPreviewScene scene);
    void SetRenderScene(EditorRenderScene scene,
        EditorRenderAssetRegistry* assets,
        EditorRenderGeometryCache* geometryCache);
    void TogglePreviewLabels();
    bool ArePreviewLabelsVisible() const;
    EditorPreviewLabelPolicy PreviewLabelPolicy() const { return overlayOptions_.labels; }
    void SetPreviewLabelPolicy(EditorPreviewLabelPolicy value);
    void ToggleRuntimeMarkers();
    bool AreRuntimeMarkersVisible() const { return overlayOptions_.runtimeMarkers; }
    void ToggleGlowMarkers();
    bool AreGlowMarkersVisible() const { return overlayOptions_.glowMarkers; }
    void ToggleLightMarkers();
    bool AreLightMarkersVisible() const { return overlayOptions_.lightMarkers; }
    void ToggleUnsupportedBounds();
    bool AreUnsupportedBoundsVisible() const { return overlayOptions_.unsupportedBounds; }
    void ToggleDepthTestRuntimeMarkers();
    bool IsRuntimeMarkerDepthTestEnabled() const { return overlayOptions_.depthTestRuntimeMarkers; }
    void ToggleObjectBounds();
    bool AreObjectBoundsVisible() const;
    void ToggleRenderAssetDiagnostics();
    bool AreRenderAssetDiagnosticsVisible() const;
    void ToggleRealMeshWireframe();
    bool IsRealMeshWireframeVisible() const { return wireframeVisible_; }
    void ToggleBackfaceCulling();
    bool IsBackfaceCullingEnabled() const { return backfaceCulling_; }
    void SetBackend(EditorViewportBackend backend);
    EditorViewportBackend Backend() const { return backend_; }
    bool IsDirect3D11Available() const { return d3dAvailable_; }
    void ToggleFilledMeshes();
    bool AreFilledMeshesVisible() const { return d3dOptions_.filledMeshes; }
    void ToggleWireframeOverlay();
    bool IsWireframeOverlayVisible() const { return d3dOptions_.wireframeOverlay; }
    void ToggleIsolateSelected();
    bool IsolateSelected() const { return d3dOptions_.isolateSelected; }
    bool FrameSelected();
    void ToggleMoveSnap();
    bool IsMoveSnapEnabled() const { return moveSnapEnabled_; }
    void SetToolMode(EditorToolMode mode);
    EditorToolMode GetToolMode() const { return toolMode_; }
    bool CancelTransientOperation();
    void SetPlacementDescriptor(const EditorAssetDescriptor* descriptor);
    void SetSelectionHandler(std::function<void(const std::string&)> handler)
    { selectionHandler_ = std::move(handler); }
    void SetTransformHandler(std::function<bool(
        const std::string&, const EditorTransform&)> handler)
    { transformHandler_ = std::move(handler); }
    void SetPlacementHandler(std::function<bool(
        EditorToolMode, const EditorTransform&)> handler)
    { placementHandler_ = std::move(handler); }
    void SetCancelToolHandler(std::function<void()> handler)
    { cancelToolHandler_ = std::move(handler); }

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnFocus(wxFocusEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseButton(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnDestroy(wxWindowDestroyEvent& event);
    bool EnsureD3D11();
    void PrepareActiveSceneGeometry();

    EditorPreviewScene previewScene_;
    EditorRenderScene renderScene_;
    EditorPreviewRenderer renderer_;
    EditorSoftwareWireframeRenderer wireframeRenderer_;
    EditorD3D11Renderer d3dRenderer_;
    EditorD3D11RenderOptions d3dOptions_;
    EditorRenderOverlayOptions overlayOptions_;
    EditorWireframeFrame wireframeFrame_;
    EditorRenderAssetRegistry* renderAssets_ = nullptr;
    EditorRenderGeometryCache* geometryCache_ = nullptr;
    EditorViewportController controller_;
    wxTimer timer_;
    std::function<void(const std::string&)> selectionHandler_;
    std::function<bool(const std::string&, const EditorTransform&)>
        transformHandler_;
    std::function<bool(EditorToolMode, const EditorTransform&)>
        placementHandler_;
    std::function<void()> cancelToolHandler_;
    EditorMoveGizmo moveGizmo_;
    EditorToolMode toolMode_ = EditorToolMode::Select;
    EditorPreviewWorldPoint placementPreview_;
    std::string placementAssetId_;
    std::string placementAssetName_;
    EditorTransform placementDefaults_;
    EditorPreviewKind placementPreviewKind_ = EditorPreviewKind::Marker;
    bool moveSnapEnabled_ = false;
    bool wireframeVisible_ = true;
    bool backfaceCulling_ = true;
    EditorViewportBackend backend_ = EditorViewportBackend::Direct3D11;
    bool d3dAttempted_ = false;
    bool d3dAvailable_ = false;
    std::size_t renderAssetGeneration_ = 0;
    std::string d3dFailure_;
};

#endif
