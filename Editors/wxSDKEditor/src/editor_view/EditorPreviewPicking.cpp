#include "editor_view/EditorPreviewPicking.h"

#include "editor_view/EditorViewportState.h"

#include <cmath>

EditorPreviewProjectionContext MakeEditorPreviewProjectionContext(
    const EditorViewportState& state, int width, int height)
{
    return {width, height, state.camera.x, state.camera.z, 40.0f};
}

EditorPreviewProjectedPoint ProjectEditorPreviewObject(
    const EditorPreviewObject& object,
    const EditorPreviewProjectionContext& context)
{
    EditorPreviewProjectedPoint point;
    if (!object.visible || context.width <= 0 || context.height <= 0)
        return point;
    point.x = static_cast<float>(context.width) * 0.5f +
        (object.x - context.cameraX) * context.scale;
    point.y = static_cast<float>(context.height) * 0.5f +
        (object.z - context.cameraZ) * context.scale;
    point.visible = point.x >= -40.0f && point.y >= -40.0f &&
        point.x <= static_cast<float>(context.width + 40) &&
        point.y <= static_cast<float>(context.height + 40);
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
            shape.halfWidth = 14.0f;
            shape.halfHeight = 10.0f;
        }
        else if (object.kind == EditorPreviewKind::Light)
        {
            shape.radius = 9.0f;
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
