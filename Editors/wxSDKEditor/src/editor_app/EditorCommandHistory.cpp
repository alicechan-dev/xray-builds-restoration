#include "editor_app/EditorCommandHistory.h"
#include "editor_app/IEditorCommand.h"

namespace { bool Fail(std::string* reason, const char* text) { if (reason) *reason = text; return false; } }

bool EditorCommandHistory::Execute(std::unique_ptr<IEditorCommand> command, std::string* reason)
{
    if (!command) return Fail(reason, "Command is missing.");
    if (!command->Execute(reason)) return false;
    selectionPath_ = command->GetExecuteSelectionPath();
    redo_.clear();
    undo_.push_back(std::move(command));
    if (undo_.size() > HistoryLimit) undo_.erase(undo_.begin());
    return true;
}

bool EditorCommandHistory::Undo(std::string* reason)
{
    if (undo_.empty()) return Fail(reason, "Nothing to undo.");
    if (!undo_.back()->Undo(reason)) return false;
    selectionPath_ = undo_.back()->GetUndoSelectionPath();
    redo_.push_back(std::move(undo_.back()));
    undo_.pop_back();
    return true;
}

bool EditorCommandHistory::Redo(std::string* reason)
{
    if (redo_.empty()) return Fail(reason, "Nothing to redo.");
    if (!redo_.back()->Execute(reason)) return false;
    selectionPath_ = redo_.back()->GetExecuteSelectionPath();
    undo_.push_back(std::move(redo_.back()));
    redo_.pop_back();
    return true;
}

std::string EditorCommandHistory::GetUndoName() const { return CanUndo() ? undo_.back()->GetName() : std::string(); }
std::string EditorCommandHistory::GetRedoName() const { return CanRedo() ? redo_.back()->GetName() : std::string(); }
void EditorCommandHistory::Clear()
{
    undo_.clear();
    redo_.clear();
    selectionPath_.clear();
}
