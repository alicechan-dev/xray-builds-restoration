#ifndef XR_WX_SDK_EDITOR_EDITOR_PREVIEW_PICKING_H
#define XR_WX_SDK_EDITOR_EDITOR_PREVIEW_PICKING_H

#include "editor_view/EditorPreviewScene.h"
#include "editor_render/EditorRenderProjection.h"

#include <string>
#include <vector>

struct EditorViewportState;

struct EditorPreviewProjectionContext
{
    EditorRenderFrameContext frame;
};

struct EditorPreviewProjectedPoint
{
    float x = 0.0f;
    float y = 0.0f;
    bool visible = false;
};

struct EditorPreviewWorldPoint
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    bool valid = false;
};

struct EditorPreviewPickShape
{
    std::string logicalPath;
    EditorPreviewKind kind = EditorPreviewKind::Unknown;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float halfWidth = 0.0f;
    float halfHeight = 0.0f;
    float radius = 0.0f;
    bool circular = false;
    bool selectable = true;
};

struct EditorPreviewPickResult
{
    bool hit = false;
    std::string logicalPath;
    float distance = 0.0f;
    EditorPreviewKind kind = EditorPreviewKind::Unknown;
};

EditorPreviewProjectionContext MakeEditorPreviewProjectionContext(
    const EditorViewportState& state, int width, int height);
EditorPreviewProjectedPoint ProjectEditorPreviewObject(
    const EditorPreviewObject& object,
    const EditorPreviewProjectionContext& context);
EditorPreviewWorldPoint UnprojectEditorPreviewToGround(
    float screenX, float screenY,
    const EditorPreviewProjectionContext& context,
    float worldY = 0.0f, bool snap = false, float snapStep = 1.0f);
std::vector<EditorPreviewPickShape> BuildEditorPreviewPickShapes(
    const EditorPreviewScene& scene,
    const EditorPreviewProjectionContext& context);
EditorPreviewPickResult PickEditorPreview(
    const std::vector<EditorPreviewPickShape>& shapes, float x, float y,
    float markerTolerance = 4.0f);

#endif
