#ifndef XR_WX_SDK_EDITOR_HISTORICAL_SPAWN_POINT_DECODER_H
#define XR_WX_SDK_EDITOR_HISTORICAL_SPAWN_POINT_DECODER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct EditorHistoricalObjectBodyDecodeLimits;
struct EditorHistoricalObjectBodyDecodeResult;

EditorHistoricalObjectBodyDecodeResult DecodeHistoricalSpawnPointBody(
    const std::vector<std::uint8_t>& bytes, std::size_t sourceBase,
    const std::string& bodyPath,
    const EditorHistoricalObjectBodyDecodeLimits& limits);

#endif
