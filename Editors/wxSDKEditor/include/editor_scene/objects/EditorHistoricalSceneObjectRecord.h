#ifndef XR_WX_SDK_EDITOR_HISTORICAL_SCENE_OBJECT_RECORD_H
#define XR_WX_SDK_EDITOR_HISTORICAL_SCENE_OBJECT_RECORD_H

#include "editor_scene/objects/EditorHistoricalGlowRecord.h"
#include "editor_scene/objects/EditorHistoricalLightRecord.h"
#include "editor_scene/objects/EditorHistoricalSpawnPointRecord.h"
#include "editor_scene/objects/EditorHistoricalObjectBodyRecord.h"

#include <cstdint>
#include <string>
#include <vector>

struct EditorHistoricalSceneObjectBodyRecord
{
    std::uint16_t version = 0;
    std::int32_t referenceVersion = 0;
    std::int32_t referenceReserved = 0;
    std::string referenceName;
    bool hasFlags = false;
    std::uint32_t flags = 0;
    EditorHistoricalFieldProvenance versionProvenance;
    EditorHistoricalFieldProvenance referenceProvenance;
    EditorHistoricalFieldProvenance flagsProvenance;
};

struct EditorHistoricalObjectBodyDecodeResult
{
    EditorHistoricalObjectDecodeStatus status =
        EditorHistoricalObjectDecodeStatus::Unsupported;
    std::string typeName = "Unknown historical object";
    bool hasBodyVersion = false;
    std::uint16_t bodyVersion = 0;
    bool hasSceneObject = false;
    EditorHistoricalSceneObjectBodyRecord sceneObject;
    bool hasGlow = false;
    EditorHistoricalGlowRecord glow;
    bool hasLight = false;
    EditorHistoricalLightRecord light;
    bool hasSpawnPoint = false;
    EditorHistoricalSpawnPointRecord spawnPoint;
    std::vector<EditorHistoricalBodyChunkRecord> unknownChunks;
    std::vector<EditorHistoricalBodyChunkRecord> unsupportedChunks;
    std::vector<std::string> diagnostics;
};

#endif
