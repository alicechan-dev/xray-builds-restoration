#include "editor_assets/EditorObjectLibraryLoader.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <limits>
#include <set>

namespace
{
constexpr std::uint32_t kObjectBody = 0x7777;
constexpr std::uint32_t kVersion = 0x0900;
constexpr std::uint32_t kFlags = 0x0903;
constexpr std::uint32_t kSurfacesOld = 0x0905;
constexpr std::uint32_t kSurfaces2 = 0x0906;
constexpr std::uint32_t kSurfaces3 = 0x0907;
constexpr std::uint32_t kMeshes = 0x0910;
constexpr std::uint32_t kLibraryVersion = 0x0911;
constexpr std::uint32_t kBones = 0x0913;
constexpr std::uint32_t kMotions = 0x0916;
constexpr std::uint32_t kBones2 = 0x0921;
constexpr std::uint32_t kMotionRefs = 0x0924;
constexpr std::uint16_t kCurrentVersion = 0x0010;
constexpr std::uint32_t kDynamicFlag = 1u << 0;

struct Chunk { std::uint32_t id = 0; std::uint64_t begin = 0; std::uint64_t size = 0; };

class FileReader
{
public:
    FileReader(const std::filesystem::path& path, std::uint64_t size)
        : stream_(path, std::ios::binary), fileSize_(size) {}
    bool Good() const { return stream_.is_open(); }
    std::uint64_t BytesRead() const { return bytesRead_; }
    bool ReadAt(std::uint64_t offset, void* destination, std::size_t size)
    {
        if (offset > fileSize_ || size > fileSize_ - offset ||
            offset > static_cast<std::uint64_t>(std::numeric_limits<std::streamoff>::max()))
            return false;
        stream_.clear(); stream_.seekg(static_cast<std::streamoff>(offset));
        stream_.read(static_cast<char*>(destination), static_cast<std::streamsize>(size));
        if (!stream_) return false;
        bytesRead_ += size; return true;
    }
    bool U16(std::uint64_t offset, std::uint16_t& value)
    {
        std::array<unsigned char, 2> b{}; if (!ReadAt(offset, b.data(), b.size())) return false;
        value = static_cast<std::uint16_t>(b[0] | (b[1] << 8)); return true;
    }
    bool U32(std::uint64_t offset, std::uint32_t& value)
    {
        std::array<unsigned char, 4> b{}; if (!ReadAt(offset, b.data(), b.size())) return false;
        value = static_cast<std::uint32_t>(b[0]) |
            (static_cast<std::uint32_t>(b[1]) << 8) |
            (static_cast<std::uint32_t>(b[2]) << 16) |
            (static_cast<std::uint32_t>(b[3]) << 24); return true;
    }
    bool CString(std::uint64_t& offset, std::uint64_t end, std::size_t limit,
        std::string& value)
    {
        value.clear();
        while (offset < end && value.size() < limit) {
            char ch = 0; if (!ReadAt(offset++, &ch, 1)) return false;
            if (!ch) return true; value.push_back(ch);
        }
        return false;
    }
private:
    std::ifstream stream_; std::uint64_t fileSize_ = 0; std::uint64_t bytesRead_ = 0;
};

bool ReadChunk(FileReader& reader, std::uint64_t offset, std::uint64_t end,
    Chunk& chunk)
{
    std::uint32_t size = 0;
    if (end - offset < 8 || !reader.U32(offset, chunk.id) || !reader.U32(offset + 4, size))
        return false;
    chunk.begin = offset + 8; chunk.size = size;
    return chunk.begin <= end && chunk.size <= end - chunk.begin;
}

void AddDiagnostic(EditorObjectLibraryEntry& entry, std::string message,
    const EditorObjectLibraryLimits& limits)
{
    if (entry.diagnostics.size() < limits.maximumDiagnostics)
        entry.diagnostics.push_back(std::move(message));
}

void RetainUnique(std::vector<std::string>& values, std::string value,
    std::size_t maximum)
{
    if (value.empty() || values.size() >= maximum ||
        std::find(values.begin(), values.end(), value) != values.end()) return;
    values.push_back(std::move(value));
}

bool ParseSurfaces(FileReader& reader, const Chunk& chunk, bool version3,
    EditorObjectLibraryEntry& entry, const EditorObjectLibraryLimits& limits)
{
    const std::uint64_t end = chunk.begin + chunk.size;
    std::uint32_t count = 0;
    if (chunk.size < 4 || !reader.U32(chunk.begin, count) ||
        count > limits.maximumMetadataRecords) return false;
    entry.surfaceCount = count;
    std::uint64_t cursor = chunk.begin + 4;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        std::string name, shader, compiler, material, texture, vmap;
        if (!reader.CString(cursor, end, limits.maximumStringBytes, name) ||
            !reader.CString(cursor, end, limits.maximumStringBytes, shader) ||
            !reader.CString(cursor, end, limits.maximumStringBytes, compiler)) return false;
        if (version3 && !reader.CString(cursor, end, limits.maximumStringBytes, material)) return false;
        if (!reader.CString(cursor, end, limits.maximumStringBytes, texture) ||
            !reader.CString(cursor, end, limits.maximumStringBytes, vmap) ||
            end - cursor < 12) return false;
        cursor += 12;
        RetainUnique(entry.shaderReferences, shader, limits.maximumRetainedReferences);
        RetainUnique(entry.shaderReferences, compiler, limits.maximumRetainedReferences);
        RetainUnique(entry.materialReferences, material, limits.maximumRetainedReferences);
        RetainUnique(entry.textureReferences, texture, limits.maximumRetainedReferences);
    }
    return true;
}

