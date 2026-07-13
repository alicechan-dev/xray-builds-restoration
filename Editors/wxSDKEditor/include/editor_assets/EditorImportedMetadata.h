#ifndef XR_WX_SDK_EDITOR_EDITOR_IMPORTED_METADATA_H
#define XR_WX_SDK_EDITOR_EDITOR_IMPORTED_METADATA_H

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

enum class EditorMetadataDiagnosticSeverity { Warning, Error };

struct EditorMetadataDiagnostic
{
    EditorMetadataDiagnosticSeverity severity =
        EditorMetadataDiagnosticSeverity::Warning;
    std::string sourceFile;
    std::size_t sourceLine = 0;
    std::string message;
};

struct EditorMetadataEntry
{
    std::string key;
    std::string value;
    std::string sourceFile;
    std::size_t sourceLine = 0;
};

struct EditorMetadataSection
{
    std::string name;
    std::string sourceFile;
    std::size_t sourceLine = 0;
    std::vector<EditorMetadataEntry> entries;

    const EditorMetadataEntry* FindFirst(std::string_view key) const;
};

struct EditorImportedMetadata
{
    std::string rootPath;
    std::string entryFile;
    std::vector<std::string> files;
    std::vector<EditorMetadataSection> sections;
    std::vector<EditorMetadataDiagnostic> diagnostics;
    std::size_t includesFollowed = 0;
    std::size_t totalBytes = 0;
};

#endif
