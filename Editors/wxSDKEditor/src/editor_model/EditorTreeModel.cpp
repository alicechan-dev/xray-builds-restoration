#include "editor_model/EditorTreeModel.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace
{
bool EqualIgnoringCase(const std::string& left, const std::string& right)
{
    return left.size() == right.size() &&
        std::equal(left.begin(), left.end(), right.begin(),
            [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
}

EditorTreeNode* FindNode(EditorTreeNode* node, const std::string& value,
    const std::string& (EditorTreeNode::*accessor)() const)
{
    if (!node)
        return nullptr;
    if ((node->*accessor)() == value)
        return node;

    for (const auto& child : node->ChildrenView())
    {
        if (EditorTreeNode* found = FindNode(child.get(), value, accessor))
            return found;
    }
    return nullptr;
}

bool Fail(std::string* reason, const char* message)
{
    if (reason)
        *reason = message;
    return false;
}
}

EditorTreeNode::EditorTreeNode(
    std::string label, std::string category, EditorTreeNode* parent) :
    label_(std::move(label)), category_(std::move(category)), parent_(parent)
{
    RefreshPath();
}

void EditorTreeNode::RefreshPath()
{
    path_ = parent_ ? parent_->path_ + "/" + label_ : label_;
    for (const auto& child : children_)
        child->RefreshPath();
}

EditorTreeNode& EditorTreeModel::CreateRoot(std::string label, std::string category)
{
    root_ = std::unique_ptr<EditorTreeNode>(
        new EditorTreeNode(std::move(label), std::move(category), nullptr));
    return *root_;
}

EditorTreeNode& EditorTreeModel::AddChild(
    EditorTreeNode& parent, std::string label, std::string category)
{
    parent.children_.push_back(std::unique_ptr<EditorTreeNode>(
        new EditorTreeNode(std::move(label), std::move(category), &parent)));
    return *parent.children_.back();
}

EditorTreeNode* EditorTreeModel::FindByPath(const std::string& path)
{
    return FindNode(root_.get(), path, &EditorTreeNode::Path);
}

EditorTreeNode* EditorTreeModel::FindByLabel(const std::string& label)
{
    return FindNode(root_.get(), label, &EditorTreeNode::Label);
}

bool EditorTreeModel::RenameNode(
    EditorTreeNode& node, std::string newName, std::string* reason)
{
    if (newName.empty())
        return Fail(reason, "Name cannot be empty.");

    if (node.parent_)
    {
        for (const auto& sibling : node.parent_->children_)
        {
            if (sibling.get() != &node && EqualIgnoringCase(sibling->label_, newName))
                return Fail(reason, "A sibling with this name already exists.");
        }
    }

    node.label_ = std::move(newName);
    node.RefreshPath();
    if (reason)
        reason->clear();
    return true;
}

EditorTreeModel EditorTreeModel::CreateDemoScene()
{
    EditorTreeModel model;
    EditorTreeNode& scene = model.CreateRoot("Scene (demo data)", "demo scene root");

    EditorTreeNode& objects = model.AddChild(scene, "Objects", "demo group");
    model.AddChild(objects, "actor", "demo scene object");
    model.AddChild(objects, "level_changer", "demo scene object");
    model.AddChild(objects, "physic_object", "demo scene object");

    EditorTreeNode& lights = model.AddChild(scene, "Lights", "demo group");
    model.AddChild(lights, "sun", "demo light");
    model.AddChild(lights, "point_light", "demo light");

    EditorTreeNode& sounds = model.AddChild(scene, "Sounds", "demo group");
    model.AddChild(sounds, "ambient", "demo sound");
    model.AddChild(scene, "Sectors / Portals", "demo group");
    model.AddChild(scene, "Spawn Elements", "demo group");
    return model;
}

bool RunEditorTreeModelSelfCheck(std::string* failureReason)
{
    EditorTreeModel model = EditorTreeModel::CreateDemoScene();
    EditorTreeNode* actor = model.FindByPath("Scene (demo data)/Objects/actor");
    if (!actor)
        return Fail(failureReason, "Demo actor node was not found.");

    if (model.RenameNode(*actor, "physic_object", failureReason))
        return Fail(failureReason, "Duplicate sibling rename was accepted.");
    if (!model.RenameNode(*actor, "stalker", failureReason))
        return false;
    if (actor->Path() != "Scene (demo data)/Objects/stalker")
        return Fail(failureReason, "Unique rename did not refresh the node path.");
    if (model.RenameNode(*actor, "", failureReason))
        return Fail(failureReason, "Empty rename was accepted.");

    if (failureReason)
        failureReason->clear();
    return true;
}
