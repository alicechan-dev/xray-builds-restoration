#ifndef XR_WX_SDK_EDITOR_EDITOR_ITEM_TYPE_H
#define XR_WX_SDK_EDITOR_EDITOR_ITEM_TYPE_H

#include <string_view>

enum class EditorItemKind
{
    Unknown,
    Root,
    Folder,
    Object
};

std::string_view ToString(EditorItemKind kind);
bool ParseEditorItemKind(std::string_view value, EditorItemKind& kind);
EditorItemKind InferEditorItemKind(std::string_view category);
bool IsGroupKind(EditorItemKind kind);
bool IsLeafKind(EditorItemKind kind);

#endif
