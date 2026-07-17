#include "editor_assets/EditorStaticMeshDecoder.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>

namespace
{
constexpr std::uint32_t ObjectBody = 0x7777;
constexpr std::uint32_t ObjectMeshes = 0x0910;
constexpr std::uint32_t MeshVersion = 0x1000;
constexpr std::uint32_t MeshName = 0x1001;
constexpr std::uint32_t MeshBounds = 0x1004;
constexpr std::uint32_t MeshVertices = 0x1005;
constexpr std::uint32_t MeshFaces = 0x1006;
constexpr std::uint16_t SupportedMeshVersion = 0x0011;

struct Chunk
{
    std::uint32_t id = 0;
    std::uint64_t begin = 0;
    std::uint64_t size = 0;
};

class Reader
{
public:
    Reader(const std::filesystem::path& path, std::uint64_t size)
        : stream_(path, std::ios::binary), size_(size) {}

    bool Good() const { return stream_.is_open(); }
    bool Read(std::uint64_t offset, void* target, std::size_t size)
    {
        if (offset > size_ || size > size_ - offset ||
            offset > static_cast<std::uint64_t>(
                std::numeric_limits<std::streamoff>::max()))
            return false;
        stream_.clear();
        stream_.seekg(static_cast<std::streamoff>(offset));
        stream_.read(static_cast<char*>(target),
            static_cast<std::streamsize>(size));
        return static_cast<bool>(stream_);
    }
    bool U8(std::uint64_t offset, std::uint8_t& value)
    { return Read(offset, &value, sizeof(value)); }
    bool U16(std::uint64_t offset, std::uint16_t& value)
    {
        std::array<unsigned char, 2> bytes{};
        if (!Read(offset, bytes.data(), bytes.size())) return false;
        value = static_cast<std::uint16_t>(bytes[0] | (bytes[1] << 8));
        return true;
    }
    bool U32(std::uint64_t offset, std::uint32_t& value)
    {
        std::array<unsigned char, 4> bytes{};
        if (!Read(offset, bytes.data(), bytes.size())) return false;
        value = static_cast<std::uint32_t>(bytes[0]) |
            (static_cast<std::uint32_t>(bytes[1]) << 8) |
            (static_cast<std::uint32_t>(bytes[2]) << 16) |
            (static_cast<std::uint32_t>(bytes[3]) << 24);
        return true;
    }
    bool S32(std::uint64_t offset, std::int32_t& value)
    {
        std::uint32_t bits = 0;
        if (!U32(offset, bits)) return false;
        std::memcpy(&value, &bits, sizeof(value));
        return true;
    }
    bool Float(std::uint64_t offset, float& value)
    {
        std::uint32_t bits = 0;
        if (!U32(offset, bits)) return false;
        std::memcpy(&value, &bits, sizeof(value));
        return true;
    }
    bool CString(std::uint64_t& offset, std::uint64_t end,
        std::string& value)
    {
        value.clear();
        while (offset < end && value.size() < 16 * 1024)
        {
            char ch = 0;
            if (!Read(offset++, &ch, 1)) return false;
            if (ch == 0) return true;
            value.push_back(ch);
        }
        return false;
    }

private:
    std::ifstream stream_;
    std::uint64_t size_ = 0;
};

bool ReadChunk(Reader& reader, std::uint64_t offset, std::uint64_t end,
    Chunk& chunk)
{
    std::uint32_t size = 0;
    if (offset > end || end - offset < 8 ||
        !reader.U32(offset, chunk.id) || !reader.U32(offset + 4, size))
        return false;
    chunk.begin = offset + 8;
    chunk.size = size;
    return chunk.begin <= end && chunk.size <= end - chunk.begin;
}

bool IsInside(const std::filesystem::path& root,
    const std::filesystem::path& candidate)
{
    auto r = root.begin();
    auto c = candidate.begin();
    for (; r != root.end(); ++r, ++c)
        if (c == candidate.end() ||
            std::filesystem::path(*r).compare(*c) != 0)
            return false;
    return true;
}

bool AddSize(std::size_t& total, std::size_t count, std::size_t element,
    std::size_t maximum)
{
    if (count != 0 && element > maximum / count) return false;
    const std::size_t bytes = count * element;
    if (total > maximum - bytes) return false;
    total += bytes;
    return true;
}

void Include(EditorRenderBounds& bounds, const EditorGeometryPosition& point)
{
    if (!bounds.valid)
    {
        bounds.minX = bounds.maxX = point.x;
        bounds.minY = bounds.maxY = point.y;
        bounds.minZ = bounds.maxZ = point.z;
        bounds.valid = true;
        return;
    }
    bounds.minX = (std::min)(bounds.minX, point.x);
    bounds.minY = (std::min)(bounds.minY, point.y);
    bounds.minZ = (std::min)(bounds.minZ, point.z);
    bounds.maxX = (std::max)(bounds.maxX, point.x);
    bounds.maxY = (std::max)(bounds.maxY, point.y);
    bounds.maxZ = (std::max)(bounds.maxZ, point.z);
}

bool NearlyEqual(float left, float right)
{
    const float scale = (std::max)(1.0f,
        (std::max)(std::fabs(left), std::fabs(right)));
    return std::fabs(left - right) <= scale * 0.001f;
}

bool BoundsMatch(const EditorRenderBounds& left,
    const EditorRenderBounds& right)
{
    return left.valid && right.valid &&
        NearlyEqual(left.minX, right.minX) &&
        NearlyEqual(left.minY, right.minY) &&
        NearlyEqual(left.minZ, right.minZ) &&
        NearlyEqual(left.maxX, right.maxX) &&
        NearlyEqual(left.maxY, right.maxY) &&
        NearlyEqual(left.maxZ, right.maxZ);
}

bool IsZeroArea(const EditorGeometryPosition& a,
    const EditorGeometryPosition& b, const EditorGeometryPosition& c)
{
    const double abx = static_cast<double>(b.x) - a.x;
    const double aby = static_cast<double>(b.y) - a.y;
    const double abz = static_cast<double>(b.z) - a.z;
    const double acx = static_cast<double>(c.x) - a.x;
    const double acy = static_cast<double>(c.y) - a.y;
    const double acz = static_cast<double>(c.z) - a.z;
    const double cx = aby * acz - abz * acy;
    const double cy = abz * acx - abx * acz;
    const double cz = abx * acy - aby * acx;
    return cx * cx + cy * cy + cz * cz <= 1.0e-20;
}

EditorStaticGeometryDecodeStatus Fail(
    EditorStaticGeometryDecodeStatus status, const char* message,
    std::string* reason)
{
    if (reason) *reason = message;
    return status;
}
}

