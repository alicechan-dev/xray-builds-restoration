#include "wxPropertyPanel.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

wxPropertyPanel::wxPropertyPanel(wxWindow* parent) : wxPanel(parent)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    placeholder_ = new wxStaticText(this, wxID_ANY, "Properties placeholder");
    sizer->Add(placeholder_, 0, wxALL, 12);
    SetSizer(sizer);
}

void wxPropertyPanel::Clear()
{
    placeholder_->SetLabel(wxEmptyString);
    Layout();
}

void wxPropertyPanel::ShowPlaceholder(const char* text)
{
    placeholder_->SetLabel(wxString::FromUTF8(text ? text : ""));
    Layout();
}
