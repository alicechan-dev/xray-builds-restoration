#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_PROPERTIES_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_PROPERTIES_H

#include "editor_model/EditorPropertySet.h"

struct EditorHistoricalSceneObjectData;

EditorPropertySet BuildHistoricalSceneObjectPropertySet(
    const EditorHistoricalSceneObjectData& object);

#endif
