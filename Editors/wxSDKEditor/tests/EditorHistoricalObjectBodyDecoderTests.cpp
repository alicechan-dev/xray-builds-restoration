#include "editor_scene/EditorHistoricalObjectBodyDecoder.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace
{
using Bytes = std::vector<std::uint8_t>;

void U16(Bytes& bytes, std::uint16_t value)
{
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8));
}

void U32(Bytes& bytes, std::uint32_t value)
{
    for (int shift = 0; shift != 32; shift += 8)
        bytes.push_back(static_cast<std::uint8_t>(value >> shift));
}

void Float(Bytes& bytes, float value)
{
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    U32(bytes, bits);
}

void CString(Bytes& bytes, const char* value)
{
    while (*value)
        bytes.push_back(static_cast<std::uint8_t>(*value++));
    bytes.push_back(0);
}

Bytes Chunk(std::uint32_t id, const Bytes& payload)
{
    Bytes result;
    U32(result, id);
    U32(result, static_cast<std::uint32_t>(payload.size()));
    result.insert(result.end(), payload.begin(), payload.end());
    return result;
}

void Append(Bytes& destination, const Bytes& source)
{
    destination.insert(destination.end(), source.begin(), source.end());
}

Bytes SceneObjectBody(bool flags = false)
{
    Bytes body;
    Bytes version;
    U16(version, 0x0011);
    Append(body, Chunk(0x0900, version));
    Bytes reference;
    U32(reference, 42);
    U32(reference, 0);
    CString(reference, "objects\\crate");
    Append(body, Chunk(0x0902, reference));
    if (flags)
    {
        Bytes value;
        U32(value, 0x12345678);
        Append(body, Chunk(0x0905, value));
    }
    return body;
}

Bytes GlowBody(std::uint16_t version = 0x0012, bool optional = false)
{
    Bytes body;
    Bytes versionValue;
    U16(versionValue, version);
    Append(body, Chunk(0xc411, versionValue));
    Bytes params;
    Float(params, 2.5f);
    if (version == 0x0011)
    {
        Float(params, 1.0f);
        Float(params, 2.0f);
        Float(params, 3.0f);
    }
    Append(body, Chunk(0xc413, params));
    if (optional)
    {
        Bytes shader;
        CString(shader, "effects\\glow");
        Append(body, Chunk(0xc414, shader));
    }
    Bytes texture;
    CString(texture, "glow\\lamp");
    Append(body, Chunk(0xc415, texture));
    if (optional)
    {
        Bytes flags;
        U16(flags, 1);
        Append(body, Chunk(0xc416, flags));
    }
    return body;
}

Bytes LightBody(std::uint32_t type = 1, bool optional = false)
{
    Bytes body;
    Append(body, Chunk(0xb411, {0x11, 0x00}));
    Bytes params;
    U32(params, type);
    Float(params, 0.25f);
    Float(params, 0.5f);
    Float(params, 0.75f);
    Float(params, 0.0f);
    Float(params, 2.0f);
    Float(params, 15.0f);
    Float(params, 1.0f);
    Float(params, 0.1f);
    Float(params, 0.01f);
    Float(params, 0.392699f);
    Float(params, 0.0f);
    Append(body, Chunk(0xb442, params));
    Bytes use;
    U32(use, 1);
    Append(body, Chunk(0xb436, use));
    if (optional)
    {
        Bytes flags;
        U32(flags, 0x12345679);
        Append(body, Chunk(0xb413, flags));
        Bytes control;
        U32(control, 2);
        Append(body, Chunk(0xb441, control));
        Bytes falloff;
        CString(falloff, "lights\\falloff");
        Append(body, Chunk(0xb439, falloff));
    }
    return body;
}

