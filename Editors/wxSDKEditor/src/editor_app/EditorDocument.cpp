#include "editor_app/EditorDocument.h"

#include "editor_model/EditorTreePathListImport.h"
#include "editor_model/EditorTreeSnapshot.h"

#include <utility>

EditorDocument::EditorDocument()
{
    NewDocument();
}

void EditorDocument::NewDocument()
{
    model_ = EditorTreeModel::CreateDemoScene();
    selection_.Clear();
    history_.Clear();
    filePath_.clear();
    importedDirty_ = false;
    CaptureSavedState();
}

bool EditorDocument::LoadFromSnapshot(
    const std::filesystem::path& path, std::string* reason)
{
    EditorTreeModel loaded;
    if (!LoadEditorTreeSnapshot(loaded, path, reason))
        return false;

    std::string snapshot;
    if (!SerializeEditorTreeSnapshot(loaded, snapshot, reason))
        return false;

    model_ = std::move(loaded);
    selection_.Clear();
    history_.Clear();
    filePath_ = path;
    savedSnapshot_ = std::move(snapshot);
    importedDirty_ = false;
    return true;
}

bool EditorDocument::Save(std::string* reason)
{
    if (!HasFilePath())
    {
        if (reason) *reason = "The document has no file path.";
        return false;
    }
    return SaveAs(filePath_, reason);
}

bool EditorDocument::SaveAs(
    const std::filesystem::path& path, std::string* reason)
{
    if (!SaveEditorTreeSnapshot(model_, path, reason))
        return false;

    filePath_ = path;
    importedDirty_ = false;
    return CaptureSavedState(reason);
}

bool EditorDocument::ImportPathList(std::string_view text, std::string* reason)
{
    if (!ImportEditorTreePathList(model_, text, reason))
        return false;

    selection_.Clear();
    history_.Clear();
    filePath_.clear();
    importedDirty_ = true;
    return true;
}

bool EditorDocument::ReplaceWithConvertedModel(
    EditorTreeModel model, std::string* reason)
{
    std::string snapshot;
    if (!SerializeEditorTreeSnapshot(model, snapshot, reason))
        return false;

    model_ = std::move(model);
    selection_.Clear();
    history_.Clear();
    filePath_.clear();
    savedSnapshot_.clear();
    importedDirty_ = true;
    if (reason)
        reason->clear();
    return true;
}

bool EditorDocument::IsModified() const
{
    if (importedDirty_)
        return true;

    std::string current;
    if (!SerializeEditorTreeSnapshot(model_, current, nullptr))
        return true;
    return current != savedSnapshot_;
}

std::string EditorDocument::GetDisplayName() const
{
    return HasFilePath() ? filePath_.filename().string() : "Untitled";
}

bool EditorDocument::CaptureSavedState(std::string* reason)
{
    return SerializeEditorTreeSnapshot(model_, savedSnapshot_, reason);
}
