#include "wxAssetBrowser.h"

#include <algorithm>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/listbox.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

wxAssetBrowser::wxAssetBrowser(wxWindow* parent) : wxPanel(parent)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    search_ = new wxSearchCtrl(this, wxID_ANY);
    search_->SetDescriptiveText("Search synthetic assets");
    category_ = new wxChoice(this, wxID_ANY);
    category_->Append("All categories");
    for (const std::string& category : catalog_.CategoryPaths())
        category_->Append(wxString::FromUTF8(category));
    category_->SetSelection(0);
    assets_ = new wxListBox(this, wxID_ANY);
    details_ = new wxStaticText(this, wxID_ANY,
        "Select a synthetic editor prototype.");
    details_->Wrap(240);
    auto* place = new wxButton(this, wxID_ANY, "Place Selected");

    sizer->Add(search_, 0, wxEXPAND | wxALL, 6);
    sizer->Add(category_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    sizer->Add(assets_, 1, wxEXPAND | wxLEFT | wxRIGHT, 6);
    sizer->Add(details_, 0, wxEXPAND | wxALL, 6);
    sizer->Add(place, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    SetSizer(sizer);

    search_->Bind(wxEVT_TEXT, &wxAssetBrowser::OnFilterChanged, this);
    category_->Bind(wxEVT_CHOICE, &wxAssetBrowser::OnFilterChanged, this);
    assets_->Bind(wxEVT_LISTBOX, &wxAssetBrowser::OnAssetSelected, this);
    assets_->Bind(wxEVT_LISTBOX_DCLICK,
        &wxAssetBrowser::OnAssetActivated, this);
    place->Bind(wxEVT_BUTTON, &wxAssetBrowser::OnAssetActivated, this);
    RefreshAssets();
}

void wxAssetBrowser::RefreshAssets()
{
    const std::string text = search_->GetValue().ToStdString();
    const std::string category = category_->GetSelection() > 0
        ? category_->GetStringSelection().ToStdString() : std::string();
    assets_->Clear();
    visibleIds_.clear();
    for (const EditorAssetDescriptor* descriptor : catalog_.Search(text))
    {
        if (!category.empty() && descriptor->categoryPath != category)
            continue;
        assets_->Append(wxString::FromUTF8(descriptor->displayName));
        visibleIds_.push_back(descriptor->id);
    }
    if (!visibleIds_.empty())
        assets_->SetSelection(0);
    UpdateDetails();
}

void wxAssetBrowser::UpdateDetails()
{
    const int selection = assets_->GetSelection();
    if (selection == wxNOT_FOUND ||
        static_cast<std::size_t>(selection) >= visibleIds_.size())
    {
        details_->SetLabel("No matching synthetic assets.");
        return;
    }
    const EditorAssetDescriptor* descriptor =
        catalog_.FindById(visibleIds_[selection]);
    details_->SetLabel(wxString::FromUTF8(descriptor->displayName + "\n" +
        descriptor->id + "\n" + descriptor->categoryPath + "\n\n" +
        descriptor->description));
    details_->Wrap(std::max(180, GetClientSize().GetWidth() - 20));
}

void wxAssetBrowser::ActivateSelected()
{
    const int selection = assets_->GetSelection();
    if (selection != wxNOT_FOUND &&
        static_cast<std::size_t>(selection) < visibleIds_.size() &&
        activateHandler_)
        activateHandler_(visibleIds_[selection]);
}

void wxAssetBrowser::OnFilterChanged(wxCommandEvent&)
{
    RefreshAssets();
}

void wxAssetBrowser::OnAssetSelected(wxCommandEvent&)
{
    UpdateDetails();
}

void wxAssetBrowser::OnAssetActivated(wxCommandEvent&)
{
    ActivateSelected();
}
