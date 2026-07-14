#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_MODEL_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_MODEL_H

#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTransform.h"

#include <memory>
#include <string>
#include <vector>

class EditorTreeNode
{
public:
    using Children = std::vector<std::unique_ptr<EditorTreeNode>>;

    const std::string& Label() const { return label_; }
    const std::string& Category() const { return category_; }
    EditorItemKind Kind() const { return kind_; }
    const std::string& Path() const { return path_; }
    EditorTreeNode* Parent() { return parent_; }
    const EditorTreeNode* Parent() const { return parent_; }
    const Children& ChildrenView() const { return children_; }
    const EditorTransform& Transform() const { return transform_; }
    const std::string& AssetId() const { return assetId_; }

private:
    friend class EditorTreeModel;

    EditorTreeNode(std::string label, std::string category,
        EditorItemKind kind, EditorTreeNode* parent);
    void RefreshPath();

    std::string label_;
    std::string category_;
    EditorItemKind kind_ = EditorItemKind::Unknown;
    std::string path_;
    EditorTreeNode* parent_ = nullptr; // Non-owning; the model owns every node.
    Children children_;
    EditorTransform transform_;
    std::string assetId_;
};

class EditorTreeModel
{
public:
    EditorTreeNode& CreateRoot(std::string label, std::string category = {},
        EditorItemKind kind = EditorItemKind::Root);
    EditorTreeNode& AddChild(EditorTreeNode& parent, std::string label,
        std::string category = {},
        EditorItemKind kind = EditorItemKind::Unknown);

    EditorTreeNode* Root() { return root_.get(); }
    const EditorTreeNode* Root() const { return root_.get(); }
    EditorTreeNode* FindByPath(const std::string& path);
    const EditorTreeNode* FindByPath(const std::string& path) const;
    EditorTreeNode* FindByLabel(const std::string& label);
    const EditorTreeNode* FindByLabel(const std::string& label) const;
    EditorTreeNode* FindChildCaseInsensitive(
        EditorTreeNode& parent, const std::string& label);
    std::string MakeUniqueChildName(
        EditorTreeNode& parent, const std::string& baseName);
    bool RenameNode(EditorTreeNode& node, std::string newName,
        std::string* reason = nullptr);
    void SetNodeCategory(EditorTreeNode& node, std::string category);
    bool SetNodeTransform(EditorTreeNode& node, const EditorTransform& transform,
        std::string* reason = nullptr);
    void SetNodeAssetId(EditorTreeNode& node, std::string assetId);
    bool CanDeleteNode(const EditorTreeNode& node,
        std::string* reason = nullptr) const;
    bool DeleteNode(EditorTreeNode& node, std::string* reason = nullptr);
    bool CanMoveNode(const EditorTreeNode& node,
        const EditorTreeNode& newParent,
        std::string* reason = nullptr) const;
    bool MoveNode(EditorTreeNode& node, EditorTreeNode& newParent,
        std::string* reason = nullptr);

    static EditorTreeModel CreateDemoScene();

private:
    std::unique_ptr<EditorTreeNode> root_;
};

bool RunEditorTreeModelSelfCheck(std::string* failureReason = nullptr);

#endif
