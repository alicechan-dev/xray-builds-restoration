#ifndef XR_WX_SDK_EDITOR_HISTORICAL_GLOW_RECORD_H
#define XR_WX_SDK_EDITOR_HISTORICAL_GLOW_RECORD_H

#include "editor_scene/objects/EditorHistoricalObjectBodyRecord.h"

#include <array>
#include <cstdint>
#include <string>

struct EditorHistoricalGlowRecord
{
    std::uint16_t version = 0;
    bool hasShader = false;
    std::string shaderName;
    std::string textureName;
    float radius = 0.0f;
    bool hasFlags = false;
    std::uint16_t flags = 0;
    bool hasLegacyPosition = false;
    std::array<float, 3> legacyPosition{};
    EditorHistoricalFieldProvenance versionProvenance;
    EditorHistoricalFieldProvenance shaderProvenance;
    EditorHistoricalFieldProvenance textureProvenance;
    EditorHistoricalFieldProvenance radiusProvenance;
    EditorHistoricalFieldProvenance flagsProvenance;
};

#endif
