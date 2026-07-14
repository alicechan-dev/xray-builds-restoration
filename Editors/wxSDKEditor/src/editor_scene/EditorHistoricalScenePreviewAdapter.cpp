#include "editor_scene/EditorHistoricalScenePreviewAdapter.h"

#include "editor_scene/EditorHistoricalSceneDocument.h"

EditorPreviewScene BuildHistoricalScenePreview(
    const EditorHistoricalSceneDocument& document,
    const std::string& selectedStableId)
{
    EditorPreviewScene scene;
    for (const EditorHistoricalSceneObjectData& object : document.Objects())
    {
        if (!object.transformConfirmed)
            continue;
        scene.AddObject({object.stableRecordId,
            object.sourceName.empty() ? object.stableRecordId : object.sourceName,
            object.transform.x, object.transform.y, object.transform.z,
            1.0f, 1.0f, 1.0f, EditorPreviewKind::Box});
    }
    scene.SetSelectedPath(selectedStableId);
    return scene;
}
