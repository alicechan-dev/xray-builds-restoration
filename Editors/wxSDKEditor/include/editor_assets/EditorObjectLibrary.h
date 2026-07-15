#ifndef XR_WX_SDK_EDITOR_EDITOR_OBJECT_LIBRARY_H
#define XR_WX_SDK_EDITOR_EDITOR_OBJECT_LIBRARY_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "editor_render/EditorRenderMeshMetadata.h"

enum class EditorObjectKind { Static, Skeletal, Unknown };
enum class EditorObjectParseStatus { Supported, Partial, Malformed };

struct EditorObjectLibraryEntry
{
    std::string referenceId;
    std::string originalReference;
    std::string displayName;
    std::string category;
    std::string sourceRelativeFile;
    std::uintmax_t sourceFileSize = 0;
    std::uint16_t version = 0;
    std::int32_t libraryVersion = 0;
    std::uint32_t flags = 0;
    EditorObjectKind kind = EditorObjectKind::Unknown;
    std::size_t meshCount = 0;
    EditorRenderBounds bounds;
    std::vector<EditorRenderMeshMetadata> meshes;
    std::size_t surfaceCount = 0;
    bool motionPresent = false;
    std::vector<std::string> textureReferences;
    std::vector<std::string> shaderReferences;
    std::vector<std::string> materialReferences;
    EditorObjectParseStatus parseStatus = EditorObjectParseStatus::Malformed;
    std::size_t unknownChunkCount = 0;
    std::vector<std::string> diagnostics;
};

const char* ToString(EditorObjectKind value);
const char* ToString(EditorObjectParseStatus value);

bool NormalizeHistoricalObjectReference(std::string_view input,
    std::string& normalized, std::string* reason = nullptr);
bool ValidateHistoricalObjectReference(std::string_view input,
    std::string* reason = nullptr);

class EditorObjectLibrary
{
public:
    bool IsLoaded() const { return loaded_; }
    const std::vector<EditorObjectLibraryEntry>& Entries() const
    { return entries_; }
    const std::filesystem::path& Root() const { return root_; }
    std::vector<const EditorObjectLibraryEntry*> Find(
        std::string_view normalizedReference) const;
    void Clear();

private:
    friend class EditorObjectLibraryLoader;
    void RebuildIndex();

    bool loaded_ = false;
    std::filesystem::path root_;
    std::vector<EditorObjectLibraryEntry> entries_;
    std::unordered_map<std::string, std::vector<std::size_t>> index_;
};

#endif