bool ParseObject(const std::filesystem::path& path, std::uint64_t fileSize,
    EditorObjectLibraryEntry& entry, const EditorObjectLibraryLimits& limits,
    std::uint64_t& bytesRead)
{
    FileReader reader(path, fileSize);
    if (!reader.Good()) { AddDiagnostic(entry, "Object file could not be opened.", limits); return false; }
    Chunk outer;
    if (!ReadChunk(reader, 0, fileSize, outer) || outer.id != kObjectBody) {
        AddDiagnostic(entry, "Required EOBJ_CHUNK_OBJECT_BODY (0x7777) is absent.", limits);
        bytesRead += reader.BytesRead(); return false;
    }
    bool haveVersion = false, haveFlags = false, haveLibraryVersion = false;
    bool partial = false, skeletal = false;
    std::size_t chunks = 0;
    std::uint64_t cursor = outer.begin, end = outer.begin + outer.size;
    while (cursor < end)
    {
        if (++chunks > limits.maximumChunksPerFile) {
            AddDiagnostic(entry, "Chunk count exceeds the configured limit.", limits);
            bytesRead += reader.BytesRead(); return false;
        }
        Chunk chunk;
        if (!ReadChunk(reader, cursor, end, chunk)) {
            AddDiagnostic(entry, "Truncated or out-of-bounds object chunk.", limits);
            bytesRead += reader.BytesRead(); return false;
        }
        switch (chunk.id)
        {
        case kVersion:
            haveVersion = chunk.size >= 2 && reader.U16(chunk.begin, entry.version);
            if (haveVersion && entry.version != kCurrentVersion) {
                partial = true; AddDiagnostic(entry, "Object version is not build-1935 version 0x0010.", limits);
            }
            break;
        case kFlags:
            haveFlags = chunk.size >= 4 && reader.U32(chunk.begin, entry.flags);
            break;
        case kLibraryVersion: {
            std::uint32_t value = 0;
            haveLibraryVersion = chunk.size >= 8 && reader.U32(chunk.begin, value);
            entry.libraryVersion = static_cast<std::int32_t>(value); break; }
        case kSurfaces3:
            if (!ParseSurfaces(reader, chunk, true, entry, limits)) {
                AddDiagnostic(entry, "Malformed version-3 surface metadata.", limits);
                bytesRead += reader.BytesRead(); return false;
            }
            break;
        case kSurfaces2:
            if (!ParseSurfaces(reader, chunk, false, entry, limits)) {
                AddDiagnostic(entry, "Malformed version-2 surface metadata.", limits);
                bytesRead += reader.BytesRead(); return false;
            }
            partial = true; break;
        case kSurfacesOld:
            partial = true; AddDiagnostic(entry, "Legacy surface metadata is inventoried but not decoded.", limits); break;
        case kMeshes: {
            std::uint64_t meshCursor = chunk.begin, meshEnd = chunk.begin + chunk.size;
            while (meshCursor < meshEnd) {
                if (entry.meshCount >= limits.maximumMetadataRecords) {
                    AddDiagnostic(entry, "Mesh count exceeds the configured limit.", limits);
                    bytesRead += reader.BytesRead(); return false;
                }
                Chunk mesh;
                if (!ReadChunk(reader, meshCursor, meshEnd, mesh)) {
                    AddDiagnostic(entry, "Malformed mesh container; mesh payload was not read.", limits);
                    bytesRead += reader.BytesRead(); return false;
                }
                ++entry.meshCount; meshCursor = mesh.begin + mesh.size;
            }
            break; }
        case kBones: case kBones2: skeletal = true; break;
        case kMotions: case kMotionRefs: entry.motionPresent = true; break;
        default: ++entry.unknownChunkCount; break;
        }
        cursor = chunk.begin + chunk.size;
    }
    if (!haveVersion || !haveFlags || !haveLibraryVersion) {
        AddDiagnostic(entry, "One or more required version/flags/library-version chunks are absent.", limits);
        bytesRead += reader.BytesRead(); return false;
    }
    entry.kind = (skeletal || (entry.flags & kDynamicFlag)) ?
        EditorObjectKind::Skeletal : EditorObjectKind::Static;
    entry.parseStatus = partial ? EditorObjectParseStatus::Partial :
        EditorObjectParseStatus::Supported;
    bytesRead += reader.BytesRead(); return true;
}

