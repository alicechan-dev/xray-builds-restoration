#ifndef XR_WX_SDK_EDITOR_EDITOR_OBJECT_LIBRARY_LOADER_H
#define XR_WX_SDK_EDITOR_EDITOR_OBJECT_LIBRARY_LOADER_H

#include "editor_assets/EditorObjectLibrary.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct EditorObjectLibraryLimits
{
    std::size_t maximumFiles = 250000;
    std::uintmax_t maximumFileSize = 96ull * 1024ull * 1024ull;
    std::uintmax_t maximumTotalFileSize = 2ull * 1024ull * 1024ull * 1024ull;
    std::size_t maximumChunksPerFile = 100000;
    std::size_t maximumStringBytes = 16 * 1024;
    std::size_t maximumMetadataRecords = 100000;
    std::size_t maximumRetainedReferences = 256;
    std::size_t maximumDiagnostics = 128;
};

struct EditorObjectLibraryLoadStatistics
{
    std::size_t filesScanned = 0;
    std::size_t entriesLoaded = 0;
    std::size_t supported = 0;
    std::size_t partial = 0;
    std::size_t malformed = 0;
    std::size_t duplicateReferences = 0;
    std::uintmax_t sourceBytes = 0;
    std::uintmax_t bytesRead = 0;
    std::vector<std::string> diagnostics;
};

class EditorObjectLibraryLoader
{
public:
    explicit EditorObjectLibraryLoader(EditorObjectLibraryLimits limits = {})
        : limits_(limits) {}

    bool Load(const std::filesystem::path& root, EditorObjectLibrary& library,
        EditorObjectLibraryLoadStatistics& statistics,
        std::string* reason = nullptr) const;

private:
    EditorObjectLibraryLimits limits_;
};

#endif
