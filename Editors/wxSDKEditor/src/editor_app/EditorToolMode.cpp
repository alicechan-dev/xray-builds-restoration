#include "editor_app/EditorToolMode.h"

const char* EditorToolModeName(EditorToolMode mode)
{
    switch (mode)
    {
    case EditorToolMode::Select: return "Select";
    case EditorToolMode::Move: return "Move";
    case EditorToolMode::PlaceObject: return "Place Object";
    case EditorToolMode::PlaceLight: return "Place Light";
    case EditorToolMode::PlaceAsset: return "Place Asset";
    }
    return "Select";
}
