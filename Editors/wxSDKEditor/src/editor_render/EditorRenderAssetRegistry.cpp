#include "editor_render/EditorRenderAssetRegistry.h"

#include <limits>

bool EditorRenderAssetRegistry::Build(const EditorObjectLibrary& library,
    std::string* reason)
{
    if (!library.IsLoaded()) { if (reason) *reason = "Object Library is not loaded."; return false; }
    EditorRenderAssetRegistry candidate; candidate.loaded_ = true;
    for (const auto& entry : library.Entries()) {
        EditorRenderObjectAsset asset;
        asset.assetId = entry.referenceId; asset.objectKind = entry.kind;
        asset.bounds = entry.bounds; asset.meshCount = entry.meshCount;
        asset.surfaceCount = entry.surfaceCount; asset.sourceRelativeFile = entry.sourceRelativeFile;
        asset.meshes = entry.meshes; asset.diagnostics = entry.diagnostics;
        bool allSupported = !asset.meshes.empty();
        bool allValidated = !asset.meshes.empty();
        for (const auto& mesh : asset.meshes) {
            if (asset.totalVertices > std::numeric_limits<std::size_t>::max() - mesh.vertexCount ||
                asset.totalTriangles > std::numeric_limits<std::size_t>::max() - mesh.triangleCount) {
                if (reason) *reason = "Render asset count overflow."; return false;
            }
            asset.totalVertices += mesh.vertexCount; asset.totalTriangles += mesh.triangleCount;
            allSupported &= mesh.supported; allValidated &= mesh.countsValidated;
        }
        if (entry.parseStatus == EditorObjectParseStatus::Malformed)
            asset.readiness = EditorRenderAssetReadiness::Malformed;
        else if (entry.kind == EditorObjectKind::Skeletal)
            asset.readiness = EditorRenderAssetReadiness::SkeletalDeferred;
        else if (!asset.bounds.valid)
            asset.readiness = EditorRenderAssetReadiness::Unsupported;
        else if (allSupported && allValidated)
            asset.readiness = EditorRenderAssetReadiness::StaticGeometryDecodeCandidate;
        else if (!asset.meshes.empty())
            asset.readiness = EditorRenderAssetReadiness::StaticGeometryMetadataReady;
        else asset.readiness = EditorRenderAssetReadiness::BoundsOnly;
        if (candidate.index_.count(asset.assetId)) continue;
        candidate.index_[asset.assetId] = candidate.assets_.size();
        candidate.assets_.push_back(std::move(asset));
    }
    *this = std::move(candidate); return true;
}

void EditorRenderAssetRegistry::Clear() { loaded_ = false; assets_.clear(); index_.clear(); }
const EditorRenderObjectAsset* EditorRenderAssetRegistry::Find(std::string_view id) const
{ auto it=index_.find(std::string(id)); return it==index_.end()?nullptr:&assets_[it->second]; }
EditorRenderAssetStatistics EditorRenderAssetRegistry::Statistics() const
{
    EditorRenderAssetStatistics s; s.assets=assets_.size();
    for(const auto& a:assets_){s.meshes+=a.meshCount;s.vertices+=a.totalVertices;s.triangles+=a.totalTriangles;
        switch(a.readiness){case EditorRenderAssetReadiness::BoundsOnly:++s.boundsOnly;break;
        case EditorRenderAssetReadiness::StaticGeometryMetadataReady:++s.metadataReady;break;
        case EditorRenderAssetReadiness::StaticGeometryDecodeCandidate:++s.decodeCandidates;break;
        case EditorRenderAssetReadiness::SkeletalDeferred:++s.skeletalDeferred;break;
        case EditorRenderAssetReadiness::Malformed:++s.malformed;break;default:++s.unsupported;break;}}
    return s;
}
