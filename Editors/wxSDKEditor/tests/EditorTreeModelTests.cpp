#include "editor_model/EditorTreeModel.h"
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

    if (failures)
    {
        std::cerr << failures << " editor model test(s) failed.\n";
        return 1;
    }

    std::cout << "PASS: wxSDKEditor model and snapshot tests\n";
    return 0;
}
