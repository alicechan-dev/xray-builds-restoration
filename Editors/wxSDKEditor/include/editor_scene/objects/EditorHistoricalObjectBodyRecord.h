#ifndef XR_WX_SDK_EDITOR_HISTORICAL_OBJECT_BODY_RECORD_H
#define XR_WX_SDK_EDITOR_HISTORICAL_OBJECT_BODY_RECORD_H

#include <cstddef>
#include <cstdint>
#include <string>

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

#endif
