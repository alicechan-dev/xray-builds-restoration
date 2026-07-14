#include "editor_scene/objects/EditorHistoricalSpawnPointRecord.h"

const char* EditorHistoricalSpawnPointTypeName(std::uint32_t type)
{
    switch (type)
    {
    case 0: return "Respawn Point";
    case 1: return "Environment Modifier";
    case 2: return "Runtime Entity";
    default: return "Unknown";
    }
}
