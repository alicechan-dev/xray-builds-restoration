#include "editor_app/EditorToolController.h"

bool EditorToolController::SetMode(EditorToolMode mode)
{
    const bool changed = mode_ != mode;
    mode_ = mode;
    return changed;
}

bool EditorToolController::IsPlacementMode() const
{
    return mode_ == EditorToolMode::PlaceObject ||
        mode_ == EditorToolMode::PlaceLight;
}

bool EditorToolController::CancelCurrentOperation()
{
    if (mode_ == EditorToolMode::Select)
        return false;
    mode_ = EditorToolMode::Select;
    return true;
}

const char* EditorToolController::StatusText() const
{
    switch (mode_)
    {
    case EditorToolMode::Select: return "Select preview objects.";
    case EditorToolMode::Move: return "Move selected objects on X/Z axes.";
    case EditorToolMode::PlaceObject: return "Click viewport to place a demo object.";
    case EditorToolMode::PlaceLight: return "Click viewport to place a demo light.";
    }
    return "Select preview objects.";
}
