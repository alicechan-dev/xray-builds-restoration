#ifndef XR_WX_SDK_EDITOR_EDITOR_STATIC_MESH_DECODER_H
#define XR_WX_SDK_EDITOR_EDITOR_STATIC_MESH_DECODER_H

#include "editor_render/EditorRenderAsset.h"
#include "editor_render/EditorStaticMeshGeometry.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

struct EditorStaticMeshDecoderLimits
{
    std::uintmax_t maximumFileSize = 96ull * 1024ull * 1024ull;
    std::size_t maximumMeshes = 4096;
    std::size_t maximumVerticesPerMesh = 4000000;
    std::size_t maximumTrianglesPerMesh = 4000000;
    std::size_t maximumDecodedBytes = 96ull * 1024ull * 1024ull;
};

class EditorStaticMeshDecoder
{
public:
    explicit EditorStaticMeshDecoder(EditorStaticMeshDecoderLimits limits = {})
        : limits_(limits) {}

    EditorStaticGeometryDecodeStatus Decode(
        const std::filesystem::path& libraryRoot,
        const EditorRenderObjectAsset& asset,
        EditorStaticAssetGeometry& geometry,
        std::string* reason = nullptr) const;

private:
    EditorStaticMeshDecoderLimits limits_;
};

#endif
