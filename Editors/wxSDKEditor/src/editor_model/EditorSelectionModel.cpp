#include "editor_model/EditorSelectionModel.h"

#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorTreeQuery.h"

#include <algorithm>
#include <cctype>

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
}

void EditorSelectionModel::Clear()
{
    paths_.clear();
}

bool EditorSelectionModel::ContainsPath(const std::string& path) const
{
    return std::any_of(paths_.begin(), paths_.end(),
        [&path](const std::string& selected) {
            return EqualIgnoringCase(selected, path);
        });
}

void EditorSelectionModel::Select(const EditorTreeNode* node)
{
    if (node && !ContainsPath(node->Path()))
        paths_.push_back(node->Path());
}

void EditorSelectionModel::Deselect(const EditorTreeNode* node)
{
    if (!node)
        return;
    paths_.erase(std::remove_if(paths_.begin(), paths_.end(),
        [node](const std::string& selected) {
            return EqualIgnoringCase(selected, node->Path());
        }), paths_.end());
}

void EditorSelectionModel::Toggle(const EditorTreeNode* node)
{
    if (!node)
        return;
    if (IsSelected(node))
        Deselect(node);
    else
        Select(node);
}

bool EditorSelectionModel::IsSelected(const EditorTreeNode* node) const
{
    return node && ContainsPath(node->Path());
}

std::vector<const EditorTreeNode*> EditorSelectionModel::GetSelectedNodes(
    const EditorTreeModel& model, std::optional<EditorItemKind> kind) const
{
    EditorTreeQueryOptions query;
    query.kind = kind;
    const EditorTreeQueryResult nodes = QueryEditorTree(model, query);
    std::vector<const EditorTreeNode*> selected;
    for (const EditorTreeNode* node : nodes)
    {
        if (ContainsPath(node->Path()))
            selected.push_back(node);
    }
    return selected;
}

std::vector<std::string> EditorSelectionModel::GetSelectedPaths(
    const EditorTreeModel& model, std::string prefix,
    std::optional<EditorItemKind> kind) const
{
    std::vector<std::string> paths;
    for (const EditorTreeNode* node : GetSelectedNodes(model, kind))
    {
        if (prefix.empty() || node->Path().rfind(prefix, 0) == 0)
            paths.push_back(node->Path());
    }
    return paths;
}

std::vector<std::string> EditorSelectionModel::GetSelectedLabels(
    const EditorTreeModel& model, std::string prefix,
    std::optional<EditorItemKind> kind) const
{
    std::vector<std::string> labels;
    for (const EditorTreeNode* node : GetSelectedNodes(model, kind))
    {
        if (prefix.empty() || node->Path().rfind(prefix, 0) == 0)
            labels.push_back(node->Label());
    }
    return labels;
}

void EditorSelectionModel::Prune(const EditorTreeModel& model)
{
    const std::vector<const EditorTreeNode*> nodes = GetSelectedNodes(model);
    paths_.clear();
    for (const EditorTreeNode* node : nodes)
        paths_.push_back(node->Path());
}

void EditorSelectionModel::RemapPathPrefix(
    const std::string& oldPrefix, const std::string& newPrefix)
{
    if (oldPrefix.empty())
        return;

    for (std::string& path : paths_)
    {
        if (path.size() < oldPrefix.size() ||
            !EqualIgnoringCase(path.substr(0, oldPrefix.size()), oldPrefix) ||
            (path.size() != oldPrefix.size() && path[oldPrefix.size()] != '/'))
            continue;
        path = newPrefix + path.substr(oldPrefix.size());
    }
}
