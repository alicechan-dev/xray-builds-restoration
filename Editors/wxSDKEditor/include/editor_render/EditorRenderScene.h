#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_SCENE_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_SCENE_H

#include "editor_model/EditorTransform.h"
#include "editor_render/EditorRenderAsset.h"

#include <string>
#include <vector>

class EditorTreeModel;
class EditorRenderAssetRegistry;

struct EditorRenderInstance
{
    std::string logicalPath;
    std::string assetId;
    EditorTransform transform;
    EditorRenderBounds objectBounds;
    EditorRenderAssetReadiness readiness = EditorRenderAssetReadiness::Unsupported;
    bool selected = false;
    bool visible = true;
    bool fallback = true;
};

class EditorRenderScene
{
public:
    const std::vector<EditorRenderInstance>& Instances() const { return instances_; }
    void Add(EditorRenderInstance instance) { instances_.push_back(std::move(instance)); }
private:
    std::vector<EditorRenderInstance> instances_;
};

EditorRenderScene BuildEditorRenderScene(const EditorTreeModel& model,
    const EditorRenderAssetRegistry& registry,
    const std::string& selectedPath = {});

#endif
