#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_GEOMETRY_CACHE_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_GEOMETRY_CACHE_H

#include "editor_assets/EditorStaticMeshDecoder.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

class EditorRenderAssetRegistry;

struct EditorRenderGeometryCacheLimits
{
    std::size_t maximumEntries = 512;
    std::size_t maximumTotalBytes = 256ull * 1024ull * 1024ull;
    std::size_t maximumAssetBytes = 96ull * 1024ull * 1024ull;
};

struct EditorRenderGeometryCacheStatistics
{
    std::size_t requests = 0;
    std::size_t hits = 0;
    std::size_t decoded = 0;
    std::size_t unsupported = 0;
    std::size_t malformed = 0;
    std::size_t bytes = 0;
};

class EditorRenderGeometryCache
{
public:
    explicit EditorRenderGeometryCache(
        EditorRenderGeometryCacheLimits limits = {}) : limits_(limits) {}

    void Bind(const std::filesystem::path& libraryRoot,
        EditorRenderAssetRegistry* registry);
    void Clear();
    const EditorStaticAssetGeometry* Request(
        const EditorRenderObjectAsset& asset,
        std::string* reason = nullptr);
    const EditorStaticAssetGeometry* Find(std::string_view assetId) const;
    EditorRenderGeometryCacheStatistics Statistics() const { return statistics_; }
    bool IsBound() const { return registry_ != nullptr && !root_.empty(); }

private:
    struct Entry
    {
        EditorStaticGeometryDecodeStatus status =
            EditorStaticGeometryDecodeStatus::Unsupported;
        EditorRenderAssetReadiness originalReadiness =
            EditorRenderAssetReadiness::Unsupported;
        EditorStaticAssetGeometry geometry;
        std::string reason;
    };

    EditorRenderGeometryCacheLimits limits_;
    std::filesystem::path root_;
    EditorRenderAssetRegistry* registry_ = nullptr;
    std::unordered_map<std::string, Entry> entries_;
    EditorRenderGeometryCacheStatistics statistics_;
};

#endif
