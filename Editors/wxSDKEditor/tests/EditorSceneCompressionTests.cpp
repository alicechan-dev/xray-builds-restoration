#include "editor_scene/EditorHistoricalSceneDocument.h"
#include "editor_scene/EditorHistoricalSceneProbe.h"
#include "editor_scene/EditorSceneCompression.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace
{
using Bytes = std::vector<std::uint8_t>;

// These tiny fixtures were produced with a test-only literal encoder using
// the tables and adaptive-Huffman update rules in xrCore/LzHuf.cpp. They are
// format fixtures, not bytes copied from a game scene or asset.
const Bytes CompressedAbc{
    3, 0, 0, 0, 230, 243, 185, 224};

const Bytes CompressedAbcBackReference{
    6, 0, 0, 0, 230, 243, 185, 241, 128, 32};

const Bytes CompressedTool{
    117, 0, 0, 0, 199, 99, 49, 48, 228, 48, 223, 126, 54, 251, 237,
    143, 181, 173, 237, 181, 175, 122, 170, 242, 213, 85, 152, 29, 86,
    13, 85, 98, 213, 85, 132, 202, 174, 139, 111, 39, 10, 222, 117,
    183, 239, 253, 254, 127, 159, 239, 143, 254, 54, 83, 225, 235, 178,
    153, 0, 152, 149, 113, 166, 219, 216, 54, 219, 12, 229, 182, 249,
    141, 197, 43, 172, 214, 33, 185, 48, 234, 123, 28, 162, 98, 84,
    184, 22, 227, 34, 219, 166, 33, 109, 196};

const Bytes NestedCompressedTool{
    99, 0, 0, 0, 199, 99, 49, 48, 228, 48, 223, 126, 54, 251, 237,
    143, 181, 131, 59, 118, 181, 189, 54, 181, 41, 123, 155, 84, 107,
    82, 162, 55, 173, 128, 158, 125, 111, 222, 155, 83, 73, 234, 94,
    172, 119, 39, 240, 249, 120, 158, 99, 133, 153, 224, 170, 190, 93,
    179, 42, 118, 39, 40, 207, 107, 52, 194, 225, 204, 172, 72, 95,
    63, 110, 255, 128, 159, 197, 252, 191, 247, 255, 87, 8, 188, 90,
    228, 90, 159, 231, 31, 80, 93, 92, 232, 145, 245, 245, 175, 146,
    141, 218, 66, 87, 128};

void U32(Bytes& bytes, std::uint32_t value)
{
    for (int shift = 0; shift < 32; shift += 8)
        bytes.push_back(static_cast<std::uint8_t>(value >> shift));
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

Bytes CompressedScene(const Bytes& compressedTool)
{
    Bytes scene;
    Bytes version;
    U32(version, 5);
    Append(scene, Chunk(0x9df3, version));
    Bytes count;
    U32(count, 1);
    Append(scene, Chunk(0x7712, count));
    Append(scene, Chunk(0x80008002u, compressedTool));
    return scene;
}
}

int RunEditorSceneCompressionTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition)
            return;
        ++failures;
        std::cerr << "FAIL: scene compression " << message << '\n';
    };

    std::string reason;
    std::vector<std::uint8_t> output{9, 9};
    check(DecompressHistoricalSceneChunk(Bytes{0, 0, 0, 0}, output, {},
        &reason) && output.empty(), "empty historical stream");
    check(DecompressHistoricalSceneChunk(CompressedAbc, output, {}, &reason) &&
        output == Bytes({'A', 'B', 'C'}), "small literal stream");
    check(DecompressHistoricalSceneChunk(CompressedAbcBackReference, output,
        {}, &reason) && output == Bytes({'A', 'B', 'C', 'A', 'B', 'C'}),
        "historical LZSS back-reference stream");

    const Bytes preserved = output;
    Bytes truncated = CompressedAbc;
    truncated.pop_back();
    check(!DecompressHistoricalSceneChunk(truncated, output, {}, &reason) &&
        output == preserved && reason.find("truncated") != std::string::npos,
        "truncated stream fails atomically");
    Bytes trailing = CompressedAbc;
    trailing.push_back(1);
    check(!DecompressHistoricalSceneChunk(trailing, output, {}, &reason) &&
        output == preserved && reason.find("trailing") != std::string::npos,
        "trailing compressed data rejected");

    EditorSceneDecompressionLimits byteLimits;
    byteLimits.maximumDecompressedBytes = 2;
    check(!DecompressHistoricalSceneChunk(CompressedAbc, output, byteLimits,
        &reason) && output == preserved,
        "declared output byte limit enforced");
    EditorSceneDecompressionLimits ratioLimits;
    ratioLimits.maximumExpansionRatio = 0.25;
    check(!DecompressHistoricalSceneChunk(CompressedAbc, output, ratioLimits,
        &reason) && output == preserved,
        "expansion ratio limit enforced");
    check(!DecompressHistoricalSceneChunk(Bytes{1, 0, 0}, output, {},
        &reason) && output == preserved,
        "missing size header rejected atomically");

    EditorHistoricalSceneProbe probe;
    EditorSceneManifest manifest;
    check(probe.ProbeSceneBytes(CompressedScene(CompressedTool),
        "compressed.level", manifest, &reason) &&
        manifest.compressedChunkCount == 1 &&
        manifest.decompressedChunkCount == 1 &&
        manifest.decompressionFailureCount == 0 &&
        manifest.objects.size() == 1 &&
        manifest.objects[0].name == "compressed_actor" &&
        manifest.objects[0].hasTransform,
        "compressed tool container contributes confirmed object records");
    check(manifest.chunks[2].compressed &&
        manifest.chunks[2].decompressionSucceeded &&
        manifest.chunks[2].compressedSize == CompressedTool.size() &&
        manifest.chunks[2].decompressedSize == 117 &&
        manifest.compressionAlgorithm == HistoricalSceneCompressionAlgorithm(),
        "compressed provenance metadata retained");

    EditorHistoricalSceneDocument document;
    check(document.BuildFromManifest(manifest, &reason) &&
        document.Objects().size() == 1 &&
        document.Objects()[0].fromDecompressedPayload &&
        document.Objects()[0].stableRecordId.find(".decoded.") !=
            std::string::npos,
        "historical identity retains decoded provenance");
    EditorSceneManifest duplicateManifest = manifest;
    EditorSceneObjectRecord duplicate = duplicateManifest.objects.front();
    duplicate.recordIndex += 1;
    duplicate.decompressedOffset += 16;
    duplicateManifest.objects.push_back(duplicate);
    check(document.BuildFromManifest(duplicateManifest, &reason) &&
        document.Objects().size() == 2 &&
        document.Objects()[0].stableRecordId !=
            document.Objects()[1].stableRecordId,
        "duplicate decoded names retain distinct stable identities");

    check(probe.ProbeSceneBytes(CompressedScene(NestedCompressedTool),
        "nested.level", manifest, &reason) &&
        manifest.compressedChunkCount == 2 &&
        manifest.decompressedChunkCount == 2 &&
        manifest.objects.size() == 1 &&
        manifest.objects[0].name == "nested_actor",
        "historically permitted nested compressed chunk decoded");

    EditorSceneProbeLimits nestedLimits;
    nestedLimits.maximumCompressedNestingDepth = 1;
    check(EditorHistoricalSceneProbe(nestedLimits).ProbeSceneBytes(
        CompressedScene(NestedCompressedTool), "nested-limit.level", manifest,
        &reason) && manifest.objects.empty() &&
        manifest.decompressionFailureCount == 1 &&
        !manifest.diagnostics.empty(),
        "nested compression policy retains diagnostic without parse-through");

    EditorSceneProbeLimits totalLimits;
    totalLimits.maximumTotalDecompressedBytes = 64;
    check(EditorHistoricalSceneProbe(totalLimits).ProbeSceneBytes(
        CompressedScene(CompressedTool), "budget.level", manifest, &reason) &&
        manifest.objects.empty() && manifest.decompressionFailureCount == 1 &&
        !manifest.diagnostics.empty(),
        "scene-wide decompressed byte budget enforced");

    Bytes corrupt = CompressedTool;
    corrupt.resize(5);
    check(probe.ProbeSceneBytes(CompressedScene(corrupt), "corrupt.level",
        manifest, &reason) && manifest.objects.empty() &&
        manifest.decompressionFailureCount == 1 &&
        !manifest.diagnostics.empty(),
        "corrupt non-critical container is diagnosed and skipped");

    return failures;
}
