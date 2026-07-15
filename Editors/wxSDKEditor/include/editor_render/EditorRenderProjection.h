#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_PROJECTION_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_PROJECTION_H

#include <array>

struct EditorViewportState;

struct EditorRenderPoint3D
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct EditorRenderFrameContext
{
    int viewportWidth = 0;
    int viewportHeight = 0;
    EditorRenderPoint3D cameraPosition;
    float yawDegrees = 0.0f;
    float pitchDegrees = 0.0f;
    float verticalFovDegrees = 60.0f;
    float nearPlane = 0.05f;
    float farPlane = 5000.0f;
};

struct EditorProjectedPoint
{
    float screenX = 0.0f;
    float screenY = 0.0f;
    float depth = 0.0f;
    float clipW = 0.0f;
    bool inFront = false;
    bool insideDepth = false;
    bool insideViewport = false;
    bool finite = false;
};

EditorRenderFrameContext MakeEditorRenderFrameContext(
    const EditorViewportState& state);
EditorProjectedPoint ProjectEditorWorldPoint(
    const EditorRenderPoint3D& position,
    const EditorRenderFrameContext& frame);
std::array<float, 16> BuildEditorViewProjectionMatrix(
    const EditorRenderFrameContext& frame);

#endif
