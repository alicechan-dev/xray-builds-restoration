#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorTreePathListImport.h"
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

int main()
{
    EditorTreeModel model = EditorTreeModel::CreateDemoScene();
    Check(model.Root() != nullptr, "demo root exists");
    Check(model.FindByPath("Scene (demo data)/Objects/actor") != nullptr,
        "known demo path exists");

    EditorTreeNode* objects = model.FindByPath("Scene (demo data)/Objects");
    Check(objects != nullptr, "objects group exists");
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

    std::string reason;
    Check(!model.DeleteNode(*model.Root(), &reason), "root deletion rejected");

    EditorTreeNode& escaped = model.AddChild(*model.Root(),
        "quoted\"\\line\nitem", "category\tvalue");
    std::string snapshot;
    Check(SerializeEditorTreeSnapshot(model, snapshot, &reason),
        "snapshot serialization succeeds");

    EditorTreeModel loaded;
    Check(DeserializeEditorTreeSnapshot(loaded, snapshot, &reason),
        "snapshot deserialization succeeds");
    Check(loaded.FindByPath("Scene (demo data)/Objects/actor") != nullptr,
        "known path survives snapshot round trip");
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
    EditorTreeNode* defaultCategory =
        imported.FindByPath("Scene/Objects/level_changer");
    Check(defaultCategory && defaultCategory->Category() == "imported item",
        "missing category uses imported item default");
    Check(imported.FindByPath("Scene/Lights/sun") != nullptr,
        "comments and blank lines are ignored");

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

    if (failures)
    {
        std::cerr << failures << " editor model test(s) failed.\n";
        return 1;
    }

    std::cout << "PASS: wxSDKEditor model and snapshot tests\n";
    return 0;
}
