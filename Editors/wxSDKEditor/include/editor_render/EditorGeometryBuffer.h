#ifndef XR_WX_SDK_EDITOR_EDITOR_GEOMETRY_BUFFER_H
#define XR_WX_SDK_EDITOR_EDITOR_GEOMETRY_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <vector>

struct EditorGeometryPosition
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct EditorGeometryTriangle
{
    std::uint32_t a = 0;
    std::uint32_t b = 0;
    std::uint32_t c = 0;
};

struct EditorGeometryBuffer
{
    std::vector<EditorGeometryPosition> positions;
    std::vector<EditorGeometryTriangle> triangles;

    std::size_t MemoryBytes() const;
};

#endif
