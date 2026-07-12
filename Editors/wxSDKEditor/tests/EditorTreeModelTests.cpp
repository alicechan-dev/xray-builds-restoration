#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorPropertySet.h"
#include "editor_model/EditorItemType.h"
#include "editor_model/EditorSelectionModel.h"
#include "editor_model/EditorTreePathListImport.h"
#include "editor_model/EditorTreeQuery.h"
#include "editor_model/EditorTreeSnapshot.h"

#include <iostream>
#include <string>

namespace
{
int failures = 0;

void Check(bool condition, const char* message)
{
    if (condition)
        return;
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

bool RejectsSnapshot(EditorTreeModel& model, const std::string& snapshot)
{
    std::string reason;
    return !DeserializeEditorTreeSnapshot(model, snapshot, &reason) && !reason.empty();
}

const char* Header = "# wxSDKEditor tree snapshot v1\n";
}

int RunEditorTreePresenterTests();

int main()
{
    EditorItemKind parsedKind = EditorItemKind::Unknown;
    Check(ToString(EditorItemKind::Folder) == "folder",
        "folder kind string conversion");
    Check(ParseEditorItemKind("OBJECT", parsedKind) &&
        parsedKind == EditorItemKind::Object,
        "known item kind parsed case-insensitively");
    Check(!ParseEditorItemKind("light", parsedKind),
        "unknown item kind text rejected");
    Check(IsGroupKind(EditorItemKind::Root) &&
        IsGroupKind(EditorItemKind::Folder),
        "root and folder classified as groups");
    Check(IsLeafKind(EditorItemKind::Object) &&
        !IsLeafKind(EditorItemKind::Unknown),
        "object classified as leaf without guessing unknown");

    EditorTreeModel model = EditorTreeModel::CreateDemoScene();
    Check(model.Root() != nullptr, "demo root exists");
    Check(model.FindByPath("Scene (demo data)/Objects/actor") != nullptr,
        "known demo path exists");
    Check(model.Root()->Kind() == EditorItemKind::Root,
        "demo root kind");

    Check(ToString(EditorPropertyType::String) == "string" &&
        ToString(EditorPropertyType::Integer) == "integer" &&
        ToString(EditorPropertyType::Float) == "float" &&
        ToString(EditorPropertyType::Boolean) == "boolean" &&
        ToString(EditorPropertyType::Choice) == "choice" &&
        ToString(EditorPropertyType::ReadOnlyText) == "read-only text",
        "property type names");

    EditorTreeNode* objects = model.FindByPath("Scene (demo data)/Objects");
    Check(objects != nullptr, "objects group exists");
    Check(objects && objects->Kind() == EditorItemKind::Folder,
        "demo group kind");
    EditorTreeNode* demoActor =
        model.FindByPath("Scene (demo data)/Objects/actor");
    Check(demoActor && demoActor->Kind() == EditorItemKind::Object,
        "demo scene object kind");
    EditorTreeNode* demoLight =
        model.FindByPath("Scene (demo data)/Lights/sun");
    Check(demoLight && demoLight->Kind() == EditorItemKind::Object,
        "demo light remains generic object kind");
    EditorTreeNode* demoSound =
        model.FindByPath("Scene (demo data)/Sounds/ambient");
    Check(demoSound && demoSound->Kind() == EditorItemKind::Object,
        "demo sound remains generic object kind");
    EditorTreeNode* demoSpawn =
        model.FindByPath("Scene (demo data)/Spawn Elements");
    Check(demoSpawn && demoSpawn->Kind() == EditorItemKind::Folder,
        "demo spawn collection remains folder kind");
    if (objects)
    {
        const std::string name0 = model.MakeUniqueChildName(*objects, "new_object");
        EditorTreeNode& object0 = model.AddChild(*objects, name0, "demo scene object");
        const std::string name1 = model.MakeUniqueChildName(*objects, "new_object");
        EditorTreeNode& object1 = model.AddChild(*objects, name1, "demo scene object");
        const std::string name2 = model.MakeUniqueChildName(*objects, "new_object");
        Check(name0 == "new_object", "first unique child name");
        Check(name1 == "new_object_1", "second unique child name");
        Check(name2 == "new_object_2", "third unique child name");
        Check(model.FindChildCaseInsensitive(*objects, "NEW_OBJECT_1") == &object1,
            "case-insensitive child lookup");

        std::string reason;
        Check(model.RenameNode(object0, "renamed_object", &reason),
            "unique rename accepted");
        Check(object0.Path() == "Scene (demo data)/Objects/renamed_object",
            "renamed node path refreshed");
        Check(!model.RenameNode(object0, "", &reason), "empty rename rejected");
        Check(!model.RenameNode(object0, "NEW_OBJECT_1", &reason),
            "case-insensitive duplicate rename rejected");

        EditorTreeNode& parent = model.AddChild(*objects, "parent", "demo group");
        EditorTreeNode& descendant = model.AddChild(parent, "child", "demo scene object");
        Check(model.RenameNode(parent, "renamed_parent", &reason),
            "parent rename accepted");
        Check(descendant.Path() ==
            "Scene (demo data)/Objects/renamed_parent/child",
            "descendant path refreshed after parent rename");
        const std::string descendantPath = descendant.Path();
        Check(model.DeleteNode(parent, &reason), "child subtree deletion accepted");
        Check(model.FindByPath(descendantPath) == nullptr,
            "deleted descendant removed from lookup");
    }

    EditorTreeModel propertyModel = EditorTreeModel::CreateDemoScene();
    EditorTreeNode* propertyActor =
        propertyModel.FindByPath("Scene (demo data)/Objects/actor");
    EditorTreeNode* propertyObjects =
        propertyModel.FindByPath("Scene (demo data)/Objects");
    Check(propertyActor && propertyObjects, "property test fixtures exist");
    if (propertyActor && propertyObjects)
    {
        EditorTreeNode& propertyChild = propertyModel.AddChild(
            *propertyActor, "child", "demo scene object", EditorItemKind::Object);
        EditorPropertySet propertySet = BuildEditorNodePropertySet(*propertyActor);
        Check(propertySet.Properties().size() == 4 &&
            propertySet.Find("LABEL") && propertySet.Find("category") &&
            propertySet.Find("kind") && propertySet.Find("path"),
            "node property set contains stable keys");
        Check(!propertySet.Find("label")->readOnly &&
            !propertySet.Find("category")->readOnly &&
            propertySet.Find("kind")->readOnly &&
            propertySet.Find("path")->readOnly,
            "node property editability");

        EditorPropertyApplyResult apply = ApplyEditorNodeProperty(
            propertyModel, *propertyActor, "label", "stalker");
        Check(apply.success && apply.requiresTreeRebuild &&
            apply.requiresPropertyRefresh,
            "label property apply result flags");
        Check(propertyActor->Path() == "Scene (demo data)/Objects/stalker" &&
            propertyChild.Path() == "Scene (demo data)/Objects/stalker/child",
            "label property refreshes node and descendant paths");
        const std::string acceptedPath = propertyActor->Path();
        apply = ApplyEditorNodeProperty(propertyModel, *propertyActor, "label", "");
        Check(!apply.success && propertyActor->Path() == acceptedPath,
            "empty label property rejected without mutation");
        apply = ApplyEditorNodeProperty(
            propertyModel, *propertyActor, "label", "PHYSIC_OBJECT");
        Check(!apply.success && propertyActor->Label() == "stalker",
            "case-insensitive duplicate label property rejected");
        apply = ApplyEditorNodeProperty(
            propertyModel, *propertyActor, "category", "custom display");
        Check(apply.success && !apply.requiresTreeRebuild &&
            apply.requiresPropertyRefresh &&
            propertyActor->Category() == "custom display",
            "category property applies without tree rebuild");
        apply = ApplyEditorNodeProperty(propertyModel, *propertyActor, "kind", "folder");
        Check(!apply.success && propertyActor->Kind() == EditorItemKind::Object,
            "read-only kind property rejected");
        apply = ApplyEditorNodeProperty(propertyModel, *propertyActor, "path", "other");
        Check(!apply.success && propertyActor->Path() == acceptedPath,
            "read-only path property rejected");
    }

    std::string reason;
    Check(!model.DeleteNode(*model.Root(), &reason), "root deletion rejected");

    EditorTreeNode& escaped = model.AddChild(*model.Root(),
        "quoted\"\\line\nitem", "category\tvalue");
    std::string snapshot;
    Check(SerializeEditorTreeSnapshot(model, snapshot, &reason),
        "snapshot serialization succeeds");
    Check(snapshot.find("# wxSDKEditor tree snapshot v2\n") == 0,
        "snapshot writer emits v2 header");

    EditorTreeModel loaded;
    Check(DeserializeEditorTreeSnapshot(loaded, snapshot, &reason),
        "snapshot deserialization succeeds");
    Check(loaded.FindByPath("Scene (demo data)/Objects/actor") != nullptr,
        "known path survives snapshot round trip");
    Check(loaded.FindByPath("Scene (demo data)/Objects/actor")->Kind() ==
        EditorItemKind::Object,
        "item kind survives v2 snapshot round trip");
    EditorTreeNode* loadedEscaped = loaded.FindByLabel(escaped.Label());
    Check(loadedEscaped != nullptr, "escaped label survives snapshot round trip");
    Check(loadedEscaped && loadedEscaped->Category() == "category\tvalue",
        "escaped category survives snapshot round trip");

    EditorTreeModel preserved = EditorTreeModel::CreateDemoScene();
    Check(RejectsSnapshot(preserved, "bad header\n"), "malformed header rejected");
    Check(preserved.FindByPath("Scene (demo data)/Objects/actor") != nullptr,
        "failed load preserves existing model");
    Check(RejectsSnapshot(preserved, std::string(Header) + "bad node\n"),
        "malformed node line rejected");
    Check(RejectsSnapshot(preserved, std::string(Header) +
        "node depth=0 label=\"Root\" category=\"root\" path=\"Root\"\n"
        "node depth=2 label=\"child\" category=\"item\" path=\"Root/child\"\n"),
        "invalid hierarchy depth rejected");
    Check(RejectsSnapshot(preserved, std::string(Header) +
        "node depth=0 label=\"\" category=\"root\" path=\"\"\n"),
        "empty label rejected");
    Check(RejectsSnapshot(preserved, std::string(Header) +
        "node depth=0 label=\"Root\" category=\"root\" path=\"Root\"\n"
        "node depth=1 label=\"child\" category=\"item\" path=\"Root/child\"\n"
        "node depth=1 label=\"CHILD\" category=\"item\" path=\"Root/CHILD\"\n"),
        "case-insensitive duplicate sibling rejected");
    Check(RejectsSnapshot(preserved, std::string(Header) +
        "node depth=0 label=\"Root\" category=\"root\" path=\"Wrong\"\n"),
        "stored path mismatch rejected");

    const std::string legacySnapshot = std::string(Header) +
        "node depth=0 label=\"Legacy\" category=\"custom root\" path=\"Legacy\"\n"
        "node depth=1 label=\"Folder\" category=\"demo group\" path=\"Legacy/Folder\"\n"
        "node depth=2 label=\"Item\" category=\"custom category\" path=\"Legacy/Folder/Item\"\n";
    EditorTreeModel legacyLoaded;
    Check(DeserializeEditorTreeSnapshot(legacyLoaded, legacySnapshot, &reason),
        "v1 snapshot remains readable");
    Check(legacyLoaded.Root()->Kind() == EditorItemKind::Root,
        "v1 structural root inferred as root kind");
    Check(legacyLoaded.FindByPath("Legacy/Folder")->Kind() ==
        EditorItemKind::Folder,
        "v1 known group category inferred as folder kind");
    Check(legacyLoaded.FindByPath("Legacy/Folder/Item")->Kind() ==
        EditorItemKind::Unknown,
        "v1 custom category remains unknown kind");

    const std::string pathList =
        "# wxSDKEditor path list v1\n"
        "\n"
        "/Scene/Objects/actor | demo scene object\n"
        "/Scene/Objects/level_changer\n"
        "/Scene/Lights/sun | demo light\n";
    EditorTreeModel imported;
    Check(ImportEditorTreePathList(imported, pathList, &reason),
        "path-list import succeeds");
    Check(imported.Root() && imported.Root()->Label() == "Scene",
        "path-list root created from first component");
    EditorTreeNode* importedObjects = imported.FindByPath("Scene/Objects");
    Check(importedObjects && importedObjects->Category() == "imported group",
        "implicit group uses imported group category");
    Check(importedObjects && importedObjects->Kind() == EditorItemKind::Folder,
        "implicit group maps to folder kind");
    EditorTreeNode* defaultCategory =
        imported.FindByPath("Scene/Objects/level_changer");
    Check(defaultCategory && defaultCategory->Category() == "imported item",
        "missing category uses imported item default");
    Check(defaultCategory && defaultCategory->Kind() == EditorItemKind::Object,
        "default imported item maps to object kind");
    Check(imported.FindByPath("Scene/Lights/sun") != nullptr,
        "comments and blank lines are ignored");

    EditorTreeModel customImported;
    Check(ImportEditorTreePathList(customImported,
        "/Custom/Items/value | custom category\n", &reason),
        "custom category import succeeds");
    Check(customImported.FindByPath("Custom/Items/value")->Kind() ==
        EditorItemKind::Unknown,
        "custom category falls back to unknown kind");

    EditorTreeModel queryModel = EditorTreeModel::CreateDemoScene();
    queryModel.AddChild(*queryModel.Root(), "custom", "custom category");
    EditorTreeQueryOptions query;
    query.text = "ACT";
    EditorTreeQueryResult queryResults = QueryEditorTree(queryModel, query);
    Check(queryResults.size() == 1 && queryResults.front()->Label() == "actor",
        "default query is case-insensitive label substring");

    query.exactMatch = true;
    query.text = "ACT";
    Check(QueryEditorTree(queryModel, query).empty(),
        "exact query compares the complete label");
    query.text = "actor";
    Check(QueryEditorTree(queryModel, query).size() == 1,
        "exact label query succeeds");

    query.matchLabel = false;
    query.matchPath = true;
    query.exactMatch = false;
    query.text = "objects/actor";
    Check(QueryEditorTree(queryModel, query).size() == 1,
        "query can match model-generated path");
    query.caseSensitive = true;
    query.text = "Objects/Actor";
    Check(QueryEditorTree(queryModel, query).empty(),
        "case-sensitive query preserves case");

    query = {};
    query.kind = EditorItemKind::Folder;
    queryResults = QueryEditorTree(queryModel, query);
    Check(queryResults.size() == 5 && queryResults.front()->Label() == "Objects",
        "empty folder query returns folders in traversal order");
    query.kind = EditorItemKind::Object;
    Check(QueryEditorTree(queryModel, query).size() == 6,
        "object kind filter");
    query.kind = EditorItemKind::Root;
    Check(QueryEditorTree(queryModel, query).size() == 1,
        "root kind filter");
    query.kind = EditorItemKind::Unknown;
    queryResults = QueryEditorTree(queryModel, query);
    Check(queryResults.size() == 1 && queryResults.front()->Label() == "custom",
        "unknown kind filter");

    query = {};
    queryResults = QueryEditorTree(queryModel, query);
    Check(queryResults.size() == 13 &&
        queryResults[0]->Label() == "Scene (demo data)" &&
        queryResults[1]->Label() == "Objects" &&
        queryResults[2]->Label() == "actor",
        "empty query returns all nodes in stable pre-order");
    query.text = "does-not-exist";
    Check(QueryEditorTree(queryModel, query).empty(), "query no-result behavior");
    Check(queryModel.FindByPath("Scene (demo data)/Objects/actor") != nullptr,
        "query does not mutate model");

    EditorTreeModel moveModel;
    EditorTreeNode& moveRoot = moveModel.CreateRoot(
        "Root", "test root", EditorItemKind::Root);
    EditorTreeNode& folderA = moveModel.AddChild(
        moveRoot, "FolderA", "test folder", EditorItemKind::Folder);
    EditorTreeNode& folderB = moveModel.AddChild(
        moveRoot, "FolderB", "test folder", EditorItemKind::Folder);
    EditorTreeNode& movingObject = moveModel.AddChild(
        folderA, "actor", "test object", EditorItemKind::Object);
    EditorTreeNode& nestedFolder = moveModel.AddChild(
        folderA, "Nested", "test folder", EditorItemKind::Folder);
    EditorTreeNode& nestedObject = moveModel.AddChild(
        nestedFolder, "child", "test object", EditorItemKind::Object);
    EditorTreeNode& duplicate = moveModel.AddChild(
        folderB, "ACTOR", "test object", EditorItemKind::Object);
    std::string moveReason;
    Check(!moveModel.MoveNode(movingObject, folderB, &moveReason),
        "case-insensitive destination duplicate rejects move");
    Check(movingObject.Parent() == &folderA &&
        movingObject.Path() == "Root/FolderA/actor",
        "failed duplicate move leaves hierarchy unchanged");
    Check(moveModel.DeleteNode(duplicate, &moveReason),
        "duplicate fixture removed");
    EditorTreeNode* movingAddress = &movingObject;
    Check(moveModel.MoveNode(movingObject, folderB, &moveReason),
        "object moves between folders");
    Check(&movingObject == movingAddress && movingObject.Parent() == &folderB,
        "move preserves node address and updates parent");
    Check(moveModel.FindByPath("Root/FolderA/actor") == nullptr &&
        moveModel.FindByPath("Root/FolderB/actor") == &movingObject,
        "object old path stops resolving and new path resolves");
    Check(movingObject.Label() == "actor" &&
        movingObject.Category() == "test object" &&
        movingObject.Kind() == EditorItemKind::Object,
        "move preserves node metadata");
    Check(!moveModel.MoveNode(moveRoot, folderA, &moveReason),
        "root move rejected");
    Check(!moveModel.MoveNode(folderA, folderA, &moveReason),
        "self move rejected");
    Check(!moveModel.MoveNode(folderA, nestedFolder, &moveReason),
        "descendant-cycle move rejected");
    Check(!moveModel.MoveNode(nestedFolder, movingObject, &moveReason),
        "move into object rejected");

    EditorSelectionModel movedSelection;
    movedSelection.Select(&folderA);
    movedSelection.Select(&nestedObject);
    const std::string oldFolderPath = folderA.Path();
    Check(moveModel.MoveNode(folderA, folderB, &moveReason),
        "folder with descendants moves");
    movedSelection.RemapPathPrefix(oldFolderPath, folderA.Path());
    Check(folderA.Path() == "Root/FolderB/FolderA" &&
        nestedObject.Path() == "Root/FolderB/FolderA/Nested/child",
        "folder move refreshes descendant paths");
    Check(moveModel.FindByPath("Root/FolderA/Nested/child") == nullptr &&
        moveModel.FindByPath("Root/FolderB/FolderA/Nested/child") == &nestedObject,
        "folder old descendant path stops resolving and new path resolves");
    const std::vector<std::string> remappedPaths =
        movedSelection.GetSelectedPaths(moveModel);
    Check(remappedPaths.size() == 2 &&
        remappedPaths[0] == "Root/FolderB/FolderA" &&
        remappedPaths[1] == "Root/FolderB/FolderA/Nested/child",
        "selected node and descendant paths remap after folder move");
    Check(movedSelection.IsSelected(&folderA) &&
        movedSelection.IsSelected(&nestedObject),
        "remapped selection resolves deterministically");

    std::string movedSnapshot;
    EditorTreeModel movedRoundTrip;
    Check(SerializeEditorTreeSnapshot(moveModel, movedSnapshot, &moveReason) &&
        DeserializeEditorTreeSnapshot(movedRoundTrip, movedSnapshot, &moveReason),
        "moved model survives snapshot round trip");
    Check(movedRoundTrip.FindByPath(
        "Root/FolderB/FolderA/Nested/child") != nullptr,
        "moved descendant path survives snapshot round trip");

    EditorSelectionModel selection;
    EditorTreeNode* selectedActor =
        queryModel.FindByPath("Scene (demo data)/Objects/actor");
    EditorTreeNode* selectedLights =
        queryModel.FindByPath("Scene (demo data)/Lights");
    EditorTreeNode* selectedSun =
        queryModel.FindByPath("Scene (demo data)/Lights/sun");
    selection.Select(nullptr);
    selection.Select(selectedSun);
    selection.Select(selectedActor);
    selection.Select(selectedActor);
    selection.Select(selectedLights);
    Check(selection.SelectedCount() == 3,
        "selection ignores null and duplicate nodes");
    Check(selection.IsSelected(selectedActor), "selected node lookup");
    std::vector<std::string> selectedPaths =
        selection.GetSelectedPaths(queryModel);
    Check(selectedPaths.size() == 3 &&
        selectedPaths[0] == "Scene (demo data)/Objects/actor" &&
        selectedPaths[1] == "Scene (demo data)/Lights" &&
        selectedPaths[2] == "Scene (demo data)/Lights/sun",
        "selected paths follow deterministic model traversal order");
    const std::vector<std::string> selectedLabels =
        selection.GetSelectedLabels(queryModel);
    Check(selectedLabels.size() == 3 && selectedLabels[0] == "actor",
        "selected labels are distinct from full paths");
    selectedPaths = selection.GetSelectedPaths(
        queryModel, "Scene (demo data)/Lights");
    Check(selectedPaths.size() == 2,
        "selected path prefix filter uses raw path prefix");
    selectedPaths = selection.GetSelectedPaths(
        queryModel, {}, EditorItemKind::Folder);
    Check(selectedPaths.size() == 1 && selectedPaths[0] ==
        "Scene (demo data)/Lights",
        "selected kind filter");
    selection.Toggle(selectedActor);
    Check(!selection.IsSelected(selectedActor), "toggle deselects selected node");
    selection.Toggle(selectedActor);
    selection.Deselect(selectedSun);
    Check(!selection.IsSelected(selectedSun), "explicit deselect");
    selection.Select(selectedSun);
    std::string deleteReason;
    Check(queryModel.DeleteNode(*selectedSun, &deleteReason),
        "selection stale-path test deletes node");
    selection.Prune(queryModel);
    Check(selection.SelectedCount() == 2,
        "prune removes stale path after delete");
    EditorTreeModel replacement = EditorTreeModel::CreateDemoScene();
    selection.Prune(replacement);
    Check(selection.SelectedCount() == 2,
        "path selection resolves across matching model replacement");
    selection.Clear();
    Check(selection.SelectedCount() == 0, "selection clear");

    std::string importedSnapshot;
    EditorTreeModel importedRoundTrip;
    Check(SerializeEditorTreeSnapshot(imported, importedSnapshot, &reason) &&
        DeserializeEditorTreeSnapshot(importedRoundTrip, importedSnapshot, &reason),
        "imported model survives snapshot round trip");
    Check(importedRoundTrip.FindByPath("Scene/Lights/sun") != nullptr,
        "imported path survives snapshot round trip");

    EditorTreeModel importPreserved = EditorTreeModel::CreateDemoScene();
    Check(!ImportEditorTreePathList(importPreserved,
        "/Scene/Objects/actor\n/scene/objects/ACTOR\n", &reason),
        "case-insensitive duplicate full path rejected");
    Check(importPreserved.FindByPath("Scene (demo data)/Objects/actor") != nullptr,
        "failed path-list import preserves existing model");
    Check(!ImportEditorTreePathList(importPreserved,
        "Scene/Objects/actor\n", &reason),
        "relative path rejected");
    Check(!ImportEditorTreePathList(importPreserved,
        "/Scene//actor\n", &reason),
        "empty path component rejected");

    failures += RunEditorTreePresenterTests();

    if (failures)
    {
        std::cerr << failures << " editor model test(s) failed.\n";
        return 1;
    }

    std::cout << "PASS: wxSDKEditor model, snapshot, and presenter tests\n";
    return 0;
}
