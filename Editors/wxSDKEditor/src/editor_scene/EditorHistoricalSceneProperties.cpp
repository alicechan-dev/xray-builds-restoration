#include "editor_scene/EditorHistoricalSceneProperties.h"

#include "editor_scene/EditorHistoricalSceneDocument.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace
{
EditorProperty Property(const char* key, const char* label, std::string value)
{
    EditorProperty property;
    property.key = key;
    property.label = label;
    property.type = EditorPropertyType::ReadOnlyText;
    property.value = std::move(value);
    property.readOnly = true;
    property.description = "Read-only historical scene provenance.";
    return property;
}

std::string Hex(std::uint32_t value)
{
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << std::setw(8)
        << std::setfill('0') << value;
    return output.str();
}
}

EditorPropertySet BuildHistoricalSceneObjectPropertySet(
    const EditorHistoricalSceneObjectData& object)
{
    EditorPropertySet properties;
    properties.Add(Property("label", "Name",
        object.sourceName.empty() ? "unnamed" : object.sourceName));
    properties.Add(Property("stable_record_id", "Stable Record ID",
        object.stableRecordId));
    properties.Add(Property("object_index", "Object Index",
        std::to_string(object.objectIndex)));
    properties.Add(Property("class_id", "Class ID", Hex(object.classId)));
    properties.Add(Property("position.x", "Position X",
        std::to_string(object.transform.x)));
    properties.Add(Property("position.y", "Position Y",
        std::to_string(object.transform.y)));
    properties.Add(Property("position.z", "Position Z",
        std::to_string(object.transform.z)));
    properties.Add(Property("rotation.x", "Rotation X",
        std::to_string(object.transform.pitch)));
    properties.Add(Property("rotation.y", "Rotation Y",
        std::to_string(object.transform.yaw)));
    properties.Add(Property("rotation.z", "Rotation Z",
        std::to_string(object.transform.roll)));
    properties.Add(Property("scale.x", "Scale X",
        std::to_string(object.transform.sx)));
    properties.Add(Property("scale.y", "Scale Y",
        std::to_string(object.transform.sy)));
    properties.Add(Property("scale.z", "Scale Z",
        std::to_string(object.transform.sz)));
    properties.Add(Property("transform_confirmed", "Transform Confirmed",
        object.transformConfirmed ? "true" : "false"));
    properties.Add(Property("body_supported", "Body Supported",
        object.bodySupported ? "true" : "false"));
    properties.Add(Property("source_chunk_path", "Source Chunk Path",
        object.chunkPath));
    properties.Add(Property("source_offset", "Source Offset",
        std::to_string(object.sourceOffset)));
    properties.Add(Property("scene_version", "Scene Version",
        std::to_string(object.sceneVersion)));
    properties.Add(Property("read_only", "Read-Only", "true"));
    properties.Add(Property("diagnostics", "Diagnostics",
        object.diagnosticsSummary.empty() ? "none" : object.diagnosticsSummary));
    return properties;
}
