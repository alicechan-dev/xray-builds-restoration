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

bool ContainsNode(const EditorTreeNode* root, const EditorTreeNode* candidate)
{
    if (!root)
        return false;
    if (root == candidate)
        return true;
    for (const auto& child : root->ChildrenView())
    {
        if (ContainsNode(child.get(), candidate))
            return true;
    }
    return false;
}
}

EditorTreeNode::EditorTreeNode(
    std::string label, std::string category, EditorItemKind kind,
    EditorTreeNode* parent) :
    label_(std::move(label)), category_(std::move(category)), kind_(kind),
    parent_(parent)
{
    RefreshPath();
}

void EditorTreeNode::RefreshPath()
{
    path_ = parent_ ? parent_->path_ + "/" + label_ : label_;
    for (const auto& child : children_)
        child->RefreshPath();
}

EditorTreeNode& EditorTreeModel::CreateRoot(
    std::string label, std::string category, EditorItemKind kind)
{
    root_ = std::unique_ptr<EditorTreeNode>(
        new EditorTreeNode(
            std::move(label), std::move(category), kind, nullptr));
    return *root_;
}

EditorTreeNode& EditorTreeModel::AddChild(
    EditorTreeNode& parent, std::string label, std::string category,
    EditorItemKind kind)
{
    if (kind == EditorItemKind::Unknown)
        kind = InferEditorItemKind(category);
    parent.children_.push_back(std::unique_ptr<EditorTreeNode>(
        new EditorTreeNode(
            std::move(label), std::move(category), kind, &parent)));
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

EditorTreeNode* EditorTreeModel::FindChildCaseInsensitive(
    EditorTreeNode& parent, const std::string& label)
{
    for (const auto& child : parent.children_)
    {
        if (EqualIgnoringCase(child->label_, label))
            return child.get();
    }
    return nullptr;
}

std::string EditorTreeModel::MakeUniqueChildName(
    EditorTreeNode& parent, const std::string& baseName)
{
    if (!FindChildCaseInsensitive(parent, baseName))
        return baseName;

    for (unsigned int suffix = 1;; ++suffix)
    {
        const std::string candidate = baseName + "_" + std::to_string(suffix);
        if (!FindChildCaseInsensitive(parent, candidate))
            return candidate;
    }
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

bool EditorTreeModel::CanDeleteNode(
    const EditorTreeNode& node, std::string* reason) const
{
    if (&node == root_.get())
        return Fail(reason, "The root node cannot be deleted.");
    if (!node.parent_)
        return Fail(reason, "The node is not attached to this model.");

    const auto& siblings = node.parent_->children_;
    const auto owned = std::find_if(siblings.begin(), siblings.end(),
        [&node](const std::unique_ptr<EditorTreeNode>& candidate) {
            return candidate.get() == &node;
        });
    if (owned == siblings.end())
        return Fail(reason, "The node is not owned by this model.");

    if (reason)
        reason->clear();
    return true;
}

bool EditorTreeModel::DeleteNode(EditorTreeNode& node, std::string* reason)
{
    if (!CanDeleteNode(node, reason))
        return false;

    auto& siblings = node.parent_->children_;
    const auto owned = std::find_if(siblings.begin(), siblings.end(),
        [&node](const std::unique_ptr<EditorTreeNode>& candidate) {
            return candidate.get() == &node;
        });
    siblings.erase(owned);
    if (reason)
        reason->clear();
    return true;
}

bool EditorTreeModel::CanMoveNode(const EditorTreeNode& node,
    const EditorTreeNode& newParent, std::string* reason) const
{
    if (!root_ || !ContainsNode(root_.get(), &node) ||
        !ContainsNode(root_.get(), &newParent))
        return Fail(reason, "Both nodes must belong to this model.");
    if (&node == root_.get())
        return Fail(reason, "The root node cannot be moved.");
    if (&node == &newParent)
        return Fail(reason, "A node cannot be moved under itself.");
    if (!IsGroupKind(newParent.kind_))
        return Fail(reason, "Only the root or a folder can contain children.");
    if (node.parent_ == &newParent)
        return Fail(reason, "The node already belongs to this parent.");

    for (const EditorTreeNode* ancestor = &newParent; ancestor;
         ancestor = ancestor->parent_)
    {
        if (ancestor == &node)
            return Fail(reason, "A node cannot be moved under one of its descendants.");
    }

    for (const auto& child : newParent.children_)
    {
        if (EqualIgnoringCase(child->label_, node.label_))
            return Fail(reason, "A child with this name already exists at the destination.");
    }

    if (reason)
        reason->clear();
    return true;
}

bool EditorTreeModel::MoveNode(EditorTreeNode& node,
    EditorTreeNode& newParent, std::string* reason)
{
    if (!CanMoveNode(node, newParent, reason))
        return false;

    auto& oldChildren = node.parent_->children_;
    const auto owned = std::find_if(oldChildren.begin(), oldChildren.end(),
        [&node](const std::unique_ptr<EditorTreeNode>& candidate) {
            return candidate.get() == &node;
        });
    if (owned == oldChildren.end())
        return Fail(reason, "The node is not owned by its recorded parent.");

    newParent.children_.reserve(newParent.children_.size() + 1);
    std::unique_ptr<EditorTreeNode> moved = std::move(*owned);
    oldChildren.erase(owned);
    moved->parent_ = &newParent;
    moved->RefreshPath();
    newParent.children_.push_back(std::move(moved));

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

    EditorTreeNode* objects = model.FindByPath("Scene (demo data)/Objects");
    if (!objects)
        return Fail(failureReason, "Demo objects group was not found.");
    const std::string firstName = model.MakeUniqueChildName(*objects, "new_object");
    EditorTreeNode& first = model.AddChild(*objects, firstName, "demo scene object");
    if (first.Label() != "new_object" ||
        first.Path() != "Scene (demo data)/Objects/new_object")
        return Fail(failureReason, "First generated child name or path is invalid.");
    const std::string secondName = model.MakeUniqueChildName(*objects, "new_object");
    if (secondName != "new_object_1")
        return Fail(failureReason, "Duplicate child name was not made unique.");
    EditorTreeNode& second = model.AddChild(*objects, secondName, "demo scene object");
    if (model.FindChildCaseInsensitive(*objects, "NEW_OBJECT_1") != &second)
        return Fail(failureReason, "Case-insensitive child lookup failed.");

    if (model.DeleteNode(*model.Root(), failureReason))
        return Fail(failureReason, "Root deletion was accepted.");
    const std::string deletedPath = second.Path();
    if (!model.DeleteNode(second, failureReason))
        return false;
    if (model.FindByPath(deletedPath))
        return Fail(failureReason, "Deleted child remains in path lookup.");

    if (failureReason)
        failureReason->clear();
    return true;
}
