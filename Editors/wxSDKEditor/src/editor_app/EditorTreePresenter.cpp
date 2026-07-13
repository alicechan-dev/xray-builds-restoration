#include "editor_app/EditorTreePresenter.h"
#include "editor_app/EditorModelCommand.h"

#include "editor_model/EditorTreePathListImport.h"
#include "editor_model/EditorItemType.h"
#include "editor_model/EditorPropertySet.h"
#include "editor_model/EditorTreeQuery.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_ui/IDialogService.h"
#include "editor_ui/IEditorTree.h"
#include "editor_ui/IPropertyPanel.h"

#include <utility>
#include <memory>

EditorTreePresenter::EditorTreePresenter(EditorDocument& document,
    IEditorTree& tree, IPropertyPanel& properties, IDialogService& dialogs,
    MessageCallback status, MessageCallback output,
    MessageCallback documentChanged, PreviewCallback previewChanged) :
    tree_(tree), properties_(properties), dialogs_(dialogs),
    status_(std::move(status)), output_(std::move(output)),
    documentChanged_(std::move(documentChanged)),
    previewChanged_(std::move(previewChanged)), document_(document),
    model_(document.Model()), selection_(document.Selection()),
    history_(document.History())
{
    properties_.SetApplyHandler(
        [this](const std::string& key, const std::string& value) {
            return ApplySelectedProperty(key, value);
        });
}

void EditorTreePresenter::InitializeDemo()
{
    Rebuild(nullptr, true);
    NotifyDocumentChanged();
}

void EditorTreePresenter::NewDocument()
{
    document_.NewDocument();
    tools_.Reset();
    Rebuild(nullptr, true);
    SetStatus("Created a new development document.");
    NotifyDocumentChanged();
}

bool EditorTreePresenter::SetToolMode(EditorToolMode mode)
{
    const bool changed = tools_.SetMode(mode);
    SetStatus(std::string("Tool: ") + EditorToolModeName(mode) + ". " +
        tools_.StatusText());
    return changed;
}

bool EditorTreePresenter::CancelActiveTool()
{
    if (!tools_.CancelCurrentOperation())
        return false;
    SetStatus("Tool: Select. Placement canceled.");
    return true;
}

bool EditorTreePresenter::SelectAsset(const std::string& assetId)
{
    const EditorAssetDescriptor* descriptor = assetCatalog_.FindById(assetId);
    const EditorAssetCatalog* catalog = &assetCatalog_;
    if (!descriptor)
    {
        descriptor = importedAssetCatalog_.FindById(assetId);
        catalog = &importedAssetCatalog_;
    }
    if (!descriptor || !assetSelection_.Select(*catalog, assetId))
        return false;
    tools_.SetMode(EditorToolMode::PlaceAsset);
    SetStatus("Tool: Place Asset. Selected '" + descriptor->displayName +
        "' (" + descriptor->id + ").");
    return true;
}

void EditorTreePresenter::SetImportedAssetCatalog(EditorAssetCatalog catalog)
{
    importedAssetCatalog_ = std::move(catalog);
    RefreshSelection();
}

void EditorTreePresenter::ClearImportedAssetCatalog()
{
    const bool selectedImported =
        importedAssetCatalog_.FindById(assetSelection_.SelectedId()) != nullptr;
    importedAssetCatalog_ = {};
    if (selectedImported)
    {
        assetSelection_.Clear();
        if (tools_.GetMode() == EditorToolMode::PlaceAsset)
            tools_.SetMode(EditorToolMode::Select);
    }
    RefreshSelection();
}

const EditorAssetDescriptor* EditorTreePresenter::FindAsset(
    const std::string& assetId) const
{
    if (const EditorAssetDescriptor* descriptor = assetCatalog_.FindById(assetId))
        return descriptor;
    return importedAssetCatalog_.FindById(assetId);
}

const EditorAssetDescriptor* EditorTreePresenter::SelectedAsset() const
{
    return FindAsset(assetSelection_.SelectedId());
}

