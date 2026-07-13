#include "editor_scene/EditorHistoricalSceneProbe.h"

#include "editor_scene/EditorBinaryReader.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

namespace
{
constexpr std::uint32_t CompressMark = 0x80000000u;
constexpr std::uint32_t SceneVersionChunk = 0x00009df3u;
constexpr std::uint32_t SceneVersion = 5u;
constexpr std::uint32_t SceneObjectListChunk = 0x00007708u;
constexpr std::uint32_t SceneObjectCountChunk = 0x00007712u;
constexpr std::uint32_t SceneToolOffset = 0x00008000u;
constexpr std::uint32_t LastObjectToolClass = 11u;
constexpr std::uint32_t ToolObjectCountChunk = 0x00000002u;
constexpr std::uint32_t ToolObjectsChunk = 0x00000003u;
constexpr std::uint32_t ObjectClassChunk = 0x00007703u;
constexpr std::uint32_t ObjectBodyChunk = 0x00007777u;
constexpr std::uint32_t ObjectTransformChunk = 0x0000f903u;
constexpr std::uint32_t ObjectNameChunk = 0x0000f907u;

struct Chunk
{
    std::uint32_t rawId = 0;
    std::uint32_t id = 0;
    std::uint32_t size = 0;
    std::size_t headerOffset = 0;
    std::size_t dataOffset = 0;
    bool compressed = false;
    EditorBinaryReader payload;
};

std::string HexId(std::uint32_t id)
{
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << std::setw(8)
        << std::setfill('0') << id;
    return output.str();
}

std::string ChildPath(const std::string& parent, std::uint32_t id)
{
    return parent.empty() ? HexId(id) : parent + "/" + HexId(id);
}

std::string ChunkLabel(std::uint32_t id, std::size_t depth)
{
    if (depth == 0)
    {
        if (id == SceneVersionChunk) return "Scene version";
        if (id == SceneObjectCountChunk) return "Declared object count";
        if (id == SceneObjectListChunk) return "Legacy object list";
        if (id >= SceneToolOffset && id < SceneToolOffset + 15u)
            return "Scene tool class " + std::to_string(id - SceneToolOffset);
    }
    if (depth == 1 && id == ToolObjectCountChunk)
        return "Tool object count";
    if (depth == 1 && id == ToolObjectsChunk)
        return "Tool object records";
    if (depth == 3 && id == ObjectClassChunk)
        return "Object class";
    if (depth == 3 && id == ObjectBodyChunk)
        return "Object body";
    if (depth == 4 && id == ObjectNameChunk)
        return "Object name";
    if (depth == 4 && id == ObjectTransformChunk)
        return "Object transform";
    return "Unknown";
}

bool SetReason(std::string* reason, const std::string& message)
{
    if (reason)
        *reason = message;
    return false;
}

class Parser
{
public:
    Parser(const EditorSceneProbeLimits& limits, EditorSceneManifest& manifest,
        std::string* reason) : limits_(limits), manifest_(manifest),
        reason_(reason)
    {
    }

    bool Parse(EditorBinaryReader reader)
    {
        bool versionSeen = false;
        while (!reader.Empty())
        {
            Chunk chunk;
            if (!ReadChunk(reader, chunk))
                return false;
            const std::string path = ChildPath({}, chunk.id);
            if (!RecordChunk(chunk, path, 0))
                return false;

            if (chunk.id == SceneVersionChunk)
            {
                if (versionSeen)
                    return Fail(chunk.headerOffset,
                        "duplicate scene version chunk");
                if (chunk.compressed || chunk.size != 4)
                    return Fail(chunk.headerOffset,
                        "scene version chunk must be an uncompressed u32");
                if (!chunk.payload.ReadU32(manifest_.version))
                    return ReaderFail(chunk.payload);
                manifest_.hasVersion = true;
                versionSeen = true;
            }
            else if (chunk.id == SceneObjectCountChunk)
            {
                if (chunk.compressed || chunk.size != 4)
                    return Fail(chunk.headerOffset,
                        "object count chunk must be an uncompressed u32");
                if (!chunk.payload.ReadU32(manifest_.declaredObjectCount))
                    return ReaderFail(chunk.payload);
                manifest_.hasDeclaredObjectCount = true;
            }
            else if (chunk.id == SceneObjectListChunk)
            {
                if (chunk.compressed)
                    return Fail(chunk.headerOffset,
                        "compressed legacy object list is unsupported");
                if (!ParseObjectContainer(chunk.payload, path, 1, false, 0))
                    return false;
            }
            else if (chunk.id >= SceneToolOffset &&
                chunk.id <= SceneToolOffset + LastObjectToolClass)
            {
                if (chunk.compressed)
                    return Fail(chunk.headerOffset,
                        "compressed object-tool chunk is unsupported");
                if (!ParseObjectTool(chunk.payload, path, 1,
                    chunk.id - SceneToolOffset))
                    return false;
            }
            else
            {
                ++manifest_.unknownChunkCount;
            }
        }

        if (!manifest_.hasVersion)
            return Fail(0, "scene version chunk 0x00009DF3 is missing");
        if (manifest_.version != SceneVersion)
            return Fail(0, "unsupported historical scene version " +
                std::to_string(manifest_.version) + " (expected 5)");
        manifest_.format = "Build 1935 LevelEditor scene v5";
        if (manifest_.hasDeclaredObjectCount &&
            manifest_.declaredObjectCount != manifest_.objects.size())
        {
            AddDiagnostic(0, "declared object count " +
                std::to_string(manifest_.declaredObjectCount) +
                " differs from confirmed object records " +
                std::to_string(manifest_.objects.size()));
        }
        return true;
    }

private:
    bool ParseObjectTool(EditorBinaryReader reader, const std::string& path,
        std::size_t depth, std::uint32_t toolClass)
    {
        if (depth > limits_.maximumNestingDepth)
            return Fail(reader.AbsoluteOffset(), "scene nesting limit exceeded");
        while (!reader.Empty())
        {
            Chunk chunk;
            if (!ReadChunk(reader, chunk))
                return false;
            const std::string childPath = ChildPath(path, chunk.id);
            if (!RecordChunk(chunk, childPath, depth))
                return false;
            if (chunk.id == ToolObjectsChunk)
            {
                if (chunk.compressed)
                    return Fail(chunk.headerOffset,
                        "compressed tool object records are unsupported");
                if (!ParseObjectContainer(chunk.payload, childPath, depth + 1,
                    true, toolClass))
                    return false;
            }
            else if (chunk.id != ToolObjectCountChunk)
                ++manifest_.unknownChunkCount;
        }
        return true;
    }

