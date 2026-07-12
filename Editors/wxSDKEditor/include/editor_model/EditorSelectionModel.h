#ifndef XR_WX_SDK_EDITOR_EDITOR_SELECTION_MODEL_H
#define XR_WX_SDK_EDITOR_EDITOR_SELECTION_MODEL_H

#include "editor_model/EditorItemType.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

class EditorTreeModel;
class EditorTreeNode;

class EditorSelectionModel
{
public:
    void Clear();
    void Select(const EditorTreeNode* node);
    void Deselect(const EditorTreeNode* node);
    void Toggle(const EditorTreeNode* node);
    bool IsSelected(const EditorTreeNode* node) const;
    std::size_t SelectedCount() const { return paths_.size(); }

    std::vector<const EditorTreeNode*> GetSelectedNodes(
        const EditorTreeModel& model,
        std::optional<EditorItemKind> kind = std::nullopt) const;
    std::vector<std::string> GetSelectedPaths(const EditorTreeModel& model,
        std::string prefix = {},
        std::optional<EditorItemKind> kind = std::nullopt) const;
    std::vector<std::string> GetSelectedLabels(const EditorTreeModel& model,
        std::string prefix = {},
        std::optional<EditorItemKind> kind = std::nullopt) const;
    void RemapPathPrefix(const std::string& oldPrefix,
        const std::string& newPrefix);
    void Prune(const EditorTreeModel& model);

private:
    bool ContainsPath(const std::string& path) const;
    std::vector<std::string> paths_;
};

#endif
