#ifndef XR_WX_SDK_EDITOR_WX_PROPERTY_PANEL_H
#define XR_WX_SDK_EDITOR_WX_PROPERTY_PANEL_H

#include "editor_ui/IPropertyPanel.h"

#include <wx/panel.h>

class wxStaticText;

class wxPropertyPanel final : public wxPanel, public IPropertyPanel
{
public:
    explicit wxPropertyPanel(wxWindow* parent);

    void Clear() override;
    void ShowPlaceholder(const char* text) override;

private:
    wxStaticText* placeholder_ = nullptr;
};

#endif
