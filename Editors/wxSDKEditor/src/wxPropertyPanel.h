#ifndef XR_WX_SDK_EDITOR_WX_PROPERTY_PANEL_H
#define XR_WX_SDK_EDITOR_WX_PROPERTY_PANEL_H

#include "editor_ui/IPropertyPanel.h"

#include <wx/panel.h>

class wxStaticText;
class wxTextCtrl;
class wxButton;
class wxCommandEvent;

class wxPropertyPanel final : public wxPanel, public IPropertyPanel
{
public:
    explicit wxPropertyPanel(wxWindow* parent);

    void Clear() override;
    void ShowPlaceholder(const char* text) override;
    void ShowProperties(const EditorPropertySet& properties) override;
    void SetApplyHandler(ApplyHandler handler) override;
    void SetEditingEnabled(bool enabled) override;

private:
    void OnApply(wxCommandEvent& event);

    wxStaticText* placeholder_ = nullptr;
    wxTextCtrl* label_ = nullptr;
    wxTextCtrl* category_ = nullptr;
    wxTextCtrl* kind_ = nullptr;
    wxTextCtrl* path_ = nullptr;
    wxTextCtrl* details_ = nullptr;
    wxButton* apply_ = nullptr;
    ApplyHandler applyHandler_;
};

#endif
