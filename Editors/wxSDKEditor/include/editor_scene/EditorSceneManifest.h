#ifndef XR_WX_SDK_EDITOR_EDITOR_SCENE_MANIFEST_H
#define XR_WX_SDK_EDITOR_EDITOR_SCENE_MANIFEST_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class EditorSceneDiagnosticSeverity { Information, Warning };

struct EditorSceneDiagnostic
{
    EditorSceneDiagnosticSeverity severity =
        EditorSceneDiagnosticSeverity::Warning;
    std::size_t offset = 0;
    std::string message;
};

struct EditorSceneChunkRecord
{
    std::uint32_t id = 0;
    std::uint32_t rawId = 0;
    std::uint32_t size = 0;
    std::size_t headerOffset = 0;
    std::size_t dataOffset = 0;
    std::size_t depth = 0;
    bool compressed = false;
    std::size_t compressedSize = 0;
    std::size_t decompressedSize = 0;
    bool decompressionSupported = false;
    bool decompressionSucceeded = false;
    std::string compressionAlgorithm;
    std::string compressionDiagnostic;
    bool fromDecompressedPayload = false;
    std::size_t compressedSourceOffset = 0;
    std::size_t decompressedOffset = 0;
    std::string path;
    std::string label;
};

struct EditorSceneObjectRecord
{
    std::size_t recordIndex = 0;
    std::size_t sourceOffset = 0;
    std::uint32_t classId = 0;
    bool hasClassId = false;
    std::string name;
    bool hasName = false;
    std::array<float, 3> position{};
    std::array<float, 3> rotation{};
    std::array<float, 3> scale{};
    bool hasTransform = false;
    std::string chunkPath;
    bool fromDecompressedPayload = false;
    std::size_t compressedSourceOffset = 0;
    std::size_t decompressedOffset = 0;
};

struct EditorSceneManifest
{
    std::string sourceFile;
    std::size_t totalSize = 0;
    std::string format;
    std::uint32_t version = 0;
    bool hasVersion = false;
    std::uint32_t declaredObjectCount = 0;
    bool hasDeclaredObjectCount = false;
    std::vector<EditorSceneChunkRecord> chunks;
    std::vector<EditorSceneObjectRecord> objects;
    std::vector<EditorSceneDiagnostic> diagnostics;
    std::size_t unknownChunkCount = 0;
    std::size_t compressedChunkCount = 0;
    std::size_t decompressedChunkCount = 0;
    std::size_t decompressionFailureCount = 0;
    std::size_t totalCompressedBytes = 0;
    std::size_t totalDecompressedBytes = 0;
    std::string compressionAlgorithm;
};

#endif
