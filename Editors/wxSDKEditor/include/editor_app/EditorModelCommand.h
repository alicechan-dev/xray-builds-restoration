#ifndef XR_WX_SDK_EDITOR_EDITOR_MODEL_COMMAND_H
#define XR_WX_SDK_EDITOR_EDITOR_MODEL_COMMAND_H

#include "editor_app/IEditorCommand.h"
#include <functional>

class EditorTreeModel;

class EditorModelCommand final : public IEditorCommand
{
public:
    using Mutation = std::function<bool(std::string*, std::string*)>;
    EditorModelCommand(EditorTreeModel& model, std::string name,
        std::string undoSelectionPath, Mutation mutation);
    bool Execute(std::string* reason) override;
    bool Undo(std::string* reason) override;
    std::string GetName() const override { return name_; }
    std::string GetExecuteSelectionPath() const override
    { return executeSelectionPath_; }
    std::string GetUndoSelectionPath() const override
    { return undoSelectionPath_; }

private:
    EditorTreeModel& model_;
    std::string name_;
    Mutation mutation_;
    std::string before_;
    std::string after_;
    std::string undoSelectionPath_;
    std::string executeSelectionPath_;
    bool captured_ = false;
};

#endif
