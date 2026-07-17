#include "editor_view/EditorViewportController.h"
#include "editor_view/IEditorViewportRenderer.h"
#include "editor_view/EditorMoveGizmo.h"

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
    controller.OnMouseMove(10, 20);
    controller.OnMouseMove(30, 12);
    check(Near(controller.State().camera.yaw, 5.0f) &&
        Near(controller.State().camera.pitch, 2.0f),
        "right drag looks right and up conventionally");
    const float anchoredYaw = controller.State().camera.yaw;
    controller.OnMouseButton(EditorViewportMouseButton::Right, false);
    controller.OnMouseMove(900, 700);
    controller.OnMouseButton(EditorViewportMouseButton::Right, true);
    controller.OnMouseMove(900, 700);
    check(Near(controller.State().camera.yaw, anchoredYaw),
        "first captured motion anchors without a jump");
    EditorMouseLookSettings look;
    look.invertHorizontal = true;
    look.invertVertical = true;
    look.sensitivity = 0.5f;
    controller.SetMouseLookSettings(look);
    controller.OnMouseMove(910, 710);
    check(Near(controller.State().camera.yaw, anchoredYaw - 5.0f) &&
        Near(controller.State().camera.pitch, 7.0f),
        "mouse inversion and sensitivity apply independently");
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
    EditorViewportController rotated;
    rotated.OnFocusChanged(true);
    rotated.OnMouseButton(EditorViewportMouseButton::Right, true);
    rotated.OnMouseMove(0, 0);
    rotated.OnMouseMove(360, 0);
    rotated.OnMouseButton(EditorViewportMouseButton::Right, false);
    rotated.OnKeyDown(EditorViewportKey::Forward);
    rotated.Tick(1.0);
    check(rotated.State().camera.x > 4.9f &&
        Near(rotated.State().camera.z, -5.0f),
        "forward movement follows camera yaw");
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
    controller.FrameCameraOn(7.0f, 3.0f, 11.0f, 2.0f);
    check(Near(controller.State().camera.x, 7.0f) &&
        Near(controller.State().camera.y, 3.0f) &&
        controller.State().camera.z < 9.0f &&
        11.0f - controller.State().camera.z > 0.0f &&
        Near(controller.State().camera.yaw, 0.0f) &&
        Near(controller.State().camera.pitch, 0.0f),
        "frame-selected seam places camera with target in front along positive Z");

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
    EditorMoveGizmo gizmo; EditorTransform gt; gt.x=2; gt.z=3;
    check(gizmo.Hit(100,100,130,100)==EditorGizmoAxis::X && gizmo.Hit(100,100,100,70)==EditorGizmoAxis::Z && gizmo.Hit(100,100,50,50)==EditorGizmoAxis::None,"gizmo axis hit testing");
    gizmo.Begin(EditorGizmoAxis::X,100,100,gt); check(gizmo.Update(180,100,false).x==4,"gizmo X drag delta");
    gizmo.Begin(EditorGizmoAxis::Z,100,100,gt); check(gizmo.Update(100,160,true).z==5,"gizmo Z snapped delta"); gizmo.Cancel(); check(!gizmo.Active(),"gizmo cancel");
    return failures;
}
