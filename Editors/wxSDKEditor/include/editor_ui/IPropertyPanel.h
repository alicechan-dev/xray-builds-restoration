#ifndef XR_WX_SDK_EDITOR_I_PROPERTY_PANEL_H
#define XR_WX_SDK_EDITOR_I_PROPERTY_PANEL_H

#include <functional>
#include <string>

class EditorPropertySet;

class IPropertyPanel
{
public:
    using ApplyHandler =
        std::function<bool(const std::string&, const std::string&)>;

    virtual ~IPropertyPanel() = default;

    virtual void Clear() = 0;
    virtual void ShowPlaceholder(const char* text) = 0;
    virtual void ShowProperties(const EditorPropertySet& properties) = 0;
    virtual void SetApplyHandler(ApplyHandler handler) = 0;
    virtual void SetEditingEnabled(bool enabled) = 0;
};

#endif
