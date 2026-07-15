#ifndef XR_WX_SDK_EDITOR_I_EDITOR_VIEWPORT_RENDERER_H
#define XR_WX_SDK_EDITOR_I_EDITOR_VIEWPORT_RENDERER_H

struct EditorViewportState;
class EditorRenderScene;

class IEditorViewportRenderer
{
public:
    virtual ~IEditorViewportRenderer() = default;
    virtual void Resize(int width, int height) = 0;
    virtual void SubmitScene(const EditorRenderScene&) {}
    virtual void Render(const EditorViewportState& state) = 0;
};

class NullEditorViewportRenderer final : public IEditorViewportRenderer
{
public:
    void Resize(int, int) override {}
    void Render(const EditorViewportState&) override {}
};

#endif
