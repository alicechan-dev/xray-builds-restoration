#ifndef XR_WX_SDK_EDITOR_EDITOR_STATIC_MESH_GEOMETRY_H
#define XR_WX_SDK_EDITOR_EDITOR_STATIC_MESH_GEOMETRY_H

#include "editor_render/EditorGeometryBuffer.h"
#include "editor_render/EditorRenderMeshMetadata.h"

#include <cstddef>
#include <string>
#include <vector>

enum class EditorStaticGeometryDecodeStatus
{
    Decoded,
    Unsupported,
    Malformed
};

struct EditorStaticMeshGeometry
{
    std::string meshId;
    std::string name;
    EditorGeometryBuffer buffer;
    EditorRenderBounds bounds;
    std::size_t degenerateTriangles = 0;
    std::size_t zeroAreaTriangles = 0;
    bool boundsMismatch = false;
};

struct EditorStaticAssetGeometry
{
    std::string assetId;
    std::string sourceRelativeFile;
    EditorStaticGeometryDecodeStatus status =
        EditorStaticGeometryDecodeStatus::Unsupported;
    std::vector<EditorStaticMeshGeometry> meshes;
    std::size_t totalVertices = 0;
    std::size_t totalTriangles = 0;
    std::size_t degenerateTriangles = 0;
    std::size_t zeroAreaTriangles = 0;
    std::size_t boundsMismatches = 0;
    std::vector<std::string> diagnostics;

    std::size_t MemoryBytes() const;
};

const char* ToString(EditorStaticGeometryDecodeStatus value);

#endif
