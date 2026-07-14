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
        const bool light = object.bodyDecode.hasLight &&
            std::isfinite(object.bodyDecode.light.range) &&
            object.bodyDecode.light.range >= 0.0f;
        if (light)
            label += " [" + std::string(EditorHistoricalLightTypeName(
                object.bodyDecode.light.type)) + "]";
        const float diagnosticSize = light ? object.bodyDecode.light.range :
            glow ? object.bodyDecode.glow.radius : 1.0f;
        scene.AddObject({object.stableRecordId, label,
            object.transform.x, object.transform.y, object.transform.z,
            diagnosticSize, diagnosticSize, diagnosticSize,
            light ? EditorPreviewKind::HistoricalLight :
                glow ? EditorPreviewKind::Glow : EditorPreviewKind::Box});
    }
    scene.SetSelectedPath(selectedStableId);
    return scene;
}
