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
    properties.Add(Property("historical_type", "Historical Type",
        object.bodyDecode.typeName));
    properties.Add(Property("decode_status", "Decode Status",
        ToString(object.bodyDecode.status)));
    if (object.bodyDecode.hasBodyVersion)
        properties.Add(Property("body_version", "Body Version",
            Hex(object.bodyDecode.bodyVersion)));
    if (object.bodyDecode.hasSceneObject)
    {
        const EditorHistoricalSceneObjectBodyRecord& sceneObject =
            object.bodyDecode.sceneObject;
        properties.Add(Property("scene_object.reference", "Reference Name",
            sceneObject.referenceName));
        properties.Add(Property("scene_object.reference_version",
            "Reference Version", std::to_string(sceneObject.referenceVersion)));
        properties.Add(Property("scene_object.reference_reserved",
            "Reference Reserved", std::to_string(sceneObject.referenceReserved)));
        properties.Add(Property("scene_object.flags", "Scene Object Flags",
            sceneObject.hasFlags ? Hex(sceneObject.flags) : "not present"));
        properties.Add(Property("scene_object.unknown_chunks",
            "Unknown Body Chunks",
            std::to_string(object.bodyDecode.unknownChunks.size())));
        properties.Add(Property("scene_object.unsupported_chunks",
            "Retained Unsupported Chunks",
            std::to_string(object.bodyDecode.unsupportedChunks.size())));
    }
    if (object.bodyDecode.hasGlow)
    {
        const EditorHistoricalGlowRecord& glow = object.bodyDecode.glow;
        properties.Add(Property("glow.shader", "Glow Shader",
            glow.hasShader ? glow.shaderName : "not present"));
        properties.Add(Property("glow.texture", "Glow Texture",
            glow.textureName));
        properties.Add(Property("glow.radius", "Glow Radius",
            std::to_string(glow.radius)));
        properties.Add(Property("glow.flags", "Glow Flags",
            glow.hasFlags ? Hex(glow.flags) : "not present"));
        properties.Add(Property("glow.texture_source_offset",
            "Glow Texture Source Offset",
            std::to_string(glow.textureProvenance.sourceOffset)));
        properties.Add(Property("glow.radius_source_offset",
            "Glow Radius Source Offset",
            std::to_string(glow.radiusProvenance.sourceOffset)));
        properties.Add(Property("glow.unknown_chunks", "Unknown Body Chunks",
            std::to_string(object.bodyDecode.unknownChunks.size())));
        properties.Add(Property("glow.unsupported_chunks",
            "Retained Unsupported Chunks",
            std::to_string(object.bodyDecode.unsupportedChunks.size())));
    }
    if (object.bodyDecode.hasLight)
    {
        const EditorHistoricalLightRecord& light = object.bodyDecode.light;
        properties.Add(Property("light.type", "Light Type",
            EditorHistoricalLightTypeName(light.type)));
        properties.Add(Property("light.color", "Light Color RGBA",
            std::to_string(light.color[0]) + ", " +
            std::to_string(light.color[1]) + ", " +
            std::to_string(light.color[2]) + ", " +
            std::to_string(light.color[3])));
        properties.Add(Property("light.brightness", "Light Brightness",
            std::to_string(light.brightness)));
        properties.Add(Property("light.range", "Light Range",
            std::to_string(light.range)));
        properties.Add(Property("light.attenuation", "Light Attenuation",
            std::to_string(light.attenuation[0]) + ", " +
            std::to_string(light.attenuation[1]) + ", " +
            std::to_string(light.attenuation[2])));
        properties.Add(Property("light.cone", "Light Cone",
            std::to_string(light.cone)));
        properties.Add(Property("light.virtual_size", "Light Virtual Size",
            std::to_string(light.virtualSize)));
        properties.Add(Property("light.use_in_d3d", "Use In D3D",
            light.useInD3D ? "true" : "false"));
        properties.Add(Property("light.flags", "Light Flags",
            light.hasFlags ? Hex(light.flags) : "not present"));
        properties.Add(Property("light.control", "Light Control",
            light.hasLightControl ? std::to_string(light.lightControl) :
                "not present"));
        properties.Add(Property("light.animation", "Animation Reference",
            light.hasAnimationReference ? light.animationReference :
                "not present"));
        properties.Add(Property("light.falloff", "Falloff Texture",
            light.hasFalloffTexture ? light.falloffTexture : "not present"));
        properties.Add(Property("light.fuzzy", "Fuzzy Placement",
            light.hasFuzzyData ?
                (std::string(light.fuzzyShape == 0 ? "sphere, " : "box, ") +
                    std::to_string(light.fuzzyPointCount) + " points") :
                "not present"));
        properties.Add(Property("light.params_source_offset",
            "Light Params Source Offset",
            std::to_string(light.paramsProvenance.sourceOffset)));
        properties.Add(Property("light.unknown_chunks", "Unknown Body Chunks",
            std::to_string(object.bodyDecode.unknownChunks.size())));
        properties.Add(Property("light.unsupported_chunks",
            "Retained Unsupported Chunks",
            std::to_string(object.bodyDecode.unsupportedChunks.size())));
    }
    if (object.bodyDecode.hasSpawnPoint)
    {
        const EditorHistoricalSpawnPointRecord& spawn =
            object.bodyDecode.spawnPoint;
        properties.Add(Property("spawn.type", "Spawn Point Type",
            EditorHistoricalSpawnPointTypeName(spawn.type)));
        properties.Add(Property("spawn.entity_reference", "Entity Reference",
            spawn.hasEntityReference ? spawn.entityReference : "not present"));
        properties.Add(Property("spawn.runtime_packet", "Runtime Packet",
            spawn.hasRuntimePacket ?
                ("opaque, " + std::to_string(spawn.runtimePacketSize) +
                    " bytes") : "not present"));
        properties.Add(Property("spawn.attached_object", "Attached Object",
            spawn.hasAttachedObject ?
                ("opaque, " + std::to_string(spawn.attachedObjectSize) +
                    " bytes") : "not present"));
        properties.Add(Property("spawn.flags", "Spawn Point Flags",
            spawn.hasFlags ? Hex(spawn.flags) : "not present"));
        if (spawn.hasRespawnPoint)
        {
            properties.Add(Property("spawn.respawn_team", "Respawn Team",
                std::to_string(spawn.respawnTeam)));
            properties.Add(Property("spawn.respawn_type", "Respawn Type",
                std::to_string(spawn.respawnType)));
        }
        if (spawn.hasEnvironmentModifier)
        {
            properties.Add(Property("spawn.environment_radius",
                "Environment Radius", std::to_string(spawn.environmentRadius)));
            properties.Add(Property("spawn.environment_power",
                "Environment Power", std::to_string(spawn.environmentPower)));
            properties.Add(Property("spawn.environment_view_distance",
                "Environment View Distance",
                std::to_string(spawn.environmentViewDistance)));
            properties.Add(Property("spawn.environment_fog_density",
                "Environment Fog Density",
                std::to_string(spawn.environmentFogDensity)));
            properties.Add(Property("spawn.environment_fog_color",
                "Environment Fog Color", Hex(spawn.environmentFogColor)));
            properties.Add(Property("spawn.environment_ambient_color",
                "Environment Ambient Color",
                Hex(spawn.environmentAmbientColor)));
            properties.Add(Property("spawn.environment_lightmap_color",
                "Environment Light Map Color",
                Hex(spawn.environmentLightMapColor)));
        }
        properties.Add(Property("spawn.type_source_offset",
            "Spawn Metadata Source Offset",
            std::to_string(spawn.hasEntityReference ?
                spawn.entityReferenceProvenance.sourceOffset :
                spawn.typeProvenance.sourceOffset)));
        properties.Add(Property("spawn.unknown_chunks", "Unknown Body Chunks",
            std::to_string(object.bodyDecode.unknownChunks.size())));
        properties.Add(Property("spawn.unsupported_chunks",
            "Retained Opaque/Unsupported Chunks",
            std::to_string(object.bodyDecode.unsupportedChunks.size())));
    }
    properties.Add(Property("source_chunk_path", "Source Chunk Path",
        object.chunkPath));
    properties.Add(Property("source_offset", "Source Offset",
        std::to_string(object.sourceOffset)));
    properties.Add(Property("from_decompressed_payload",
        "From Decompressed Payload",
        object.fromDecompressedPayload ? "true" : "false"));
    if (object.fromDecompressedPayload)
    {
        properties.Add(Property("compressed_source_offset",
            "Compressed Source Offset",
            std::to_string(object.compressedSourceOffset)));
        properties.Add(Property("decompressed_offset",
            "Decoded Stream Offset",
            std::to_string(object.decompressedOffset)));
    }
    properties.Add(Property("scene_version", "Scene Version",
        std::to_string(object.sceneVersion)));
    properties.Add(Property("read_only", "Read-Only", "true"));
    properties.Add(Property("diagnostics", "Diagnostics",
        object.diagnosticsSummary.empty() ? "none" : object.diagnosticsSummary));
    return properties;
}