std::string EditorTreePresenter::ResolvePlacementParentPath() const
{
    const EditorTreeNode* selected = SelectedNode();
    if (selected && IsGroupKind(selected->Kind()))
        return selected->Path();
    if (selected && selected->Parent())
        return selected->Parent()->Path();

    const EditorTreeNode* objects = model_.FindByLabel("Objects");
    if (objects && IsGroupKind(objects->Kind()))
        return objects->Path();
    return model_.Root() ? model_.Root()->Path() : std::string();
}

bool EditorTreePresenter::PlaceAt(const EditorTransform& transform)
{
    if (!tools_.IsPlacementMode() || !transform.IsFinite())
        return false;

    const EditorAssetDescriptor* descriptor = nullptr;
    if (tools_.GetMode() == EditorToolMode::PlaceAsset)
        descriptor = SelectedAsset();
    else if (tools_.GetMode() == EditorToolMode::PlaceLight)
        descriptor = assetCatalog_.FindById("demo.point_light");
    else
        descriptor = assetCatalog_.FindById("demo.physic_object");
    if (!descriptor || !descriptor->placeable)
        return false;

    const std::string parentPath = ResolvePlacementParentPath();
    if (parentPath.empty())
        return false;
    EditorTreeNode* placementParent = model_.FindByPath(parentPath);
    if (!placementParent || !IsGroupKind(placementParent->Kind()))
        return false;
    EditorTransform placedTransform = descriptor->defaultTransform;
    placedTransform.x = transform.x;
    placedTransform.z = transform.z;
    const std::string baseName = tools_.GetMode() == EditorToolMode::PlaceAsset
        ? descriptor->baseNodeName
        : (tools_.GetMode() == EditorToolMode::PlaceLight
            ? "new_light" : "new_object");
    const std::string category = descriptor->nodeCategory;
    const std::string assetId = descriptor->id;
    const EditorItemKind itemKind = descriptor->itemKind;
    const std::string placedName = model_.MakeUniqueChildName(
        *placementParent, baseName);
    std::string reason;
    auto command = std::make_unique<EditorModelCommand>(model_,
        "Place " + placedName, parentPath,
        [this, parentPath, placedName, category, assetId, itemKind,
            placedTransform](
            std::string* selectionPath, std::string* mutationReason) {
            EditorTreeNode* parent = model_.FindByPath(parentPath);
            if (!parent || !IsGroupKind(parent->Kind()))
            {
                if (mutationReason)
                    *mutationReason = "Placement parent is unavailable.";
                return false;
            }
            if (model_.FindChildCaseInsensitive(*parent, placedName))
            {
                if (mutationReason)
                    *mutationReason = "Placement name already exists.";
                return false;
            }
            EditorTreeNode& placed = model_.AddChild(*parent, placedName,
                category, itemKind);
            model_.SetNodeAssetId(placed, assetId);
            if (!model_.SetNodeTransform(
                placed, placedTransform, mutationReason))
                return false;
            *selectionPath = placed.Path();
            return true;
        });
    if (!history_.Execute(std::move(command), &reason))
    {
        dialogs_.Warning("Placement rejected", reason.c_str());
        return false;
    }
    RebuildByPath(history_.GetSelectionPath());
    NotifyDocumentChanged();
    SetStatus("Placed '" + placedName + "' at (" +
        std::to_string(placedTransform.x) + ", " +
        std::to_string(placedTransform.y) + ", " +
        std::to_string(placedTransform.z) + ").");
    return true;
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
        RefreshPreview();
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

void EditorTreePresenter::RebuildByPath(
    const std::string& selectedPath, bool selectFirst)
{
    Rebuild(selectedPath.empty() ? nullptr : model_.FindByPath(selectedPath),
        selectFirst);
}

void EditorTreePresenter::RefreshSelection()
{
    const EditorTreeNode* node = SelectedNode();
    selection_.Clear();
    selection_.Select(node);
    RefreshPreview();
    if (!node)
    {
        properties_.Clear();
        return;
    }

    properties_.ShowProperties(BuildEditorNodePropertySet(
        *node, FindAsset(node->AssetId())));
}

void EditorTreePresenter::RefreshPreview() const
{
    if (!previewChanged_)
        return;
    const std::vector<std::string> paths = selection_.GetSelectedPaths(model_);
    previewChanged_(model_, paths.empty() ? std::string() : paths.front());
}

bool EditorTreePresenter::SelectLogicalPath(const std::string& logicalPath)
{
    if (logicalPath.empty())
    {
        ClearSelection();
        SetStatus("No preview object under cursor.");
        return false;
    }
    EditorTreeNode* node = model_.FindByPath(logicalPath);
    if (!node)
    {
        ClearSelection();
        SetStatus("Preview selection path is no longer available.");
        return false;
    }
    tree_.SelectByUserData(reinterpret_cast<IEditorTree::UserData>(node));
    RefreshSelection();
    SetStatus("Selected preview object: " + logicalPath);
    if (output_)
        output_("Selected preview object: " + logicalPath);
    return true;
}

bool EditorTreePresenter::SetLogicalTransform(
    const std::string& path, const EditorTransform& transform)
{
    EditorTreeNode* node = model_.FindByPath(path);
    if (!node)
        return false;
    std::string reason;
    auto command = std::make_unique<EditorModelCommand>(model_,
        "Move " + node->Label(), path,
        [this, path, transform](
            std::string* selection, std::string* mutationReason) {
            EditorTreeNode* current = model_.FindByPath(path);
            if (!current)
            {
                if (mutationReason)
                    *mutationReason = "Node no longer exists.";
                return false;
            }
            if (!model_.SetNodeTransform(
                *current, transform, mutationReason))
                return false;
            *selection = path;
            return true;
        });
    if (!history_.Execute(std::move(command), &reason))
    {
        dialogs_.Warning("Move rejected", reason.c_str());
        return false;
    }
    RebuildByPath(path);
    NotifyDocumentChanged();
    SetStatus("Moved preview object: " + path);
    return true;
}

void EditorTreePresenter::SetStatus(const std::string& message) const
{
    if (status_)
        status_(message);
}

void EditorTreePresenter::NotifyDocumentChanged() const
{
    if (documentChanged_)
        documentChanged_(document_.GetDisplayName());
}

void EditorTreePresenter::AddDemoNode(const char* baseName, const char* category)
{
    EditorTreeNode* parent = SelectedNode();
    if (!parent)
        parent = model_.Root();
    if (!parent)
        return;

    const std::string parentPath = parent->Path();
    const std::string base = baseName;
    const std::string nodeCategory = category;
    std::string reason;
    auto command = std::make_unique<EditorModelCommand>(model_, "Add " + base,
        parentPath, [this, parentPath, base, nodeCategory](
            std::string* selectionPath, std::string* mutationReason) {
            EditorTreeNode* currentParent = model_.FindByPath(parentPath);
            if (!currentParent)
            {
                if (mutationReason) *mutationReason = "Parent no longer exists.";
                return false;
            }
            const std::string name =
                model_.MakeUniqueChildName(*currentParent, base);
            EditorTreeNode& added =
                model_.AddChild(*currentParent, name, nodeCategory);
            *selectionPath = added.Path();
            return true;
        });
    if (!history_.Execute(std::move(command), &reason))
    {
        dialogs_.Warning("Add rejected", reason.c_str());
        return;
    }
    RebuildByPath(history_.GetSelectionPath());
    SetStatus("Added '" + tree_.GetSelectedLabel() + "'.");
    NotifyDocumentChanged();
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

    const std::string selectedPath = selected->Path();
    const std::string parentPath = selected->Parent()
        ? selected->Parent()->Path() : std::string();
    auto command = std::make_unique<EditorModelCommand>(model_,
        "Delete " + label, selectedPath,
        [this, selectedPath, parentPath](
            std::string* selectionPath, std::string* mutationReason) {
            EditorTreeNode* current = model_.FindByPath(selectedPath);
            if (!current)
            {
                if (mutationReason) *mutationReason = "Node no longer exists.";
                return false;
            }
            if (!model_.DeleteNode(*current, mutationReason))
                return false;
            *selectionPath = parentPath;
            return true;
        });
    if (!history_.Execute(std::move(command), &reason))
    {
        dialogs_.Warning("Delete rejected", reason.c_str());
        return;
    }

    RebuildByPath(history_.GetSelectionPath(), true);
    SetStatus("Deleted '" + label + "'.");
    NotifyDocumentChanged();
}

std::vector<std::string> EditorTreePresenter::GetMoveDestinations() const
{
    std::vector<std::string> paths;
    const EditorTreeNode* selected = SelectedNode();
    if (!selected)
        return paths;

    EditorTreeQueryOptions options;
    const EditorTreeQueryResult nodes = QueryEditorTree(model_, options);
    for (const EditorTreeNode* node : nodes)
    {
        std::string reason;
        if (model_.CanMoveNode(*selected, *node, &reason))
            paths.push_back(node->Path());
    }
    return paths;
}

bool EditorTreePresenter::MoveSelectedTo(const std::string& newParentPath)
{
    EditorTreeNode* selected = SelectedNode();
    if (!selected)
    {
        dialogs_.Warning("Move rejected", "No tree node is selected.");
        SetStatus("Move rejected: no tree node is selected.");
        return false;
    }

    EditorTreeNode* newParent = model_.FindByPath(newParentPath);
    if (!newParent)
    {
        dialogs_.Warning("Move rejected", "The destination no longer exists.");
        SetStatus("Move rejected: destination not found.");
        return false;
    }

    const std::string oldPath = selected->Path();
    const std::string label = selected->Label();
    std::string reason;
    auto command = std::make_unique<EditorModelCommand>(model_,
        "Move " + label, oldPath,
        [this, oldPath, newParentPath](
            std::string* selectionPath, std::string* mutationReason) {
            EditorTreeNode* current = model_.FindByPath(oldPath);
            EditorTreeNode* destination = model_.FindByPath(newParentPath);
            if (!current || !destination)
            {
                if (mutationReason) *mutationReason =
                    "Source or destination no longer exists.";
                return false;
            }
            if (!model_.MoveNode(*current, *destination, mutationReason))
                return false;
            *selectionPath = current->Path();
            return true;
        });
    if (!history_.Execute(std::move(command), &reason))
    {
        dialogs_.Warning("Move rejected", reason.c_str());
        SetStatus("Move rejected: " + reason);
        return false;
    }

    RebuildByPath(history_.GetSelectionPath());
    if (output_)
        output_("Moved '" + label + "' to '" + newParentPath + "'.");
    SetStatus("Moved '" + label + "' to '" + newParentPath + "'.");
    NotifyDocumentChanged();
    return true;
}

bool EditorTreePresenter::ApplySelectedProperty(
    const std::string& key, std::string value)
{
    EditorTreeNode* selected = SelectedNode();
    if (!selected)
    {
        dialogs_.Warning("Property edit rejected", "No tree node is selected.");
        SetStatus("Property edit rejected: no tree node is selected.");
        return false;
    }

    const std::string selectedPath = selected->Path();
    std::string reason;
    auto command = std::make_unique<EditorModelCommand>(model_,
        "Edit " + key, selectedPath,
        [this, selectedPath, key, value = std::move(value)](
            std::string* selectionPath, std::string* mutationReason) mutable {
            EditorTreeNode* current = model_.FindByPath(selectedPath);
            if (!current)
            {
                if (mutationReason) *mutationReason = "Node no longer exists.";
                return false;
            }
            const EditorPropertyApplyResult result =
                ApplyEditorNodeProperty(model_, *current, key, std::move(value));
            if (!result.success)
            {
                if (mutationReason) *mutationReason = result.reason;
                return false;
            }
            *selectionPath = current->Path();
            return true;
        });
    if (!history_.Execute(std::move(command), &reason))
    {
        dialogs_.Warning("Property edit rejected", reason.c_str());
        SetStatus("Property edit rejected: " + reason);
        RefreshSelection();
        return false;
    }
    RebuildByPath(history_.GetSelectionPath());
    SetStatus("Updated property '" + key + "'.");
    NotifyDocumentChanged();
    return true;
}

bool EditorTreePresenter::RenameNode(
    EditorTreeNode& node, std::string newName, std::string* reason)
{
    const std::string previousLabel = node.Label();
    const std::string oldPath = node.Path();
    auto command = std::make_unique<EditorModelCommand>(model_,
        "Rename " + previousLabel, oldPath,
        [this, oldPath, newName = std::move(newName)](
            std::string* selectionPath, std::string* mutationReason) mutable {
            EditorTreeNode* current = model_.FindByPath(oldPath);
            if (!current)
            {
                if (mutationReason) *mutationReason = "Node no longer exists.";
                return false;
            }
            if (!model_.RenameNode(*current, std::move(newName), mutationReason))
                return false;
            *selectionPath = current->Path();
            return true;
        });
    if (!history_.Execute(std::move(command), reason))
    {
        SetStatus("Rename rejected: " + (reason ? *reason : std::string()));
        return false;
    }

    RefreshSelection();
    SetStatus("Renamed '" + previousLabel + "' to '" + node.Label() + "'.");
    NotifyDocumentChanged();
    return true;
}

bool EditorTreePresenter::LoadSnapshot(const std::filesystem::path& path)
{
    std::string reason;
    if (!document_.LoadFromSnapshot(path, &reason))
    {
        dialogs_.Error("Snapshot load failed", reason.c_str());
        SetStatus("Snapshot load failed: " + reason);
        return false;
    }

    tools_.Reset();
    Rebuild(nullptr, true);
    SetStatus("Loaded demo tree snapshot.");
    NotifyDocumentChanged();
    return true;
}

bool EditorTreePresenter::Undo()
{
    const std::string name = history_.GetUndoName();
    std::string reason;
    if (!history_.Undo(&reason))
    {
        SetStatus(reason);
        return false;
    }
    RebuildByPath(history_.GetSelectionPath(), true);
    SetStatus("Undid '" + name + "'.");
    NotifyDocumentChanged();
    return true;
}

bool EditorTreePresenter::Redo()
{
    const std::string name = history_.GetRedoName();
    std::string reason;
    if (!history_.Redo(&reason))
    {
        SetStatus(reason);
        return false;
    }
    RebuildByPath(history_.GetSelectionPath(), true);
    SetStatus("Redid '" + name + "'.");
    NotifyDocumentChanged();
    return true;
}

bool EditorTreePresenter::SaveSnapshot(const std::filesystem::path& path)
{
    std::string reason;
    if (!document_.SaveAs(path, &reason))
    {
        dialogs_.Error("Snapshot save failed", reason.c_str());
        SetStatus("Snapshot save failed: " + reason);
        return false;
    }
    tools_.Reset();
    SetStatus("Saved demo tree snapshot.");
    NotifyDocumentChanged();
    return true;
}

bool EditorTreePresenter::ImportPathList(
    std::string_view text, const std::string& sourceName)
{
    std::string reason;
    if (!document_.ImportPathList(text, &reason))
    {
        dialogs_.Error("Path-list import failed", reason.c_str());
        SetStatus("Path-list import failed: " + reason);
        return false;
    }

    tools_.Reset();
    Rebuild(nullptr, true);
    if (output_)
        output_("Imported development path list: " + sourceName);
    SetStatus("Imported development path list.");
    NotifyDocumentChanged();
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
    RefreshPreview();
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