    bool ParseObjectContainer(EditorBinaryReader reader,
        const std::string& path, std::size_t depth, bool hasToolClass,
        std::uint32_t toolClass)
    {
        if (depth > limits_.maximumNestingDepth)
            return Fail(reader.AbsoluteOffset(), "scene nesting limit exceeded");
        while (!reader.Empty())
        {
            Chunk chunk;
            if (!ReadChunk(reader, chunk))
                return false;
            const std::string childPath = ChildPath(path, chunk.id);
            if (!RecordChunk(chunk, childPath, depth))
                return false;
            if (chunk.compressed)
                return Fail(chunk.headerOffset,
                    "compressed object record is unsupported");
            if (!ParseObjectRecord(chunk.payload, childPath, depth + 1,
                chunk.id, hasToolClass, toolClass))
                return false;
        }
        return true;
    }

    bool ParseObjectRecord(EditorBinaryReader reader, const std::string& path,
        std::size_t depth, std::size_t recordIndex, bool hasToolClass,
        std::uint32_t toolClass)
    {
        if (manifest_.objects.size() >= limits_.maximumObjects)
            return Fail(reader.AbsoluteOffset(), "scene object limit exceeded");
        EditorSceneObjectRecord object;
        object.recordIndex = recordIndex;
        object.sourceOffset = reader.AbsoluteOffset();
        object.chunkPath = path;
        bool bodySeen = false;
        while (!reader.Empty())
        {
            Chunk chunk;
            if (!ReadChunk(reader, chunk))
                return false;
            const std::string childPath = ChildPath(path, chunk.id);
            if (!RecordChunk(chunk, childPath, depth))
                return false;
            if (chunk.compressed)
                return Fail(chunk.headerOffset,
                    "compressed object wrapper chunk is unsupported");
            if (chunk.id == ObjectClassChunk)
            {
                if (object.hasClassId || chunk.size != 4)
                    return Fail(chunk.headerOffset,
                        "object class chunk is duplicate or not a u32");
                if (!chunk.payload.ReadU32(object.classId))
                    return ReaderFail(chunk.payload);
                object.hasClassId = true;
            }
            else if (chunk.id == ObjectBodyChunk)
            {
                if (bodySeen)
                    return Fail(chunk.headerOffset, "duplicate object body chunk");
                bodySeen = true;
                if (!ParseObjectBody(chunk.payload, childPath, depth + 1,
                    object))
                    return false;
            }
            else
                ++manifest_.unknownChunkCount;
        }
        if (!object.hasClassId || !bodySeen)
            return Fail(object.sourceOffset,
                "object record lacks confirmed class or body chunk");
        if (hasToolClass && object.classId != toolClass)
            AddDiagnostic(object.sourceOffset, "object class " +
                std::to_string(object.classId) + " differs from tool class " +
                std::to_string(toolClass));
        manifest_.objects.push_back(std::move(object));
        return true;
    }

