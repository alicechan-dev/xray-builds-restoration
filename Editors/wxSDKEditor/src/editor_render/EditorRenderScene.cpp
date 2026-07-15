#include "editor_render/EditorRenderScene.h"

#include "editor_assets/EditorObjectLibrary.h"
#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_render/EditorRenderAssetRegistry.h"
#include "editor_scene/EditorHistoricalSceneDocument.h"

namespace
{
void AddNode(const EditorTreeNode& node, const EditorRenderAssetRegistry& registry,
    const std::string& selectedPath, EditorRenderScene& scene)
{
    if (node.Kind()!=EditorItemKind::Root && node.Kind()!=EditorItemKind::Folder) {
        EditorRenderInstance instance; instance.logicalPath=node.Path();
        instance.transform=node.Transform();instance.selected=node.Path()==selectedPath;
        if(node.HistoricalOrigin()&&!node.HistoricalOrigin()->referenceName.empty()){
            std::string id;if(NormalizeHistoricalObjectReference(node.HistoricalOrigin()->referenceName,id)){
                instance.assetId=id;if(const auto* asset=registry.Find(id)){instance.objectBounds=asset->bounds;
                    instance.readiness=asset->readiness;instance.fallback=!asset->bounds.valid;}}}
        scene.Add(std::move(instance));
    }
    for(const auto& child:node.ChildrenView())AddNode(*child,registry,selectedPath,scene);
}
}

EditorRenderScene BuildEditorRenderScene(const EditorTreeModel& model,
    const EditorRenderAssetRegistry& registry,const std::string& selectedPath)
{
    EditorRenderScene scene;if(model.Root())AddNode(*model.Root(),registry,selectedPath,scene);return scene;
}

EditorRenderScene BuildHistoricalRenderScene(
    const EditorHistoricalSceneDocument& document,
    const EditorRenderAssetRegistry& registry,
    const std::string& selectedStableId)
{
    EditorRenderScene scene;
    for (const EditorHistoricalSceneObjectData& object : document.Objects())
    {
        if (!object.transformConfirmed)
            continue;
        EditorRenderInstance instance;
        instance.logicalPath = object.stableRecordId;
        instance.transform = object.transform;
        instance.selected = object.stableRecordId == selectedStableId;
        if (object.bodyDecode.hasSceneObject &&
            !object.bodyDecode.sceneObject.referenceName.empty())
        {
            std::string id;
            if (NormalizeHistoricalObjectReference(
                object.bodyDecode.sceneObject.referenceName, id))
            {
                instance.assetId = id;
                if (const EditorRenderObjectAsset* asset = registry.Find(id))
                {
                    instance.objectBounds = asset->bounds;
                    instance.readiness = asset->readiness;
                    instance.fallback = !asset->bounds.valid;
                }
            }
        }
        scene.Add(std::move(instance));
    }
    return scene;
}
