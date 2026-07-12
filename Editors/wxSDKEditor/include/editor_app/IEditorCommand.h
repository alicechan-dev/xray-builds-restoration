#ifndef XR_WX_SDK_EDITOR_I_EDITOR_COMMAND_H
#define XR_WX_SDK_EDITOR_I_EDITOR_COMMAND_H

#include <string>

class IEditorCommand
{
public:
    virtual ~IEditorCommand() = default;
    virtual bool Execute(std::string* reason) = 0;
    virtual bool Undo(std::string* reason) = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetExecuteSelectionPath() const { return {}; }
    virtual std::string GetUndoSelectionPath() const { return {}; }
};

#endif
