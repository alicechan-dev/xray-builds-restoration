#include "editor_app/EditorModelCommand.h"
#include "editor_model/EditorTreeSnapshot.h"

#include <utility>

EditorModelCommand::EditorModelCommand(EditorTreeModel& model,
    std::string name, std::string undoSelectionPath, Mutation mutation) :
    model_(model), name_(std::move(name)), mutation_(std::move(mutation)),
    undoSelectionPath_(std::move(undoSelectionPath)) {}

bool EditorModelCommand::Execute(std::string* reason)
{
    if (captured_)
        return DeserializeEditorTreeSnapshot(model_, after_, reason);
    if (!SerializeEditorTreeSnapshot(model_, before_, reason)) return false;
    if (!mutation_(&executeSelectionPath_, reason)) return false;
    if (!SerializeEditorTreeSnapshot(model_, after_, reason)) {
        DeserializeEditorTreeSnapshot(model_, before_, nullptr);
        return false;
    }
    captured_ = true;
    return true;
}

bool EditorModelCommand::Undo(std::string* reason)
{
    return captured_ && DeserializeEditorTreeSnapshot(model_, before_, reason);
}
