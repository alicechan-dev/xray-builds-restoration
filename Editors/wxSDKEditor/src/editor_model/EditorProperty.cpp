#include "editor_model/EditorProperty.h"

std::string_view ToString(EditorPropertyType type)
{
    switch (type)
    {
    case EditorPropertyType::String: return "string";
    case EditorPropertyType::Integer: return "integer";
    case EditorPropertyType::Float: return "float";
    case EditorPropertyType::Boolean: return "boolean";
    case EditorPropertyType::Choice: return "choice";
    case EditorPropertyType::ReadOnlyText: return "read-only text";
    }
    return "unknown";
}
