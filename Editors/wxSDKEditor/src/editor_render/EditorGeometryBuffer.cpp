#include "editor_render/EditorGeometryBuffer.h"

std::size_t EditorGeometryBuffer::MemoryBytes() const
{
    return positions.size() * sizeof(EditorGeometryPosition) +
        triangles.size() * sizeof(EditorGeometryTriangle);
}
