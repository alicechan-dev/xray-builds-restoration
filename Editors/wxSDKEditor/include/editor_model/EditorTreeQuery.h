#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_QUERY_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_QUERY_H

#include "editor_model/EditorItemType.h"

#include <optional>
#include <string>
#include <vector>

class EditorTreeModel;
class EditorTreeNode;

struct EditorTreeQueryOptions
{
    std::string text;
    bool matchLabel = true;
    bool matchPath = false;
    bool caseSensitive = false;
    bool exactMatch = false;
    std::optional<EditorItemKind> kind;
};

using EditorTreeQueryResult = std::vector<const EditorTreeNode*>;

EditorTreeQueryResult QueryEditorTree(
    const EditorTreeModel& model, const EditorTreeQueryOptions& options);

#endif
