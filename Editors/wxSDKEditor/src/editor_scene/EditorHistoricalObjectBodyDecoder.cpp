#include "editor_scene/EditorHistoricalObjectBodyDecoder.h"

#include "editor_scene/EditorBinaryReader.h"
#include "editor_scene/objects/EditorHistoricalGlowDecoder.h"
#include "editor_scene/objects/EditorHistoricalLightDecoder.h"
#include "editor_scene/objects/EditorHistoricalSpawnPointDecoder.h"

#include <set>
#include <cstring>
#include <utility>

namespace
{
constexpr std::uint32_t CompressMark = 0x80000000u;
constexpr std::uint32_t SceneObjectClass = 2u;
constexpr std::uint32_t GlowClass = 1u;
constexpr std::uint32_t LightClass = 3u;
constexpr std::uint32_t SpawnPointClass = 6u;
constexpr std::uint32_t SceneObjectVersionChunk = 0x0900u;
constexpr std::uint32_t SceneObjectReferenceChunk = 0x0902u;
constexpr std::uint32_t SceneObjectPlacementChunk = 0x0904u;
constexpr std::uint32_t SceneObjectFlagsChunk = 0x0905u;
constexpr std::uint32_t CustomObjectTransformChunk = 0xf903u;
constexpr std::uint32_t CustomObjectMotionChunk = 0xf905u;
constexpr std::uint32_t CustomObjectFlagsChunk = 0xf906u;
constexpr std::uint32_t CustomObjectNameChunk = 0xf907u;
constexpr std::uint32_t CustomObjectMotionParamChunk = 0xf908u;
constexpr std::uint16_t SceneObjectVersion10 = 0x0010u;
constexpr std::uint16_t SceneObjectVersion11 = 0x0011u;

struct BodyChunk
{
    std::uint32_t id = 0;
    std::uint32_t rawId = 0;
    std::size_t headerOffset = 0;
    std::size_t dataOffset = 0;
    std::size_t size = 0;
    EditorBinaryReader payload;
};

std::string ChildPath(const std::string& parent, std::uint32_t id)
{
    static const char* digits = "0123456789ABCDEF";
    std::string value = "0x00000000";
    for (int index = 0; index != 8; ++index)
        value[9 - index] = digits[(id >> (index * 4)) & 0xfu];
    return parent.empty() ? value : parent + "/" + value;
}

EditorHistoricalBodyChunkRecord Retained(const BodyChunk& chunk,
    std::size_t sourceBase, const std::string& bodyPath)
{
    EditorHistoricalBodyChunkRecord result;
    result.id = chunk.id;
    result.headerOffset = chunk.headerOffset;
    result.dataOffset = chunk.dataOffset;
    result.size = chunk.size;
    result.path = ChildPath(bodyPath, chunk.id);
    (void)sourceBase;
    return result;
}

EditorHistoricalFieldProvenance Provenance(const BodyChunk& chunk,
    std::size_t sourceBase, const std::string& bodyPath)
{
    EditorHistoricalFieldProvenance result;
    result.chunkId = chunk.id;
    result.bodyOffset = chunk.dataOffset;
    result.sourceOffset = sourceBase + chunk.dataOffset;
    result.size = chunk.size;
    result.chunkPath = ChildPath(bodyPath, chunk.id);
    return result;
}

bool ReadChunks(const std::vector<std::uint8_t>& bytes,
    const EditorHistoricalObjectBodyDecodeLimits& limits,
    std::vector<BodyChunk>& chunks, std::string& diagnostic)
{
    EditorBinaryReader reader(bytes.data(), bytes.size());
    while (!reader.Empty())
    {
        if (chunks.size() >= limits.maximumChunks)
        {
            diagnostic = "object body chunk limit exceeded";
            return false;
        }
        if (reader.Remaining() < 8)
        {
            diagnostic = "truncated object body chunk header";
            return false;
        }
        BodyChunk chunk;
        chunk.headerOffset = reader.Position();
        std::uint32_t size = 0;
        if (!reader.ReadU32(chunk.rawId) || !reader.ReadU32(size))
        {
            diagnostic = reader.Error();
            return false;
        }
        chunk.id = chunk.rawId & ~CompressMark;
        chunk.dataOffset = reader.Position();
        chunk.size = size;
        if (chunk.size > reader.Remaining())
        {
            diagnostic = "object body child exceeds body bounds";
            return false;
        }
        if (!reader.Slice(chunk.size, chunk.payload))
        {
            diagnostic = reader.Error();
            return false;
        }
        chunks.push_back(chunk);
    }
    return true;
}

bool ReadSigned32(EditorBinaryReader& reader, std::int32_t& value)
{
    std::uint32_t bits = 0;
    if (!reader.ReadU32(bits))
        return false;
    std::memcpy(&value, &bits, sizeof(value));
    return true;
}

EditorHistoricalObjectBodyDecodeResult DecodeSceneObject(
    const std::vector<std::uint8_t>& bytes, std::size_t sourceBase,
    const std::string& bodyPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits)
{
    EditorHistoricalObjectBodyDecodeResult result;
    result.typeName = "Scene Object";
    result.hasSceneObject = true;
    if (bytes.size() > limits.maximumBodySize)
    {
        result.status = EditorHistoricalObjectDecodeStatus::Malformed;
        result.diagnostics.push_back("scene-object body exceeds decoder limit");
        return result;
    }

    std::vector<BodyChunk> chunks;
    std::string chunkDiagnostic;
    if (!ReadChunks(bytes, limits, chunks, chunkDiagnostic))
    {
        result.status = EditorHistoricalObjectDecodeStatus::Malformed;
        result.diagnostics.push_back(std::move(chunkDiagnostic));
        return result;
    }

    bool versionSeen = false;
    bool referenceSeen = false;
    bool flagsSeen = false;
    bool partial = false;
    std::set<std::uint32_t> uniqueSpecialized;
    for (const BodyChunk& chunk : chunks)
    {
        if ((chunk.rawId & CompressMark) != 0)
        {
            result.status = EditorHistoricalObjectDecodeStatus::Malformed;
            result.diagnostics.push_back(
                "compressed scene-object child chunk is unsupported");
            return result;
        }
        if (chunk.id == SceneObjectVersionChunk)
        {
            if (!uniqueSpecialized.insert(chunk.id).second || chunk.size != 2)
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(
                    "scene-object version chunk is duplicate or not a u16");
                return result;
            }
            EditorBinaryReader payload = chunk.payload;
            if (!payload.ReadU16(result.sceneObject.version))
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(payload.Error());
                return result;
            }
            versionSeen = true;
            result.hasBodyVersion = true;
            result.bodyVersion = result.sceneObject.version;
            result.sceneObject.versionProvenance =
                Provenance(chunk, sourceBase, bodyPath);
        }
        else if (chunk.id == SceneObjectReferenceChunk)
        {
            if (!uniqueSpecialized.insert(chunk.id).second)
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(
                    "duplicate scene-object reference chunk");
                return result;
            }
            EditorBinaryReader payload = chunk.payload;
            if (!ReadSigned32(payload, result.sceneObject.referenceVersion) ||
                !ReadSigned32(payload, result.sceneObject.referenceReserved) ||
                !payload.ReadCString(result.sceneObject.referenceName,
                    limits.maximumStringLength) || !payload.Empty())
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(payload.Error().empty()
                    ? "scene-object reference chunk has trailing bytes"
                    : payload.Error());
                return result;
            }
            referenceSeen = true;
            result.sceneObject.referenceProvenance =
                Provenance(chunk, sourceBase, bodyPath);
        }
        else if (chunk.id == SceneObjectFlagsChunk)
        {
            if (flagsSeen || chunk.size != 4)
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(
                    "scene-object flags chunk is duplicate or not a u32");
                return result;
            }
            EditorBinaryReader payload = chunk.payload;
            if (!payload.ReadU32(result.sceneObject.flags))
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(payload.Error());
                return result;
            }
            flagsSeen = true;
            result.sceneObject.hasFlags = true;
            result.sceneObject.flagsProvenance =
                Provenance(chunk, sourceBase, bodyPath);
        }
        else if (chunk.id == CustomObjectMotionChunk ||
            chunk.id == CustomObjectMotionParamChunk ||
            chunk.id == SceneObjectPlacementChunk)
        {
            if (result.unsupportedChunks.size() >=
                limits.maximumRetainedUnknownChunks)
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(
                    "retained unsupported chunk limit exceeded");
                return result;
            }
            result.unsupportedChunks.push_back(
                Retained(chunk, sourceBase, bodyPath));
            partial = true;
        }
        else if (chunk.id != CustomObjectTransformChunk &&
            chunk.id != CustomObjectFlagsChunk &&
            chunk.id != CustomObjectNameChunk)
        {
            if (result.unknownChunks.size() >=
                limits.maximumRetainedUnknownChunks)
            {
                result.status = EditorHistoricalObjectDecodeStatus::Malformed;
                result.diagnostics.push_back(
                    "retained unknown chunk limit exceeded");
                return result;
            }
            result.unknownChunks.push_back(
                Retained(chunk, sourceBase, bodyPath));
            partial = true;
        }
    }

    if (!versionSeen || !referenceSeen)
    {
        result.status = EditorHistoricalObjectDecodeStatus::Malformed;
        result.diagnostics.push_back(!versionSeen
            ? "required scene-object version chunk is missing"
            : "required scene-object reference chunk is missing");
        return result;
    }
    if (result.sceneObject.version != SceneObjectVersion10 &&
        result.sceneObject.version != SceneObjectVersion11)
    {
        result.status = EditorHistoricalObjectDecodeStatus::Malformed;
        result.diagnostics.push_back("unsupported scene-object version");
        return result;
    }
    if (result.sceneObject.version == SceneObjectVersion10)
    {
        partial = true;
        result.diagnostics.push_back(
            "legacy version 0x0010 placement is retained but not applied");
    }
    if (!result.unsupportedChunks.empty())
        result.diagnostics.push_back(
            "historical motion or placement chunks are retained but not decoded");
    if (!result.unknownChunks.empty())
        result.diagnostics.push_back(
            "unknown scene-object child chunks are retained but not decoded");
    result.status = partial ? EditorHistoricalObjectDecodeStatus::Partial
                            : EditorHistoricalObjectDecodeStatus::Supported;
    return result;
}
}

EditorHistoricalObjectBodyDecodeResult DecodeHistoricalObjectBody(
    std::uint32_t classId, const std::vector<std::uint8_t>& bodyBytes,
    std::size_t bodyDataOffset, const std::string& bodyChunkPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits)
{
    if (classId == GlowClass)
        return DecodeHistoricalGlowBody(bodyBytes, bodyDataOffset,
            bodyChunkPath, limits);
    if (classId == SceneObjectClass)
        return DecodeSceneObject(bodyBytes, bodyDataOffset, bodyChunkPath,
            limits);
    if (classId == LightClass)
        return DecodeHistoricalLightBody(bodyBytes, bodyDataOffset,
            bodyChunkPath, limits);
    if (classId == SpawnPointClass)
        return DecodeHistoricalSpawnPointBody(bodyBytes, bodyDataOffset,
            bodyChunkPath, limits);
    EditorHistoricalObjectBodyDecodeResult result;
    result.status = EditorHistoricalObjectDecodeStatus::Unsupported;
    result.typeName = "Unknown historical class " + std::to_string(classId);
    return result;
}
