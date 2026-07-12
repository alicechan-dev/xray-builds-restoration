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
    if (!scene_ || width_ <= 0 || height_ <= 0)
        return;

    constexpr float scale = 40.0f;
    for (const EditorPreviewObject& object : scene_->GetObjects())
    {
        if (!object.visible)
            continue;
        const float x = static_cast<float>(width_) * 0.5f +
            (object.x - state.camera.x) * scale;
        const float y = static_cast<float>(height_) * 0.5f +
            (object.z - state.camera.z) * scale;
        if (x < -40.0f || y < -40.0f ||
            x > static_cast<float>(width_ + 40) ||
            y > static_cast<float>(height_ + 40))
            continue;

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
            drawList_.Add({EditorViewportPrimitiveType::Rectangle,
                EditorViewportStyle::Selected,
                x - 19.0f, y - 15.0f, x + 19.0f, y + 15.0f});
        if (labelsVisible_)
            drawList_.Add({EditorViewportPrimitiveType::Text,
                EditorViewportStyle::Label, x + 18.0f, y - 8.0f,
                0.0f, 0.0f, 0.0f, object.label});
    }
}
