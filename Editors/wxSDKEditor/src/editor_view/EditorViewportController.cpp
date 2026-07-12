#include "editor_view/EditorViewportController.h"

#include "editor_view/IEditorViewportRenderer.h"

#include <algorithm>

EditorViewportController::EditorViewportController(
    IEditorViewportRenderer* renderer) : renderer_(renderer)
{
}

void EditorViewportController::OnResize(int width, int height)
{
    state_.width = (std::max)(0, width);
    state_.height = (std::max)(0, height);
    if (renderer_)
        renderer_->Resize(state_.width, state_.height);
}

void EditorViewportController::OnFocusChanged(bool focused)
{
    state_.focused = focused;
    if (!focused)
        ClearInput();
}

void EditorViewportController::OnMouseEnter()
{
    state_.mouseInside = true;
}

void EditorViewportController::OnMouseLeave()
{
    state_.mouseInside = false;
    state_.leftButton = false;
    state_.rightButton = false;
    state_.middleButton = false;
    haveMousePosition_ = false;
}

void EditorViewportController::OnMouseMove(int x, int y)
{
    if (haveMousePosition_ && state_.rightButton)
    {
        state_.camera.yaw += static_cast<float>(x - state_.mouseX) * 0.25f;
        state_.camera.pitch = (std::clamp)(state_.camera.pitch +
            static_cast<float>(y - state_.mouseY) * 0.25f, -89.0f, 89.0f);
    }
    state_.mouseX = x;
    state_.mouseY = y;
    haveMousePosition_ = true;
}

void EditorViewportController::OnMouseButton(
    EditorViewportMouseButton button, bool pressed)
{
    switch (button)
    {
    case EditorViewportMouseButton::Left: state_.leftButton = pressed; break;
    case EditorViewportMouseButton::Right: state_.rightButton = pressed; break;
    case EditorViewportMouseButton::Middle: state_.middleButton = pressed; break;
    }
}

void EditorViewportController::OnMouseWheel(int delta)
{
    state_.camera.movementSpeed = (std::clamp)(
        state_.camera.movementSpeed + static_cast<float>(delta) / 240.0f,
        0.5f, 50.0f);
}

void EditorViewportController::OnKeyDown(EditorViewportKey key)
{
    if (state_.focused)
        SetKey(key, true);
}

void EditorViewportController::OnKeyUp(EditorViewportKey key)
{
    SetKey(key, false);
}

void EditorViewportController::Tick(double deltaSeconds)
{
    const double delta = (std::max)(0.0, deltaSeconds);
    state_.elapsedSeconds += delta;
    ++state_.frameCount;
    if (!state_.focused)
        return;

    const float movement = state_.camera.movementSpeed *
        static_cast<float>(delta);
    if (forward_) state_.camera.z += movement;
    if (backward_) state_.camera.z -= movement;
    if (left_) state_.camera.x -= movement;
    if (right_) state_.camera.x += movement;
    if (up_) state_.camera.y += movement;
    if (down_) state_.camera.y -= movement;
}

void EditorViewportController::Render()
{
    if (renderer_)
        renderer_->Render(state_);
}

void EditorViewportController::ToggleGrid()
{
    state_.gridVisible = !state_.gridVisible;
}

void EditorViewportController::ResetCamera()
{
    state_.camera = {};
    ClearInput();
}

void EditorViewportController::FrameCameraOn(float x, float, float z)
{
    state_.camera.x = x;
    state_.camera.z = z;
}

std::string EditorViewportController::OnPrimaryClick(int x, int y) const
{
    if (!pickHandler_ || state_.width <= 0 || state_.height <= 0 ||
        state_.rightButton)
        return {};
    return pickHandler_(x, y);
}

void EditorViewportController::SetKey(EditorViewportKey key, bool pressed)
{
    switch (key)
    {
    case EditorViewportKey::Forward: forward_ = pressed; break;
    case EditorViewportKey::Backward: backward_ = pressed; break;
    case EditorViewportKey::Left: left_ = pressed; break;
    case EditorViewportKey::Right: right_ = pressed; break;
    case EditorViewportKey::Up: up_ = pressed; break;
    case EditorViewportKey::Down: down_ = pressed; break;
    }
}

void EditorViewportController::ClearInput()
{
    forward_ = backward_ = left_ = right_ = up_ = down_ = false;
}
