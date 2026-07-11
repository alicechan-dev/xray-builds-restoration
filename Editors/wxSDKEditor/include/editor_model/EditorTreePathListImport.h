#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_PATH_LIST_IMPORT_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_PATH_LIST_IMPORT_H

#include <string>
#include <string_view>

class EditorTreeModel;

// Builds a tree from absolute-style logical paths. The output model is changed
// only after the complete input has been validated.
bool ImportEditorTreePathList(EditorTreeModel& output,
    std::string_view text, std::string* reason = nullptr);

#endif
