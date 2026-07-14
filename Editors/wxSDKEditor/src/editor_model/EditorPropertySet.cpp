#include "editor_model/EditorPropertySet.h"

#include "editor_assets/EditorAssetDescriptor.h"
#include "editor_assets/EditorImportedPrototype.h"
#include "editor_assets/EditorObjectLibraryResolver.h"
#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"

#include <algorithm>
#include <cctype>
#include <utility>
#include <cstdlib>
#include <cerrno>
#include <cmath>

namespace
{
bool EqualIgnoringCase(std::string_view left, std::string_view right)
{
    return left.size() == right.size() &&
        std::equal(left.begin(), left.end(), right.begin(),
            [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
}

bool ParseFiniteFloat(const std::string& text, float& value)
{
    char* end=nullptr; errno=0; value=std::strtof(text.c_str(),&end);
    return end!=text.c_str() && *end=='\0' && errno!=ERANGE && std::isfinite(value);
}

EditorProperty MakeProperty(const char* key, const char* label,
    EditorPropertyType type, std::string value, bool readOnly,
    const char* description)
{
    EditorProperty property;
    const std::string_view propertyKey(key);
    property.section = propertyKey.rfind("object_library.", 0) == 0
        ? "Historical Object Library Resolution - Read-Only"
        : propertyKey.rfind("historical.", 0) == 0
            ? "Historical Origin - Read-Only" : "Editable";
    property.key = key;
    property.label = label;
    property.type = type;
    property.value = std::move(value);
    property.readOnly = readOnly;
    property.description = description;
    return property;
}
}

void EditorPropertySet::Add(EditorProperty property)
{
    properties_.push_back(std::move(property));
}

const EditorProperty* EditorPropertySet::Find(std::string_view key) const
{
    const auto found = std::find_if(properties_.begin(), properties_.end(),
        [key](const EditorProperty& property) {
            return EqualIgnoringCase(property.key, key);
        });
    return found == properties_.end() ? nullptr : &*found;
}

EditorPropertySet BuildEditorNodePropertySet(const EditorTreeNode& node,
    const EditorAssetDescriptor* resolvedAsset,
    const EditorObjectResolutionResult* objectResolution)
{
    EditorPropertySet properties;
    properties.Add(MakeProperty("label", "Label", EditorPropertyType::String,
        node.Label(), false, "Tree label and generated path component."));
    properties.Add(MakeProperty("category", "Category", EditorPropertyType::String,
        node.Category(), false, "Development-only display category."));
    properties.Add(MakeProperty("kind", "Kind", EditorPropertyType::ReadOnlyText,
        std::string(ToString(node.Kind())), true, "Audited structural item kind."));
    properties.Add(MakeProperty("path", "Path", EditorPropertyType::ReadOnlyText,
        node.Path(), true, "Generated canonical hierarchy path."));
    properties.Add(MakeProperty("asset_id", "Asset ID",
        EditorPropertyType::ReadOnlyText,
        node.AssetId().empty() ? "none" : node.AssetId(), true,
        "Optional synthetic catalog prototype identity."));
    if (IsImportedAssetId(node.AssetId()))
    {
        const bool resolved = resolvedAsset && resolvedAsset->sourceKind ==
            EditorAssetSourceKind::ImportedSpawnMetadata;
        const std::string section = resolved &&
                !resolvedAsset->sourceSection.empty()
            ? resolvedAsset->sourceSection
            : ImportedSectionFromAssetId(node.AssetId());
        properties.Add(MakeProperty("prototype_section", "Prototype Section",
            EditorPropertyType::ReadOnlyText, section, true,
            "Stable imported metadata section identity."));
        properties.Add(MakeProperty("metadata_resolution", "Metadata",
            EditorPropertyType::ReadOnlyText,
            resolved ? "resolved" : "unresolved", true,
            "Whether the session catalog currently resolves this prototype."));
        if (resolved && !resolvedAsset->sourceFile.empty())
            properties.Add(MakeProperty("metadata_source", "Metadata Source",
                EditorPropertyType::ReadOnlyText, resolvedAsset->sourceFile,
                true, "Read-only metadata provenance for this session."));
    }
    if (node.HistoricalOrigin())
    {
        const EditorHistoricalOriginMetadata& origin = *node.HistoricalOrigin();
        properties.Add(MakeProperty("historical.scene_name",
            "Source Scene", EditorPropertyType::ReadOnlyText,
            origin.sourceSceneName, true,
            "Original scene filename only; absolute paths are never retained."));
        properties.Add(MakeProperty("historical.class_id",
            "Historical Class ID", EditorPropertyType::ReadOnlyText,
            std::to_string(origin.sourceClassId), true,
            "Original historical object class ID."));
        properties.Add(MakeProperty("historical.source_name",
            "Historical Name", EditorPropertyType::ReadOnlyText,
            origin.sourceName.empty() ? "unnamed" : origin.sourceName, true,
            "Original source object name; never written back to .level."));
        properties.Add(MakeProperty("historical.object_index",
            "Historical Object Index", EditorPropertyType::ReadOnlyText,
            std::to_string(origin.sourceObjectIndex), true,
            "Original manifest record index."));
        properties.Add(MakeProperty("historical.source_offset",
            "Historical Source Offset", EditorPropertyType::ReadOnlyText,
            std::to_string(origin.sourceOffset), true,
            "Original source byte offset retained as inert provenance."));
        properties.Add(MakeProperty("historical.stable_id",
            "Stable Historical ID", EditorPropertyType::ReadOnlyText,
            origin.sourceStableRecordId.empty() ? "not retained" :
                origin.sourceStableRecordId, true,
            "Immutable source-record identity retained as provenance."));
        properties.Add(MakeProperty("historical.decode_status",
            "Historical Decode Status", EditorPropertyType::ReadOnlyText,
            origin.decodeStatus, true, "Specialized decoder result."));
        properties.Add(MakeProperty("historical.disposition",
            "Conversion Disposition", EditorPropertyType::ReadOnlyText,
            ToString(origin.disposition), true,
            "How this node was migrated into the editable snapshot."));
        if (!origin.referenceName.empty())
        {
            properties.Add(MakeProperty("historical.reference",
                "Historical Reference", EditorPropertyType::ReadOnlyText,
                origin.referenceName, true,
                "Inert historical reference; resolution is session-only."));
            const EditorObjectResolutionState state = objectResolution ?
                objectResolution->state : EditorObjectResolutionState::LibraryNotLoaded;
            properties.Add(MakeProperty("object_library.resolution",
                "Resolution", EditorPropertyType::ReadOnlyText,
                ToString(state), true, "Session-only Object Library lookup state."));
            if (objectResolution)
            {
                properties.Add(MakeProperty("object_library.normalized",
                    "Normalized Reference", EditorPropertyType::ReadOnlyText,
                    objectResolution->normalizedQuery.empty() ? "unavailable" :
                        objectResolution->normalizedQuery, true,
                    "Historical lowercase, extensionless, root-relative identity."));
                if (objectResolution->entry)
                {
                    const auto& entry = *objectResolution->entry;
                    properties.Add(MakeProperty("object_library.matched_id",
                        "Matched ID", EditorPropertyType::ReadOnlyText,
                        entry.referenceId, true, "Resolved library identity."));
                    properties.Add(MakeProperty("object_library.kind",
                        "Object Kind", EditorPropertyType::ReadOnlyText,
                        ToString(entry.kind), true, "Source-confirmed flags/bone classification."));
                    properties.Add(MakeProperty("object_library.source",
                        "Source Relative File", EditorPropertyType::ReadOnlyText,
                        entry.sourceRelativeFile, true, "No absolute library path is retained."));
                    properties.Add(MakeProperty("object_library.counts",
                        "Mesh / Surface Counts", EditorPropertyType::ReadOnlyText,
                        std::to_string(entry.meshCount) + " / " +
                            std::to_string(entry.surfaceCount), true,
                        "Metadata counts only; mesh payloads remain unloaded."));
                    properties.Add(MakeProperty("object_library.motion",
                        "Motion Present", EditorPropertyType::ReadOnlyText,
                        entry.motionPresent ? "yes" : "no", true,
                        "Presence of historical motion chunks; motions remain unloaded."));
                    properties.Add(MakeProperty("object_library.bounds",
                        "Bounds", EditorPropertyType::ReadOnlyText,
                        "unavailable without mesh payload decoding", true,
                        "Build-1935 computes object bounds from loaded meshes."));
                    properties.Add(MakeProperty("object_library.parse_status",
                        "Parse Status", EditorPropertyType::ReadOnlyText,
                        ToString(entry.parseStatus), true, "Bounded metadata parser status."));
                }
                else if (!objectResolution->reason.empty())
                    properties.Add(MakeProperty("object_library.reason",
                        "Reason", EditorPropertyType::ReadOnlyText,
                        objectResolution->reason, true, "Resolution diagnostic."));
            }
        }
        if (!origin.retainedFieldSummary.empty())
            properties.Add(MakeProperty("historical.fields",
                "Preserved Historical Fields",
                EditorPropertyType::ReadOnlyText,
                origin.retainedFieldSummary, true,
                "Bounded summary of source-confirmed inert values."));
        if (!origin.retainedWarningSummary.empty())
            properties.Add(MakeProperty("historical.warnings",
                "Migration Warnings", EditorPropertyType::ReadOnlyText,
                origin.retainedWarningSummary, true,
                "Historical behavior or payload not migrated."));
        if (!origin.opaqueDataSummary.empty())
            properties.Add(MakeProperty("historical.opaque",
                "Excluded Opaque Data", EditorPropertyType::ReadOnlyText,
                origin.opaqueDataSummary, true,
                "Size/status metadata only; raw bytes were not copied."));
    }
    if (!IsGroupKind(node.Kind()))
    {
        const EditorTransform& transform = node.Transform();
        properties.Add(MakeProperty("position.x", "Position X",
            EditorPropertyType::String, std::to_string(transform.x), false,
            "Development transform X."));
        properties.Add(MakeProperty("position.y", "Position Y",
            EditorPropertyType::String, std::to_string(transform.y), false,
            "Development transform Y."));
        properties.Add(MakeProperty("position.z", "Position Z",
            EditorPropertyType::String, std::to_string(transform.z), false,
            "Development transform Z."));
    }
    return properties;
}

EditorPropertyApplyResult ApplyEditorNodeProperty(EditorTreeModel& model,
    EditorTreeNode& node, std::string_view key, std::string value)
{
    EditorPropertyApplyResult result;
    const EditorPropertySet properties = BuildEditorNodePropertySet(node);
    const EditorProperty* property = properties.Find(key);
    if (!property)
    {
        result.reason = "Unknown property key.";
        return result;
    }
    if (property->readOnly)
    {
        result.reason = "This property is read-only.";
        return result;
    }

    if (EqualIgnoringCase(key, "label"))
    {
        if (!model.RenameNode(node, std::move(value), &result.reason))
            return result;
        result.requiresTreeRebuild = true;
    }
    else if (EqualIgnoringCase(key, "category"))
    {
        model.SetNodeCategory(node, std::move(value));
    }
    else if (key.rfind("position.", 0) == 0)
    {
        float parsed = 0.0f;
        if (!ParseFiniteFloat(value, parsed))
        {
            result.reason = "Position must be a finite number.";
            return result;
        }
        EditorTransform transform = node.Transform();
        if (EqualIgnoringCase(key, "position.x")) transform.x = parsed;
        else if (EqualIgnoringCase(key, "position.y")) transform.y = parsed;
        else if (EqualIgnoringCase(key, "position.z")) transform.z = parsed;
        else
        {
            result.reason = "Unknown position property.";
            return result;
        }
        if (!model.SetNodeTransform(node, transform, &result.reason))
            return result;
    }

    result.success = true;
    result.requiresPropertyRefresh = true;
    return result;
}
