#ifndef XR_WX_SDK_EDITOR_EDITOR_METADATA_LOADER_H
#define XR_WX_SDK_EDITOR_EDITOR_METADATA_LOADER_H

#include "editor_assets/EditorImportedMetadata.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

struct EditorMetadataLimits
{
    std::size_t maxIncludeDepth = 16;
    std::size_t maxFiles = 256;
    std::size_t maxBytesPerFile = 2 * 1024 * 1024;
    std::size_t maxTotalBytes = 16 * 1024 * 1024;
    std::size_t maxLineBytes = 16 * 1024;
};

class EditorMetadataLoader
{
public:
    explicit EditorMetadataLoader(EditorMetadataLimits limits = {}) :
        limits_(limits) {}

    bool LoadMetadataRoot(const std::filesystem::path& root,
        const std::filesystem::path& entryFile,
        EditorImportedMetadata& result, std::string* reason = nullptr) const;
    bool ParseMetadataText(std::string_view text, std::string sourceName,
        EditorImportedMetadata& result, std::string* reason = nullptr) const;

private:
    EditorMetadataLimits limits_;
};

#endif
