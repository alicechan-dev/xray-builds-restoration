#ifndef XR_WX_SDK_EDITOR_EDITOR_ASSET_DESCRIPTOR_H
#define XR_WX_SDK_EDITOR_EDITOR_ASSET_DESCRIPTOR_H

#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTransform.h"
#include "editor_view/EditorPreviewScene.h"

#include <string>

enum class EditorAssetPlacementType { Object, Light, Marker, Spawn };

struct EditorAssetDescriptor
{
    std::string id;
    std::string displayName;
    std::string categoryPath;
    std::string description;
    std::string baseNodeName;
    EditorItemKind itemKind = EditorItemKind::Object;
    std::string nodeCategory;
    EditorTransform defaultTransform;
    EditorPreviewKind previewKind = EditorPreviewKind::Unknown;
    EditorAssetPlacementType placementType = EditorAssetPlacementType::Object;
    bool placeable = true;
};

#endif