    bool ParseObjectBody(EditorBinaryReader reader, const std::string& path,
        std::size_t depth, EditorSceneObjectRecord& object)
    {
        if (depth > limits_.maximumNestingDepth)
            return Fail(reader.AbsoluteOffset(), "scene nesting limit exceeded");
        while (!reader.Empty())
        {
            Chunk chunk;
            if (!ReadChunk(reader, chunk))
                return false;
            const std::string childPath = ChildPath(path, chunk.id);
            if (!RecordChunk(chunk, childPath, depth))
                return false;
            if (chunk.compressed)
                return Fail(chunk.headerOffset,
                    "compressed common object chunk is unsupported");
            if (chunk.id == ObjectNameChunk)
            {
                if (object.hasName || !chunk.payload.ReadCString(
                    object.name, limits_.maximumStringLength))
                    return object.hasName
                        ? Fail(chunk.headerOffset, "duplicate object name chunk")
                        : ReaderFail(chunk.payload);
                object.hasName = true;
                if (!chunk.payload.Empty())
                    AddDiagnostic(chunk.payload.AbsoluteOffset(),
                        "object name chunk has trailing bytes");
            }
            else if (chunk.id == ObjectTransformChunk)
            {
                if (object.hasTransform || chunk.size != 9u * sizeof(float))
                    return Fail(chunk.headerOffset,
                        "object transform chunk is duplicate or not nine floats");
                for (float& value : object.position)
                    if (!chunk.payload.ReadFloat(value)) return ReaderFail(chunk.payload);
                for (float& value : object.rotation)
                    if (!chunk.payload.ReadFloat(value)) return ReaderFail(chunk.payload);
                for (float& value : object.scale)
                    if (!chunk.payload.ReadFloat(value)) return ReaderFail(chunk.payload);
                object.hasTransform = true;
            }
            else
                ++manifest_.unknownChunkCount;
        }
        return true;
    }

    bool ReadChunk(EditorBinaryReader& reader, Chunk& chunk)
    {
        if (reader.Remaining() < 8)
            return Fail(reader.AbsoluteOffset(), "truncated XR chunk header");
        chunk.headerOffset = reader.AbsoluteOffset();
        if (!reader.ReadU32(chunk.rawId) || !reader.ReadU32(chunk.size))
            return ReaderFail(reader);
        chunk.id = chunk.rawId & ~CompressMark;
        chunk.compressed = (chunk.rawId & CompressMark) != 0;
        chunk.dataOffset = reader.AbsoluteOffset();
        if (static_cast<std::size_t>(chunk.size) > reader.Remaining())
            return Fail(chunk.headerOffset, "XR chunk payload exceeds parent bounds");
        if (!reader.Slice(chunk.size, chunk.payload))
            return ReaderFail(reader);
        return true;
    }

    bool RecordChunk(const Chunk& chunk, const std::string& path,
        std::size_t depth)
    {
        if (depth > limits_.maximumNestingDepth)
            return Fail(chunk.headerOffset, "scene nesting limit exceeded");
        if (manifest_.chunks.size() >= limits_.maximumChunks)
            return Fail(chunk.headerOffset, "scene chunk limit exceeded");
        manifest_.chunks.push_back({chunk.id, chunk.rawId, chunk.size,
            chunk.headerOffset, chunk.dataOffset, depth, chunk.compressed,
            path, ChunkLabel(chunk.id, depth)});
        return true;
    }

    void AddDiagnostic(std::size_t offset, std::string message)
    {
        if (manifest_.diagnostics.size() < limits_.maximumDiagnostics)
            manifest_.diagnostics.push_back({
                EditorSceneDiagnosticSeverity::Warning, offset,
                std::move(message)});
    }

    bool ReaderFail(const EditorBinaryReader& reader)
    {
        return SetReason(reason_, reader.Error());
    }

    bool Fail(std::size_t offset, const std::string& message)
    {
        return SetReason(reason_, message + " at byte offset " +
            std::to_string(offset));
    }

    const EditorSceneProbeLimits& limits_;
    EditorSceneManifest& manifest_;
    std::string* reason_ = nullptr;
};
}

EditorHistoricalSceneProbe::EditorHistoricalSceneProbe(
    EditorSceneProbeLimits limits) : limits_(limits)
{
}

bool EditorHistoricalSceneProbe::ProbeSceneFile(
    const std::filesystem::path& path, EditorSceneManifest& result,
    std::string* reason) const
{
    std::error_code error;
    const std::uintmax_t size = std::filesystem::file_size(path, error);
    if (error)
        return SetReason(reason, "historical scene file is not readable");
    if (size > limits_.maximumFileSize)
        return SetReason(reason, "historical scene exceeds file-size limit");
    std::ifstream input(path, std::ios::binary);
    if (!input)
        return SetReason(reason, "historical scene file could not be opened");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    if (!bytes.empty() && !input.read(
        reinterpret_cast<char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size())))
        return SetReason(reason, "historical scene file could not be read");
    return ProbeSceneBytes(bytes, path.generic_string(), result, reason);
}

bool EditorHistoricalSceneProbe::ProbeSceneBytes(
    const std::vector<std::uint8_t>& bytes, std::string sourceName,
    EditorSceneManifest& result, std::string* reason) const
{
    if (bytes.size() > limits_.maximumFileSize)
        return SetReason(reason, "historical scene exceeds file-size limit");
    if (bytes.empty())
        return SetReason(reason, "historical scene is empty");
    EditorSceneManifest candidate;
    candidate.sourceFile = std::move(sourceName);
    candidate.totalSize = bytes.size();
    EditorBinaryReader reader(bytes.data(), bytes.size());
    Parser parser(limits_, candidate, reason);
    if (!parser.Parse(reader))
        return false;
    result = std::move(candidate);
    return true;
}
