#include "editor_view/EditorPreviewRenderer.h"

#include "editor_view/EditorPreviewScene.h"
#include "editor_view/EditorViewportState.h"

namespace
{
EditorViewportStyle StyleFor(EditorPreviewKind kind)
{
    switch (kind)
    {
    case EditorPreviewKind::Box: return EditorViewportStyle::Object;
    case EditorPreviewKind::Light: return EditorViewportStyle::Light;
    case EditorPreviewKind::Spawn: return EditorViewportStyle::Spawn;
    default: return EditorViewportStyle::Marker;
    }
}
}

void EditorPreviewRenderer::Resize(int width, int height)
{
    width_ = width;
    height_ = height;
}

void EditorPreviewRenderer::Render(const EditorViewportState& state)
{
    drawList_.Clear();
    pickShapes_.clear();
    if (!scene_ || width_ <= 0 || height_ <= 0)
        return;

    projection_ = MakeEditorPreviewProjectionContext(state, width_, height_);
    pickShapes_ = BuildEditorPreviewPickShapes(*scene_, projection_);
    for (const EditorPreviewObject& object : scene_->GetObjects())
    {
        if (!object.visible)
            continue;
        const EditorPreviewProjectedPoint point =
            ProjectEditorPreviewObject(object, projection_);
        if (!point.visible)
            continue;
        const float x = point.x;
        const float y = point.y;

        const EditorViewportStyle style = StyleFor(object.kind);
        if (object.kind == EditorPreviewKind::Box)
            drawList_.Add({EditorViewportPrimitiveType::Rectangle, style,
                x - 14.0f, y - 10.0f, x + 14.0f, y + 10.0f});
        else if (object.kind == EditorPreviewKind::Light)
            drawList_.Add({EditorViewportPrimitiveType::Circle, style,
                x, y, 0.0f, 0.0f, 9.0f});
        else
        {
            drawList_.Add({EditorViewportPrimitiveType::Line, style,
                x - 9.0f, y, x + 9.0f, y});
            drawList_.Add({EditorViewportPrimitiveType::Line, style,
                x, y - 9.0f, x, y + 9.0f});
        }

        if (object.selected)
        {
            drawList_.Add({EditorViewportPrimitiveType::Rectangle,
                EditorViewportStyle::Selected,
                x - 19.0f, y - 15.0f, x + 19.0f, y + 15.0f});
            if (gizmoVisible_)
            {
                drawList_.Add({EditorViewportPrimitiveType::Line,
                    activeGizmoAxis_ == EditorGizmoAxis::X
                        ? EditorViewportStyle::GizmoActive
                        : EditorViewportStyle::GizmoX,
                    x, y, x + 45.0f, y});
                drawList_.Add({EditorViewportPrimitiveType::Line,
                    activeGizmoAxis_ == EditorGizmoAxis::Z
                        ? EditorViewportStyle::GizmoActive
                        : EditorViewportStyle::GizmoZ,
                    x, y, x, y - 45.0f});
            }
        }
        if (labelsVisible_)
            drawList_.Add({EditorViewportPrimitiveType::Text,
                EditorViewportStyle::Label, x + 18.0f, y - 8.0f,
                0.0f, 0.0f, 0.0f, object.label});
    }

    if (placementPreview_.valid)
    {
        EditorPreviewObject marker;
        marker.x = placementPreview_.x;
        marker.y = placementPreview_.y;
        marker.z = placementPreview_.z;
        const EditorPreviewProjectedPoint point =
            ProjectEditorPreviewObject(marker, projection_);
        if (point.visible)
        {
            drawList_.Add({EditorViewportPrimitiveType::Circle,
                EditorViewportStyle::Placement, point.x, point.y,
                0.0f, 0.0f, 8.0f});
            drawList_.Add({EditorViewportPrimitiveType::Line,
                EditorViewportStyle::Placement,
                point.x - 12.0f, point.y, point.x + 12.0f, point.y});
            drawList_.Add({EditorViewportPrimitiveType::Line,
                EditorViewportStyle::Placement,
                point.x, point.y - 12.0f, point.x, point.y + 12.0f});
        }
    }
}

EditorPreviewProjectedPoint EditorPreviewRenderer::SelectedPoint() const
{
    if (!scene_)
        return {};
    const EditorPreviewObject* object =
        scene_->FindByLogicalPath(scene_->SelectedPath());
    return object ? ProjectEditorPreviewObject(*object, projection_)
                  : EditorPreviewProjectedPoint{};
}

EditorPreviewPickResult EditorPreviewRenderer::Pick(float x, float y) const
{
    return PickEditorPreview(pickShapes_, x, y);
}
