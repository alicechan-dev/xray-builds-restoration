#include "editor_view/EditorPreviewPicking.h"

#include "editor_view/EditorViewportState.h"

#include <algorithm>
#include <cmath>

EditorPreviewProjectionContext MakeEditorPreviewProjectionContext(
    const EditorViewportState& state, int width, int height)
{
    EditorViewportState adjusted=state;
    adjusted.width=width;
    adjusted.height=height;
    return {MakeEditorRenderFrameContext(adjusted)};
}

EditorPreviewProjectedPoint ProjectEditorPreviewObject(
    const EditorPreviewObject& object,
    const EditorPreviewProjectionContext& context)
{
    EditorPreviewProjectedPoint point;
    if (!object.visible)
        return point;
    const EditorProjectedPoint projected=ProjectEditorWorldPoint(
        {object.x,object.y,object.z},context.frame);
    point.x=projected.screenX;
    point.y=projected.screenY;
    point.visible=projected.finite&&projected.inFront&&projected.insideDepth&&
        projected.insideViewport;
    return point;
}

EditorPreviewWorldPoint UnprojectEditorPreviewToGround(
    float screenX, float screenY,
    const EditorPreviewProjectionContext& context,
    float worldY, bool snap, float snapStep)
{
    EditorPreviewWorldPoint point;
    const auto& frame=context.frame;
    if (frame.viewportWidth <= 0 || frame.viewportHeight <= 0 ||
        !std::isfinite(screenX) || !std::isfinite(screenY) ||
        !std::isfinite(worldY) || (snap && snapStep <= 0.0f))
        return point;
    constexpr float Pi=3.14159265358979323846f;
    const float yaw=frame.yawDegrees*Pi/180.0f;
    const float pitch=-frame.pitchDegrees*Pi/180.0f;
    const float tanY=std::tan(frame.verticalFovDegrees*Pi/360.0f);
    const float aspect=static_cast<float>(frame.viewportWidth)/frame.viewportHeight;
    const float nx=2.0f*screenX/frame.viewportWidth-1.0f;
    const float ny=1.0f-2.0f*screenY/frame.viewportHeight;
    const float fx=std::cos(pitch)*std::sin(yaw);
    const float fy=std::sin(pitch);
    const float fz=std::cos(pitch)*std::cos(yaw);
    const float rx=std::cos(yaw), rz=-std::sin(yaw);
    const float ux=-std::sin(pitch)*std::sin(yaw);
    const float uy=std::cos(pitch);
    const float uz=-std::sin(pitch)*std::cos(yaw);
    const float dx=fx+rx*nx*tanY*aspect+ux*ny*tanY;
    const float dy=fy+uy*ny*tanY;
    const float dz=fz+rz*nx*tanY*aspect+uz*ny*tanY;
    if(std::fabs(dy)<1.0e-6f) return point;
    const float t=(worldY-frame.cameraPosition.y)/dy;
    if(t<=0.0f) return point;
    point.x = frame.cameraPosition.x+dx*t;
    point.y = worldY;
    point.z = frame.cameraPosition.z+dz*t;
    if (snap)
    {
        point.x = std::round(point.x / snapStep) * snapStep;
        point.z = std::round(point.z / snapStep) * snapStep;
    }
    point.valid = std::isfinite(point.x) && std::isfinite(point.z);
    return point;
}

std::vector<EditorPreviewPickShape> BuildEditorPreviewPickShapes(
    const EditorPreviewScene& scene,
    const EditorPreviewProjectionContext& context)
{
    std::vector<EditorPreviewPickShape> shapes;
    for (const EditorPreviewObject& object : scene.GetObjects())
    {
        const EditorPreviewProjectedPoint point =
            ProjectEditorPreviewObject(object, context);
        if (!point.visible)
            continue;
        EditorPreviewPickShape shape;
        shape.logicalPath = object.logicalPath;
        shape.kind = object.kind;
        shape.centerX = point.x;
        shape.centerY = point.y;
        if (object.kind == EditorPreviewKind::Box)
        {
            const EditorProjectedPoint center=ProjectEditorWorldPoint(
                {object.x,object.y,object.z},context.frame);
            const float scale=center.depth>0.0f?
                context.frame.viewportHeight*0.5f/
                std::tan(context.frame.verticalFovDegrees*3.14159265358979323846f/360.0f)/center.depth:1.0f;
            shape.halfWidth = object.realBounds ?
                (std::clamp)(object.sizeX * scale * 0.5f, 4.0f, 240.0f) : 14.0f;
            shape.halfHeight = object.realBounds ?
                (std::clamp)(object.sizeY * scale * 0.5f, 4.0f, 240.0f) : 10.0f;
        }
        else if (object.kind == EditorPreviewKind::Light ||
            object.kind == EditorPreviewKind::HistoricalLight ||
            object.kind == EditorPreviewKind::Glow)
        {
            shape.radius = object.kind == EditorPreviewKind::Glow ||
                    object.kind == EditorPreviewKind::HistoricalLight
                ? 9.0f
                : 9.0f;
            shape.circular = true;
        }
        else
        {
            shape.halfWidth = 9.0f;
            shape.halfHeight = 9.0f;
        }
        shapes.push_back(std::move(shape));
    }
    return shapes;
}

EditorPreviewPickResult PickEditorPreview(
    const std::vector<EditorPreviewPickShape>& shapes, float x, float y,
    float markerTolerance)
{
    for (auto shape = shapes.rbegin(); shape != shapes.rend(); ++shape)
    {
        if (!shape->selectable)
            continue;
        const float dx = x - shape->centerX;
        const float dy = y - shape->centerY;
        bool hit = false;
        if (shape->circular)
            hit = dx * dx + dy * dy <= shape->radius * shape->radius;
        else
        {
            const float tolerance = shape->kind == EditorPreviewKind::Box
                ? 0.0f : markerTolerance;
            hit = std::fabs(dx) <= shape->halfWidth + tolerance &&
                std::fabs(dy) <= shape->halfHeight + tolerance;
        }
        if (hit)
            return {true, shape->logicalPath,
                std::sqrt(dx * dx + dy * dy), shape->kind};
    }
    return {};
}
