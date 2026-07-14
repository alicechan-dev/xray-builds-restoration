#ifndef XR_WX_SDK_EDITOR_EDITOR_SCENE_COMPRESSION_H
#define XR_WX_SDK_EDITOR_EDITOR_SCENE_COMPRESSION_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct EditorSceneDecompressionLimits
{
    std::size_t maximumCompressedBytes = 64u * 1024u * 1024u;
    std::size_t maximumDecompressedBytes = 256u * 1024u * 1024u;
    double maximumExpansionRatio = 64.0;
};

const char* HistoricalSceneCompressionAlgorithm();

bool DecompressHistoricalSceneChunk(
    const std::uint8_t* compressed, std::size_t compressedSize,
    std::vector<std::uint8_t>& output,
    const EditorSceneDecompressionLimits& limits = {},
    std::string* reason = nullptr);

inline bool DecompressHistoricalSceneChunk(
    const std::vector<std::uint8_t>& compressed,
    std::vector<std::uint8_t>& output,
    const EditorSceneDecompressionLimits& limits = {},
    std::string* reason = nullptr)
{
    return DecompressHistoricalSceneChunk(compressed.data(), compressed.size(),
        output, limits, reason);
}

#endif