EditorStaticGeometryDecodeStatus EditorStaticMeshDecoder::Decode(
    const std::filesystem::path& libraryRoot,
    const EditorRenderObjectAsset& asset,
    EditorStaticAssetGeometry& geometry, std::string* reason) const
{
    if (asset.objectKind != EditorObjectKind::Static ||
        asset.readiness != EditorRenderAssetReadiness::StaticGeometryDecodeCandidate)
        return Fail(EditorStaticGeometryDecodeStatus::Unsupported,
            "Asset is not a supported static geometry decode candidate.", reason);

    std::error_code error;
    const auto root = std::filesystem::weakly_canonical(libraryRoot, error);
    if (error || !std::filesystem::is_directory(root, error))
        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
            "Object Library root is unavailable.", reason);
    const auto source = std::filesystem::weakly_canonical(
        root / std::filesystem::path(asset.sourceRelativeFile), error);
    if (error || !IsInside(root, source) ||
        !std::filesystem::is_regular_file(source, error))
        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
            "Geometry source escapes or is absent from the Object Library.", reason);
    const auto fileSize = std::filesystem::file_size(source, error);
    if (error || fileSize > limits_.maximumFileSize)
        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
            "Geometry source exceeds the configured file-size limit.", reason);

    std::size_t expectedBytes = 0;
    if (!AddSize(expectedBytes, asset.totalVertices,
            sizeof(EditorGeometryPosition) + sizeof(EditorGeometryNormal),
            limits_.maximumDecodedBytes) ||
        !AddSize(expectedBytes, asset.totalTriangles,
            sizeof(EditorGeometryTriangle), limits_.maximumDecodedBytes))
        return Fail(EditorStaticGeometryDecodeStatus::Unsupported,
            "Decoded geometry would exceed the per-asset memory limit.", reason);

    Reader reader(source, fileSize);
    Chunk outer;
    if (!reader.Good() || !ReadChunk(reader, 0, fileSize, outer) ||
        outer.id != ObjectBody)
        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
            "Object body is missing or truncated.", reason);

    Chunk meshesChunk;
    bool foundMeshes = false;
    std::uint64_t objectCursor = outer.begin;
    const std::uint64_t objectEnd = outer.begin + outer.size;
    while (objectCursor < objectEnd)
    {
        Chunk chunk;
        if (!ReadChunk(reader, objectCursor, objectEnd, chunk))
            return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                "Object chunk table is truncated.", reason);
        if (chunk.id == ObjectMeshes)
        {
            meshesChunk = chunk;
            foundMeshes = true;
            break;
        }
        objectCursor = chunk.begin + chunk.size;
    }
    if (!foundMeshes)
        return Fail(EditorStaticGeometryDecodeStatus::Unsupported,
            "Static object has no mesh container.", reason);

    EditorStaticAssetGeometry candidate;
    candidate.assetId = asset.assetId;
    candidate.sourceRelativeFile = asset.sourceRelativeFile;
    std::uint64_t meshCursor = meshesChunk.begin;
    const std::uint64_t meshEnd = meshesChunk.begin + meshesChunk.size;
    std::size_t meshIndex = 0;
    while (meshCursor < meshEnd)
    {
        if (meshIndex >= limits_.maximumMeshes)
            return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                "Mesh count exceeds the configured limit.", reason);
        Chunk wrapper;
        if (!ReadChunk(reader, meshCursor, meshEnd, wrapper))
            return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                "Mesh wrapper is truncated.", reason);

        EditorStaticMeshGeometry mesh;
        mesh.meshId = "mesh:" + std::to_string(meshIndex);
        std::uint16_t version = 0;
        bool haveVersion = false;
        bool haveVertices = false;
        bool haveFaces = false;
        EditorRenderBounds serializedBounds;
        std::uint64_t cursor = wrapper.begin;
        const std::uint64_t end = wrapper.begin + wrapper.size;
        while (cursor < end)
        {
            Chunk chunk;
            if (!ReadChunk(reader, cursor, end, chunk))
                return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                    "Mesh chunk is truncated.", reason);
            const std::uint64_t chunkEnd = chunk.begin + chunk.size;
            if (chunk.id == MeshVersion)
                haveVersion = chunk.size >= 2 && reader.U16(chunk.begin, version);
            else if (chunk.id == MeshName)
            {
                std::uint64_t stringCursor = chunk.begin;
                if (!reader.CString(stringCursor, chunkEnd, mesh.name))
                    return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                        "Mesh name is malformed.", reason);
            }
            else if (chunk.id == MeshBounds)
            {
                serializedBounds.valid = chunk.size == 24 &&
                    reader.Float(chunk.begin, serializedBounds.minX) &&
                    reader.Float(chunk.begin + 4, serializedBounds.minY) &&
                    reader.Float(chunk.begin + 8, serializedBounds.minZ) &&
                    reader.Float(chunk.begin + 12, serializedBounds.maxX) &&
                    reader.Float(chunk.begin + 16, serializedBounds.maxY) &&
                    reader.Float(chunk.begin + 20, serializedBounds.maxZ);
            }
            else if (chunk.id == MeshVertices)
            {
                std::uint32_t count = 0;
                if (chunk.size < 4 || !reader.U32(chunk.begin, count) ||
                    count == 0 || count > limits_.maximumVerticesPerMesh)
                    return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                        "Vertex count is empty, absent, or excessive.", reason);
                mesh.buffer.positions.reserve(count);
                std::uint64_t positionCursor = chunk.begin + 4;
                const std::uint64_t positionBytes =
                    static_cast<std::uint64_t>(count) * 12;
                if (positionBytes > chunkEnd - positionCursor)
                    return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                        "Vertex position array is truncated.", reason);
                for (std::uint32_t i = 0; i < count; ++i)
                {
                    EditorGeometryPosition point;
                    if (!reader.Float(positionCursor, point.x) ||
                        !reader.Float(positionCursor + 4, point.y) ||
                        !reader.Float(positionCursor + 8, point.z) ||
                        !std::isfinite(point.x) || !std::isfinite(point.y) ||
                        !std::isfinite(point.z))
                        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                            "Vertex position is truncated or non-finite.", reason);
                    mesh.buffer.positions.push_back(point);
                    Include(mesh.bounds, point);
                    positionCursor += 12;
                }
                for (std::uint32_t i = 0; i < count; ++i)
                {
                    std::uint8_t adjacency = 0;
                    if (!reader.U8(positionCursor, adjacency))
                        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                            "Vertex adjacency list is truncated.", reason);
                    ++positionCursor;
                    const std::uint64_t adjacencyBytes =
                        static_cast<std::uint64_t>(adjacency) * 4;
                    if (adjacencyBytes > chunkEnd - positionCursor)
                        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                            "Vertex adjacency list exceeds its chunk.", reason);
                    positionCursor += adjacencyBytes;
                }
                if (positionCursor != chunkEnd)
                    return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                        "Vertex chunk has unexplained trailing bytes.", reason);
                haveVertices = true;
            }
            else if (chunk.id == MeshFaces)
            {
                std::uint32_t count = 0;
                if (chunk.size < 4 || !reader.U32(chunk.begin, count) ||
                    count == 0 || count > limits_.maximumTrianglesPerMesh ||
                    chunk.size - 4 != static_cast<std::uint64_t>(count) * 24)
                    return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                        "Triangle count or face layout is invalid.", reason);
                mesh.buffer.triangles.reserve(count);
                std::uint64_t faceCursor = chunk.begin + 4;
                for (std::uint32_t i = 0; i < count; ++i)
                {
                    std::int32_t indices[3]{};
                    if (!reader.S32(faceCursor, indices[0]) ||
                        !reader.S32(faceCursor + 8, indices[1]) ||
                        !reader.S32(faceCursor + 16, indices[2]))
                        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                            "Face record is truncated.", reason);
                    for (std::int32_t index : indices)
                        if (index < 0 || static_cast<std::size_t>(index) >=
                                mesh.buffer.positions.size())
                            return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                                "Face contains an out-of-range vertex index.", reason);
                    EditorGeometryTriangle triangle{
                        static_cast<std::uint32_t>(indices[0]),
                        static_cast<std::uint32_t>(indices[1]),
                        static_cast<std::uint32_t>(indices[2])};
                    if (triangle.a == triangle.b || triangle.b == triangle.c ||
                        triangle.c == triangle.a)
                        ++mesh.degenerateTriangles;
                    if (IsZeroArea(mesh.buffer.positions[triangle.a],
                            mesh.buffer.positions[triangle.b],
                            mesh.buffer.positions[triangle.c]))
                        ++mesh.zeroAreaTriangles;
                    mesh.buffer.triangles.push_back(triangle);
                    faceCursor += 24;
                }
                haveFaces = true;
            }
            cursor = chunkEnd;
        }

        if (!haveVersion || version != SupportedMeshVersion)
            return Fail(EditorStaticGeometryDecodeStatus::Unsupported,
                "Mesh version is absent or unsupported.", reason);
        if (!haveVertices || !haveFaces)
            return Fail(EditorStaticGeometryDecodeStatus::Malformed,
                "Required static vertex or face chunk is absent.", reason);
        mesh.boundsMismatch = serializedBounds.valid &&
            !BoundsMatch(serializedBounds, mesh.bounds);
        std::size_t ignoredZeroArea = 0;
        if (GenerateEditorGeometryNormals(mesh.buffer, &ignoredZeroArea))
            ++candidate.normalsGeneratedMeshes;
        else
        {
            ++candidate.normalGenerationFailures;
            candidate.diagnostics.push_back(
                "Normal generation failed for " + mesh.meshId + ".");
        }
        candidate.totalVertices += mesh.buffer.positions.size();
        candidate.totalTriangles += mesh.buffer.triangles.size();
        candidate.degenerateTriangles += mesh.degenerateTriangles;
        candidate.zeroAreaTriangles += mesh.zeroAreaTriangles;
        candidate.boundsMismatches += mesh.boundsMismatch ? 1 : 0;
        candidate.meshes.push_back(std::move(mesh));
        ++meshIndex;
        meshCursor = wrapper.begin + wrapper.size;
    }

    if (candidate.meshes.size() != asset.meshCount ||
        candidate.totalVertices != asset.totalVertices ||
        candidate.totalTriangles != asset.totalTriangles)
        return Fail(EditorStaticGeometryDecodeStatus::Malformed,
            "Decoded geometry does not match inventoried mesh counts.", reason);
    candidate.status = EditorStaticGeometryDecodeStatus::Decoded;
    if (candidate.boundsMismatches != 0)
        candidate.diagnostics.push_back(
            "Computed position bounds differ from serialized mesh bounds.");
    geometry = std::move(candidate);
    if (reason) reason->clear();
    return EditorStaticGeometryDecodeStatus::Decoded;
}
