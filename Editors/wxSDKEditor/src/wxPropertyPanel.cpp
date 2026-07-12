#include "wxPropertyPanel.h"

#include "editor_model/EditorPropertySet.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include <utility>

wxPropertyPanel::wxPropertyPanel(wxWindow* parent) : wxPanel(parent)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    placeholder_ = new wxStaticText(this, wxID_ANY, "Properties placeholder");
    sizer->Add(placeholder_, 0, wxALL, 12);

    auto* grid = new wxFlexGridSizer(2, 6, 8);
    grid->AddGrowableCol(1, 1);
    const auto addRow = [this, grid](const char* name, wxTextCtrl*& control,
                            long style = 0) {
        grid->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALIGN_CENTER_VERTICAL);
        control = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDefaultSize, style);
        grid->Add(control, 1, wxEXPAND);
    };
    addRow("Label", label_);
    addRow("Category", category_);
    addRow("Kind", kind_, wxTE_READONLY);
    addRow("Path", path_, wxTE_READONLY);
    sizer->Add(grid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    auto* apply = new wxButton(this, wxID_APPLY, "Apply");
    apply->Bind(wxEVT_BUTTON, &wxPropertyPanel::OnApply, this);
    sizer->Add(apply, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    SetSizer(sizer);
}

void wxPropertyPanel::Clear()
{
    placeholder_->SetLabel(wxEmptyString);
    label_->Clear();
    category_->Clear();
    kind_->Clear();
    path_->Clear();
    Layout();
}

void wxPropertyPanel::ShowProperties(const EditorPropertySet& properties)
{
    placeholder_->SetLabel(wxEmptyString);
    const auto value = [&properties](const char* key) {
        const EditorProperty* property = properties.Find(key);
        return wxString::FromUTF8(property ? property->value.c_str() : "");
    };
    label_->ChangeValue(value("label"));
    category_->ChangeValue(value("category"));
    kind_->ChangeValue(value("kind"));
    path_->ChangeValue(value("path"));
    Layout();
}

void wxPropertyPanel::SetApplyHandler(ApplyHandler handler)
{
    applyHandler_ = std::move(handler);
}

void wxPropertyPanel::OnApply(wxCommandEvent&)
{
    if (!applyHandler_)
        return;
    const std::string label = label_->GetValue().ToStdString();
    const std::string category = category_->GetValue().ToStdString();
    if (!applyHandler_("label", label))
        return;
    applyHandler_("category", category);
}

void wxPropertyPanel::ShowPlaceholder(const char* text)
{
    placeholder_->SetLabel(wxString::FromUTF8(text ? text : ""));
    Layout();
}
