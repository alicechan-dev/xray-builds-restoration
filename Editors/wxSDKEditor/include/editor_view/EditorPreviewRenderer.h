#ifndef XR_WX_SDK_EDITOR_EDITOR_PREVIEW_RENDERER_H
#define XR_WX_SDK_EDITOR_EDITOR_PREVIEW_RENDERER_H

#include "editor_view/EditorViewportDrawList.h"
#include "editor_view/IEditorViewportRenderer.h"
#include "editor_view/EditorPreviewPicking.h"
#include "editor_view/EditorMoveGizmo.h"

class EditorPreviewScene;

class EditorPreviewRenderer final : public IEditorViewportRenderer
{
public:
    void SetScene(const EditorPreviewScene* scene) { scene_ = scene; }
    void SetLabelsVisible(bool visible) { labelsVisible_ = visible; }
    bool LabelsVisible() const { return labelsVisible_; }
    void SetObjectBoundsVisible(bool value) { objectBoundsVisible_ = value; }
    bool ObjectBoundsVisible() const { return objectBoundsVisible_; }
    void SetAssetDiagnosticsVisible(bool value) { assetDiagnosticsVisible_ = value; }
    bool AssetDiagnosticsVisible() const { return assetDiagnosticsVisible_; }
    const EditorViewportDrawList& DrawList() const { return drawList_; }
    EditorPreviewPickResult Pick(float x, float y) const;
    EditorPreviewProjectedPoint SelectedPoint() const;
    void SetActiveGizmoAxis(EditorGizmoAxis axis) { activeGizmoAxis_ = axis; }
    void SetGizmoVisible(bool visible) { gizmoVisible_ = visible; }
    void SetPlacementPreview(const EditorPreviewWorldPoint& point)
    { placementPreview_ = point; }
    void SetPlacementPreviewKind(EditorPreviewKind kind)
    { placementPreviewKind_ = kind; }

    void Resize(int width, int height) override;
    void Render(const EditorViewportState& state) override;

private:
    const EditorPreviewScene* scene_ = nullptr;
    EditorViewportDrawList drawList_;
    bool labelsVisible_ = true;
    bool objectBoundsVisible_ = true;
    bool assetDiagnosticsVisible_ = false;
    int width_ = 0;
    int height_ = 0;
    EditorPreviewProjectionContext projection_;
    std::vector<EditorPreviewPickShape> pickShapes_;
    EditorGizmoAxis activeGizmoAxis_ = EditorGizmoAxis::None;
    bool gizmoVisible_ = false;
    EditorPreviewWorldPoint placementPreview_;
    EditorPreviewKind placementPreviewKind_ = EditorPreviewKind::Marker;
};

#endif
