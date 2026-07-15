#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_ASSET_REGISTRY_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_ASSET_REGISTRY_H

#include "editor_render/EditorRenderAsset.h"

#include <string_view>
#include <unordered_map>

struct EditorRenderAssetStatistics
{
    std::size_t assets = 0, boundsOnly = 0, metadataReady = 0;
    std::size_t decodeCandidates = 0, skeletalDeferred = 0;
    std::size_t unsupported = 0, malformed = 0;
    std::size_t meshes = 0, vertices = 0, triangles = 0;
};

class EditorRenderAssetRegistry
{
public:
    bool Build(const EditorObjectLibrary& library, std::string* reason = nullptr);
    void Clear();
    bool IsLoaded() const { return loaded_; }
    const EditorRenderObjectAsset* Find(std::string_view normalizedId) const;
    const std::vector<EditorRenderObjectAsset>& Assets() const { return assets_; }
    EditorRenderAssetStatistics Statistics() const;
private:
    bool loaded_ = false;
    std::vector<EditorRenderObjectAsset> assets_;
    std::unordered_map<std::string, std::size_t> index_;
};

#endif
