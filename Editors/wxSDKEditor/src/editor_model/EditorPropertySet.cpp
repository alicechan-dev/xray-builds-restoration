#include "editor_model/EditorPropertySet.h"

#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"

#include <algorithm>
#include <cctype>
#include <utility>
#include <cstdlib>
#include <cerrno>
#include <cmath>

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

bool ParseFiniteFloat(const std::string& text, float& value)
{
    char* end=nullptr; errno=0; value=std::strtof(text.c_str(),&end);
    return end!=text.c_str() && *end=='\0' && errno!=ERANGE && std::isfinite(value);
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
    properties.Add(MakeProperty("asset_id", "Asset ID",
        EditorPropertyType::ReadOnlyText,
        node.AssetId().empty() ? "none" : node.AssetId(), true,
        "Optional synthetic catalog prototype identity."));
    if (!IsGroupKind(node.Kind()))
    {
        const EditorTransform& transform = node.Transform();
        properties.Add(MakeProperty("position.x", "Position X",
            EditorPropertyType::String, std::to_string(transform.x), false,
            "Development transform X."));
        properties.Add(MakeProperty("position.y", "Position Y",
            EditorPropertyType::String, std::to_string(transform.y), false,
            "Development transform Y."));
        properties.Add(MakeProperty("position.z", "Position Z",
            EditorPropertyType::String, std::to_string(transform.z), false,
            "Development transform Z."));
    }
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
    else if (key.rfind("position.", 0) == 0)
    {
        float parsed = 0.0f;
        if (!ParseFiniteFloat(value, parsed))
        {
            result.reason = "Position must be a finite number.";
            return result;
        }
        EditorTransform transform = node.Transform();
        if (EqualIgnoringCase(key, "position.x")) transform.x = parsed;
        else if (EqualIgnoringCase(key, "position.y")) transform.y = parsed;
        else if (EqualIgnoringCase(key, "position.z")) transform.z = parsed;
        else
        {
            result.reason = "Unknown position property.";
            return result;
        }
        if (!model.SetNodeTransform(node, transform, &result.reason))
            return result;
    }

    result.success = true;
    result.requiresPropertyRefresh = true;
    return result;
}
