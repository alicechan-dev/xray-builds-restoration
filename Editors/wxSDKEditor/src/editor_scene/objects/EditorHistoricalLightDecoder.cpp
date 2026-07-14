#include "editor_scene/objects/EditorHistoricalLightDecoder.h"

#include "editor_scene/EditorBinaryReader.h"
#include "editor_scene/EditorHistoricalObjectBodyDecoder.h"

#include <cmath>
#include <set>
#include <utility>

namespace
{
constexpr std::uint32_t CompressMark = 0x80000000u;
constexpr std::uint32_t VersionChunk = 0xb411u;
constexpr std::uint32_t FlagsChunk = 0xb413u;
constexpr std::uint32_t BrightnessChunk = 0xb425u;
constexpr std::uint32_t D3DParamsChunk = 0xb435u;
constexpr std::uint32_t UseInD3DChunk = 0xb436u;
constexpr std::uint32_t RotateChunk = 0xb437u;
constexpr std::uint32_t AnimationChunk = 0xb438u;
constexpr std::uint32_t FalloffChunk = 0xb439u;
constexpr std::uint32_t FuzzyChunk = 0xb440u;
constexpr std::uint32_t LightControlChunk = 0xb441u;
constexpr std::uint32_t ParamsChunk = 0xb442u;
constexpr std::uint32_t TransformChunk = 0xf903u;
constexpr std::uint32_t MotionChunk = 0xf905u;
constexpr std::uint32_t CustomFlagsChunk = 0xf906u;
constexpr std::uint32_t NameChunk = 0xf907u;
constexpr std::uint32_t MotionParamsChunk = 0xf908u;
constexpr std::uint16_t Version10 = 0x0010u;
constexpr std::uint16_t Version11 = 0x0011u;
constexpr std::uint32_t PointType = 1u;
constexpr std::uint32_t SpotType = 2u;
constexpr std::uint32_t DirectionalType = 3u;

struct LightChunk
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

EditorHistoricalFieldProvenance Provenance(const LightChunk& chunk,
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

EditorHistoricalBodyChunkRecord Retained(const LightChunk& chunk,
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

bool Finite(float value)
{
    return std::isfinite(value);
}

bool ReadFiniteFloat(EditorBinaryReader& reader, float& value)
{
    return reader.ReadFloat(value) && Finite(value);
}

bool ReadChunks(const std::vector<std::uint8_t>& bytes,
    const EditorHistoricalObjectBodyDecodeLimits& limits,
    std::vector<LightChunk>& chunks, std::string& diagnostic)
{
    EditorBinaryReader reader(bytes.data(), bytes.size());
    while (!reader.Empty())
    {
        if (chunks.size() >= limits.maximumChunks)
        {
            diagnostic = "light child chunk limit exceeded";
            return false;
        }
        if (reader.Remaining() < 8)
        {
            diagnostic = "truncated light child chunk header";
            return false;
        }
        LightChunk chunk;
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
            diagnostic = "light child exceeds object-body bounds";
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

bool ReadCurrentParams(const LightChunk& chunk,
    EditorHistoricalLightRecord& light)
{
    if (chunk.size != 48)
        return false;
    EditorBinaryReader reader = chunk.payload;
    if (!reader.ReadU32(light.type))
        return false;
    for (float& component : light.color)
        if (!ReadFiniteFloat(reader, component)) return false;
    if (!ReadFiniteFloat(reader, light.brightness) ||
        !ReadFiniteFloat(reader, light.range))
        return false;
    for (float& value : light.attenuation)
        if (!ReadFiniteFloat(reader, value)) return false;
    return ReadFiniteFloat(reader, light.cone) &&
        ReadFiniteFloat(reader, light.virtualSize) && reader.Empty();
}

bool ReadLegacyParams(const LightChunk& params, const LightChunk& brightness,
    EditorHistoricalLightRecord& light)
{
    if (params.size != 104 || brightness.size != 4)
        return false;
    EditorBinaryReader reader = params.payload;
    if (!reader.ReadU32(light.type))
        return false;
    for (float& component : light.color)
        if (!ReadFiniteFloat(reader, component)) return false;
    if (!reader.Skip(32))
        return false;
    for (float& value : light.legacyPosition)
        if (!ReadFiniteFloat(reader, value)) return false;
    for (float& value : light.legacyDirection)
        if (!ReadFiniteFloat(reader, value)) return false;
    float ignoredFalloff = 0.0f;
    float ignoredTheta = 0.0f;
    if (!ReadFiniteFloat(reader, light.range) ||
        !ReadFiniteFloat(reader, ignoredFalloff))
        return false;
    for (float& value : light.attenuation)
        if (!ReadFiniteFloat(reader, value)) return false;
    if (!ReadFiniteFloat(reader, ignoredTheta) ||
        !ReadFiniteFloat(reader, light.cone) || !reader.Empty())
        return false;
    EditorBinaryReader brightnessReader = brightness.payload;
    return ReadFiniteFloat(brightnessReader, light.brightness) &&
        brightnessReader.Empty();
}

bool ValidateFuzzy(const LightChunk& chunk,
    const EditorHistoricalObjectBodyDecodeLimits& limits,
    EditorHistoricalLightRecord& light)
{
    if (chunk.size < 19)
        return false;
    EditorBinaryReader reader = chunk.payload;
    std::uint8_t shape = 0;
    if (!reader.ReadBytes(&shape, 1) || shape > 1)
        return false;
    float radius = 0.0f;
    std::array<float, 3> box{};
    if (!ReadFiniteFloat(reader, radius))
        return false;
    for (float& value : box)
        if (!ReadFiniteFloat(reader, value)) return false;
    std::uint16_t pointBits = 0;
    if (!reader.ReadU16(pointBits))
        return false;
    const std::int16_t pointCount = static_cast<std::int16_t>(pointBits);
    if (pointCount < 0 || static_cast<std::size_t>(pointCount) >
        limits.maximumLightFuzzyPoints || reader.Remaining() !=
        static_cast<std::size_t>(pointCount) * 12)
        return false;
    for (std::int16_t index = 0; index != pointCount; ++index)
    {
        for (int component = 0; component != 3; ++component)
        {
            float position = 0.0f;
            if (!ReadFiniteFloat(reader, position))
                return false;
        }
    }
    light.hasFuzzyData = true;
    light.fuzzyShape = shape;
    light.fuzzyPointCount = pointCount;
    light.fuzzyPayloadSize = chunk.size;
    return reader.Empty();
}
}

EditorHistoricalObjectBodyDecodeResult DecodeHistoricalLightBody(
    const std::vector<std::uint8_t>& bytes, std::size_t sourceBase,
    const std::string& bodyPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits)
{
    EditorHistoricalObjectBodyDecodeResult result;
    result.typeName = "Light";
    result.hasLight = true;
    if (bytes.size() > limits.maximumBodySize)
    {
        Malformed(result, "light body exceeds decoder limit");
        return result;
    }

    std::vector<LightChunk> chunks;
    std::string diagnostic;
    if (!ReadChunks(bytes, limits, chunks, diagnostic))
    {
        Malformed(result, std::move(diagnostic));
        return result;
    }

    std::set<std::uint32_t> specialized;
    const LightChunk* version = nullptr;
    const LightChunk* params = nullptr;
    const LightChunk* d3dParams = nullptr;
    const LightChunk* brightness = nullptr;
    const LightChunk* useInD3D = nullptr;
    const LightChunk* flags = nullptr;
    const LightChunk* lightControl = nullptr;
    const LightChunk* animation = nullptr;
    const LightChunk* falloff = nullptr;
    const LightChunk* fuzzy = nullptr;
    bool partial = false;
    for (const LightChunk& chunk : chunks)
    {
        if ((chunk.rawId & CompressMark) != 0)
        {
            Malformed(result, "compressed light child chunk is unsupported");
            return result;
        }
        const bool known = chunk.id == VersionChunk || chunk.id == FlagsChunk ||
            chunk.id == BrightnessChunk || chunk.id == D3DParamsChunk ||
            chunk.id == UseInD3DChunk || chunk.id == RotateChunk ||
            chunk.id == AnimationChunk || chunk.id == FalloffChunk ||
            chunk.id == FuzzyChunk || chunk.id == LightControlChunk ||
            chunk.id == ParamsChunk;
        if (known && !specialized.insert(chunk.id).second)
        {
            Malformed(result, "duplicate light specialized chunk");
            return result;
        }
        if (chunk.id == VersionChunk) version = &chunk;
        else if (chunk.id == ParamsChunk) params = &chunk;
        else if (chunk.id == D3DParamsChunk) d3dParams = &chunk;
        else if (chunk.id == BrightnessChunk) brightness = &chunk;
        else if (chunk.id == UseInD3DChunk) useInD3D = &chunk;
        else if (chunk.id == FlagsChunk) flags = &chunk;
        else if (chunk.id == LightControlChunk) lightControl = &chunk;
        else if (chunk.id == AnimationChunk) animation = &chunk;
        else if (chunk.id == FalloffChunk) falloff = &chunk;
        else if (chunk.id == FuzzyChunk) fuzzy = &chunk;
        else if (chunk.id == RotateChunk || chunk.id == MotionChunk ||
            chunk.id == MotionParamsChunk)
        {
            if (result.unsupportedChunks.size() >=
                limits.maximumRetainedUnknownChunks)
            {
                Malformed(result, "retained unsupported light chunk limit exceeded");
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
                Malformed(result, "retained unknown light chunk limit exceeded");
                return result;
            }
            result.unknownChunks.push_back(Retained(chunk, bodyPath));
            partial = true;
        }
    }

    if (!version || !useInD3D || (!params && (!d3dParams || !brightness)))
    {
        Malformed(result, !version ? "required light version chunk is missing" :
            !useInD3D ? "required light UseInD3D chunk is missing" :
                         "required light params layout is missing");
        return result;
    }
    if (version->size != 2)
    {
        Malformed(result, "light version chunk is not a u16");
        return result;
    }
    EditorBinaryReader versionReader = version->payload;
    if (!versionReader.ReadU16(result.light.version) || !versionReader.Empty())
    {
        Malformed(result, "light version chunk has invalid payload");
        return result;
    }
    result.hasBodyVersion = true;
    result.bodyVersion = result.light.version;
    result.light.versionProvenance = Provenance(*version, sourceBase, bodyPath);
    if (result.light.version != Version10 && result.light.version != Version11)
    {
        Malformed(result, "unsupported light version");
        return result;
    }

    if (params)
    {
        if (!ReadCurrentParams(*params, result.light))
        {
            Malformed(result, "light params are truncated, non-finite, or wrong-sized");
            return result;
        }
        result.light.paramsProvenance = Provenance(*params, sourceBase, bodyPath);
        if (d3dParams || brightness)
        {
            partial = true;
            result.diagnostics.push_back(
                "unused legacy light params are retained but not applied");
        }
    }
    else
    {
        if (!ReadLegacyParams(*d3dParams, *brightness, result.light))
        {
            Malformed(result, "legacy light params are truncated, non-finite, or wrong-sized");
            return result;
        }
        result.light.usesLegacyD3DParams = true;
        result.light.paramsProvenance = Provenance(*d3dParams, sourceBase, bodyPath);
        partial = true;
        result.diagnostics.push_back(
            "legacy embedded light placement is retained but not applied");
    }

    if (result.light.type != PointType && result.light.type != SpotType &&
        result.light.type != DirectionalType)
    {
        Malformed(result, "invalid historical light type");
        return result;
    }
    if (useInD3D->size != 4)
    {
        Malformed(result, "light UseInD3D chunk is not a 32-bit BOOL");
        return result;
    }
    EditorBinaryReader useReader = useInD3D->payload;
    if (!useReader.ReadU32(result.light.useInD3D) || !useReader.Empty())
    {
        Malformed(result, "light UseInD3D chunk has invalid payload");
        return result;
    }
    result.light.useInD3DProvenance =
        Provenance(*useInD3D, sourceBase, bodyPath);

    if (flags)
    {
        if (flags->size != 4)
        {
            Malformed(result, "light flags chunk is not a u32");
            return result;
        }
        EditorBinaryReader reader = flags->payload;
        if (!reader.ReadU32(result.light.flags) || !reader.Empty())
        {
            Malformed(result, "light flags chunk has invalid payload");
            return result;
        }
        result.light.hasFlags = true;
        result.light.flagsProvenance = Provenance(*flags, sourceBase, bodyPath);
    }
    if (lightControl)
    {
        if (lightControl->size != 4)
        {
            Malformed(result, "light control chunk is not a u32");
            return result;
        }
        EditorBinaryReader reader = lightControl->payload;
        if (!reader.ReadU32(result.light.lightControl) || !reader.Empty())
        {
            Malformed(result, "light control chunk has invalid payload");
            return result;
        }
        result.light.hasLightControl = true;
        result.light.lightControlProvenance =
            Provenance(*lightControl, sourceBase, bodyPath);
    }

    if (animation)
    {
        EditorBinaryReader reader = animation->payload;
        if (!reader.ReadCString(result.light.animationReference,
            limits.maximumStringLength) || !reader.Empty())
        {
            Malformed(result, reader.Error().empty()
                ? "light animation chunk has trailing bytes" : reader.Error());
            return result;
        }
        result.light.hasAnimationReference = true;
        result.light.animationProvenance =
            Provenance(*animation, sourceBase, bodyPath);
        if (result.unsupportedChunks.size() >=
            limits.maximumRetainedUnknownChunks)
        {
            Malformed(result, "retained unsupported light chunk limit exceeded");
            return result;
        }
        result.unsupportedChunks.push_back(Retained(*animation, bodyPath));
        partial = true;
    }
    if (falloff)
    {
        EditorBinaryReader reader = falloff->payload;
        if (!reader.ReadCString(result.light.falloffTexture,
            limits.maximumStringLength) || !reader.Empty())
        {
            Malformed(result, reader.Error().empty()
                ? "light falloff chunk has trailing bytes" : reader.Error());
            return result;
        }
        result.light.hasFalloffTexture = true;
        result.light.falloffProvenance = Provenance(*falloff, sourceBase, bodyPath);
    }
    if (fuzzy)
    {
        if (!ValidateFuzzy(*fuzzy, limits, result.light))
        {
            Malformed(result, "light fuzzy payload is invalid or exceeds limits");
            return result;
        }
        result.light.fuzzyProvenance = Provenance(*fuzzy, sourceBase, bodyPath);
        if (result.unsupportedChunks.size() >=
            limits.maximumRetainedUnknownChunks)
        {
            Malformed(result, "retained unsupported light chunk limit exceeded");
            return result;
        }
        result.unsupportedChunks.push_back(Retained(*fuzzy, bodyPath));
        partial = true;
    }

    if (!result.unsupportedChunks.empty())
        result.diagnostics.push_back(
            "light motion, animation, legacy, or fuzzy data remains inert");
    if (!result.unknownChunks.empty())
        result.diagnostics.push_back(
            "unknown light child chunks are retained but not decoded");
    if (result.light.type == DirectionalType)
    {
        result.status = EditorHistoricalObjectDecodeStatus::Unsupported;
        result.diagnostics.push_back(
            "historical CLight loader rejects directional lights");
        return result;
    }
    result.status = partial ? EditorHistoricalObjectDecodeStatus::Partial
                            : EditorHistoricalObjectDecodeStatus::Supported;
    return result;
}
