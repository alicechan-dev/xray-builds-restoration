#include "editor_render/EditorRenderGeometryCache.h"

#include "editor_render/EditorRenderAssetRegistry.h"

void EditorRenderGeometryCache::Bind(const std::filesystem::path& libraryRoot,
    EditorRenderAssetRegistry* registry)
{
    Clear();
    root_ = libraryRoot;
    registry_ = registry;
}

void EditorRenderGeometryCache::Clear()
{
    if (registry_)
        for (const auto& item : entries_)
            registry_->UpdateReadiness(item.first, item.second.originalReadiness);
    entries_.clear();
    root_.clear();
    registry_ = nullptr;
    statistics_ = {};
}

const EditorStaticAssetGeometry* EditorRenderGeometryCache::Request(
    const EditorRenderObjectAsset& asset, std::string* reason)
{
    ++statistics_.requests;
    if (const auto found = entries_.find(asset.assetId); found != entries_.end())
    {
        ++statistics_.hits;
        if (reason) *reason = found->second.reason;
        return found->second.status == EditorStaticGeometryDecodeStatus::Decoded
            ? &found->second.geometry : nullptr;
    }
    if (!IsBound())
    {
        if (reason) *reason = "Geometry cache is not bound to an Object Library.";
        return nullptr;
    }
    if (entries_.size() >= limits_.maximumEntries)
    {
        if (reason) *reason = "Geometry cache entry limit reached.";
        return nullptr;
    }

    Entry entry;
    entry.originalReadiness = asset.readiness;
    EditorStaticMeshDecoderLimits decoderLimits;
    decoderLimits.maximumDecodedBytes = limits_.maximumAssetBytes;
    entry.status = EditorStaticMeshDecoder(decoderLimits).Decode(
        root_, asset, entry.geometry, &entry.reason);
    if (entry.status == EditorStaticGeometryDecodeStatus::Decoded)
    {
        const std::size_t bytes = entry.geometry.MemoryBytes();
        if (bytes > limits_.maximumTotalBytes - statistics_.bytes)
        {
            entry.status = EditorStaticGeometryDecodeStatus::Unsupported;
            entry.geometry = {};
            entry.reason = "Geometry cache memory budget reached.";
        }
        else
        {
            statistics_.bytes += bytes;
            ++statistics_.decoded;
            registry_->UpdateReadiness(asset.assetId,
                EditorRenderAssetReadiness::StaticGeometryDecoded);
        }
    }
    if (entry.status == EditorStaticGeometryDecodeStatus::Unsupported)
    {
        ++statistics_.unsupported;
        registry_->UpdateReadiness(asset.assetId,
            EditorRenderAssetReadiness::Unsupported);
    }
    else if (entry.status == EditorStaticGeometryDecodeStatus::Malformed)
    {
        ++statistics_.malformed;
        registry_->UpdateReadiness(asset.assetId,
            EditorRenderAssetReadiness::Malformed);
    }
    const auto inserted = entries_.emplace(asset.assetId, std::move(entry));
    if (reason) *reason = inserted.first->second.reason;
    return inserted.first->second.status ==
        EditorStaticGeometryDecodeStatus::Decoded
        ? &inserted.first->second.geometry : nullptr;
}

const EditorStaticAssetGeometry* EditorRenderGeometryCache::Find(
    std::string_view assetId) const
{
    const auto found = entries_.find(std::string(assetId));
    return found != entries_.end() && found->second.status ==
        EditorStaticGeometryDecodeStatus::Decoded
        ? &found->second.geometry : nullptr;
}