Bytes FuzzyPayload(std::uint8_t shape = 0, std::int16_t pointCount = 1)
{
    Bytes fuzzy;
    fuzzy.push_back(shape);
    Float(fuzzy, 1.5f);
    Float(fuzzy, 0.1f);
    Float(fuzzy, 0.2f);
    Float(fuzzy, 0.3f);
    U16(fuzzy, static_cast<std::uint16_t>(pointCount));
    for (std::int16_t point = 0; point < pointCount; ++point)
    {
        Float(fuzzy, static_cast<float>(point));
        Float(fuzzy, 2.0f);
        Float(fuzzy, 3.0f);
    }
    return fuzzy;
}
}

int RunEditorHistoricalObjectBodyDecoderTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition)
            return;
        ++failures;
        std::cerr << "FAIL: historical object decoder " << message << '\n';
    };

    const EditorHistoricalObjectBodyDecodeResult minimum =
        DecodeHistoricalObjectBody(2, SceneObjectBody(), 1000, "body");
    check(minimum.status == EditorHistoricalObjectDecodeStatus::Supported &&
        minimum.hasSceneObject && minimum.hasBodyVersion &&
        minimum.bodyVersion == 0x0011 &&
        minimum.sceneObject.referenceVersion == 42 &&
        minimum.sceneObject.referenceReserved == 0 &&
        minimum.sceneObject.referenceName == "objects\\crate" &&
        !minimum.sceneObject.hasFlags,
        "minimum scene-object body decodes exact inert fields");
    check(minimum.sceneObject.versionProvenance.bodyOffset == 8 &&
        minimum.sceneObject.versionProvenance.sourceOffset == 1008 &&
        minimum.sceneObject.referenceProvenance.chunkPath == "body/0x00000902",
        "field provenance retains body and source offsets");

    const EditorHistoricalObjectBodyDecodeResult full =
        DecodeHistoricalObjectBody(2, SceneObjectBody(true), 0, "body");
    check(full.status == EditorHistoricalObjectDecodeStatus::Supported &&
        full.sceneObject.hasFlags && full.sceneObject.flags == 0x12345678,
        "optional scene-object flags decode exactly");

    Bytes partialBytes = SceneObjectBody();
    Append(partialBytes, Chunk(0xf905, {1, 2, 3, 4}));
    const EditorHistoricalObjectBodyDecodeResult partial =
        DecodeHistoricalObjectBody(2, partialBytes, 0, "body");
    check(partial.status == EditorHistoricalObjectDecodeStatus::Partial &&
        partial.unsupportedChunks.size() == 1 &&
        partial.unsupportedChunks[0].id == 0xf905,
        "motion data is retained and marks scene object partial");

    Bytes unknownBytes = SceneObjectBody();
    Append(unknownBytes, Chunk(0xabcd, {9}));
    const EditorHistoricalObjectBodyDecodeResult unknown =
        DecodeHistoricalObjectBody(2, unknownBytes, 0, "body");
    check(unknown.status == EditorHistoricalObjectDecodeStatus::Partial &&
        unknown.unknownChunks.size() == 1 &&
        unknown.unknownChunks[0].size == 1,
        "unknown child chunk is retained without interpretation");

    const EditorHistoricalObjectBodyDecodeResult generic =
        DecodeHistoricalObjectBody(4, SceneObjectBody(), 0, "body");
    check(generic.status == EditorHistoricalObjectDecodeStatus::Unsupported &&
        !generic.hasSceneObject,
        "unknown dispatcher class remains generic");

    const EditorHistoricalObjectBodyDecodeResult minimumLight =
        DecodeHistoricalObjectBody(3, LightBody(), 3000, "light-body");
    check(minimumLight.status == EditorHistoricalObjectDecodeStatus::Supported &&
        minimumLight.hasLight && minimumLight.hasBodyVersion &&
        minimumLight.bodyVersion == 0x0011 && minimumLight.light.type == 1 &&
        minimumLight.light.color[0] == 0.25f &&
        minimumLight.light.brightness == 2.0f &&
        minimumLight.light.range == 15.0f &&
        minimumLight.light.attenuation[2] == 0.01f &&
        minimumLight.light.useInD3D == 1 &&
        !minimumLight.light.hasFlags && !minimumLight.light.hasFuzzyData,
        "minimum light body decodes exact inert current parameters");
    check(minimumLight.light.versionProvenance.sourceOffset == 3008 &&
        minimumLight.light.paramsProvenance.chunkPath ==
            "light-body/0x0000B442" &&
        minimumLight.light.useInD3DProvenance.chunkId == 0xb436,
        "light field provenance retains offsets and chunk paths");

    const EditorHistoricalObjectBodyDecodeResult fullLight =
        DecodeHistoricalObjectBody(3, LightBody(2, true), 0, "body");
    check(fullLight.status == EditorHistoricalObjectDecodeStatus::Supported &&
        fullLight.light.type == 2 && fullLight.light.hasFlags &&
        fullLight.light.flags == 0x12345679 &&
        fullLight.light.hasLightControl &&
        fullLight.light.lightControl == 2 &&
        fullLight.light.hasFalloffTexture &&
        fullLight.light.falloffTexture == "lights\\falloff",
        "optional spot-light metadata decodes without resource resolution");

    Bytes fuzzyLight = LightBody();
    Append(fuzzyLight, Chunk(0xb440, FuzzyPayload()));
    const EditorHistoricalObjectBodyDecodeResult partialLight =
        DecodeHistoricalObjectBody(3, fuzzyLight, 0, "body");
    check(partialLight.status == EditorHistoricalObjectDecodeStatus::Partial &&
        partialLight.light.hasFuzzyData &&
        partialLight.light.fuzzyShape == 0 &&
        partialLight.light.fuzzyPointCount == 1 &&
        partialLight.unsupportedChunks.size() == 1 &&
        partialLight.unsupportedChunks[0].id == 0xb440,
        "fuzzy light placement is validated, retained, and kept inert");

    Bytes animatedLight = LightBody();
    Bytes animation;
    CString(animation, "pulse");
    Append(animatedLight, Chunk(0xb438, animation));
    const EditorHistoricalObjectBodyDecodeResult animationResult =
        DecodeHistoricalObjectBody(3, animatedLight, 0, "body");
    check(animationResult.status == EditorHistoricalObjectDecodeStatus::Partial &&
        animationResult.light.hasAnimationReference &&
        animationResult.light.animationReference == "pulse",
        "animation reference is decoded as inert metadata");

    Bytes unknownLight = LightBody();
    Append(unknownLight, Chunk(0xbeef, {7, 8}));
    check(DecodeHistoricalObjectBody(3, unknownLight, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Partial,
        "unknown light child is retained and marks decode partial");

    check(DecodeHistoricalObjectBody(3, LightBody(3), 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Unsupported,
        "directional light follows historical loader rejection");
    check(DecodeHistoricalObjectBody(3, LightBody(4), 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "unknown historical light type is malformed");

    Bytes missingLight;
    Append(missingLight, Chunk(0xb411, {0x11, 0x00}));
    check(DecodeHistoricalObjectBody(3, missingLight, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "missing required light parameter layout is malformed");
    Bytes duplicateLight = LightBody();
    Append(duplicateLight, Chunk(0xb411, {0x11, 0x00}));
    check(DecodeHistoricalObjectBody(3, duplicateLight, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "duplicate specialized light chunk is malformed");
    Bytes unsupportedLight = LightBody();
    unsupportedLight[8] = 0x12;
    check(DecodeHistoricalObjectBody(3, unsupportedLight, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "unsupported light version is malformed");
    Bytes shortParams = LightBody();
    shortParams[14] = 47;
    check(DecodeHistoricalObjectBody(3, shortParams, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "wrong-sized light params are malformed");
    Bytes nonFiniteLight = LightBody();
    const std::uint32_t quietNan = 0x7fc00000u;
    for (int index = 0; index != 4; ++index)
        nonFiniteLight[38 + index] =
            static_cast<std::uint8_t>(quietNan >> (index * 8));
    check(DecodeHistoricalObjectBody(3, nonFiniteLight, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "non-finite light scalar is malformed");
    Bytes badFuzzy = LightBody();
    Append(badFuzzy, Chunk(0xb440, FuzzyPayload(2)));
    check(DecodeHistoricalObjectBody(3, badFuzzy, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "invalid fuzzy shape is malformed");
    EditorHistoricalObjectBodyDecodeLimits fuzzyLimits;
    fuzzyLimits.maximumLightFuzzyPoints = 0;
    check(DecodeHistoricalObjectBody(3, fuzzyLight, 0, "body", fuzzyLimits).status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "fuzzy point limit is enforced");
    Bytes compressedLight = LightBody();
    compressedLight[3] |= 0x80;
    check(DecodeHistoricalObjectBody(3, compressedLight, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "compressed specialized light child is rejected");
    Bytes badLightBounds = LightBody();
    badLightBounds[4] = 0xff;
    badLightBounds[5] = 0xff;
    check(DecodeHistoricalObjectBody(3, badLightBounds, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "light child outside body bounds is malformed");

    const EditorHistoricalObjectBodyDecodeResult minimumGlow =
        DecodeHistoricalObjectBody(1, GlowBody(), 2000, "glow-body");
    check(minimumGlow.status == EditorHistoricalObjectDecodeStatus::Supported &&
        minimumGlow.hasGlow && minimumGlow.hasBodyVersion &&
        minimumGlow.bodyVersion == 0x0012 &&
        minimumGlow.glow.radius == 2.5f &&
        minimumGlow.glow.textureName == "glow\\lamp" &&
        !minimumGlow.glow.hasShader && !minimumGlow.glow.hasFlags,
        "minimum glow body decodes required inert fields");
    check(minimumGlow.glow.versionProvenance.sourceOffset == 2008 &&
        minimumGlow.glow.radiusProvenance.chunkPath ==
            "glow-body/0x0000C413" &&
        minimumGlow.glow.textureProvenance.chunkId == 0xc415,
        "glow field provenance retains offsets and chunk paths");

    const EditorHistoricalObjectBodyDecodeResult fullGlow =
        DecodeHistoricalObjectBody(1, GlowBody(0x0012, true), 0, "body");
    check(fullGlow.status == EditorHistoricalObjectDecodeStatus::Supported &&
        fullGlow.glow.hasShader &&
        fullGlow.glow.shaderName == "effects\\glow" &&
        fullGlow.glow.hasFlags && fullGlow.glow.flags == 1,
        "optional glow shader and flags decode exactly");

    const EditorHistoricalObjectBodyDecodeResult legacyGlow =
        DecodeHistoricalObjectBody(1, GlowBody(0x0011), 0, "body");
    check(legacyGlow.status == EditorHistoricalObjectDecodeStatus::Partial &&
        legacyGlow.glow.hasLegacyPosition &&
        legacyGlow.glow.legacyPosition[0] == 1.0f &&
        legacyGlow.glow.legacyPosition[2] == 3.0f,
        "legacy glow position is retained without changing shared placement");

    Bytes unknownGlow = GlowBody();
    Append(unknownGlow, Chunk(0xdead, {1, 2}));
    const EditorHistoricalObjectBodyDecodeResult partialGlow =
        DecodeHistoricalObjectBody(1, unknownGlow, 0, "body");
    check(partialGlow.status == EditorHistoricalObjectDecodeStatus::Partial &&
        partialGlow.unknownChunks.size() == 1 &&
        partialGlow.unknownChunks[0].id == 0xdead,
        "unknown glow child is retained and marks decode partial");

    Bytes missingGlow;
    Append(missingGlow, Chunk(0xc411, {0x12, 0x00}));
    Bytes missingTextureParams;
    Float(missingTextureParams, 1.0f);
    Append(missingGlow, Chunk(0xc413, missingTextureParams));
    check(DecodeHistoricalObjectBody(1, missingGlow, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "missing required glow texture is malformed");

    Bytes duplicateGlow = GlowBody();
    Append(duplicateGlow, Chunk(0xc411, {0x12, 0x00}));
    check(DecodeHistoricalObjectBody(1, duplicateGlow, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "duplicate glow specialized chunk is malformed");

    Bytes unsupportedGlow = GlowBody();
    unsupportedGlow[8] = 0x13;
    check(DecodeHistoricalObjectBody(1, unsupportedGlow, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "unsupported glow version is malformed");

    Bytes truncatedGlow = GlowBody();
    truncatedGlow[14] = 3;
    check(DecodeHistoricalObjectBody(1, truncatedGlow, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "wrong-sized glow params are malformed");

    Bytes nonFiniteGlow = GlowBody();
    const float nan = (std::numeric_limits<float>::quiet_NaN)();
    std::uint32_t nanBits = 0;
    std::memcpy(&nanBits, &nan, sizeof(nanBits));
    for (int index = 0; index != 4; ++index)
        nonFiniteGlow[18 + index] =
            static_cast<std::uint8_t>(nanBits >> (index * 8));
    check(DecodeHistoricalObjectBody(1, nonFiniteGlow, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "non-finite glow radius is malformed");

    EditorHistoricalObjectBodyDecodeLimits glowStringLimits;
    glowStringLimits.maximumStringLength = 3;
    check(DecodeHistoricalObjectBody(1, GlowBody(), 0, "body",
        glowStringLimits).status == EditorHistoricalObjectDecodeStatus::Malformed,
        "over-limit glow texture string is malformed");

    Bytes compressedGlow = GlowBody();
    compressedGlow[3] |= 0x80;
    check(DecodeHistoricalObjectBody(1, compressedGlow, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "compressed specialized glow child is rejected");

    Bytes badGlowBounds = GlowBody();
    badGlowBounds[4] = 0xff;
    badGlowBounds[5] = 0xff;
    check(DecodeHistoricalObjectBody(1, badGlowBounds, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "glow child outside body bounds is malformed");

    Bytes noVersion;
    Bytes reference;
    U32(reference, 1);
    U32(reference, 0);
    CString(reference, "object");
    Append(noVersion, Chunk(0x0902, reference));
    check(DecodeHistoricalObjectBody(2, noVersion, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "missing required version is malformed");

    Bytes duplicate = SceneObjectBody();
    Bytes duplicateVersion;
    U16(duplicateVersion, 0x0011);
    Append(duplicate, Chunk(0x0900, duplicateVersion));
    check(DecodeHistoricalObjectBody(2, duplicate, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "duplicate required version is malformed");

    Bytes unsupported = SceneObjectBody();
    unsupported[8] = 0x12;
    check(DecodeHistoricalObjectBody(2, unsupported, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "unsupported scene-object version is malformed");

    Bytes truncatedReference;
    U32(truncatedReference, 1);
    Append(truncatedReference, Chunk(0x0900, {0x11, 0x00}));
    Append(truncatedReference, Chunk(0x0902, {1, 2, 3}));
    check(DecodeHistoricalObjectBody(2, truncatedReference, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "truncated reference field is malformed");

    EditorHistoricalObjectBodyDecodeLimits stringLimits;
    stringLimits.maximumStringLength = 3;
    check(DecodeHistoricalObjectBody(2, SceneObjectBody(), 0, "body",
        stringLimits).status == EditorHistoricalObjectDecodeStatus::Malformed,
        "over-limit reference string is malformed");

    Bytes badBounds = SceneObjectBody();
    badBounds[4] = 0xff;
    badBounds[5] = 0xff;
    check(DecodeHistoricalObjectBody(2, badBounds, 0, "body").status ==
        EditorHistoricalObjectDecodeStatus::Malformed,
        "child bounds outside body are malformed");

    EditorHistoricalObjectBodyDecodeLimits bodyLimits;
    bodyLimits.maximumBodySize = 4;
    check(DecodeHistoricalObjectBody(2, SceneObjectBody(), 0, "body",
        bodyLimits).status == EditorHistoricalObjectDecodeStatus::Malformed,
        "body byte limit is enforced");

    return failures;
}
