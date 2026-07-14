#ifndef XR_WX_SDK_EDITOR_HISTORICAL_GLOW_DECODER_H
#define XR_WX_SDK_EDITOR_HISTORICAL_GLOW_DECODER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct EditorHistoricalObjectBodyDecodeLimits;
struct EditorHistoricalObjectBodyDecodeResult;

EditorHistoricalObjectBodyDecodeResult DecodeHistoricalGlowBody(
    const std::vector<std::uint8_t>& bodyBytes,
    std::size_t bodyDataOffset, const std::string& bodyChunkPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits);

#endif
