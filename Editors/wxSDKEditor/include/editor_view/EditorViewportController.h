#ifndef XR_WX_SDK_EDITOR_EDITOR_VIEWPORT_CONTROLLER_H
#define XR_WX_SDK_EDITOR_EDITOR_VIEWPORT_CONTROLLER_H

#include "editor_view/EditorViewportState.h"

#include <functional>
#include <string>
#include <utility>
class IEditorViewportRenderer;

enum class EditorViewportMouseButton
{
    Left,
    Right,
    Middle
};

enum class EditorViewportKey
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

struct EditorMouseLookSettings
{
    bool invertHorizontal = false;
    bool invertVertical = false;
    float sensitivity = 0.25f;
};

class EditorViewportController
{
public:
    explicit EditorViewportController(IEditorViewportRenderer* renderer = nullptr);

    void OnResize(int width, int height);
    void OnFocusChanged(bool focused);
    void OnMouseEnter();
    void OnMouseLeave();
    void OnMouseMove(int x, int y);
    void OnMouseButton(EditorViewportMouseButton button, bool pressed);
    void OnMouseWheel(int delta);
    void OnKeyDown(EditorViewportKey key);
    void OnKeyUp(EditorViewportKey key);
    void Tick(double deltaSeconds);
    void Render();
    void ToggleGrid();
    void ResetCamera();
    void FrameCameraOn(float x, float y, float z, float radius);
    void SetMouseLookSettings(EditorMouseLookSettings settings);
    const EditorMouseLookSettings& MouseLookSettings() const
    { return mouseLookSettings_; }
    bool IsMouseLooking() const { return state_.rightButton; }
    void SetPickHandler(std::function<std::string(int, int)> handler)
    { pickHandler_ = std::move(handler); }
    std::string OnPrimaryClick(int x, int y) const;

    const EditorViewportState& State() const { return state_; }

private:
    void SetKey(EditorViewportKey key, bool pressed);
    void ClearInput();

    IEditorViewportRenderer* renderer_ = nullptr;
    EditorViewportState state_;
    bool forward_ = false;
    bool backward_ = false;
    bool left_ = false;
    bool right_ = false;
    bool up_ = false;
    bool down_ = false;
    bool haveMousePosition_ = false;
    EditorMouseLookSettings mouseLookSettings_;
    std::function<std::string(int, int)> pickHandler_;
};

#endif
