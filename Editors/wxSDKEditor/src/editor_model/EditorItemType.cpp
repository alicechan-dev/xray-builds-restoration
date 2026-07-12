#include "editor_model/EditorItemType.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace
{
std::string Lowercase(std::string_view value)
{
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
}
}

std::string_view ToString(EditorItemKind kind)
{
    switch (kind)
    {
    case EditorItemKind::Root: return "root";
    case EditorItemKind::Folder: return "folder";
    case EditorItemKind::Object: return "object";
    default: return "unknown";
    }
}

bool ParseEditorItemKind(std::string_view value, EditorItemKind& kind)
{
    const std::string normalized = Lowercase(value);
    if (normalized == "unknown") kind = EditorItemKind::Unknown;
    else if (normalized == "root") kind = EditorItemKind::Root;
    else if (normalized == "folder") kind = EditorItemKind::Folder;
    else if (normalized == "object") kind = EditorItemKind::Object;
    else return false;
    return true;
}

EditorItemKind InferEditorItemKind(std::string_view category)
{
    const std::string normalized = Lowercase(category);
    if (normalized == "demo scene root") return EditorItemKind::Root;
    if (normalized == "demo group" || normalized == "imported group")
        return EditorItemKind::Folder;
    if (normalized == "demo scene object" || normalized == "demo light" ||
        normalized == "demo sound" || normalized == "imported item" ||
        normalized == "imported object")
        return EditorItemKind::Object;
    return EditorItemKind::Unknown;
}

bool IsGroupKind(EditorItemKind kind)
{
    return kind == EditorItemKind::Root || kind == EditorItemKind::Folder;
}

bool IsLeafKind(EditorItemKind kind)
{
    return kind == EditorItemKind::Object;
}
