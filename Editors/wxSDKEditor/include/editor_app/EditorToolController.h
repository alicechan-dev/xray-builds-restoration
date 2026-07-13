#ifndef XR_WX_SDK_EDITOR_EDITOR_TOOL_CONTROLLER_H
#define XR_WX_SDK_EDITOR_EDITOR_TOOL_CONTROLLER_H

#include "editor_app/EditorToolMode.h"

class EditorToolController
{
public:
    EditorToolMode GetMode() const { return mode_; }
    bool SetMode(EditorToolMode mode);
    bool IsPlacementMode() const;
    bool CancelCurrentOperation();
    void Reset() { mode_ = EditorToolMode::Select; }
    const char* StatusText() const;

private:
    EditorToolMode mode_ = EditorToolMode::Select;
};

#endif
