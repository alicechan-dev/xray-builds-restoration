#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_ASSET_WORKING_SET_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_ASSET_WORKING_SET_H

#include <cstddef>
#include <string>
#include <vector>

class EditorRenderAssetRegistry;
class EditorRenderScene;

struct EditorRenderAssetWorkingSetEntry
{
    std::string assetId;
    std::size_t instanceCount = 0;
    bool selected = false;
    bool available = false;
    bool staticGeometry = false;
    bool skeletalDeferred = false;
};

struct EditorRenderAssetWorkingSetStatistics
{
    std::size_t uniqueAssets = 0;
    std::size_t referencedInstances = 0;
    std::size_t staticAssets = 0;
    std::size_t skeletalDeferredAssets = 0;
    std::size_t missingAssets = 0;
};

class EditorRenderAssetWorkingSet
{
public:
    void Rebuild(const EditorRenderScene& scene,
        const EditorRenderAssetRegistry& registry);
    void Clear();

    const std::vector<EditorRenderAssetWorkingSetEntry>& Entries() const
    { return entries_; }
    const EditorRenderAssetWorkingSetStatistics& Statistics() const
    { return statistics_; }

private:
    std::vector<EditorRenderAssetWorkingSetEntry> entries_;
    EditorRenderAssetWorkingSetStatistics statistics_;
};

#endif
