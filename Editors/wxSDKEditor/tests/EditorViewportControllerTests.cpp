#include "editor_view/EditorViewportController.h"
#include "editor_view/IEditorViewportRenderer.h"

#include <cmath>
#include <iostream>

namespace
{
class FakeViewportRenderer final : public IEditorViewportRenderer
{
public:
    void Resize(int width, int height) override
    {
        ++resizeCount;
        lastWidth = width;
        lastHeight = height;
    }
    void Render(const EditorViewportState& state) override
    {
        ++renderCount;
        lastFrame = state.frameCount;
    }

    int resizeCount = 0;
    int renderCount = 0;
    int lastWidth = 0;
    int lastHeight = 0;
    std::uint64_t lastFrame = 0;
};

bool Near(float left, float right)
{
    return std::fabs(left - right) < 0.001f;
}
}

int RunEditorViewportControllerTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition)
            return;
        ++failures;
        std::cerr << "FAIL: viewport " << message << '\n';
    };

    FakeViewportRenderer renderer;
    EditorViewportController controller(&renderer);
    controller.OnResize(1280, 720);
    check(controller.State().width == 1280 &&
        controller.State().height == 720 && renderer.resizeCount == 1 &&
        renderer.lastWidth == 1280 && renderer.lastHeight == 720,
        "resize records dimensions and notifies renderer");
    controller.OnResize(-1, -10);
    check(controller.State().width == 0 && controller.State().height == 0,
        "resize clamps negative dimensions");

    controller.OnMouseEnter();
    controller.OnMouseMove(10, 20);
    check(controller.State().mouseInside && controller.State().mouseX == 10 &&
        controller.State().mouseY == 20,
        "mouse enter and movement update state");
    controller.OnMouseButton(EditorViewportMouseButton::Right, true);
    controller.OnMouseMove(30, 12);
    check(Near(controller.State().camera.yaw, 5.0f) &&
        Near(controller.State().camera.pitch, -2.0f),
        "right drag updates placeholder orientation");
    controller.OnMouseLeave();
    check(!controller.State().mouseInside && !controller.State().rightButton,
        "mouse leave clears pointer button state");

    const float defaultSpeed = controller.State().camera.movementSpeed;
    controller.OnMouseWheel(120);
    check(controller.State().camera.movementSpeed > defaultSpeed,
        "mouse wheel adjusts placeholder movement speed");

    const float initialZ = controller.State().camera.z;
    controller.OnKeyDown(EditorViewportKey::Forward);
    controller.Tick(1.0);
    check(Near(controller.State().camera.z, initialZ),
        "unfocused key input does not move camera");
    controller.OnFocusChanged(true);
    controller.OnKeyDown(EditorViewportKey::Forward);
    controller.OnKeyDown(EditorViewportKey::Right);
    controller.Tick(0.5);
    check(controller.State().camera.z > initialZ &&
        controller.State().camera.x > 0.0f,
        "focused keys move placeholder camera deterministically");
    const std::uint64_t frameAfterMove = controller.State().frameCount;
    controller.OnFocusChanged(false);
    const float stoppedZ = controller.State().camera.z;
    controller.Tick(0.5);
    check(Near(controller.State().camera.z, stoppedZ) &&
        controller.State().frameCount == frameAfterMove + 1,
        "focus loss clears movement while tick remains deterministic");

    check(controller.State().gridVisible, "grid defaults visible");
    controller.ToggleGrid();
    check(!controller.State().gridVisible, "grid toggle updates state");
    controller.ResetCamera();
    check(Near(controller.State().camera.x, 0.0f) &&
        Near(controller.State().camera.y, 1.0f) &&
        Near(controller.State().camera.z, -5.0f) &&
        Near(controller.State().camera.yaw, 0.0f) &&
        Near(controller.State().camera.pitch, 0.0f) &&
        Near(controller.State().camera.movementSpeed, 5.0f),
        "camera reset restores documented defaults");
    controller.FrameCameraOn(7.0f, 3.0f, 11.0f);
    check(Near(controller.State().camera.x, 7.0f) &&
        Near(controller.State().camera.z, 11.0f),
        "frame-selected seam centers camera on preview position");

    controller.Render();
    check(renderer.renderCount == 1 &&
        renderer.lastFrame == controller.State().frameCount,
        "render boundary receives current state");
    controller.SetPickHandler([](int x, int y) {
        return x == 25 && y == 30 ? std::string("Root/object") : std::string();
    });
    controller.OnResize(640, 480);
    check(controller.OnPrimaryClick(25, 30) == "Root/object" &&
        controller.OnPrimaryClick(1, 1).empty(),
        "primary click reports hit and empty result");
    controller.OnMouseButton(EditorViewportMouseButton::Right, true);
    check(controller.OnPrimaryClick(25, 30).empty(),
        "right-button camera state suppresses picking");
    controller.OnMouseButton(EditorViewportMouseButton::Right, false);
    controller.OnResize(0, 0);
    check(controller.OnPrimaryClick(25, 30).empty(),
        "invalid dimensions suppress picking");
    return failures;
}
