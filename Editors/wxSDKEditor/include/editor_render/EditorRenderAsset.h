#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_ASSET_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_ASSET_H

#include "editor_assets/EditorObjectLibrary.h"
#include "editor_render/EditorRenderMeshMetadata.h"

#include <cstddef>
#include <string>
#include <vector>

enum class EditorRenderAssetReadiness
{
    BoundsOnly,
    StaticGeometryMetadataReady,
    StaticGeometryDecodeCandidate,
    SkeletalDeferred,
    Unsupported,
    Malformed
};

struct EditorRenderObjectAsset
{
    std::string assetId;
    EditorObjectKind objectKind = EditorObjectKind::Unknown;
    EditorRenderAssetReadiness readiness = EditorRenderAssetReadiness::Unsupported;
    EditorRenderBounds bounds;
    std::size_t meshCount = 0;
    std::size_t surfaceCount = 0;
    std::size_t totalVertices = 0;
    std::size_t totalTriangles = 0;
    std::string sourceRelativeFile;
    std::vector<EditorRenderMeshMetadata> meshes;
    std::vector<std::string> diagnostics;
};

const char* ToString(EditorRenderAssetReadiness value);

#endif
