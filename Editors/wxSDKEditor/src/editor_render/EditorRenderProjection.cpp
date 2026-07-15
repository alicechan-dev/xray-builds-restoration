#include "editor_render/EditorRenderProjection.h"

#include "editor_view/EditorViewportState.h"

#include <cmath>

namespace
{
constexpr float Pi = 3.14159265358979323846f;

bool IsFinite(const EditorRenderFrameContext& frame,
    const EditorRenderPoint3D& point)
{
    return std::isfinite(point.x) && std::isfinite(point.y) &&
        std::isfinite(point.z) && std::isfinite(frame.cameraPosition.x) &&
        std::isfinite(frame.cameraPosition.y) &&
        std::isfinite(frame.cameraPosition.z) &&
        std::isfinite(frame.yawDegrees) &&
        std::isfinite(frame.pitchDegrees) &&
        std::isfinite(frame.verticalFovDegrees) &&
        std::isfinite(frame.nearPlane) && std::isfinite(frame.farPlane);
}
}

EditorRenderFrameContext MakeEditorRenderFrameContext(
    const EditorViewportState& state)
{
    EditorRenderFrameContext frame;
    frame.viewportWidth = state.width;
    frame.viewportHeight = state.height;
    frame.cameraPosition = {state.camera.x, state.camera.y, state.camera.z};
    frame.yawDegrees = state.camera.yaw;
    frame.pitchDegrees = state.camera.pitch;
    return frame;
}

EditorProjectedPoint ProjectEditorWorldPoint(
    const EditorRenderPoint3D& position,
    const EditorRenderFrameContext& frame)
{
    EditorProjectedPoint result;
    if (frame.viewportWidth <= 0 || frame.viewportHeight <= 0 ||
        frame.nearPlane <= 0.0f || frame.farPlane <= frame.nearPlane ||
        frame.verticalFovDegrees <= 0.0f ||
        frame.verticalFovDegrees >= 179.0f || !IsFinite(frame, position))
        return result;

    const float yaw = frame.yawDegrees * Pi / 180.0f;
    const float pitch = frame.pitchDegrees * Pi / 180.0f;
    const float dx = position.x - frame.cameraPosition.x;
    const float dy = position.y - frame.cameraPosition.y;
    const float dz = position.z - frame.cameraPosition.z;
    const float x = std::cos(yaw) * dx - std::sin(yaw) * dz;
    const float yawDepth = std::sin(yaw) * dx + std::cos(yaw) * dz;
    const float y = std::cos(pitch) * dy - std::sin(pitch) * yawDepth;
    const float z = std::sin(pitch) * dy + std::cos(pitch) * yawDepth;
    result.depth = z;
    result.clipW = z;
    result.inFront = z > 0.0f;
    result.insideDepth = z >= frame.nearPlane && z <= frame.farPlane;
    if (!result.inFront)
    {
        result.finite = std::isfinite(z);
        return result;
    }

    const float focal = static_cast<float>(frame.viewportHeight) * 0.5f /
        std::tan(frame.verticalFovDegrees * Pi / 360.0f);
    result.screenX = static_cast<float>(frame.viewportWidth) * 0.5f +
        x * focal / z;
    result.screenY = static_cast<float>(frame.viewportHeight) * 0.5f -
        y * focal / z;
    result.finite = std::isfinite(result.screenX) &&
        std::isfinite(result.screenY) && std::isfinite(result.depth);
    result.insideViewport = result.finite && result.screenX >= 0.0f &&
        result.screenY >= 0.0f &&
        result.screenX < static_cast<float>(frame.viewportWidth) &&
        result.screenY < static_cast<float>(frame.viewportHeight);
    return result;
}

std::array<float, 16> BuildEditorViewProjectionMatrix(
    const EditorRenderFrameContext& frame)
{
    const float yaw = frame.yawDegrees * Pi / 180.0f;
    const float pitch = frame.pitchDegrees * Pi / 180.0f;
    const float cy = std::cos(yaw), sy = std::sin(yaw);
    const float cp = std::cos(pitch), sp = std::sin(pitch);
    const float aspect = static_cast<float>(frame.viewportWidth) /
        static_cast<float>(frame.viewportHeight > 0 ? frame.viewportHeight : 1);
    const float yScale = 1.0f /
        std::tan(frame.verticalFovDegrees * Pi / 360.0f);
    const float xScale = yScale / aspect;
    const float zScale = frame.farPlane / (frame.farPlane - frame.nearPlane);
    const float zOffset = -frame.nearPlane * zScale;

    const float tx = -(cy * frame.cameraPosition.x -
        sy * frame.cameraPosition.z);
    const float yawDepth = sy * frame.cameraPosition.x +
        cy * frame.cameraPosition.z;
    const float ty = -(cp * frame.cameraPosition.y - sp * yawDepth);
    const float tz = -(sp * frame.cameraPosition.y + cp * yawDepth);

    return {cy*xScale, -sy*sp*yScale, sy*cp*zScale, sy*cp,
        0.0f, cp*yScale, sp*zScale, sp,
        -sy*xScale, -cy*sp*yScale, cy*cp*zScale, cy*cp,
        tx*xScale, ty*yScale, tz*zScale + zOffset, tz};
}
