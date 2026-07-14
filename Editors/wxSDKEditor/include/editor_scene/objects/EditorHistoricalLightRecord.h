#ifndef XR_WX_SDK_EDITOR_HISTORICAL_LIGHT_RECORD_H
#define XR_WX_SDK_EDITOR_HISTORICAL_LIGHT_RECORD_H

#include "editor_scene/objects/EditorHistoricalObjectBodyRecord.h"

#include <array>
#include <cstdint>
#include <string>

struct EditorHistoricalLightRecord
{
    std::uint16_t version = 0;
    std::uint32_t type = 0;
    std::array<float, 4> color{};
    float brightness = 0.0f;
    float range = 0.0f;
    std::array<float, 3> attenuation{};
    float cone = 0.0f;
    float virtualSize = 0.0f;
    std::uint32_t useInD3D = 0;
    bool hasFlags = false;
    std::uint32_t flags = 0;
    bool hasLightControl = false;
    std::uint32_t lightControl = 0;
    bool hasAnimationReference = false;
    std::string animationReference;
    bool hasFalloffTexture = false;
    std::string falloffTexture;
    bool hasFuzzyData = false;
    std::uint8_t fuzzyShape = 0;
    std::int16_t fuzzyPointCount = 0;
    std::size_t fuzzyPayloadSize = 0;
    bool usesLegacyD3DParams = false;
    std::array<float, 3> legacyPosition{};
    std::array<float, 3> legacyDirection{};
    EditorHistoricalFieldProvenance versionProvenance;
    EditorHistoricalFieldProvenance paramsProvenance;
    EditorHistoricalFieldProvenance useInD3DProvenance;
    EditorHistoricalFieldProvenance flagsProvenance;
    EditorHistoricalFieldProvenance lightControlProvenance;
    EditorHistoricalFieldProvenance animationProvenance;
    EditorHistoricalFieldProvenance falloffProvenance;
    EditorHistoricalFieldProvenance fuzzyProvenance;
};

const char* EditorHistoricalLightTypeName(std::uint32_t type);

#endif
