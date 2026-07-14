#include "editor_app/EditorDocument.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_scene/EditorBinaryReader.h"
#include "editor_scene/EditorHistoricalSceneProbe.h"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
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
    for (int shift = 0; shift < 32; shift += 8)
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

Bytes ValidScene(bool includeObject = true)
{
    Bytes scene;
    Bytes version;
    U32(version, 5);
    Append(scene, Chunk(0x9df3, version));

    Bytes declaredCount;
    U32(declaredCount, includeObject ? 1u : 0u);
    Append(scene, Chunk(0x7712, declaredCount));
    if (!includeObject)
        return scene;

    Bytes name;
    CString(name, "actor_marker");
    Bytes transform;
    for (float value : {1.0f, 2.0f, 3.0f, 0.1f, 0.2f, 0.3f,
        1.0f, 1.5f, 2.0f})
        Float(transform, value);
    Bytes body;
    Append(body, Chunk(0xf907, name));
    Append(body, Chunk(0xf903, transform));
    Append(body, Chunk(0xabcd, {1, 2, 3}));

    Bytes classId;
    U32(classId, 2);
    Bytes wrapper;
    Append(wrapper, Chunk(0x7703, classId));
    Append(wrapper, Chunk(0x7777, body));
    Bytes objects;
    Append(objects, Chunk(0, wrapper));
    Bytes toolCount;
    U32(toolCount, 1);
    Bytes tool;
    Append(tool, Chunk(2, toolCount));
    Append(tool, Chunk(3, objects));
    Append(scene, Chunk(0x8002, tool));
    return scene;
}
}

int RunEditorSceneProbeTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition) return;
        ++failures;
        std::cerr << "FAIL: scene probe " << message << '\n';
    };

    Bytes primitives;
    U16(primitives, 0x1234);
    U32(primitives, 0x89abcdef);
    Float(primitives, 2.5f);
    CString(primitives, "name");
    EditorBinaryReader reader(primitives.data(), primitives.size(), 100);
    std::uint16_t u16 = 0;
    std::uint32_t u32 = 0;
    float number = 0;
    std::string text;
    check(reader.ReadU16(u16) && u16 == 0x1234 &&
        reader.ReadU32(u32) && u32 == 0x89abcdef &&
        reader.ReadFloat(number) && number == 2.5f &&
        reader.ReadCString(text, 16) && text == "name" && reader.Empty(),
        "little-endian primitives and zero-terminated string");
    check(!reader.ReadU32(u32) &&
        reader.Error().find("byte offset 115") != std::string::npos,
        "primitive bounds failure reports absolute offset");

    EditorBinaryReader whole(primitives.data(), primitives.size());
    EditorBinaryReader slice;
    check(whole.Slice(2, slice) && slice.ReadU16(u16) && u16 == 0x1234 &&
        !whole.Slice(primitives.size(), slice),
        "bounded sub-reader and oversized slice rejection");
    Bytes unterminated{'a', 'b', 'c'};
    EditorBinaryReader stringReader(unterminated.data(), unterminated.size());
    check(!stringReader.ReadCString(text, 2),
        "unterminated and over-limit string rejected");

    EditorHistoricalSceneProbe probe;
    EditorSceneManifest manifest;
    std::string reason;
    check(probe.ProbeSceneBytes(ValidScene(false), "empty.level", manifest,
        &reason) && manifest.version == 5 && manifest.objects.empty() &&
        manifest.chunks.size() == 2,
        "valid scene container with version and no objects");
    check(probe.ProbeSceneBytes(ValidScene(), "object.level", manifest,
        &reason) && manifest.format == "Build 1935 LevelEditor scene v5" &&
        manifest.declaredObjectCount == 1 && manifest.objects.size() == 1 &&
        manifest.objects[0].classId == 2 &&
        manifest.objects[0].name == "actor_marker" &&
        manifest.objects[0].hasTransform &&
        manifest.objects[0].position[0] == 1.0f &&
        manifest.objects[0].rotation[2] == 0.3f &&
        manifest.objects[0].scale[1] == 1.5f &&
        manifest.unknownChunkCount == 1,
        "confirmed class, name, transform, and unknown chunk inventory");
    check(manifest.chunks.size() == 11 &&
        manifest.chunks[0].path == "0x00009DF3" &&
        manifest.chunks.back().path.find("0x0000ABCD") != std::string::npos,
        "deterministic nested chunk paths and ordering");

    EditorSceneManifest preserved = manifest;
    Bytes malformed;
    U32(malformed, 0x9df3);
    U32(malformed, 100);
    malformed.push_back(5);
    check(!probe.ProbeSceneBytes(malformed, "bad.level", manifest, &reason) &&
        reason.find("payload exceeds parent bounds") != std::string::npos &&
        manifest.sourceFile == preserved.sourceFile,
        "malformed top-level bounds fail atomically");
    check(!probe.ProbeSceneBytes({1, 2, 3, 4}, "short.level", manifest,
        &reason) && reason.find("truncated XR chunk header") != std::string::npos,
        "truncated chunk header rejected");

    Bytes badNested = ValidScene(false);
    Bytes nested;
    U32(nested, 3);
    U32(nested, 20);
    nested.push_back(0);
    Append(badNested, Chunk(0x8002, nested));
    check(!probe.ProbeSceneBytes(badNested, "nested.level", manifest, &reason) &&
        reason.find("payload exceeds parent bounds") != std::string::npos,
        "chunk outside confirmed parent rejected");

    EditorSceneProbeLimits depthLimits;
    depthLimits.maximumNestingDepth = 2;
    check(!EditorHistoricalSceneProbe(depthLimits).ProbeSceneBytes(
        ValidScene(), "deep.level", manifest, &reason) &&
        reason.find("nesting limit") != std::string::npos,
        "nesting depth limit enforced");
    EditorSceneProbeLimits chunkLimits;
    chunkLimits.maximumChunks = 3;
    check(!EditorHistoricalSceneProbe(chunkLimits).ProbeSceneBytes(
        ValidScene(), "chunks.level", manifest, &reason) &&
        reason.find("chunk limit") != std::string::npos,
        "chunk count limit enforced");

    Bytes unsupported;
    Bytes version;
    U32(version, 4);
    Append(unsupported, Chunk(0x9df3, version));
    check(!probe.ProbeSceneBytes(unsupported, "v4.level", manifest, &reason) &&
        reason.find("unsupported historical scene version 4") !=
            std::string::npos,
        "unsupported historical generation rejected explicitly");
    check(!probe.ProbeSceneBytes(Chunk(0x1234, {}), "other.bin", manifest,
        &reason) && reason.find("version chunk") != std::string::npos,
        "unrelated chunked binary rejected by format detection");
    check(!probe.ProbeSceneBytes({}, "empty.bin", manifest, &reason),
        "empty file rejected");
    EditorSceneProbeLimits sizeLimits;
    sizeLimits.maximumFileSize = 8;
    check(!EditorHistoricalSceneProbe(sizeLimits).ProbeSceneBytes(
        ValidScene(false), "large.level", manifest, &reason),
        "file size policy enforced before parsing");

    const Bytes compressedVersionPayload{
        4, 0, 0, 0, 200, 227, 49, 48, 128};
    Bytes compressedVersion;
    Append(compressedVersion,
        Chunk(0x80009df3u, compressedVersionPayload));
    check(probe.ProbeSceneBytes(compressedVersion, "compressed.level",
        manifest, &reason) && manifest.version == 5 &&
        manifest.compressedChunkCount == 1 &&
        manifest.decompressedChunkCount == 1,
        "compressed critical u32 decoded with historical LZHUF");
    const EditorSceneManifest compressedPreserved = manifest;
    Append(compressedVersion, Chunk(0x80007712u, {4, 0, 0, 0}));
    check(!probe.ProbeSceneBytes(compressedVersion, "bad-compressed.level",
        manifest, &reason) && reason.find("readable u32") != std::string::npos &&
        manifest.sourceFile == compressedPreserved.sourceFile,
        "failed compressed critical field preserves prior manifest");

    const std::filesystem::path temporary =
        std::filesystem::temp_directory_path() / "wx_scene_probe_test.level";
    {
        const Bytes fileBytes = ValidScene(false);
        std::ofstream output(temporary, std::ios::binary);
        output.write(reinterpret_cast<const char*>(fileBytes.data()),
            static_cast<std::streamsize>(fileBytes.size()));
    }
    check(probe.ProbeSceneFile(temporary, manifest, &reason) &&
        manifest.sourceFile.find("wx_scene_probe_test.level") !=
            std::string::npos,
        "read-only temporary file probe");
    std::error_code cleanupError;
    std::filesystem::remove(temporary, cleanupError);

    EditorDocument document;
    const bool dirtyBefore = document.IsModified();
    const bool undoBefore = document.History().CanUndo();
    const EditorTreeNode* rootBefore = document.Model().Root();
    check(probe.ProbeSceneBytes(ValidScene(false), "session.level", manifest,
        &reason) && document.IsModified() == dirtyBefore &&
        document.History().CanUndo() == undoBefore &&
        document.Model().Root() == rootBefore,
        "scene inspection remains independent of document, history, and model");

    return failures;
}
