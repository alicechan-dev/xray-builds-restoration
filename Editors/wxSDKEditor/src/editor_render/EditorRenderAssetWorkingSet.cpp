#include "editor_render/EditorRenderAssetWorkingSet.h"

#include "editor_render/EditorRenderAssetRegistry.h"
#include "editor_render/EditorRenderScene.h"

#include <algorithm>
#include <unordered_map>

void EditorRenderAssetWorkingSet::Rebuild(const EditorRenderScene& scene,
    const EditorRenderAssetRegistry& registry)
{
    Clear();
    std::unordered_map<std::string, std::size_t> positions;
    for (const EditorRenderInstance& instance : scene.Instances())
    {
        if (instance.assetId.empty())
            continue;
        ++statistics_.referencedInstances;
        const auto inserted = positions.emplace(instance.assetId, entries_.size());
        if (inserted.second)
        {
            EditorRenderAssetWorkingSetEntry entry;
            entry.assetId = instance.assetId;
            if (const EditorRenderObjectAsset* asset = registry.Find(instance.assetId))
            {
                entry.available = true;
                entry.staticGeometry = asset->objectKind == EditorObjectKind::Static;
                entry.skeletalDeferred = asset->objectKind == EditorObjectKind::Skeletal;
            }
            entries_.push_back(std::move(entry));
        }
        EditorRenderAssetWorkingSetEntry& entry = entries_[inserted.first->second];
        ++entry.instanceCount;
        entry.selected = entry.selected || instance.selected;
    }
    std::stable_sort(entries_.begin(), entries_.end(),
        [](const auto& left, const auto& right)
        {
            if (left.selected != right.selected) return left.selected;
            return left.assetId < right.assetId;
        });
    statistics_.uniqueAssets = entries_.size();
    for (const auto& entry : entries_)
    {
        if (!entry.available) ++statistics_.missingAssets;
        else if (entry.staticGeometry) ++statistics_.staticAssets;
        else if (entry.skeletalDeferred) ++statistics_.skeletalDeferredAssets;
    }
}

void EditorRenderAssetWorkingSet::Clear()
{
    entries_.clear();
    statistics_ = {};
}
