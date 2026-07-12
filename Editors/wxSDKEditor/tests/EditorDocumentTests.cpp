#include "editor_app/EditorDocument.h"
#include "editor_app/EditorModelCommand.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace
{
bool AddNode(EditorDocument& document, const std::string& name,
    std::string* reason)
{
    const std::string rootPath = document.Model().Root()->Path();
    auto command = std::make_unique<EditorModelCommand>(document.Model(),
        "Add " + name, rootPath,
        [&document, rootPath, name](std::string* selectionPath,
            std::string* mutationReason) {
            EditorTreeNode* root = document.Model().FindByPath(rootPath);
            if (!root)
            {
                if (mutationReason) *mutationReason = "Root is missing.";
                return false;
            }
            EditorTreeNode& child = document.Model().AddChild(
                *root, name, "document test");
            *selectionPath = child.Path();
            return true;
        });
    return document.History().Execute(std::move(command), reason);
}
}

int RunEditorDocumentTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition)
            return;
        ++failures;
        std::cerr << "FAIL: document " << message << '\n';
    };

    EditorDocument document;
    check(!document.IsModified() && !document.HasFilePath() &&
        document.GetDisplayName() == "Untitled",
        "new document is clean and untitled");

    std::string reason;
    auto failed = std::make_unique<EditorModelCommand>(document.Model(),
        "Failed mutation", document.Model().Root()->Path(),
        [](std::string*, std::string* mutationReason) {
            if (mutationReason) *mutationReason = "Expected failure.";
            return false;
        });
    check(!document.History().Execute(std::move(failed), &reason) &&
        !document.IsModified() && !document.History().CanUndo(),
        "failed command leaves document clean and history unchanged");

    check(AddNode(document, "saved_node", &reason) && document.IsModified(),
        "successful command marks document dirty");

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        "wx_sdk_editor_document_contract.wx_tree_snapshot";
    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    check(document.SaveAs(path, &reason) && !document.IsModified() &&
        document.HasFilePath() && document.GetFilePath() == path &&
        document.GetDisplayName() == path.filename().string(),
        "save as stores path and establishes clean baseline");

    check(AddNode(document, "later_node", &reason) && document.IsModified(),
        "mutation after save is dirty");
    check(document.History().Undo(&reason) && !document.IsModified(),
        "undo to saved snapshot clears dirty state");
    check(document.History().Redo(&reason) && document.IsModified(),
        "redo away from saved snapshot restores dirty state");
    check(document.Save(&reason) && !document.IsModified(),
        "save updates existing path and clears dirty state");

    document.Selection().Select(document.Model().Root());
    check(document.LoadFromSnapshot(path, &reason) && !document.IsModified() &&
        !document.History().CanUndo() &&
        document.Selection().SelectedCount() == 0,
        "load replaces model and resets history selection and dirty state");

    const std::string beforeFailure = document.Model().Root()->Path();
    check(!document.LoadFromSnapshot(path.string() + ".missing", &reason) &&
        document.GetFilePath() == path && !document.IsModified() &&
        document.Model().Root()->Path() == beforeFailure,
        "failed load preserves document state");

    check(document.ImportPathList(
        "/Imported/Objects/item | imported object\n", &reason) &&
        document.IsModified() && !document.HasFilePath() &&
        document.GetDisplayName() == "Untitled" &&
        !document.History().CanUndo() &&
        document.Selection().SelectedCount() == 0,
        "import creates a modified untitled document baseline");
    check(!document.ImportPathList("relative/path\n", &reason) &&
        document.IsModified() &&
        document.Model().FindByPath("Imported/Objects/item") != nullptr,
        "failed import preserves imported document");

    document.NewDocument();
    check(!document.IsModified() && !document.HasFilePath() &&
        !document.History().CanUndo() &&
        document.Selection().SelectedCount() == 0,
        "new resets document services and saved state");

    std::filesystem::remove(path, removeError);
    return failures;
}
