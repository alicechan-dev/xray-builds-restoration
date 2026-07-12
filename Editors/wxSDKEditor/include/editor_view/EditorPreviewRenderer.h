#ifndef XR_WX_SDK_EDITOR_EDITOR_PREVIEW_RENDERER_H
#define XR_WX_SDK_EDITOR_EDITOR_PREVIEW_RENDERER_H

#include "editor_view/EditorViewportDrawList.h"
#include "editor_view/IEditorViewportRenderer.h"
#include "editor_view/EditorPreviewPicking.h"

class EditorPreviewScene;

class EditorPreviewRenderer final : public IEditorViewportRenderer
{
public:
    void SetScene(const EditorPreviewScene* scene) { scene_ = scene; }
    void SetLabelsVisible(bool visible) { labelsVisible_ = visible; }
    bool LabelsVisible() const { return labelsVisible_; }
    const EditorViewportDrawList& DrawList() const { return drawList_; }
    EditorPreviewPickResult Pick(float x, float y) const;

    void Resize(int width, int height) override;
    void Render(const EditorViewportState& state) override;

private:
    const EditorPreviewScene* scene_ = nullptr;
    EditorViewportDrawList drawList_;
    bool labelsVisible_ = true;
    int width_ = 0;
    int height_ = 0;
    EditorPreviewProjectionContext projection_;
    std::vector<EditorPreviewPickShape> pickShapes_;
};

#endif
