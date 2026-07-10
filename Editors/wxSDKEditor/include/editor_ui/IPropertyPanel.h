#ifndef XR_WX_SDK_EDITOR_I_PROPERTY_PANEL_H
#define XR_WX_SDK_EDITOR_I_PROPERTY_PANEL_H

class IPropertyPanel
{
public:
    virtual ~IPropertyPanel() = default;

    virtual void Clear() = 0;
    virtual void ShowPlaceholder(const char* text) = 0;
};

#endif
