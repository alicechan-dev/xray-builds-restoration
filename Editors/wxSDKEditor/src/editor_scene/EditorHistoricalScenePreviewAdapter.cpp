#include "editor_scene/EditorHistoricalScenePreviewAdapter.h"

#include "editor_scene/EditorHistoricalSceneDocument.h"

#include <cmath>

EditorPreviewScene BuildHistoricalScenePreview(
    const EditorHistoricalSceneDocument& document,
    const std::string& selectedStableId)
{
    EditorPreviewScene scene;
    for (const EditorHistoricalSceneObjectData& object : document.Objects())
    {
        if (!object.transformConfirmed)
            continue;
        std::string label = object.sourceName.empty()
            ? object.stableRecordId : object.sourceName;
        if (object.bodyDecode.hasSceneObject &&
            !object.bodyDecode.sceneObject.referenceName.empty())
            label += " [" + object.bodyDecode.sceneObject.referenceName + "]";
        const bool glow = object.bodyDecode.hasGlow &&
            std::isfinite(object.bodyDecode.glow.radius);
        scene.AddObject({object.stableRecordId, label,
            object.transform.x, object.transform.y, object.transform.z,
            glow ? object.bodyDecode.glow.radius : 1.0f,
            glow ? object.bodyDecode.glow.radius : 1.0f,
            glow ? object.bodyDecode.glow.radius : 1.0f,
            glow ? EditorPreviewKind::Glow : EditorPreviewKind::Box});
    }
    scene.SetSelectedPath(selectedStableId);
    return scene;
}
