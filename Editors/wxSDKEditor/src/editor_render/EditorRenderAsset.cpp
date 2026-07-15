#include "editor_render/EditorRenderAsset.h"

const char* ToString(EditorRenderAssetReadiness value)
{
    switch (value) {
    case EditorRenderAssetReadiness::BoundsOnly: return "Bounds Only";
    case EditorRenderAssetReadiness::StaticGeometryMetadataReady: return "Static Geometry Metadata Ready";
    case EditorRenderAssetReadiness::StaticGeometryDecodeCandidate: return "Static Geometry Decode Candidate";
    case EditorRenderAssetReadiness::StaticGeometryDecoded: return "Static Geometry Decoded";
    case EditorRenderAssetReadiness::SkeletalDeferred: return "Skeletal Deferred";
    case EditorRenderAssetReadiness::Malformed: return "Malformed";
    default: return "Unsupported";
    }
}
