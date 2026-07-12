#include "editor_app/EditorTreePresenter.h"

#include "editor_model/EditorTreePathListImport.h"
#include "editor_model/EditorTreeQuery.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_ui/IDialogService.h"
#include "editor_ui/IEditorTree.h"
#include "editor_ui/IPropertyPanel.h"

#include <utility>

EditorTreePresenter::EditorTreePresenter(IEditorTree& tree,
    IPropertyPanel& properties, IDialogService& dialogs,
    MessageCallback status, MessageCallback output) :
    tree_(tree), properties_(properties), dialogs_(dialogs),
    status_(std::move(status)), output_(std::move(output))
{
}

void EditorTreePresenter::InitializeDemo()
{
    model_ = EditorTreeModel::CreateDemoScene();
    Rebuild(nullptr, true);
}

EditorTreeNode* EditorTreePresenter::SelectedNode() const
{
    return reinterpret_cast<EditorTreeNode*>(tree_.GetSelectedUserData());
}

void EditorTreePresenter::PopulateNode(
    const EditorTreeNode& node, std::uintptr_t parentItem)
{
    const auto item = parentItem == IEditorTree::InvalidItem
        ? tree_.AddRoot(node.Label().c_str())
        : tree_.AddChild(parentItem, node.Label().c_str());
    tree_.SetItemUserData(item,
        reinterpret_cast<IEditorTree::UserData>(&node));

    for (const auto& child : node.ChildrenView())
        PopulateNode(*child, item);
}

void EditorTreePresenter::Rebuild(EditorTreeNode* selectedNode, bool selectFirst)
{
    tree_.Clear();
    if (!model_.Root())
    {
        properties_.Clear();
        return;
    }

    PopulateNode(*model_.Root(), IEditorTree::InvalidItem);
    tree_.ExpandAllItems();
    if (selectedNode)
        tree_.SelectByUserData(reinterpret_cast<IEditorTree::UserData>(selectedNode));
    else if (selectFirst)
        tree_.SelectFirst();
    RefreshSelection();
}

void EditorTreePresenter::RefreshSelection()
{
    const EditorTreeNode* node = SelectedNode();
    selection_.Clear();
    selection_.Select(node);
    if (!node)
    {
        properties_.Clear();
        return;
    }

    const std::string text = "Selected: " + node->Label() +
        "\nKind: " + std::string(ToString(node->Kind())) +
        "\nType: " + node->Category() +
        "\nPath: " + node->Path() +
        "\nProperties: placeholder only"
        "\n\nNo real SDK data is loaded.";
    properties_.ShowPlaceholder(text.c_str());
}

void EditorTreePresenter::SetStatus(const std::string& message) const
{
    if (status_)
        status_(message);
}

void EditorTreePresenter::AddDemoNode(const char* baseName, const char* category)
{
    EditorTreeNode* parent = SelectedNode();
    if (!parent)
        parent = model_.Root();
    if (!parent)
        return;

    const std::string name = model_.MakeUniqueChildName(*parent, baseName);
    EditorTreeNode& added = model_.AddChild(*parent, name, category);
    Rebuild(&added);
    SetStatus("Added '" + added.Label() + "' under '" + parent->Label() + "'.");
}

void EditorTreePresenter::DeleteSelected()
{
    EditorTreeNode* selected = SelectedNode();
    if (!selected)
    {
        dialogs_.Warning("Delete rejected", "No tree node is selected.");
        return;
    }

    std::string reason;
    if (!model_.CanDeleteNode(*selected, &reason))
    {
        dialogs_.Warning("Delete rejected", reason.c_str());
        SetStatus("Delete rejected: " + reason);
        return;
    }

    const std::string label = selected->Label();
    const std::string prompt = "Delete '" + label + "' and all of its children?";
    if (!dialogs_.Confirm("Delete demo node", prompt.c_str()))
        return;

    EditorTreeNode* parent = selected->Parent();
    tree_.Clear();
    if (!model_.DeleteNode(*selected, &reason))
    {
        Rebuild(selected);
        dialogs_.Warning("Delete rejected", reason.c_str());
        return;
    }

    Rebuild(parent ? parent : model_.Root());
    SetStatus("Deleted '" + label + "'.");
}

bool EditorTreePresenter::RenameNode(
    EditorTreeNode& node, std::string newName, std::string* reason)
{
    const std::string previousLabel = node.Label();
    if (!model_.RenameNode(node, std::move(newName), reason))
    {
        SetStatus("Rename rejected: " + (reason ? *reason : std::string()));
        return false;
    }

    SetStatus("Renamed '" + previousLabel + "' to '" + node.Label() + "'.");
    RefreshSelection();
    return true;
}

bool EditorTreePresenter::LoadSnapshot(const std::filesystem::path& path)
{
    EditorTreeModel loaded;
    std::string reason;
    if (!LoadEditorTreeSnapshot(loaded, path, &reason))
    {
        dialogs_.Error("Snapshot load failed", reason.c_str());
        SetStatus("Snapshot load failed: " + reason);
        return false;
    }

    model_ = std::move(loaded);
    Rebuild(nullptr, true);
    SetStatus("Loaded demo tree snapshot.");
    return true;
}

bool EditorTreePresenter::SaveSnapshot(const std::filesystem::path& path)
{
    std::string reason;
    if (!SaveEditorTreeSnapshot(model_, path, &reason))
    {
        dialogs_.Error("Snapshot save failed", reason.c_str());
        SetStatus("Snapshot save failed: " + reason);
        return false;
    }
    SetStatus("Saved demo tree snapshot.");
    return true;
}

bool EditorTreePresenter::ImportPathList(
    std::string_view text, const std::string& sourceName)
{
    std::string reason;
    if (!ImportEditorTreePathList(model_, text, &reason))
    {
        dialogs_.Error("Path-list import failed", reason.c_str());
        SetStatus("Path-list import failed: " + reason);
        return false;
    }

    Rebuild(nullptr, true);
    if (output_)
        output_("Imported development path list: " + sourceName);
    SetStatus("Imported development path list.");
    return true;
}

std::size_t EditorTreePresenter::FindFirst(std::string text)
{
    EditorTreeQueryOptions options;
    options.text = std::move(text);
    const EditorTreeQueryResult matches = QueryEditorTree(model_, options);
    if (matches.empty())
    {
        SetStatus("No matching tree items found.");
        return 0;
    }

    tree_.SelectByUserData(
        reinterpret_cast<IEditorTree::UserData>(matches.front()));
    RefreshSelection();
    SetStatus("Found " + std::to_string(matches.size()) +
        " matching item(s); selected '" + matches.front()->Label() + "'.");
    return matches.size();
}

void EditorTreePresenter::ClearSelection()
{
    tree_.ClearSelection();
    selection_.Clear();
    properties_.Clear();
    SetStatus("Selection cleared.");
}

void EditorTreePresenter::ReportSelection()
{
    const std::vector<std::string> paths = selection_.GetSelectedPaths(model_);
    if (paths.empty())
    {
        SetStatus("No model items selected.");
        return;
    }

    if (output_)
    {
        output_("Selected model path(s):");
        for (const std::string& path : paths)
            output_("  " + path);
    }
    SetStatus(std::to_string(paths.size()) + " model item(s) selected.");
}
