#ifndef XR_WX_SDK_EDITOR_HISTORICAL_SCENE_OBJECT_RECORD_H
#define XR_WX_SDK_EDITOR_HISTORICAL_SCENE_OBJECT_RECORD_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class EditorHistoricalObjectDecodeStatus
{
    Supported,
    Partial,
    Unsupported,
    Malformed
};

const char* ToString(EditorHistoricalObjectDecodeStatus status);

struct EditorHistoricalFieldProvenance
{
    std::uint32_t chunkId = 0;
    std::size_t bodyOffset = 0;
    std::size_t sourceOffset = 0;
    std::size_t size = 0;
    std::string chunkPath;
};

struct EditorHistoricalBodyChunkRecord
{
    std::uint32_t id = 0;
    std::size_t headerOffset = 0;
    std::size_t dataOffset = 0;
    std::size_t size = 0;
    std::string path;
};

struct EditorHistoricalSceneObjectBodyRecord
{
    std::uint16_t version = 0;
    std::int32_t referenceVersion = 0;
    std::int32_t referenceReserved = 0;
    std::string referenceName;
    bool hasFlags = false;
    std::uint32_t flags = 0;
    EditorHistoricalFieldProvenance versionProvenance;
    EditorHistoricalFieldProvenance referenceProvenance;
    EditorHistoricalFieldProvenance flagsProvenance;
};

struct EditorHistoricalObjectBodyDecodeResult
{
    EditorHistoricalObjectDecodeStatus status =
        EditorHistoricalObjectDecodeStatus::Unsupported;
    std::string typeName = "Unknown historical object";
    bool hasBodyVersion = false;
    std::uint16_t bodyVersion = 0;
    bool hasSceneObject = false;
    EditorHistoricalSceneObjectBodyRecord sceneObject;
    std::vector<EditorHistoricalBodyChunkRecord> unknownChunks;
    std::vector<EditorHistoricalBodyChunkRecord> unsupportedChunks;
    std::vector<std::string> diagnostics;
};

#endif
