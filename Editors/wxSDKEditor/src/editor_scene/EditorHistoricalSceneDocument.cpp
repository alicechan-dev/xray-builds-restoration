#include "editor_scene/EditorHistoricalSceneDocument.h"

#include "editor_model/EditorItemType.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace
{
bool Fail(std::string* reason, const char* message)
{
    if (reason)
        *reason = message;
    return false;
}

std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return value;
}

std::string SafeDisplayLabel(std::string value)
{
    for (char& character : value)
    {
        const unsigned char byte = static_cast<unsigned char>(character);
        if (character == '/' || character == '\\' || byte < 0x20)
            character = '_';
    }
    return value.empty() ? "unnamed" : value;
}

std::string ClassCategory(std::uint32_t classId)
{
    std::ostringstream output;
    output << "historical class 0x" << std::uppercase << std::hex
        << std::setw(4) << std::setfill('0') << classId;
    return output.str();
}

bool IsFinite(const EditorSceneObjectRecord& object)
{
    const auto finite = [](float value) { return std::isfinite(value); };
    return std::all_of(object.position.begin(), object.position.end(), finite) &&
        std::all_of(object.rotation.begin(), object.rotation.end(), finite) &&
        std::all_of(object.scale.begin(), object.scale.end(), finite);
}
}

bool EditorHistoricalSceneDocument::BuildFromManifest(
    EditorSceneManifest manifest, std::string* reason)
{
    if (!manifest.hasVersion || manifest.version != 5)
        return Fail(reason, "Historical scene manifest is not version 5.");

    EditorHistoricalSceneDocument candidate;
    candidate.manifest_ = std::move(manifest);
    candidate.sourcePath_ = std::filesystem::path(candidate.manifest_.sourceFile);
    EditorTreeNode& root = candidate.model_.CreateRoot(
        "Historical Scene", "historical scene", EditorItemKind::Root);
    EditorTreeNode& objects = candidate.model_.AddChild(root, "Objects",
        "historical objects", EditorItemKind::Folder);
    std::unordered_map<std::string, std::size_t> displayCounts;

    for (std::size_t index = 0; index < candidate.manifest_.objects.size(); ++index)
    {
        const EditorSceneObjectRecord& source = candidate.manifest_.objects[index];
        if (!source.hasClassId)
            return Fail(reason, "Historical object is missing a confirmed class ID.");

        EditorHistoricalSceneObjectData data;
        data.objectIndex = source.recordIndex;
        data.classId = source.classId;
        data.sourceName = source.hasName ? source.name : std::string();
        data.chunkPath = source.chunkPath;
        data.sourceOffset = source.sourceOffset;
        data.fromDecompressedPayload = source.fromDecompressedPayload;
        data.compressedSourceOffset = source.compressedSourceOffset;
        data.decompressedOffset = source.decompressedOffset;
        data.sceneVersion = candidate.manifest_.version;
        data.stableRecordId = "historical.object." +
            std::to_string(data.objectIndex) +
            "." + std::to_string(source.sourceOffset);
        if (source.fromDecompressedPayload)
            data.stableRecordId += ".decoded." +
                std::to_string(source.decompressedOffset);
        data.transformConfirmed = source.hasTransform && IsFinite(source);
        data.bodySupported = false;
        if (source.hasTransform)
        {
            data.transform.x = source.position[0];
            data.transform.y = source.position[1];
            data.transform.z = source.position[2];
            data.transform.pitch = source.rotation[0];
            data.transform.yaw = source.rotation[1];
            data.transform.roll = source.rotation[2];
            data.transform.sx = source.scale[0];
            data.transform.sy = source.scale[1];
            data.transform.sz = source.scale[2];
            if (!data.transformConfirmed)
                data.diagnosticsSummary =
                    "Confirmed transform layout contains non-finite values; preview omitted.";
        }
        else
            data.diagnosticsSummary = "Common transform chunk not present.";

        const std::string baseLabel = data.sourceName.empty()
            ? "unnamed_object_" + std::to_string(data.objectIndex)
            : SafeDisplayLabel(data.sourceName);
        const std::size_t duplicate = ++displayCounts[Lower(baseLabel)];
        const std::string displayLabel = duplicate == 1 ? baseLabel
            : baseLabel + " [#" + std::to_string(duplicate) + "]";
        EditorTreeNode& node = candidate.model_.AddChild(objects, displayLabel,
            ClassCategory(data.classId), EditorItemKind::Object);
        if (data.transformConfirmed)
        {
            std::string transformReason;
            if (!candidate.model_.SetNodeTransform(
                node, data.transform, &transformReason))
                return Fail(reason, "Confirmed historical transform is invalid.");
        }
        data.nodePath = node.Path();
        candidate.objects_.push_back(std::move(data));
    }

    candidate.loaded_ = true;
    *this = std::move(candidate);
    if (reason)
        reason->clear();
    return true;
}

void EditorHistoricalSceneDocument::Clear()
{
    *this = {};
}

std::string EditorHistoricalSceneDocument::GetDisplayName() const
{
    if (sourcePath_.filename().empty())
        return "Historical Scene";
    return sourcePath_.filename().string();
}

std::size_t EditorHistoricalSceneDocument::GetNamedObjectCount() const
{
    return static_cast<std::size_t>(std::count_if(objects_.begin(), objects_.end(),
        [](const EditorHistoricalSceneObjectData& object) {
            return !object.sourceName.empty();
        }));
}

std::size_t EditorHistoricalSceneDocument::GetTransformedObjectCount() const
{
    return static_cast<std::size_t>(std::count_if(objects_.begin(), objects_.end(),
        [](const EditorHistoricalSceneObjectData& object) {
            return object.transformConfirmed;
        }));
}

std::size_t
EditorHistoricalSceneDocument::GetUnsupportedCompressedChunkCount() const
{
    return static_cast<std::size_t>(std::count_if(
        manifest_.chunks.begin(), manifest_.chunks.end(),
        [](const EditorSceneChunkRecord& chunk) {
            return chunk.compressed && !chunk.decompressionSucceeded;
        }));
}

std::size_t EditorHistoricalSceneDocument::GetUnsupportedBodyCount() const
{
    return static_cast<std::size_t>(std::count_if(objects_.begin(), objects_.end(),
        [](const EditorHistoricalSceneObjectData& object) {
            return !object.bodySupported;
        }));
}

const EditorHistoricalSceneObjectData*
EditorHistoricalSceneDocument::FindByStableId(const std::string& stableId) const
{
    const auto found = std::find_if(objects_.begin(), objects_.end(),
        [&stableId](const EditorHistoricalSceneObjectData& object) {
            return object.stableRecordId == stableId;
        });
    return found == objects_.end() ? nullptr : &*found;
}

const EditorHistoricalSceneObjectData*
EditorHistoricalSceneDocument::FindByNodePath(const std::string& nodePath) const
{
    const auto found = std::find_if(objects_.begin(), objects_.end(),
        [&nodePath](const EditorHistoricalSceneObjectData& object) {
            return object.nodePath == nodePath;
        });
    return found == objects_.end() ? nullptr : &*found;
}
