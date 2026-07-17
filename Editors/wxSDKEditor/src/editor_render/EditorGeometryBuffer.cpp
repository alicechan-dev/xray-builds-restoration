#include "editor_render/EditorGeometryBuffer.h"

#include <cmath>

std::size_t EditorGeometryBuffer::MemoryBytes() const
{
    return positions.size() * sizeof(EditorGeometryPosition) +
        normals.size() * sizeof(EditorGeometryNormal) +
        triangles.size() * sizeof(EditorGeometryTriangle);
}

bool GenerateEditorGeometryNormals(EditorGeometryBuffer& buffer,
    std::size_t* ignoredZeroAreaTriangles)
{
    if (ignoredZeroAreaTriangles) *ignoredZeroAreaTriangles = 0;
    buffer.normals.assign(buffer.positions.size(), {});
    for (const EditorGeometryPosition& p : buffer.positions)
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z))
        {
            buffer.normals.clear();
            buffer.normalStatus = EditorGeometryNormalStatus::NormalGenerationFailed;
            return false;
        }
    for (const EditorGeometryTriangle& triangle : buffer.triangles)
    {
        if (triangle.a >= buffer.positions.size() ||
            triangle.b >= buffer.positions.size() ||
            triangle.c >= buffer.positions.size())
        {
            buffer.normals.clear();
            buffer.normalStatus = EditorGeometryNormalStatus::NormalGenerationFailed;
            return false;
        }
        const auto& a = buffer.positions[triangle.a];
        const auto& b = buffer.positions[triangle.b];
        const auto& c = buffer.positions[triangle.c];
        const double abx = static_cast<double>(b.x) - a.x;
        const double aby = static_cast<double>(b.y) - a.y;
        const double abz = static_cast<double>(b.z) - a.z;
        const double acx = static_cast<double>(c.x) - a.x;
        const double acy = static_cast<double>(c.y) - a.y;
        const double acz = static_cast<double>(c.z) - a.z;
        const double nx = aby * acz - abz * acy;
        const double ny = abz * acx - abx * acz;
        const double nz = abx * acy - aby * acx;
        const double lengthSquared = nx * nx + ny * ny + nz * nz;
        if (!std::isfinite(lengthSquared) || lengthSquared <= 1.0e-20)
        {
            if (ignoredZeroAreaTriangles) ++*ignoredZeroAreaTriangles;
            continue;
        }
        for (std::uint32_t index : {triangle.a, triangle.b, triangle.c})
        {
            buffer.normals[index].x += static_cast<float>(nx);
            buffer.normals[index].y += static_cast<float>(ny);
            buffer.normals[index].z += static_cast<float>(nz);
        }
    }
    bool any = false;
    for (EditorGeometryNormal& normal : buffer.normals)
    {
        const double length = std::sqrt(static_cast<double>(normal.x) * normal.x +
            static_cast<double>(normal.y) * normal.y +
            static_cast<double>(normal.z) * normal.z);
        if (std::isfinite(length) && length > 1.0e-10)
        {
            normal.x = static_cast<float>(normal.x / length);
            normal.y = static_cast<float>(normal.y / length);
            normal.z = static_cast<float>(normal.z / length);
            any = true;
        }
        else
            normal = {0.0f, 1.0f, 0.0f};
    }
    buffer.normalStatus = any ? EditorGeometryNormalStatus::NormalsGenerated :
        EditorGeometryNormalStatus::NormalGenerationFailed;
    if (!any) buffer.normals.clear();
    return any;
}

const char* ToString(EditorGeometryNormalStatus value)
{
    switch (value)
    {
    case EditorGeometryNormalStatus::NormalsAvailable: return "NormalsAvailable";
    case EditorGeometryNormalStatus::NormalsGenerated: return "NormalsGenerated";
    case EditorGeometryNormalStatus::NormalGenerationFailed: return "NormalGenerationFailed";
    default: return "PositionsAndIndices";
    }
}