bool IsInside(const std::filesystem::path& root,
    const std::filesystem::path& candidate)
{
    auto r = root.begin(), c = candidate.begin();
    for (; r != root.end(); ++r, ++c)
        if (c == candidate.end() || std::filesystem::path(*r).compare(*c) != 0)
            return false;
    return true;
}

std::string RelativeReference(const std::filesystem::path& relative)
{
    return relative.generic_string();
}
}

bool EditorObjectLibraryLoader::Load(const std::filesystem::path& root,
    EditorObjectLibrary& library, EditorObjectLibraryLoadStatistics& stats,
    std::string* reason) const
{
    stats = {};
    std::error_code error;
    const auto canonicalRoot = std::filesystem::weakly_canonical(root, error);
    if (error || !std::filesystem::is_directory(canonicalRoot, error)) {
        if (reason) *reason = "Object Library root is not a readable directory."; return false;
    }
    EditorObjectLibrary candidate; candidate.loaded_ = true; candidate.root_ = canonicalRoot;
    std::filesystem::recursive_directory_iterator iterator(canonicalRoot,
        std::filesystem::directory_options::skip_permission_denied, error), end;
    for (; !error && iterator != end; iterator.increment(error))
    {
        const auto status = iterator->symlink_status(error);
        if (error) break;
        if (std::filesystem::is_symlink(status)) {
            if (iterator->is_directory(error)) iterator.disable_recursion_pending();
            continue;
        }
        if (!iterator->is_regular_file(error)) continue;
        auto extension = iterator->path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        if (extension != ".object") continue;
        if (++stats.filesScanned > limits_.maximumFiles) {
            if (reason) *reason = "Object Library exceeds the configured file-count limit."; return false;
        }
        const auto canonicalFile = std::filesystem::weakly_canonical(iterator->path(), error);
        if (error || !IsInside(canonicalRoot, canonicalFile)) {
            if (reason) *reason = "Object Library entry escapes the selected root."; return false;
        }
        const auto size = iterator->file_size(error);
        if (error) { if (reason) *reason = "Object Library file size could not be read."; return false; }
        if (size > limits_.maximumFileSize) {
            if (reason) *reason = "Object Library contains a file larger than the configured limit."; return false;
        }
        if (stats.sourceBytes > limits_.maximumTotalFileSize - size) {
            if (reason) *reason = "Object Library exceeds the configured total-size limit."; return false;
        }
        stats.sourceBytes += size;
        auto relative = std::filesystem::relative(canonicalFile, canonicalRoot, error);
        if (error) { if (reason) *reason = "Object Library relative path could not be computed."; return false; }
        EditorObjectLibraryEntry entry;
        auto referencePath = relative;
        referencePath.replace_extension();
        entry.originalReference = RelativeReference(referencePath);
        entry.sourceRelativeFile = RelativeReference(relative);
        entry.sourceFileSize = size;
        entry.displayName = relative.stem().string();
        entry.category = RelativeReference(relative.parent_path());
        std::string normalizeReason;
        if (!NormalizeHistoricalObjectReference(entry.originalReference,
            entry.referenceId, &normalizeReason)) {
            entry.parseStatus = EditorObjectParseStatus::Malformed;
            AddDiagnostic(entry, normalizeReason, limits_);
        } else {
            ParseObject(canonicalFile, size, entry, limits_, stats.bytesRead);
        }
        candidate.entries_.push_back(std::move(entry));
    }
    if (error) { if (reason) *reason = "Object Library traversal failed: " + error.message(); return false; }
    std::sort(candidate.entries_.begin(), candidate.entries_.end(),
        [](const auto& a, const auto& b) {
            if (a.referenceId != b.referenceId) return a.referenceId < b.referenceId;
            return a.sourceRelativeFile < b.sourceRelativeFile;
        });
    candidate.RebuildIndex();
    for (const auto& entry : candidate.entries_) {
        if (entry.parseStatus == EditorObjectParseStatus::Supported) ++stats.supported;
        else if (entry.parseStatus == EditorObjectParseStatus::Partial) ++stats.partial;
        else ++stats.malformed;
        if (candidate.Find(entry.referenceId).size() > 1) ++stats.duplicateReferences;
    }
    stats.entriesLoaded = candidate.entries_.size();
    library = std::move(candidate);
    return true;
}
