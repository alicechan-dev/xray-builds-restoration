#ifndef XR_WX_SDK_EDITOR_HISTORICAL_OBJECT_BODY_DECODER_H
#define XR_WX_SDK_EDITOR_HISTORICAL_OBJECT_BODY_DECODER_H

#include "editor_scene/objects/EditorHistoricalSceneObjectRecord.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct EditorHistoricalObjectBodyDecodeLimits
{
    std::size_t maximumBodySize = 4u * 1024u * 1024u;
    std::size_t maximumChunks = 64;
    std::size_t maximumStringLength = 4096;
    std::size_t maximumRetainedUnknownChunks = 32;
};

EditorHistoricalObjectBodyDecodeResult DecodeHistoricalObjectBody(
    std::uint32_t classId, const std::vector<std::uint8_t>& bodyBytes,
    std::size_t bodyDataOffset, const std::string& bodyChunkPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits = {});

#endif
