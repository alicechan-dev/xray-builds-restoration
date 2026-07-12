#ifndef XR_WX_SDK_EDITOR_EDITOR_PROPERTY_SET_H
#define XR_WX_SDK_EDITOR_EDITOR_PROPERTY_SET_H

#include "editor_model/EditorProperty.h"

class EditorTreeModel;
class EditorTreeNode;

class EditorPropertySet
{
public:
    void Add(EditorProperty property);
    const EditorProperty* Find(std::string_view key) const;
    const std::vector<EditorProperty>& Properties() const { return properties_; }

private:
    std::vector<EditorProperty> properties_;
};

struct EditorPropertyApplyResult
{
    bool success = false;
    std::string reason;
    bool requiresTreeRebuild = false;
    bool requiresPropertyRefresh = false;
};

EditorPropertySet BuildEditorNodePropertySet(const EditorTreeNode& node);
EditorPropertyApplyResult ApplyEditorNodeProperty(EditorTreeModel& model,
    EditorTreeNode& node, std::string_view key, std::string value);

#endif
