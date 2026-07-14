#include "editor_scene/objects/EditorHistoricalSceneObjectRecord.h"

const char* ToString(EditorHistoricalObjectDecodeStatus status)
{
    switch (status)
    {
    case EditorHistoricalObjectDecodeStatus::Supported: return "Supported";
    case EditorHistoricalObjectDecodeStatus::Partial: return "Partial";
    case EditorHistoricalObjectDecodeStatus::Unsupported: return "Unsupported";
    case EditorHistoricalObjectDecodeStatus::Malformed: return "Malformed";
    }
    return "Unsupported";
}
