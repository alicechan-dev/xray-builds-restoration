#include "editor_model/EditorPropertySet.h"

#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace
{
bool EqualIgnoringCase(std::string_view left, std::string_view right)
{
    return left.size() == right.size() &&
        std::equal(left.begin(), left.end(), right.begin(),
            [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
}

EditorProperty MakeProperty(const char* key, const char* label,
    EditorPropertyType type, std::string value, bool readOnly,
    const char* description)
{
    EditorProperty property;
    property.key = key;
    property.label = label;
    property.type = type;
    property.value = std::move(value);
    property.readOnly = readOnly;
    property.description = description;
    return property;
}
}

void EditorPropertySet::Add(EditorProperty property)
{
    properties_.push_back(std::move(property));
}

const EditorProperty* EditorPropertySet::Find(std::string_view key) const
{
    const auto found = std::find_if(properties_.begin(), properties_.end(),
        [key](const EditorProperty& property) {
            return EqualIgnoringCase(property.key, key);
        });
    return found == properties_.end() ? nullptr : &*found;
}

EditorPropertySet BuildEditorNodePropertySet(const EditorTreeNode& node)
{
    EditorPropertySet properties;
    properties.Add(MakeProperty("label", "Label", EditorPropertyType::String,
        node.Label(), false, "Tree label and generated path component."));
    properties.Add(MakeProperty("category", "Category", EditorPropertyType::String,
        node.Category(), false, "Development-only display category."));
    properties.Add(MakeProperty("kind", "Kind", EditorPropertyType::ReadOnlyText,
        std::string(ToString(node.Kind())), true, "Audited structural item kind."));
    properties.Add(MakeProperty("path", "Path", EditorPropertyType::ReadOnlyText,
        node.Path(), true, "Generated canonical hierarchy path."));
    return properties;
}

EditorPropertyApplyResult ApplyEditorNodeProperty(EditorTreeModel& model,
    EditorTreeNode& node, std::string_view key, std::string value)
{
    EditorPropertyApplyResult result;
    const EditorPropertySet properties = BuildEditorNodePropertySet(node);
    const EditorProperty* property = properties.Find(key);
    if (!property)
    {
        result.reason = "Unknown property key.";
        return result;
    }
    if (property->readOnly)
    {
        result.reason = "This property is read-only.";
        return result;
    }

    if (EqualIgnoringCase(key, "label"))
    {
        if (!model.RenameNode(node, std::move(value), &result.reason))
            return result;
        result.requiresTreeRebuild = true;
    }
    else if (EqualIgnoringCase(key, "category"))
    {
        model.SetNodeCategory(node, std::move(value));
    }

    result.success = true;
    result.requiresPropertyRefresh = true;
    return result;
}
