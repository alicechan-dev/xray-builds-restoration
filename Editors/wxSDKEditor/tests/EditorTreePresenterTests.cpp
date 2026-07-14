#include "editor_app/EditorTreePresenter.h"
#include "editor_assets/EditorMetadataCatalogAdapter.h"
#include "editor_model/EditorPropertySet.h"
#include "editor_scene/EditorHistoricalSceneDocument.h"
#include "editor_scene/EditorHistoricalConversionReport.h"
#include "editor_view/EditorPreviewScene.h"
#include "editor_ui/IDialogService.h"
#include "editor_ui/IEditorTree.h"
#include "editor_ui/IPropertyPanel.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace
{
class FakeEditorTree final : public IEditorTree
{
public:
    struct Item
    {
        ItemHandle handle = InvalidItem;
        ItemHandle parent = InvalidItem;
        std::string label;
        UserData userData = 0;
    };

    void Clear() override
    {
        ++clearCount;
        items.clear();
        selected = InvalidItem;
    }

    ItemHandle AddRoot(const char* label) override
    {
        return AddItem(InvalidItem, label);
    }

    ItemHandle AddChild(ItemHandle parent, const char* label) override
    {
        return AddItem(parent, label);
    }

    std::string GetSelectedLabel() const override
    {
        const Item* item = Find(selected);
        return item ? item->label : std::string();
    }

    void SetItemUserData(ItemHandle handle, UserData userData) override
    {
        if (Item* item = Find(handle))
            item->userData = userData;
    }

    UserData GetSelectedUserData() const override
    {
        const Item* item = Find(selected);
        return item ? item->userData : 0;
    }

    void ExpandAllItems() override { ++expandCount; }
    void SelectFirst() override
    {
        selected = items.empty() ? InvalidItem : items.front().handle;
    }
    void ClearSelection() override { selected = InvalidItem; }
    void BeginEditSelectedLabel() override {}
    void SelectByUserData(UserData userData) override
    {
        const auto found = std::find_if(items.begin(), items.end(),
            [userData](const Item& item) { return item.userData == userData; });
        selected = found == items.end() ? InvalidItem : found->handle;
    }

    bool SelectByLabel(const std::string& label)
    {
        const auto found = std::find_if(items.begin(), items.end(),
            [&label](const Item& item) { return item.label == label; });
        selected = found == items.end() ? InvalidItem : found->handle;
        return found != items.end();
    }

    bool Contains(const std::string& label) const
    {
        return std::any_of(items.begin(), items.end(),
            [&label](const Item& item) { return item.label == label; });
    }

    std::size_t Count(const std::string& label) const
    {
        return static_cast<std::size_t>(std::count_if(items.begin(), items.end(),
            [&label](const Item& item) { return item.label == label; }));
    }

    std::vector<Item> items;
    int clearCount = 0;
    int expandCount = 0;

private:
    ItemHandle AddItem(ItemHandle parent, const char* label)
    {
        const ItemHandle handle = nextHandle++;
        items.push_back({handle, parent, label, 0});
        return handle;
    }

    Item* Find(ItemHandle handle)
    {
        const auto found = std::find_if(items.begin(), items.end(),
            [handle](const Item& item) { return item.handle == handle; });
        return found == items.end() ? nullptr : &*found;
    }

    const Item* Find(ItemHandle handle) const
    {
        const auto found = std::find_if(items.begin(), items.end(),
            [handle](const Item& item) { return item.handle == handle; });
        return found == items.end() ? nullptr : &*found;
    }

    ItemHandle nextHandle = 1;
    ItemHandle selected = InvalidItem;
};

class FakePropertyPanel final : public IPropertyPanel
{
public:
    void Clear() override
    {
        ++clearCount;
        text.clear();
    }
    void ShowPlaceholder(const char* value) override
    {
        text = value ? value : "";
    }
    void ShowProperties(const EditorPropertySet& properties) override
    {
        ++showCount;
        lastProperties = properties;
        const auto value = [&properties](const char* key) {
            const EditorProperty* property = properties.Find(key);
            return property ? property->value : std::string();
        };
        text = "Selected: " + value("label") +
            "\nKind: " + value("kind") +
            "\nType: " + value("category") +
            "\nPath: " + value("path");
    }
    void SetApplyHandler(ApplyHandler handler) override
    {
        applyHandler = std::move(handler);
    }
    void SetEditingEnabled(bool enabled) override
    {
        editingEnabled = enabled;
    }
    bool Apply(const std::string& key, const std::string& value)
    {
        return applyHandler && applyHandler(key, value);
    }

    std::string text;
    int clearCount = 0;
    int showCount = 0;
    EditorPropertySet lastProperties;
    ApplyHandler applyHandler;
    bool editingEnabled = true;
};

class FakeDialogService final : public IDialogService
{
public:
    void Info(const char* title, const char* message) override
    {
        lastInfo = Join(title, message);
    }
    void Warning(const char* title, const char* message) override
    {
        lastWarning = Join(title, message);
    }
    void Error(const char* title, const char* message) override
    {
        lastError = Join(title, message);
    }
    bool Confirm(const char* title, const char* message) override
    {
        ++confirmCount;
        lastConfirm = Join(title, message);
        return confirmResult;
    }

    bool confirmResult = true;
    int confirmCount = 0;
    std::string lastInfo;
    std::string lastWarning;
    std::string lastError;
    std::string lastConfirm;

private:
    static std::string Join(const char* title, const char* message)
    {
        return std::string(title ? title : "") + ": " +
            (message ? message : "");
    }
};

bool ContainsText(const std::string& text, const std::string& value)
{
    return text.find(value) != std::string::npos;
}

EditorSceneManifest HistoricalManifest()
{
    EditorSceneManifest manifest;
    manifest.sourceFile = "fixture.level";
    manifest.version = 5;
    manifest.hasVersion = true;
    for (std::size_t index = 0; index < 2; ++index)
    {
        EditorSceneObjectRecord object;
        object.recordIndex = index;
        object.sourceOffset = 100 + index * 20;
        object.classId = 2;
        object.hasClassId = true;
        object.name = "duplicate";
        object.hasName = true;
        object.hasTransform = true;
        object.position = {static_cast<float>(index), 2.0f, 3.0f};
        object.scale = {1.0f, 1.0f, 1.0f};
        object.chunkPath = "object/" + std::to_string(index);
        manifest.objects.push_back(std::move(object));
    }
    return manifest;
}
}

int RunEditorTreePresenterTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition)
            return;
        ++failures;
        std::cerr << "FAIL: presenter " << message << '\n';
    };

    FakeEditorTree tree;
    FakePropertyPanel properties;
    FakeDialogService dialogs;
    std::string status;
    std::string output;
    EditorDocument document;
    EditorTreePresenter presenter(document, tree, properties, dialogs,
        [&status](const std::string& message) { status = message; },
        [&output](const std::string& message) { output += message + "\n"; });

    presenter.InitializeDemo();
    check(!document.IsModified(), "initial document state is clean");
    check(presenter.SelectLogicalPath("Scene (demo data)/Objects/actor") &&
        tree.GetSelectedLabel() == "actor" &&
        ContainsText(properties.text, "Objects/actor") &&
        !document.IsModified() && !presenter.CanUndo(),
        "preview path selection updates tree and properties without mutation");
    check(!presenter.SelectLogicalPath("") &&
        tree.GetSelectedUserData() == 0 && properties.text.empty() &&
        !document.IsModified() && !presenter.CanUndo(),
        "empty preview click clears selection without history or dirtiness");
    tree.SelectFirst();
    presenter.RefreshSelection();
    check(tree.Contains("Scene (demo data)"), "initial demo root populated");
    check(tree.Contains("actor"), "initial demo descendants populated");
    check(tree.expandCount == 1, "initial tree expanded");
    check(tree.GetSelectedLabel() == "Scene (demo data)", "initial root selected");
    check(ContainsText(properties.text, "Path: Scene (demo data)"),
        "initial selection refreshes properties");
    check(properties.lastProperties.Find("label") &&
        properties.lastProperties.Find("path")->readOnly,
        "selection exposes editable property set");
    check(presenter.GetMoveDestinations().empty(),
        "root has no valid move destinations");
    const int clearsBeforeRootMove = tree.clearCount;
    check(!presenter.MoveSelectedTo("Scene (demo data)/Lights"),
        "presenter rejects root move");
    check(tree.clearCount == clearsBeforeRootMove &&
        tree.GetSelectedLabel() == "Scene (demo data)",
        "failed root move preserves model view and selection");

    check(presenter.FindFirst("ACT") == 1,
        "search finds case-insensitive label substring");
    check(tree.GetSelectedLabel() == "actor", "search selects first result");
    check(ContainsText(properties.text, "Path: Scene (demo data)/Objects/actor"),
        "search refreshes selected properties");
    check(ContainsText(status, "Found 1 matching item"),
        "search reports match count");
    const std::string propertiesBeforeNoMatch = properties.text;
    check(presenter.FindFirst("does-not-exist") == 0,
        "search reports no results");
    check(tree.GetSelectedLabel() == "actor" &&
        properties.text == propertiesBeforeNoMatch,
        "no-result search preserves selection and properties");
    check(ContainsText(status, "No matching tree items"),
        "no-result search reports non-fatal status");

    const std::vector<std::string> moveDestinations =
        presenter.GetMoveDestinations();
    check(std::find(moveDestinations.begin(), moveDestinations.end(),
        "Scene (demo data)/Lights") != moveDestinations.end(),
        "presenter exposes valid folder destination");
    check(std::find(moveDestinations.begin(), moveDestinations.end(),
        "Scene (demo data)/Objects") == moveDestinations.end(),
        "presenter excludes current parent destination");
    output.clear();
    const int clearsBeforeMove = tree.clearCount;
    check(presenter.MoveSelectedTo("Scene (demo data)/Lights"),
        "presenter moves selected object");
    check(document.IsModified(), "presenter mutation marks document dirty");
    check(tree.clearCount == clearsBeforeMove + 1,
        "successful move rebuilds tree");
    check(tree.GetSelectedLabel() == "actor",
        "moved node remains selected");
    check(ContainsText(properties.text,
        "Path: Scene (demo data)/Lights/actor"),
        "move refreshes property path");
    check(ContainsText(status, "Moved 'actor'") &&
        ContainsText(output, "Moved 'actor'"),
        "successful move reports status and output");
    check(presenter.CanUndo() && presenter.Undo() && !document.IsModified(),
        "move can be undone to clean document baseline");
    check(ContainsText(properties.text, "Objects/actor") &&
        tree.GetSelectedLabel() == "actor",
        "move undo restores hierarchy and logical selection");
    check(presenter.CanRedo() && presenter.Redo() && document.IsModified(),
        "move can be redone to dirty document state");
    check(ContainsText(properties.text, "Lights/actor"),
        "move redo restores destination and selection");
    const int clearsBeforeRejectedMove = tree.clearCount;
    check(!presenter.MoveSelectedTo("Scene (demo data)/Lights/sun"),
        "presenter rejects object destination");
    check(tree.clearCount == clearsBeforeRejectedMove &&
        tree.GetSelectedLabel() == "actor" &&
        ContainsText(properties.text, "Lights/actor"),
        "failed move preserves selection, properties, and model view");
    check(ContainsText(dialogs.lastWarning, "Only the root or a folder"),
        "failed move reports model reason through dialogs");
    check(presenter.MoveSelectedTo("Scene (demo data)/Objects"),
        "presenter can move object back to original folder");

    const int clearsBeforePropertyRename = tree.clearCount;
    check(properties.Apply("label", "property_actor"),
        "presenter accepts property label edit");
    check(tree.clearCount == clearsBeforePropertyRename + 1 &&
        tree.GetSelectedLabel() == "property_actor",
        "property label edit rebuilds tree and preserves selection");
    check(ContainsText(properties.text,
        "Path: Scene (demo data)/Objects/property_actor") &&
        ContainsText(status, "Updated property 'label'"),
        "property label edit refreshes path and status");
    const int clearsBeforeFailedProperty = tree.clearCount;
    check(!properties.Apply("label", ""),
        "presenter rejects empty property label");
    check(tree.clearCount == clearsBeforeFailedProperty &&
        tree.GetSelectedLabel() == "property_actor" &&
        ContainsText(dialogs.lastWarning, "Name cannot be empty"),
        "failed property edit preserves tree and reports reason");
    check(!properties.Apply("label", "PHYSIC_OBJECT"),
        "presenter rejects duplicate property label");
    check(properties.Apply("category", "edited category"),
        "presenter accepts category property edit");
    check(ContainsText(properties.text, "Type: edited category"),
        "category property edit refreshes panel");
    check(presenter.Undo() && ContainsText(properties.text,
        "Type: demo scene object"),
        "category edit undo restores property value");
    check(presenter.Redo() && ContainsText(properties.text,
        "Type: edited category"),
        "category edit redo restores property value");
    check(!properties.Apply("path", "forbidden"),
        "presenter rejects read-only property edit");

    const std::string actorPath =
        "Scene (demo data)/Objects/property_actor";
    EditorTransform moved = document.Model().FindByPath(actorPath)->Transform();
    const float originalX = moved.x;
    moved.x += 3.0f;
    check(presenter.SetLogicalTransform(actorPath, moved) &&
        document.Model().FindByPath(actorPath)->Transform().NearlyEquals(moved) &&
        tree.GetSelectedLabel() == "property_actor",
        "transform command applies once and preserves selection");
    check(presenter.Undo() &&
        std::fabs(document.Model().FindByPath(actorPath)->Transform().x -
            originalX) < 0.001f,
        "transform command undo restores starting position");
    check(presenter.Redo() &&
        document.Model().FindByPath(actorPath)->Transform().NearlyEquals(moved),
        "transform command redo restores moved position");

    output.clear();
    presenter.ReportSelection();
    check(ContainsText(output, "Scene (demo data)/Objects/property_actor"),
        "selected path report uses model path");
    check(ContainsText(status, "1 model item"),
        "selected path report includes count");
    presenter.ClearSelection();
    check(tree.GetSelectedUserData() == 0, "clear selection clears tree view");
    check(properties.text.empty(), "clear selection clears properties");
    check(ContainsText(status, "Selection cleared"),
        "clear selection reports status");
    check(!presenter.MoveSelectedTo("Scene (demo data)/Lights"),
        "move without selection is safe");
    check(ContainsText(dialogs.lastWarning, "No tree node is selected"),
        "move without selection reports warning");

    check(tree.SelectByLabel("Objects"), "objects group selectable");
    presenter.RefreshSelection();
    check(ContainsText(properties.text, "Type: demo group"),
        "selection category shown in properties");
    check(ContainsText(properties.text, "Kind: folder"),
        "selection kind shown in properties");

    presenter.AddDemoNode("new_object", "demo scene object");
    check(tree.GetSelectedLabel() == "new_object", "added object selected");
    check(ContainsText(properties.text, "Path: Scene (demo data)/Objects/new_object"),
        "added object properties refreshed");
    check(ContainsText(status, "Added 'new_object'"), "add object status reported");
    check(presenter.Undo() && !tree.Contains("new_object"),
        "add undo removes node");
    check(tree.GetSelectedLabel() == "Objects",
        "add undo restores parent selection");
    check(presenter.Redo() && tree.Contains("new_object") &&
        tree.GetSelectedLabel() == "new_object",
        "add redo restores node and selection");
    check(tree.SelectByLabel("Objects"), "objects group reselectable");
    presenter.AddDemoNode("new_object", "demo scene object");
    check(tree.GetSelectedLabel() == "new_object_1", "unique object name generated");

    check(tree.SelectByLabel("Objects"), "objects group selected for group add");
    presenter.AddDemoNode("new_group", "demo group");
    check(tree.GetSelectedLabel() == "new_group", "added group selected");
    check(ContainsText(properties.text, "Type: demo group"),
        "added group properties refreshed");

    check(tree.SelectByLabel("new_object"), "first new object selectable");
    auto* renamed = reinterpret_cast<EditorTreeNode*>(tree.GetSelectedUserData());
    std::string reason;
    check(renamed && presenter.RenameNode(*renamed, "renamed_object", &reason),
        "unique rename accepted");
    check(ContainsText(properties.text, "renamed_object"),
        "accepted rename refreshes properties");
    check(presenter.Undo() && tree.Contains("new_object") &&
        tree.GetSelectedLabel() == "new_object",
        "rename undo restores old path and selection");
    check(presenter.Redo() && tree.Contains("renamed_object") &&
        tree.GetSelectedLabel() == "renamed_object",
        "rename redo restores new path and selection");
    renamed = reinterpret_cast<EditorTreeNode*>(tree.GetSelectedUserData());
    check(!presenter.RenameNode(*renamed, "new_object_1", &reason),
        "duplicate sibling rename rejected");
    check(ContainsText(status, "Rename rejected"), "rename rejection status reported");
    check(!presenter.RenameNode(*renamed, "", &reason), "empty rename rejected");

    check(tree.SelectByLabel("physic_object"), "delete candidate selectable");
    dialogs.confirmResult = false;
    presenter.DeleteSelected();
    check(dialogs.confirmCount == 1, "delete requests confirmation");
    check(tree.Contains("physic_object"), "cancelled delete preserves node");
    dialogs.confirmResult = true;
    presenter.DeleteSelected();
    check(dialogs.confirmCount == 2, "confirmed delete requests confirmation");
    check(!tree.Contains("physic_object"), "confirmed delete removes node");
    check(tree.GetSelectedLabel() == "Objects", "delete selects surviving parent");
    check(presenter.Undo() && tree.Contains("physic_object") &&
        tree.GetSelectedLabel() == "physic_object",
        "delete undo restores subtree and deleted-node selection");
    check(presenter.Redo() && !tree.Contains("physic_object") &&
        tree.GetSelectedLabel() == "Objects",
        "delete redo removes subtree and selects parent");

    tree.SelectFirst();
    const int confirmationsBeforeRootDelete = dialogs.confirmCount;
    presenter.DeleteSelected();
    check(dialogs.confirmCount == confirmationsBeforeRootDelete,
        "root delete rejected before confirmation");
    check(ContainsText(dialogs.lastWarning, "root node cannot be deleted"),
        "root delete warning reported");

    presenter.SetToolMode(EditorToolMode::PlaceObject);
    check(presenter.ImportPathList(
        "/Imported/Objects/item | imported object\n", "memory.wx_tree_paths"),
        "valid path list imported");
    check(presenter.GetToolMode() == EditorToolMode::Select,
        "successful import resets tool mode to Select");
    check(tree.Contains("Imported") && tree.Contains("item"),
        "successful import rebuilds tree");
    check(tree.GetSelectedLabel() == "Imported", "successful import selects root");
    check(document.IsModified() && !document.HasFilePath(),
        "presenter import creates modified untitled document");
    check(!presenter.CanUndo() && !presenter.CanRedo(),
        "successful import clears command history");
    check(ContainsText(output, "memory.wx_tree_paths"), "import output reported");

    presenter.AddDemoNode("new_group", "demo group");
    check(presenter.CanUndo() && tree.Contains("new_group"),
        "post-import mutation establishes new history");

    const int clearsBeforeFailedImport = tree.clearCount;
    check(!presenter.ImportPathList("relative/path\n", "bad.wx_tree_paths"),
        "invalid path list rejected");
    check(tree.clearCount == clearsBeforeFailedImport,
        "failed import does not rebuild tree");
    check(tree.Contains("Imported") && tree.Contains("item") &&
        tree.Contains("new_group") && presenter.CanUndo(),
        "failed import preserves existing model view and command history");
    check(ContainsText(dialogs.lastError, "path must begin with"),
        "failed import reports reason through dialogs");

    check(presenter.GetToolMode() == EditorToolMode::Select &&
        presenter.SetToolMode(EditorToolMode::PlaceObject) &&
        !document.History().CanRedo(),
        "tool mode changes remain outside command history");
    EditorTransform placement;
    placement.x = 2.5f;
    placement.z = -3.0f;
    check(presenter.PlaceAt(placement) && tree.Contains("new_object") &&
        tree.GetSelectedLabel() == "new_object",
        "object placement creates and selects transformed node");
    EditorTreeNode* placed = document.Model().FindByLabel("new_object");
    check(placed && placed->Kind() == EditorItemKind::Object &&
        placed->Category() == "demo scene object" &&
        placed->Transform().NearlyEquals(placement) &&
        placed->AssetId() == "demo.physic_object",
        "object placement preserves kind, category, and transform");
    check(presenter.Undo() && !tree.Contains("new_object"),
        "placement undo removes node");
    check(presenter.Redo() && tree.Contains("new_object") &&
        tree.GetSelectedLabel() == "new_object",
        "placement redo recreates node and selection");
    check(presenter.PlaceAt(placement) && tree.Contains("new_object_1"),
        "repeated placement generates deterministic unique name");
    EditorTreeNode* placedAgain = document.Model().FindByLabel("new_object_1");
    check(placedAgain && placedAgain->Parent() &&
        placedAgain->Parent()->Label() == "new_group",
        "object selection places its successor under the object's parent");

    check(presenter.SetToolMode(EditorToolMode::PlaceLight),
        "light placement mode selected");
    EditorTransform lightPlacement;
    lightPlacement.x = 4.0f;
    lightPlacement.y = 1.0f;
    lightPlacement.z = 2.0f;
    check(presenter.PlaceAt(lightPlacement) && tree.Contains("new_light"),
        "light placement creates uniquely named node");
    EditorTreeNode* light = document.Model().FindByLabel("new_light");
    check(light && light->Category() == "demo light" &&
        light->Transform().NearlyEquals(lightPlacement) &&
        light->AssetId() == "demo.point_light",
        "light placement stores demo category and height");
    check(presenter.SelectAsset("demo.spawn") &&
        presenter.GetToolMode() == EditorToolMode::PlaceAsset,
        "catalog selection enters generic Place Asset mode");
    EditorTransform spawnPlacement;
    spawnPlacement.x = 7.0f;
    spawnPlacement.z = 8.0f;
    check(presenter.PlaceAt(spawnPlacement) && tree.Contains("spawn_element"),
        "generic asset placement creates descriptor-named node");
    EditorTreeNode* spawn = document.Model().FindByLabel("spawn_element");
    check(spawn && spawn->AssetId() == "demo.spawn" &&
        spawn->Category() == "demo spawn" &&
        spawn->Transform().NearlyEquals(spawnPlacement),
        "generic placement stores descriptor metadata and transform");
    check(presenter.Undo() && !tree.Contains("spawn_element") &&
        presenter.Redo() && tree.Contains("spawn_element") &&
        document.Model().FindByLabel("spawn_element")->AssetId() == "demo.spawn",
        "asset placement undo/redo preserves asset identity");
    check(presenter.CancelActiveTool() &&
        presenter.GetToolMode() == EditorToolMode::Select,
        "cancel returns presenter to Select without a command");
    presenter.SetToolMode(EditorToolMode::Move);
    presenter.NewDocument();
    check(presenter.GetToolMode() == EditorToolMode::Select &&
        !document.IsModified(),
        "New resets tool mode without dirtying the document");

    {
        FakeEditorTree importedTree;
        FakePropertyPanel importedProperties;
        FakeDialogService importedDialogs;
        EditorDocument importedDocument;
        EditorTreePresenter importedPresenter(importedDocument, importedTree,
            importedProperties, importedDialogs, {}, {});
        importedPresenter.InitializeDemo();
        EditorImportedMetadata metadata;
        metadata.sections.push_back({"wpn_ak74", "weapons.ltx", 12,
            {{"$spawn", "\"weapons\\ak-74\"", "weapons.ltx", 13}}});
        EditorMetadataCatalogResult imported =
            BuildEditorMetadataCatalog(metadata);
        importedPresenter.SetImportedAssetCatalog(imported.catalog);
        check(!importedDocument.IsModified() &&
            !importedPresenter.CanUndo() &&
            importedPresenter.SelectAsset("imported.section.wpn_ak74") &&
            importedPresenter.GetToolMode() == EditorToolMode::PlaceAsset,
            "session import selects an audited placeable prototype without document mutation");

        const std::size_t nodesBeforeCancel = importedTree.items.size();
        check(importedPresenter.CancelActiveTool() &&
            !importedPresenter.PlaceAt({}) &&
            importedTree.items.size() == nodesBeforeCancel,
            "canceling imported placement does not mutate the document");
        check(importedPresenter.SelectAsset("imported.section.wpn_ak74"),
            "imported prototype can be selected again after cancellation");
        EditorTransform importedPlacement;
        importedPlacement.x = 3.0f;
        importedPlacement.z = 6.0f;
        check(importedPresenter.PlaceAt(importedPlacement) &&
            importedTree.Contains("wpn_ak74"),
            "imported prototype places through the generic command path");
        EditorTreeNode* importedNode =
            importedDocument.Model().FindByLabel("wpn_ak74");
        check(importedNode &&
            importedNode->AssetId() == "imported.section.wpn_ak74" &&
            importedNode->Category() == "imported spawn" &&
            importedNode->Kind() == EditorItemKind::Object &&
            importedNode->Transform().NearlyEquals(importedPlacement),
            "placed marker preserves prototype identity, category, kind, and transform");
        check(importedPresenter.Undo() &&
            !importedTree.Contains("wpn_ak74") &&
            importedPresenter.Redo() && importedTree.Contains("wpn_ak74") &&
            importedDocument.Model().FindByLabel("wpn_ak74")->AssetId() ==
                "imported.section.wpn_ak74",
            "imported placement undo/redo preserves prototype identity");
        check(importedPresenter.PlaceAt(importedPlacement) &&
            importedTree.Contains("wpn_ak74_1"),
            "repeated imported placement uses deterministic unique names");

        const bool dirtyBeforeClear = importedDocument.IsModified();
        const bool undoBeforeClear = importedPresenter.CanUndo();
        importedPresenter.ClearImportedAssetCatalog();
        check(importedDocument.Model().FindByLabel("wpn_ak74") &&
            importedDocument.IsModified() == dirtyBeforeClear &&
            importedPresenter.CanUndo() == undoBeforeClear &&
            importedPresenter.GetToolMode() == EditorToolMode::Select &&
            importedProperties.lastProperties.Find("metadata_resolution") &&
            importedProperties.lastProperties.Find("metadata_resolution")->value ==
                "unresolved",
            "clearing session catalog preserves placed nodes, history, and dirty state");

        EditorAssetCatalog blockedCatalog;
        EditorAssetDescriptor blocked;
        blocked.id = "imported.section.blocked";
        blocked.displayName = "Blocked";
        blocked.placeable = false;
        blockedCatalog.Add(std::move(blocked));
        importedPresenter.SetImportedAssetCatalog(std::move(blockedCatalog));
        check(!importedPresenter.SelectAsset("imported.section.blocked"),
            "non-placeable imported descriptor cannot enter placement mode");
    }

    {
        FakeEditorTree historicalTree;
        FakePropertyPanel historicalProperties;
        FakeDialogService historicalDialogs;
        EditorDocument editableDocument;
        EditorHistoricalSceneDocument historicalDocument;
        EditorPreviewScene preview;
        EditorTreePresenter historicalPresenter(editableDocument,
            historicalTree, historicalProperties, historicalDialogs, {}, {},
            {}, [&preview](const EditorPreviewScene& scene) { preview = scene; });
        historicalPresenter.AttachHistoricalDocument(historicalDocument);
        historicalPresenter.InitializeDemo();
        check(!historicalPresenter.CanConvertHistoricalScene(),
            "historical conversion action is unavailable in editable mode");
        check(!historicalPresenter.HasHistoricalConversionSummary(),
            "conversion summary is unavailable without historical origins");

        EditorSceneManifest invalid = HistoricalManifest();
        invalid.version = 4;
        std::string reason;
        const EditorTreeNode* editableRoot = editableDocument.Model().Root();
        check(!historicalPresenter.OpenHistoricalScene(
                std::move(invalid), &reason) &&
            historicalPresenter.GetDocumentMode() ==
                EditorDocumentMode::EditableSnapshot &&
            editableDocument.Model().Root() == editableRoot,
            "failed historical open preserves editable mode and model");

        check(historicalPresenter.OpenHistoricalScene(
                HistoricalManifest(), &reason) &&
            historicalPresenter.IsReadOnly() &&
            historicalPresenter.GetActiveDisplayName() == "fixture.level" &&
            !historicalProperties.editingEnabled &&
            historicalTree.Contains("duplicate") &&
            historicalTree.Contains("duplicate [#2]") &&
            preview.GetObjects().size() == 2,
            "historical open switches to read-only tree and preview");
        check(historicalPresenter.CanConvertHistoricalScene(),
            "historical conversion action becomes available in read-only mode");
        check(!historicalPresenter.CanUndo() &&
            !historicalPresenter.CanRedo() &&
            !historicalPresenter.SetToolMode(EditorToolMode::Move) &&
            historicalPresenter.GetToolMode() == EditorToolMode::Select &&
            !historicalPresenter.SelectAsset("demo.actor") &&
            !historicalPresenter.PlaceAt({}),
            "history, mutation tools, and placement are unavailable");

        check(historicalTree.SelectByLabel("duplicate"),
            "historical item can be selected");
        historicalPresenter.RefreshSelection();
        check(historicalProperties.lastProperties.Find("stable_record_id") &&
            historicalProperties.lastProperties.Find("class_id") &&
            historicalProperties.lastProperties.Find("source_offset") &&
            !historicalProperties.Apply("label", "changed"),
            "historical selection exposes read-only provenance properties");
        const EditorHistoricalSceneObjectData& first =
            historicalDocument.Objects().front();
        EditorTreeNode* firstNode = historicalDocument.GetTreeModel().FindByPath(
            first.nodePath);
        const std::size_t itemCount = historicalTree.items.size();
        historicalPresenter.AddDemoNode("blocked", "blocked");
        historicalPresenter.DeleteSelected();
        check(!historicalPresenter.MoveSelectedTo("Historical Scene") &&
            firstNode && !historicalPresenter.RenameNode(
                *firstNode, "changed", &reason) &&
            !historicalPresenter.SetLogicalTransform(first.nodePath, {}) &&
            !historicalPresenter.SaveSnapshot("blocked.wx_tree_snapshot") &&
            !historicalPresenter.Undo() && !historicalPresenter.Redo() &&
            historicalTree.items.size() == itemCount &&
            firstNode->Label() == "duplicate",
            "all mutation routes reject without changing historical data");
        check(historicalPresenter.FindFirst("duplicate") == 2 &&
            historicalPresenter.SelectLogicalPath(first.stableRecordId) &&
            preview.SelectedPath() == first.stableRecordId,
            "find and preview-to-tree selection remain enabled");

        EditorHistoricalConversionOptions conversionOptions;
        EditorHistoricalConversionReport conversionPreview;
        check(historicalPresenter.PreviewHistoricalConversion(
                conversionOptions, conversionPreview, &reason) &&
            conversionPreview.totalHistoricalRecords == 2 &&
            conversionPreview.placeholderConverted == 2,
            "presenter exposes deterministic conversion preview counts");
        EditorHistoricalConversionReport conversionReport;
        check(historicalPresenter.ConvertHistoricalSceneToEditableCopy(
                conversionOptions, conversionReport, &reason) &&
            !historicalPresenter.IsReadOnly() &&
            !historicalPresenter.CanConvertHistoricalScene() &&
            historicalProperties.editingEnabled &&
            editableDocument.IsModified() && !editableDocument.HasFilePath() &&
            !historicalPresenter.CanUndo() &&
            historicalTree.Contains("duplicate") &&
            historicalTree.Contains("duplicate_1"),
            "presenter conversion transitions to a dirty editable copy with "
            "empty command history");
        std::string conversionSummary;
        check(historicalPresenter.HasHistoricalConversionSummary() &&
            historicalPresenter.GetHistoricalConversionSummary(
                conversionSummary, &reason) &&
            conversionSummary.find("Original scene: fixture.level") !=
                std::string::npos &&
            conversionSummary.find("cannot be exported") != std::string::npos,
            "editable conversion exposes a provenance-only summary");

        historicalPresenter.NewDocument();
        check(!historicalPresenter.IsReadOnly() &&
            !historicalPresenter.HasHistoricalConversionSummary() &&
            historicalProperties.editingEnabled &&
            historicalPresenter.SetToolMode(EditorToolMode::Move) &&
            editableDocument.Model().Root() && !editableDocument.IsModified(),
            "New restores normal editable behavior");
    }

    return failures;
}
