#ifndef XR_WX_SDK_EDITOR_EDITOR_COMMAND_HISTORY_H
#define XR_WX_SDK_EDITOR_EDITOR_COMMAND_HISTORY_H

#include "editor_app/IEditorCommand.h"

#include <memory>
#include <string>
#include <vector>

class EditorCommandHistory
{
public:
    bool Execute(std::unique_ptr<IEditorCommand> command, std::string* reason = nullptr);
    bool Undo(std::string* reason = nullptr);
    bool Redo(std::string* reason = nullptr);
    bool CanUndo() const { return !undo_.empty(); }
    bool CanRedo() const { return !redo_.empty(); }
    std::string GetUndoName() const;
    std::string GetRedoName() const;
    const std::string& GetSelectionPath() const { return selectionPath_; }
    void Clear();

private:
    static constexpr std::size_t HistoryLimit = 100;
    std::vector<std::unique_ptr<IEditorCommand>> undo_;
    std::vector<std::unique_ptr<IEditorCommand>> redo_;
    std::string selectionPath_;
};

#endif
