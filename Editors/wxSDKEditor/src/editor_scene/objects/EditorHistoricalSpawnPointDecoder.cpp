#include "editor_scene/objects/EditorHistoricalSpawnPointDecoder.h"

#include "editor_scene/EditorBinaryReader.h"
#include "editor_scene/EditorHistoricalObjectBodyDecoder.h"

#include <cmath>
#include <set>
#include <utility>

namespace
{
constexpr std::uint32_t CompressMark = 0x80000000u;
constexpr std::uint32_t VersionChunk = 0xe411u;
constexpr std::uint32_t RespawnPointChunk = 0xe413u;
constexpr std::uint32_t TypeChunk = 0xe417u;
constexpr std::uint32_t FlagsChunk = 0xe418u;
constexpr std::uint32_t EntityReferenceChunk = 0xe419u;
constexpr std::uint32_t RuntimePacketChunk = 0xe420u;
constexpr std::uint32_t AttachedObjectChunk = 0xe421u;
constexpr std::uint32_t EnvironmentModifierChunk = 0xe422u;
constexpr std::uint32_t TransformChunk = 0xf903u;
constexpr std::uint32_t MotionChunk = 0xf905u;
constexpr std::uint32_t CustomFlagsChunk = 0xf906u;
constexpr std::uint32_t NameChunk = 0xf907u;
constexpr std::uint32_t MotionParamsChunk = 0xf908u;
constexpr std::uint16_t Version14 = 0x0014u;
constexpr std::uint32_t RespawnPointType = 0u;
constexpr std::uint32_t EnvironmentModifierType = 1u;
constexpr std::uint32_t RuntimeEntityType = 2u;

struct SpawnChunk
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

EditorHistoricalFieldProvenance Provenance(const SpawnChunk& chunk,
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

EditorHistoricalBodyChunkRecord Retained(const SpawnChunk& chunk,
    const std::string& bodyPath)
{
    EditorHistoricalBodyChunkRecord result;
    result.id = chunk.id;
    result.headerOffset = chunk.headerOffset;
    result.dataOffset = chunk.dataOffset;
    result.size = chunk.size;
    result.path = ChildPath(bodyPath, chunk.id);
    return result;
}

void Malformed(EditorHistoricalObjectBodyDecodeResult& result,
    std::string diagnostic)
{
    result.status = EditorHistoricalObjectDecodeStatus::Malformed;
    result.diagnostics.push_back(std::move(diagnostic));
}

bool ReadChunks(const std::vector<std::uint8_t>& bytes,
    const EditorHistoricalObjectBodyDecodeLimits& limits,
    std::vector<SpawnChunk>& chunks, std::string& diagnostic)
{
    EditorBinaryReader reader(bytes.data(), bytes.size());
    while (!reader.Empty())
    {
        if (chunks.size() >= limits.maximumChunks)
        {
            diagnostic = "spawn-point child chunk limit exceeded";
            return false;
        }
        if (reader.Remaining() < 8)
        {
            diagnostic = "truncated spawn-point child chunk header";
            return false;
        }
        SpawnChunk chunk;
        std::uint32_t size = 0;
        chunk.headerOffset = reader.Position();
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
            diagnostic = "spawn-point child exceeds object-body bounds";
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

bool RetainUnsupported(EditorHistoricalObjectBodyDecodeResult& result,
    const SpawnChunk& chunk, const std::string& bodyPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits)
{
    if (result.unsupportedChunks.size() >=
        limits.maximumRetainedUnknownChunks)
        return false;
    result.unsupportedChunks.push_back(Retained(chunk, bodyPath));
    return true;
}

bool ReadFiniteFloat(EditorBinaryReader& reader, float& value)
{
    return reader.ReadFloat(value) && std::isfinite(value);
}
}

EditorHistoricalObjectBodyDecodeResult DecodeHistoricalSpawnPointBody(
    const std::vector<std::uint8_t>& bytes, std::size_t sourceBase,
    const std::string& bodyPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits)
{
    EditorHistoricalObjectBodyDecodeResult result;
    result.typeName = "Spawn Point";
    result.hasSpawnPoint = true;
    if (bytes.size() > limits.maximumBodySize)
    {
        Malformed(result, "spawn-point body exceeds decoder limit");
        return result;
    }

    std::vector<SpawnChunk> chunks;
    std::string diagnostic;
    if (!ReadChunks(bytes, limits, chunks, diagnostic))
    {
        Malformed(result, std::move(diagnostic));
        return result;
    }

    std::set<std::uint32_t> specialized;
    const SpawnChunk* version = nullptr;
    const SpawnChunk* type = nullptr;
    const SpawnChunk* flags = nullptr;
    const SpawnChunk* entityReference = nullptr;
    const SpawnChunk* runtimePacket = nullptr;
    const SpawnChunk* attachedObject = nullptr;
    const SpawnChunk* respawnPoint = nullptr;
    const SpawnChunk* environmentModifier = nullptr;
    bool partial = false;
    for (const SpawnChunk& chunk : chunks)
    {
        if ((chunk.rawId & CompressMark) != 0)
        {
            Malformed(result, "compressed spawn-point child chunk is unsupported");
            return result;
        }
        const bool known = chunk.id == VersionChunk ||
            chunk.id == RespawnPointChunk || chunk.id == TypeChunk ||
            chunk.id == FlagsChunk || chunk.id == EntityReferenceChunk ||
            chunk.id == RuntimePacketChunk || chunk.id == AttachedObjectChunk ||
            chunk.id == EnvironmentModifierChunk;
        if (known && !specialized.insert(chunk.id).second)
        {
            Malformed(result, "duplicate spawn-point specialized chunk");
            return result;
        }
        if (chunk.id == VersionChunk) version = &chunk;
        else if (chunk.id == TypeChunk) type = &chunk;
        else if (chunk.id == FlagsChunk) flags = &chunk;
        else if (chunk.id == EntityReferenceChunk) entityReference = &chunk;
        else if (chunk.id == RuntimePacketChunk) runtimePacket = &chunk;
        else if (chunk.id == AttachedObjectChunk) attachedObject = &chunk;
        else if (chunk.id == RespawnPointChunk) respawnPoint = &chunk;
        else if (chunk.id == EnvironmentModifierChunk)
            environmentModifier = &chunk;
        else if (chunk.id == MotionChunk || chunk.id == MotionParamsChunk)
        {
            if (!RetainUnsupported(result, chunk, bodyPath, limits))
            {
                Malformed(result,
                    "retained unsupported spawn-point chunk limit exceeded");
                return result;
            }
            partial = true;
        }
        else if (chunk.id != TransformChunk && chunk.id != CustomFlagsChunk &&
            chunk.id != NameChunk)
        {
            if (result.unknownChunks.size() >=
                limits.maximumRetainedUnknownChunks)
            {
                Malformed(result,
                    "retained unknown spawn-point chunk limit exceeded");
                return result;
            }
            result.unknownChunks.push_back(Retained(chunk, bodyPath));
            partial = true;
        }
    }

    if (!version)
    {
        Malformed(result, "required spawn-point version chunk is missing");
        return result;
    }
    if (version->size != 2)
    {
        Malformed(result, "spawn-point version chunk is not a u16");
        return result;
    }
    EditorBinaryReader versionReader = version->payload;
    if (!versionReader.ReadU16(result.spawnPoint.version) ||
        !versionReader.Empty())
    {
        Malformed(result, "spawn-point version chunk has invalid payload");
        return result;
    }
    result.hasBodyVersion = true;
    result.bodyVersion = result.spawnPoint.version;
    result.spawnPoint.versionProvenance =
        Provenance(*version, sourceBase, bodyPath);
    if (result.spawnPoint.version != Version14)
    {
        Malformed(result, "unsupported spawn-point version");
        return result;
    }

    if (flags)
    {
        EditorBinaryReader reader = flags->payload;
        if (flags->size != 4 || !reader.ReadU32(result.spawnPoint.flags) ||
            !reader.Empty())
        {
            Malformed(result, "spawn-point flags chunk is not a u32");
            return result;
        }
        result.spawnPoint.hasFlags = true;
        result.spawnPoint.flagsProvenance =
            Provenance(*flags, sourceBase, bodyPath);
    }

    if (entityReference)
    {
        if (!runtimePacket || type || respawnPoint || environmentModifier)
        {
            Malformed(result, "spawn-point runtime entity layout is incomplete or conflicting");
            return result;
        }
        EditorBinaryReader referenceReader = entityReference->payload;
        if (!referenceReader.ReadCString(result.spawnPoint.entityReference,
            limits.maximumStringLength) || !referenceReader.Empty() ||
            result.spawnPoint.entityReference.empty())
        {
            Malformed(result, referenceReader.Error().empty()
                ? "spawn-point entity reference is empty or has trailing bytes"
                : referenceReader.Error());
            return result;
        }
        result.spawnPoint.type = RuntimeEntityType;
        result.spawnPoint.hasEntityReference = true;
        result.spawnPoint.entityReferenceProvenance =
            Provenance(*entityReference, sourceBase, bodyPath);

        EditorBinaryReader packetReader = runtimePacket->payload;
        std::uint32_t packetSize = 0;
        if (runtimePacket->size < 4 || !packetReader.ReadU32(packetSize) ||
            packetSize != packetReader.Remaining() ||
            packetSize > limits.maximumSpawnOpaquePayload)
        {
            Malformed(result,
                "spawn-point runtime packet envelope is invalid or exceeds limits");
            return result;
        }
        result.spawnPoint.hasRuntimePacket = true;
        result.spawnPoint.runtimePacketSize = packetSize;
        result.spawnPoint.runtimePacketProvenance =
            Provenance(*runtimePacket, sourceBase, bodyPath);
        if (!RetainUnsupported(result, *runtimePacket, bodyPath, limits))
        {
            Malformed(result,
                "retained unsupported spawn-point chunk limit exceeded");
            return result;
        }
        partial = true;
    }
    else
    {
        if (runtimePacket || !type)
        {
            Malformed(result,
                "spawn-point legacy layout is missing type or has orphan packet");
            return result;
        }
        EditorBinaryReader typeReader = type->payload;
        if (type->size != 4 || !typeReader.ReadU32(result.spawnPoint.type) ||
            !typeReader.Empty() || result.spawnPoint.type > RuntimeEntityType)
        {
            Malformed(result, "spawn-point type is invalid");
            return result;
        }
        result.spawnPoint.typeProvenance =
            Provenance(*type, sourceBase, bodyPath);
        if (result.spawnPoint.type == RuntimeEntityType)
        {
            Malformed(result, "runtime spawn-point type has no entity reference");
            return result;
        }
        if (result.spawnPoint.type == RespawnPointType)
        {
            if (!respawnPoint || environmentModifier || respawnPoint->size != 4)
            {
                Malformed(result, "respawn-point subtype payload is missing or conflicting");
                return result;
            }
            EditorBinaryReader reader = respawnPoint->payload;
            if (!reader.ReadBytes(&result.spawnPoint.respawnTeam, 1) ||
                !reader.ReadBytes(&result.spawnPoint.respawnType, 1) ||
                !reader.ReadU16(result.spawnPoint.respawnReserved) ||
                !reader.Empty())
            {
                Malformed(result, "respawn-point subtype payload is invalid");
                return result;
            }
            result.spawnPoint.hasRespawnPoint = true;
            result.spawnPoint.subtypeDataProvenance =
                Provenance(*respawnPoint, sourceBase, bodyPath);
        }
        else
        {
            if (!environmentModifier || respawnPoint ||
                environmentModifier->size != 28)
            {
                Malformed(result,
                    "environment-modifier payload is missing or conflicting");
                return result;
            }
            EditorBinaryReader reader = environmentModifier->payload;
            if (!ReadFiniteFloat(reader, result.spawnPoint.environmentRadius) ||
                !ReadFiniteFloat(reader, result.spawnPoint.environmentPower) ||
                !ReadFiniteFloat(reader,
                    result.spawnPoint.environmentViewDistance) ||
                !reader.ReadU32(result.spawnPoint.environmentFogColor) ||
                !ReadFiniteFloat(reader,
                    result.spawnPoint.environmentFogDensity) ||
                !reader.ReadU32(result.spawnPoint.environmentAmbientColor) ||
                !reader.ReadU32(result.spawnPoint.environmentLightMapColor) ||
                !reader.Empty())
            {
                Malformed(result,
                    "environment-modifier payload is truncated or non-finite");
                return result;
            }
            result.spawnPoint.hasEnvironmentModifier = true;
            result.spawnPoint.subtypeDataProvenance =
                Provenance(*environmentModifier, sourceBase, bodyPath);
        }
    }

    if (attachedObject)
    {
        if (attachedObject->size > limits.maximumSpawnOpaquePayload ||
            !RetainUnsupported(result, *attachedObject, bodyPath, limits))
        {
            Malformed(result,
                "spawn-point attached object exceeds retention limits");
            return result;
        }
        result.spawnPoint.hasAttachedObject = true;
        result.spawnPoint.attachedObjectSize = attachedObject->size;
        result.spawnPoint.attachedObjectProvenance =
            Provenance(*attachedObject, sourceBase, bodyPath);
        partial = true;
    }

    if (!result.unsupportedChunks.empty())
        result.diagnostics.push_back(
            "runtime packet, attachment, or motion data remains opaque and inert");
    if (!result.unknownChunks.empty())
        result.diagnostics.push_back(
            "unknown spawn-point child chunks are retained but not decoded");
    result.status = partial ? EditorHistoricalObjectDecodeStatus::Partial
                            : EditorHistoricalObjectDecodeStatus::Supported;
    return result;
}
