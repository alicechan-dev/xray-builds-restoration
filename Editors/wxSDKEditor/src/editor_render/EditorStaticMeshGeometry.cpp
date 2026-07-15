#include "editor_render/EditorStaticMeshGeometry.h"

std::size_t EditorStaticAssetGeometry::MemoryBytes() const
{
    std::size_t result = 0;
    for (const auto& mesh : meshes)
        result += mesh.buffer.MemoryBytes();
    return result;
}

const char* ToString(EditorStaticGeometryDecodeStatus value)
{
    switch (value)
    {
    case EditorStaticGeometryDecodeStatus::Decoded: return "Decoded";
    case EditorStaticGeometryDecodeStatus::Malformed: return "Malformed";
    default: return "Unsupported";
    }
}
