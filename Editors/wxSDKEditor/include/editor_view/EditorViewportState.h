#ifndef XR_WX_SDK_EDITOR_EDITOR_VIEWPORT_STATE_H
#define XR_WX_SDK_EDITOR_EDITOR_VIEWPORT_STATE_H

#include <cstdint>

struct EditorViewportCamera
{
    float x = 0.0f;
    float y = 1.0f;
    float z = -5.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float movementSpeed = 5.0f;
};

struct EditorViewportState
{
    int width = 0;
    int height = 0;
    bool focused = false;
    bool mouseInside = false;
    int mouseX = 0;
    int mouseY = 0;
    bool leftButton = false;
    bool rightButton = false;
    bool middleButton = false;
    bool gridVisible = true;
    std::uint64_t frameCount = 0;
    double elapsedSeconds = 0.0;
    EditorViewportCamera camera;
};

#endif
