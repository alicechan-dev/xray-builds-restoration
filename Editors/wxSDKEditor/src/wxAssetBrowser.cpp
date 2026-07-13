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
    RebuildCategories();
    assets_ = new wxListBox(this, wxID_ANY);
    details_ = new wxStaticText(this, wxID_ANY,
        "Select a synthetic editor prototype.");
    details_->Wrap(240);
    sourceStatus_ = new wxStaticText(this, wxID_ANY,
        "Imported metadata: not loaded");
    auto* commands = new wxBoxSizer(wxHORIZONTAL);
    auto* load = new wxButton(this, wxID_ANY, "Load Metadata...");
    clear_ = new wxButton(this, wxID_ANY, "Clear Imported");
    place_ = new wxButton(this, wxID_ANY, "Place Selected");
    commands->Add(load, 1, wxRIGHT, 4);
    commands->Add(clear_, 1);

    sizer->Add(search_, 0, wxEXPAND | wxALL, 6);
    sizer->Add(category_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    sizer->Add(assets_, 1, wxEXPAND | wxLEFT | wxRIGHT, 6);
    sizer->Add(details_, 0, wxEXPAND | wxALL, 6);
    sizer->Add(sourceStatus_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    sizer->Add(commands, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    sizer->Add(place_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    SetSizer(sizer);

    search_->Bind(wxEVT_TEXT, &wxAssetBrowser::OnFilterChanged, this);
    category_->Bind(wxEVT_CHOICE, &wxAssetBrowser::OnFilterChanged, this);
    assets_->Bind(wxEVT_LISTBOX, &wxAssetBrowser::OnAssetSelected, this);
    assets_->Bind(wxEVT_LISTBOX_DCLICK,
        &wxAssetBrowser::OnAssetActivated, this);
    place_->Bind(wxEVT_BUTTON, &wxAssetBrowser::OnAssetActivated, this);
    load->Bind(wxEVT_BUTTON, &wxAssetBrowser::OnLoadMetadata, this);
    clear_->Bind(wxEVT_BUTTON, &wxAssetBrowser::OnClearMetadata, this);
    clear_->Enable(false);
    RefreshAssets();
}

void wxAssetBrowser::RefreshAssets()
{
    const std::string text = search_->GetValue().ToStdString();
    const std::string category = category_->GetSelection() > 0
        ? category_->GetStringSelection().ToStdString() : std::string();
    assets_->Clear();
    visibleEntries_.clear();
    const bool allCategories = category.empty();
    const bool syntheticCategory = category.rfind("Synthetic/", 0) == 0;
    const bool importedCategory = category.rfind("Imported/", 0) == 0;
    for (const EditorAssetDescriptor* descriptor : catalog_.Search(text))
    {
        const std::string displayCategory =
            "Synthetic/" + descriptor->categoryPath;
        if (!allCategories && (!syntheticCategory ||
            displayCategory != category))
            continue;
        assets_->Append(wxString::FromUTF8(descriptor->displayName));
        visibleEntries_.push_back({false, descriptor->id});
    }
    for (const EditorAssetDescriptor* descriptor : importedCatalog_.Search(text))
    {
        if (!allCategories && (!importedCategory ||
            descriptor->categoryPath != category))
            continue;
        assets_->Append(wxString::FromUTF8(descriptor->displayName));
        visibleEntries_.push_back({true, descriptor->id});
    }
    if (!visibleEntries_.empty())
        assets_->SetSelection(0);
    UpdateDetails();
}

void wxAssetBrowser::UpdateDetails()
{
    const int selection = assets_->GetSelection();
    const EditorAssetDescriptor* descriptor = ResolveVisible(selection);
    if (!descriptor)
    {
        details_->SetLabel("No matching assets.");
        place_->Enable(false);
        return;
    }
    std::string detail = descriptor->displayName + "\n" +
        descriptor->id + "\n" + descriptor->categoryPath + "\n\n" +
        descriptor->description;
    if (!descriptor->sourceFile.empty())
    {
        detail += "\n\nSource: " + descriptor->sourceFile + ":" +
            std::to_string(descriptor->sourceLine) +
            "\nSection: [" + descriptor->sourceSection + "]" +
            "\n$spawn: " + descriptor->rawSpawnValue;
        detail += "\nPlaceable: ";
        detail += descriptor->placeable ? "synthetic Spawn" : "no";
        detail += "\nPolicy: " + descriptor->placeabilityReason;
    }
    details_->SetLabel(wxString::FromUTF8(detail));
    place_->Enable(descriptor->placeable);
    details_->Wrap(std::max(180, GetClientSize().GetWidth() - 20));
}

void wxAssetBrowser::ActivateSelected()
{
    const int selection = assets_->GetSelection();
    const EditorAssetDescriptor* descriptor = ResolveVisible(selection);
    if (!descriptor)
        return;
    if (!descriptor->placeable)
    {
        if (statusHandler_)
            statusHandler_(descriptor->placeabilityReason.empty()
                ? "Imported metadata is read-only in this stage."
                : descriptor->placeabilityReason);
        return;
    }
    if (activateHandler_)
        activateHandler_(descriptor->id);
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

void wxAssetBrowser::OnLoadMetadata(wxCommandEvent&)
{
    if (loadHandler_)
        loadHandler_();
}

void wxAssetBrowser::OnClearMetadata(wxCommandEvent&)
{
    if (clearHandler_)
        clearHandler_();
}

const EditorAssetDescriptor* wxAssetBrowser::ResolveVisible(int selection) const
{
    if (selection == wxNOT_FOUND ||
        static_cast<std::size_t>(selection) >= visibleEntries_.size())
        return nullptr;
    const VisibleEntry& entry = visibleEntries_[selection];
    return entry.imported ? importedCatalog_.FindById(entry.id) :
        catalog_.FindById(entry.id);
}

void wxAssetBrowser::RebuildCategories()
{
    category_->Clear();
    category_->Append("All categories");
    for (const std::string& category : catalog_.CategoryPaths())
        category_->Append(wxString::FromUTF8("Synthetic/" + category));
    for (const std::string& category : importedCatalog_.CategoryPaths())
        category_->Append(wxString::FromUTF8(category));
    category_->SetSelection(0);
}

void wxAssetBrowser::SetImportedMetadata(
    const EditorImportedMetadata& metadata,
    EditorMetadataCatalogResult catalogResult)
{
    importedMetadata_ = metadata;
    importedCatalog_ = std::move(catalogResult.catalog);
    unsupportedSections_ = catalogResult.unsupportedSections;
    sourceStatus_->SetLabel(wxString::Format(
        "Imported: %zu entries, %zu files, %zu unsupported",
        importedCatalog_.Entries().size(), importedMetadata_.files.size(),
        unsupportedSections_));
    clear_->Enable(true);
    RebuildCategories();
    RefreshAssets();
}

void wxAssetBrowser::ClearImportedMetadata()
{
    importedCatalog_ = {};
    importedMetadata_ = {};
    unsupportedSections_ = 0;
    sourceStatus_->SetLabel("Imported metadata: not loaded");
    clear_->Enable(false);
    RebuildCategories();
    RefreshAssets();
}
