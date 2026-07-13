#ifndef XR_WX_SDK_EDITOR_EDITOR_TOOL_MODE_H
#define XR_WX_SDK_EDITOR_EDITOR_TOOL_MODE_H

enum class EditorToolMode
{
    Select,
    Move,
    PlaceObject,
    PlaceLight
};

const char* EditorToolModeName(EditorToolMode mode);

#endif
