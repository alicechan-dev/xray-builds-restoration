#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_PREVIEW_ADAPTER_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_PREVIEW_ADAPTER_H

#include "editor_view/EditorPreviewScene.h"

#include <string>

class EditorTreeModel;
class EditorRenderAssetRegistry;

EditorPreviewScene BuildEditorPreviewScene(const EditorTreeModel& model,
    const std::string& selectedPath = {},
    const EditorRenderAssetRegistry* assets = nullptr);

#endif
