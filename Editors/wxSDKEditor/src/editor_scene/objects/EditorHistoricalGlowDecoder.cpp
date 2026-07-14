#include "editor_scene/objects/EditorHistoricalGlowDecoder.h"

#include "editor_scene/EditorBinaryReader.h"
#include "editor_scene/EditorHistoricalObjectBodyDecoder.h"

#include <cmath>
#include <set>
#include <utility>

namespace
{
constexpr std::uint32_t CompressMark = 0x80000000u;
constexpr std::uint32_t VersionChunk = 0xc411u;
constexpr std::uint32_t ParamsChunk = 0xc413u;
constexpr std::uint32_t ShaderChunk = 0xc414u;
constexpr std::uint32_t TextureChunk = 0xc415u;
constexpr std::uint32_t FlagsChunk = 0xc416u;
constexpr std::uint32_t TransformChunk = 0xf903u;
constexpr std::uint32_t MotionChunk = 0xf905u;
constexpr std::uint32_t CustomFlagsChunk = 0xf906u;
constexpr std::uint32_t NameChunk = 0xf907u;
constexpr std::uint32_t MotionParamsChunk = 0xf908u;
constexpr std::uint16_t Version11 = 0x0011u;
constexpr std::uint16_t Version12 = 0x0012u;

struct GlowChunk
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

EditorHistoricalFieldProvenance Provenance(const GlowChunk& chunk,
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

EditorHistoricalBodyChunkRecord Retained(const GlowChunk& chunk,
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

bool ReadChunks(const std::vector<std::uint8_t>& bytes,
    const EditorHistoricalObjectBodyDecodeLimits& limits,
    std::vector<GlowChunk>& chunks, std::string& diagnostic)
{
    EditorBinaryReader reader(bytes.data(), bytes.size());
    while (!reader.Empty())
    {
        if (chunks.size() >= limits.maximumChunks)
        {
            diagnostic = "glow child chunk limit exceeded";
            return false;
        }
        if (reader.Remaining() < 8)
        {
            diagnostic = "truncated glow child chunk header";
            return false;
        }
        GlowChunk chunk;
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
            diagnostic = "glow child exceeds object-body bounds";
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

void Malformed(EditorHistoricalObjectBodyDecodeResult& result,
    std::string diagnostic)
{
    result.status = EditorHistoricalObjectDecodeStatus::Malformed;
    result.diagnostics.push_back(std::move(diagnostic));
}
}

EditorHistoricalObjectBodyDecodeResult DecodeHistoricalGlowBody(
    const std::vector<std::uint8_t>& bytes, std::size_t sourceBase,
    const std::string& bodyPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits)
{
    EditorHistoricalObjectBodyDecodeResult result;
    result.typeName = "Glow";
    result.hasGlow = true;
    if (bytes.size() > limits.maximumBodySize)
    {
        Malformed(result, "glow body exceeds decoder limit");
        return result;
    }

    std::vector<GlowChunk> chunks;
    std::string diagnostic;
    if (!ReadChunks(bytes, limits, chunks, diagnostic))
    {
        Malformed(result, std::move(diagnostic));
        return result;
    }

    std::set<std::uint32_t> specialized;
    const GlowChunk* versionChunk = nullptr;
    const GlowChunk* paramsChunk = nullptr;
    const GlowChunk* shaderChunk = nullptr;
    const GlowChunk* textureChunk = nullptr;
    const GlowChunk* flagsChunk = nullptr;
    bool partial = false;
    for (const GlowChunk& chunk : chunks)
    {
        if ((chunk.rawId & CompressMark) != 0)
        {
            Malformed(result, "compressed glow child chunk is unsupported");
            return result;
        }
        const bool isSpecialized = chunk.id == VersionChunk ||
            chunk.id == ParamsChunk || chunk.id == ShaderChunk ||
            chunk.id == TextureChunk || chunk.id == FlagsChunk;
        if (isSpecialized && !specialized.insert(chunk.id).second)
        {
            Malformed(result, "duplicate glow specialized chunk");
            return result;
        }
        if (chunk.id == VersionChunk) versionChunk = &chunk;
        else if (chunk.id == ParamsChunk) paramsChunk = &chunk;
        else if (chunk.id == ShaderChunk) shaderChunk = &chunk;
        else if (chunk.id == TextureChunk) textureChunk = &chunk;
        else if (chunk.id == FlagsChunk) flagsChunk = &chunk;
        else if (chunk.id == MotionChunk || chunk.id == MotionParamsChunk)
        {
            if (result.unsupportedChunks.size() >=
                limits.maximumRetainedUnknownChunks)
            {
                Malformed(result, "retained unsupported glow chunk limit exceeded");
                return result;
            }
            result.unsupportedChunks.push_back(Retained(chunk, bodyPath));
            partial = true;
        }
        else if (chunk.id != TransformChunk && chunk.id != CustomFlagsChunk &&
            chunk.id != NameChunk)
        {
            if (result.unknownChunks.size() >=
                limits.maximumRetainedUnknownChunks)
            {
                Malformed(result, "retained unknown glow chunk limit exceeded");
                return result;
            }
            result.unknownChunks.push_back(Retained(chunk, bodyPath));
            partial = true;
        }
    }

    if (!versionChunk || !paramsChunk || !textureChunk)
    {
        Malformed(result, !versionChunk ? "required glow version chunk is missing" :
            !paramsChunk ? "required glow params chunk is missing" :
                           "required glow texture chunk is missing");
        return result;
    }
    if (versionChunk->size != 2)
    {
        Malformed(result, "glow version chunk is not a u16");
        return result;
    }
    EditorBinaryReader versionPayload = versionChunk->payload;
    if (!versionPayload.ReadU16(result.glow.version) || !versionPayload.Empty())
    {
        Malformed(result, "glow version chunk has invalid payload");
        return result;
    }
    result.hasBodyVersion = true;
    result.bodyVersion = result.glow.version;
    result.glow.versionProvenance = Provenance(*versionChunk, sourceBase, bodyPath);
    if (result.glow.version != Version11 && result.glow.version != Version12)
    {
        Malformed(result, "unsupported glow version");
        return result;
    }

    const std::size_t expectedParams = result.glow.version == Version11 ? 16u : 4u;
    if (paramsChunk->size != expectedParams)
    {
        Malformed(result, "glow params chunk size does not match version");
        return result;
    }
    EditorBinaryReader paramsPayload = paramsChunk->payload;
    if (!paramsPayload.ReadFloat(result.glow.radius) ||
        !std::isfinite(result.glow.radius))
    {
        Malformed(result, "glow radius is truncated or non-finite");
        return result;
    }
    result.glow.radiusProvenance = Provenance(*paramsChunk, sourceBase, bodyPath);
    if (result.glow.version == Version11)
    {
        result.glow.hasLegacyPosition = true;
        for (float& component : result.glow.legacyPosition)
        {
            if (!paramsPayload.ReadFloat(component) || !std::isfinite(component))
            {
                Malformed(result, "legacy glow position is truncated or non-finite");
                return result;
            }
        }
        partial = true;
        result.diagnostics.push_back(
            "legacy glow position is retained but not applied to shared transform");
    }
    if (!paramsPayload.Empty())
    {
        Malformed(result, "glow params chunk has trailing bytes");
        return result;
    }

    EditorBinaryReader texturePayload = textureChunk->payload;
    if (!texturePayload.ReadCString(result.glow.textureName,
        limits.maximumStringLength) || !texturePayload.Empty())
    {
        Malformed(result, texturePayload.Error().empty()
            ? "glow texture chunk has trailing bytes" : texturePayload.Error());
        return result;
    }
    result.glow.textureProvenance = Provenance(*textureChunk, sourceBase, bodyPath);

    if (shaderChunk)
    {
        EditorBinaryReader shaderPayload = shaderChunk->payload;
        if (!shaderPayload.ReadCString(result.glow.shaderName,
            limits.maximumStringLength) || !shaderPayload.Empty())
        {
            Malformed(result, shaderPayload.Error().empty()
                ? "glow shader chunk has trailing bytes" : shaderPayload.Error());
            return result;
        }
        result.glow.hasShader = true;
        result.glow.shaderProvenance = Provenance(*shaderChunk, sourceBase, bodyPath);
    }

    if (flagsChunk)
    {
        if (flagsChunk->size != 2)
        {
            Malformed(result, "glow flags chunk is not a u16");
            return result;
        }
        EditorBinaryReader flagsPayload = flagsChunk->payload;
        if (!flagsPayload.ReadU16(result.glow.flags) || !flagsPayload.Empty())
        {
            Malformed(result, "glow flags chunk has invalid payload");
            return result;
        }
        result.glow.hasFlags = true;
        result.glow.flagsProvenance = Provenance(*flagsChunk, sourceBase, bodyPath);
    }

    if (!result.unsupportedChunks.empty())
        result.diagnostics.push_back(
            "historical glow motion chunks are retained but not decoded");
    if (!result.unknownChunks.empty())
        result.diagnostics.push_back(
            "unknown glow child chunks are retained but not decoded");
    result.status = partial ? EditorHistoricalObjectDecodeStatus::Partial
                            : EditorHistoricalObjectDecodeStatus::Supported;
    return result;
}
