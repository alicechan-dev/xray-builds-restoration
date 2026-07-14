#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_PREVIEW_ADAPTER_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_PREVIEW_ADAPTER_H

#include "editor_view/EditorPreviewScene.h"

#include <string>

class EditorHistoricalSceneDocument;

EditorPreviewScene BuildHistoricalScenePreview(
    const EditorHistoricalSceneDocument& document,
    const std::string& selectedStableId = {});

#endif
