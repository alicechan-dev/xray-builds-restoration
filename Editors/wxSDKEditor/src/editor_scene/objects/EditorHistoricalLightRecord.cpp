#include "editor_scene/objects/EditorHistoricalLightRecord.h"

const char* EditorHistoricalLightTypeName(std::uint32_t type)
{
    switch (type)
    {
    case 1: return "Point";
    case 2: return "Spot";
    case 3: return "Directional";
    default: return "Unknown";
    }
}
