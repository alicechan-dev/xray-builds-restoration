#ifndef XR_WX_SDK_EDITOR_EDITOR_PROPERTY_H
#define XR_WX_SDK_EDITOR_EDITOR_PROPERTY_H

#include <string>
#include <string_view>
#include <vector>

enum class EditorPropertyType
{
    String,
    Integer,
    Float,
    Boolean,
    Choice,
    ReadOnlyText
};

std::string_view ToString(EditorPropertyType type);

struct EditorProperty
{
    std::string section;
    std::string key;
    std::string label;
    EditorPropertyType type = EditorPropertyType::String;
    std::string value;
    bool readOnly = false;
    std::vector<std::string> choices;
    std::string description;
};

#endif
