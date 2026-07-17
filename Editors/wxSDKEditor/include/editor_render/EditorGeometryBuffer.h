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

struct EditorGeometryNormal
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

enum class EditorGeometryNormalStatus
{
    PositionsAndIndices,
    NormalsAvailable,
    NormalsGenerated,
    NormalGenerationFailed
};

struct EditorGeometryBuffer
{
    std::vector<EditorGeometryPosition> positions;
    std::vector<EditorGeometryNormal> normals;
    std::vector<EditorGeometryTriangle> triangles;
    EditorGeometryNormalStatus normalStatus =
        EditorGeometryNormalStatus::PositionsAndIndices;

    std::size_t MemoryBytes() const;
};

bool GenerateEditorGeometryNormals(EditorGeometryBuffer& buffer,
    std::size_t* ignoredZeroAreaTriangles = nullptr);
const char* ToString(EditorGeometryNormalStatus value);

#endif
