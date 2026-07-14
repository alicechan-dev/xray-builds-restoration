#ifndef XR_WX_SDK_EDITOR_HISTORICAL_SPAWN_POINT_RECORD_H
#define XR_WX_SDK_EDITOR_HISTORICAL_SPAWN_POINT_RECORD_H

#include "editor_scene/objects/EditorHistoricalObjectBodyRecord.h"

#include <cstddef>
#include <cstdint>
#include <string>

struct EditorHistoricalSpawnPointRecord
{
    std::uint16_t version = 0;
    std::uint32_t type = 0;
    bool hasEntityReference = false;
    std::string entityReference;
    bool hasRuntimePacket = false;
    std::size_t runtimePacketSize = 0;
    bool hasAttachedObject = false;
    std::size_t attachedObjectSize = 0;
    bool hasFlags = false;
    std::uint32_t flags = 0;
    bool hasRespawnPoint = false;
    std::uint8_t respawnTeam = 0;
    std::uint8_t respawnType = 0;
    std::uint16_t respawnReserved = 0;
    bool hasEnvironmentModifier = false;
    float environmentRadius = 0.0f;
    float environmentPower = 0.0f;
    float environmentViewDistance = 0.0f;
    std::uint32_t environmentFogColor = 0;
    float environmentFogDensity = 0.0f;
    std::uint32_t environmentAmbientColor = 0;
    std::uint32_t environmentLightMapColor = 0;
    EditorHistoricalFieldProvenance versionProvenance;
    EditorHistoricalFieldProvenance typeProvenance;
    EditorHistoricalFieldProvenance entityReferenceProvenance;
    EditorHistoricalFieldProvenance runtimePacketProvenance;
    EditorHistoricalFieldProvenance attachedObjectProvenance;
    EditorHistoricalFieldProvenance flagsProvenance;
    EditorHistoricalFieldProvenance subtypeDataProvenance;
};

const char* EditorHistoricalSpawnPointTypeName(std::uint32_t type);

#endif
