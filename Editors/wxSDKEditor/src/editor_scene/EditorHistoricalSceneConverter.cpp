#include "editor_scene/EditorHistoricalSceneConverter.h"

#include "editor_app/EditorDocument.h"
#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_scene/EditorHistoricalConversionReport.h"
#include "editor_scene/EditorHistoricalConversionValidation.h"
#include "editor_scene/EditorHistoricalSceneDocument.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>
#include <utility>

namespace
{
constexpr std::size_t MaximumSummary = 1024;

bool Fail(std::string* reason, const char* message)
{
    if (reason)
        *reason = message;
    return false;
}

bool HasSpecializedRecord(const EditorHistoricalObjectBodyDecodeResult& body)
{
    return body.hasSceneObject || body.hasGlow || body.hasLight ||
        body.hasSpawnPoint;
}

std::string Hex(std::uint32_t value)
{
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << value;
    return output.str();
}

std::string Float(float value)
{
    std::ostringstream output;
    output << std::setprecision(7) << value;
    return output.str();
}

void Append(std::string& destination, const std::string& text)
{
    if (text.empty() || destination.size() >= MaximumSummary)
        return;
    if (!destination.empty())
        destination += "; ";
    const std::size_t available = MaximumSummary - destination.size();
    destination += text.substr(0, available);
}

std::string SafeLabel(std::string value, std::size_t index)
{
    for (char& character : value)
    {
        const unsigned char byte = static_cast<unsigned char>(character);
        if (character == '/' || character == '\\' || byte < 0x20)
            character = '_';
    }
    return value.empty() ? "historical_object_" + std::to_string(index) : value;
}

EditorHistoricalConversionClassCount& ClassCount(
    EditorHistoricalConversionReport& report, std::uint32_t classId)
{
    const auto found = std::find_if(report.perClass.begin(),
        report.perClass.end(), [classId](const auto& item) {
            return item.classId == classId;
        });
    if (found != report.perClass.end())
        return *found;
    report.perClass.push_back({classId});
    return report.perClass.back();
}

void CountDisposition(EditorHistoricalConversionReport& report,
    EditorHistoricalConversionClassCount& classCount,
    EditorHistoricalConversionDisposition disposition)
{
    switch (disposition)
    {
    case EditorHistoricalConversionDisposition::FullyConverted:
        ++report.fullyConverted; ++classCount.fullyConverted; break;
    case EditorHistoricalConversionDisposition::PartiallyConverted:
        ++report.partiallyConverted; ++classCount.partiallyConverted; break;
    case EditorHistoricalConversionDisposition::PlaceholderConverted:
        ++report.placeholderConverted; ++classCount.placeholderConverted; break;
    case EditorHistoricalConversionDisposition::Skipped:
        ++report.skipped; ++classCount.skipped; break;
    }
}

std::string CategoryFor(const EditorHistoricalSceneObjectData& source,
    EditorHistoricalConversionDisposition disposition)
{
    if (disposition ==
        EditorHistoricalConversionDisposition::PlaceholderConverted)
        return "historical unsupported class " +
            std::to_string(source.classId);
    if (source.bodyDecode.hasSceneObject) return "historical scene object";
    if (source.bodyDecode.hasGlow) return "historical glow";
    if (source.bodyDecode.hasLight) return "historical light";
    if (source.bodyDecode.hasSpawnPoint) return "historical spawn point";
    return "historical unsupported class " + std::to_string(source.classId);
}

void MapSafeFields(const EditorHistoricalSceneObjectData& source,
    EditorHistoricalOriginMetadata& origin,
    EditorHistoricalConversionReport& report)
{
    const EditorHistoricalObjectBodyDecodeResult& body = source.bodyDecode;
    if (body.hasSceneObject)
    {
        const auto& object = body.sceneObject;
        origin.referenceName = object.referenceName;
        Append(origin.retainedFieldSummary, "reference=" + object.referenceName);
        if (object.hasFlags)
            Append(origin.retainedFieldSummary, "flags=" + Hex(object.flags));
        if (!body.unsupportedChunks.empty())
        {
            Append(origin.retainedWarningSummary,
                "Motion or other retained chunks were not converted.");
            report.AddUnsupportedField("SceneObject motion/retained chunks");
        }
    }
    else if (body.hasGlow)
    {
        const auto& glow = body.glow;
        origin.referenceName = glow.textureName;
        Append(origin.retainedFieldSummary, "radius=" + Float(glow.radius));
        Append(origin.retainedFieldSummary, "shader=" + glow.shaderName);
        Append(origin.retainedFieldSummary, "texture=" + glow.textureName);
        if (glow.hasFlags)
            Append(origin.retainedFieldSummary, "flags=" + Hex(glow.flags));
        if (std::isfinite(glow.radius) && glow.radius >= 0.0f)
        {
            origin.hasPreviewSize = true;
            origin.previewSize = glow.radius;
        }
    }
    else if (body.hasLight)
    {
        const auto& light = body.light;
        Append(origin.retainedFieldSummary,
            "type=" + std::string(EditorHistoricalLightTypeName(light.type)));
        Append(origin.retainedFieldSummary, "brightness=" + Float(light.brightness));
        Append(origin.retainedFieldSummary, "range=" + Float(light.range));
        Append(origin.retainedFieldSummary, "cone=" + Float(light.cone));
        Append(origin.retainedFieldSummary,
            "virtual_size=" + Float(light.virtualSize));
        if (light.hasFlags)
            Append(origin.retainedFieldSummary, "flags=" + Hex(light.flags));
        if (std::isfinite(light.range) && light.range >= 0.0f)
        {
            origin.hasPreviewSize = true;
            origin.previewSize = light.range;
        }
        if (light.hasFuzzyData)
        {
            Append(origin.retainedWarningSummary,
                "Fuzzy placement payload was not converted.");
            report.AddUnsupportedField("Light fuzzy placement payload");
        }
        if (light.hasAnimationReference)
        {
            Append(origin.retainedWarningSummary,
                "Animation reference is inert and was not resolved.");
            report.AddUnsupportedField("Light animation behavior");
        }
    }
    else if (body.hasSpawnPoint)
    {
        const auto& spawn = body.spawnPoint;
        origin.referenceName = spawn.entityReference;
        Append(origin.retainedFieldSummary, "subtype=" +
            std::string(EditorHistoricalSpawnPointTypeName(spawn.type)));
        if (spawn.hasEntityReference)
            Append(origin.retainedFieldSummary,
                "section=" + spawn.entityReference);
        if (spawn.hasFlags)
            Append(origin.retainedFieldSummary, "flags=" + Hex(spawn.flags));
        if (spawn.hasRespawnPoint)
            Append(origin.retainedFieldSummary, "respawn_team=" +
                std::to_string(spawn.respawnTeam) + ", respawn_type=" +
                std::to_string(spawn.respawnType));
        if (spawn.hasEnvironmentModifier)
        {
            Append(origin.retainedFieldSummary,
                "environment_radius=" + Float(spawn.environmentRadius));
            Append(origin.retainedFieldSummary,
                "environment_power=" + Float(spawn.environmentPower));
            Append(origin.retainedFieldSummary,
                "environment_view_distance=" +
                Float(spawn.environmentViewDistance));
            origin.hasPreviewSize = true;
            origin.previewSize = spawn.environmentRadius;
        }
        if (spawn.hasRuntimePacket)
            Append(origin.opaqueDataSummary, "runtime_packet_bytes=" +
                std::to_string(spawn.runtimePacketSize));
        if (spawn.hasAttachedObject)
            Append(origin.opaqueDataSummary, "attached_object_bytes=" +
                std::to_string(spawn.attachedObjectSize));
        if (!origin.opaqueDataSummary.empty())
        {
            Append(origin.retainedWarningSummary,
                "Runtime packet/attachment bytes were not copied.");
            report.AddOpaqueSummary("SpawnPoint packet/attachment data retained "
                "only as bounded size metadata.");
            report.AddUnsupportedField("SpawnPoint runtime packet/attachment bytes");
        }
    }
}

bool BuildCandidate(const EditorHistoricalSceneDocument& source,
    const EditorHistoricalConversionOptions& options,
    EditorTreeModel& candidate, EditorHistoricalConversionReport& report,
    std::string* reason)
{
    if (!source.IsLoaded())
        return Fail(reason, "No historical scene is loaded.");

    report = {};
    report.sourceSceneDisplayName = source.GetDisplayName();
    report.sourceVersion = source.GetSceneVersion();
    report.totalHistoricalRecords = source.Objects().size();
    EditorTreeNode& root = candidate.CreateRoot("Converted Historical Scene",
        "editable historical conversion", EditorItemKind::Root);
    EditorTreeNode& objects = candidate.AddChild(root, "Objects",
        "converted historical objects", EditorItemKind::Folder);

    for (const EditorHistoricalSceneObjectData& sourceObject : source.Objects())
    {
        EditorHistoricalConversionClassCount& classCount =
            ClassCount(report, sourceObject.classId);
        ++classCount.total;
        const EditorHistoricalConversionDisposition disposition =
            DetermineHistoricalConversionDisposition(sourceObject.bodyDecode.status,
                HasSpecializedRecord(sourceObject.bodyDecode), options);
        CountDisposition(report, classCount, disposition);
        if (disposition == EditorHistoricalConversionDisposition::Skipped)
        {
            if (sourceObject.bodyDecode.status ==
                EditorHistoricalObjectDecodeStatus::Malformed)
                report.AddWarning("Malformed historical records were skipped.");
            continue;
        }
        if (!sourceObject.transformConfirmed)
            ++report.recordsWithoutTransforms;

        const std::string label = candidate.MakeUniqueChildName(objects,
            SafeLabel(sourceObject.sourceName, sourceObject.objectIndex));
        EditorTreeNode& node = candidate.AddChild(objects, label,
            CategoryFor(sourceObject, disposition), EditorItemKind::Object);
        if (sourceObject.transformConfirmed &&
            !candidate.SetNodeTransform(node, sourceObject.transform, reason))
            return false;

        EditorHistoricalOriginMetadata origin;
        origin.sourceSceneName = source.GetDisplayName();
        origin.sourceSceneVersion = sourceObject.sceneVersion;
        origin.sourceClassId = sourceObject.classId;
        origin.sourceObjectIndex = sourceObject.objectIndex;
        origin.decodeStatus = ToString(sourceObject.bodyDecode.status);
        origin.disposition = disposition;
        origin.sourceTransformConfirmed = sourceObject.transformConfirmed;
        origin.placeholder = disposition ==
            EditorHistoricalConversionDisposition::PlaceholderConverted;
        if (options.preserveSourceProvenance)
        {
            origin.sourceName = sourceObject.sourceName;
            origin.sourceOffset = sourceObject.sourceOffset;
            origin.sourceStableRecordId = sourceObject.stableRecordId;
        }
        if (!sourceObject.diagnosticsSummary.empty())
            Append(origin.retainedWarningSummary,
                sourceObject.diagnosticsSummary);
        if (origin.placeholder)
        {
            Append(origin.retainedWarningSummary,
                "Specialized historical body is unsupported; placeholder only.");
            report.AddUnsupportedField("Generic historical object body");
        }
        else
            MapSafeFields(sourceObject, origin, report);
        if (!sourceObject.transformConfirmed)
            Append(origin.retainedWarningSummary,
                "Source transform was not confirmed; viewport placement omitted.");
        candidate.SetNodeHistoricalOrigin(node, std::move(origin));
    }

    std::sort(report.perClass.begin(), report.perClass.end(),
        [](const auto& left, const auto& right) {
            return left.classId < right.classId;
        });
    if (!VerifyHistoricalSceneConversion(candidate, report, nullptr, reason))
        return false;
    std::string snapshot;
    if (!SerializeEditorTreeSnapshot(candidate, snapshot, reason))
        return false;
    report.estimatedSnapshotBytes = snapshot.size();
    return true;
}
}

bool DryRunHistoricalSceneConversion(
    const EditorHistoricalSceneDocument& source,
    const EditorHistoricalConversionOptions& options,
    EditorHistoricalConversionReport& report, std::string* reason)
{
    EditorTreeModel candidate;
    return BuildCandidate(source, options, candidate, report, reason);
}

bool ConvertHistoricalSceneToEditableDocument(
    const EditorHistoricalSceneDocument& source,
    const EditorHistoricalConversionOptions& options,
    EditorDocument& destination, EditorHistoricalConversionReport& report,
    std::string* reason)
{
    EditorTreeModel candidate;
    EditorHistoricalConversionReport candidateReport;
    if (!BuildCandidate(source, options, candidate, candidateReport, reason))
        return false;
    if (!destination.ReplaceWithConvertedModel(std::move(candidate), reason))
        return false;
    report = std::move(candidateReport);
    return true;
}
