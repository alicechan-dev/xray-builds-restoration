#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_OVERLAY_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_OVERLAY_H

#include "editor_render/EditorRenderProjection.h"

#include <cstdint>
#include <string>
#include <vector>

class EditorPreviewScene;
class EditorRenderScene;

enum class EditorPreviewLabelPolicy { Off, SelectedOnly, All };
enum class EditorRenderOverlayStyle
{
    Object, Runtime, Glow, Light, Unsupported, Selected, GizmoX, GizmoZ, Grid
};

struct EditorRenderLine3D
{
    EditorRenderPoint3D first;
    EditorRenderPoint3D second;
    EditorRenderOverlayStyle style = EditorRenderOverlayStyle::Object;
    bool depthTest = true;
    bool selected = false;
};

struct EditorRenderLabel
{
    std::string text;
    float screenX = 0.0f;
    float screenY = 0.0f;
    float depth = 0.0f;
    bool selected = false;
    std::size_t stableOrder = 0;
};

struct EditorRenderOverlayOptions
{
    EditorPreviewLabelPolicy labels = EditorPreviewLabelPolicy::SelectedOnly;
    bool runtimeMarkers = true;
    bool glowMarkers = true;
    bool lightMarkers = true;
    bool unsupportedBounds = true;
    bool objectBounds = true;
    bool depthTestRuntimeMarkers = true;
    bool grid = true;
    bool gizmo = false;
    std::size_t maximumLines = 65536;
    std::size_t maximumLabels = 256;
};

struct EditorRenderOverlayBatch
{
    std::vector<EditorRenderLine3D> lines;
    std::vector<EditorRenderLabel> labels;
    std::size_t skippedLines = 0;
    std::size_t skippedLabels = 0;
};

EditorRenderOverlayBatch BuildEditorRenderOverlay(
    const EditorPreviewScene& previewScene,
    const EditorRenderScene& renderScene,
    const EditorRenderFrameContext& frame,
    const EditorRenderOverlayOptions& options);

#endif
