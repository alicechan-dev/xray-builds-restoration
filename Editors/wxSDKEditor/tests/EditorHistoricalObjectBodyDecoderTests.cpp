#include "editor_scene/EditorHistoricalObjectBodyDecoder.h"

#include <cstdint>
#include <iostream>
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
        DecodeHistoricalObjectBody(3, SceneObjectBody(), 0, "body");
    check(generic.status == EditorHistoricalObjectDecodeStatus::Unsupported &&
        !generic.hasSceneObject,
        "unknown dispatcher class remains generic");

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
