#include "wxObjectLibraryBrowser.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

wxObjectLibraryBrowser::wxObjectLibraryBrowser(wxWindow* parent) : wxPanel(parent)
{
    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* actions = new wxBoxSizer(wxHORIZONTAL);
    auto* load = new wxButton(this, wxID_OPEN, "Configure...");
    auto* clear = new wxButton(this, wxID_CLEAR, "Clear Session Cache");
    actions->Add(load, 0, wxRIGHT, 5); actions->Add(clear, 0);
    root->Add(actions, 0, wxEXPAND | wxALL, 6);
    search_ = new wxTextCtrl(this, wxID_ANY);
    search_->SetHint("Search reference or category");
    root->Add(search_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    const wxString scopes[] = {"All indexed assets", "Used in Current Scene",
        "Missing in Current Scene"};
    scope_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        3, scopes);
    scope_->SetSelection(0);
    root->Add(scope_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    entries_ = new wxListBox(this, wxID_ANY);
    root->Add(entries_, 1, wxEXPAND | wxLEFT | wxRIGHT, 6);
    details_ = new wxTextCtrl(this, wxID_ANY,
        "No Object Library loaded.", wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY);
    root->Add(details_, 1, wxEXPAND | wxALL, 6);
    SetSizer(root);
    load->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { if (load_) load_(); });
    clear->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { if (clear_) clear_(); });
    search_->Bind(wxEVT_TEXT, &wxObjectLibraryBrowser::OnFilter, this);
    scope_->Bind(wxEVT_CHOICE, &wxObjectLibraryBrowser::OnFilter, this);
    entries_->Bind(wxEVT_LISTBOX, &wxObjectLibraryBrowser::OnSelection, this);
    entries_->Bind(wxEVT_LISTBOX_DCLICK, &wxObjectLibraryBrowser::OnActivate, this);
}

void wxObjectLibraryBrowser::SetCurrentSceneAssets(
    const std::vector<std::string>& assetIds)
{
    currentSceneAssets_.clear();
    currentSceneAssets_.insert(assetIds.begin(), assetIds.end());
    RefreshEntries();
}

void wxObjectLibraryBrowser::SetLibrary(const EditorObjectLibrary* library)
{
    library_ = library; RefreshEntries();
}

void wxObjectLibraryBrowser::RefreshEntries()
{
    visible_.clear(); entries_->Clear();
    if (!library_ || !library_->IsLoaded()) { details_->SetValue("No Object Library loaded."); return; }
    std::string filter = search_->GetValue().Lower().ToStdString();
    for (const auto& entry : library_->Entries()) {
        const bool used = currentSceneAssets_.find(entry.referenceId) !=
            currentSceneAssets_.end();
        if (scope_->GetSelection() == 1 && !used) continue;
        if (scope_->GetSelection() == 2) continue;
        std::string searchable = entry.referenceId + " " + entry.category;
        if (!filter.empty() && searchable.find(filter) == std::string::npos) continue;
        visible_.push_back(&entry);
        entries_->Append(wxString::FromUTF8(entry.referenceId));
    }
    if (scope_->GetSelection() == 2)
        for (const std::string& id : currentSceneAssets_)
            if (library_->Find(id).empty() &&
                (filter.empty() || id.find(filter) != std::string::npos))
                entries_->Append(wxString::FromUTF8("[missing] " + id));
    details_->SetValue(wxString::Format(
        "Read-only Object Library\n%zu entries shown of %zu.",
        visible_.size(), library_->Entries().size()));
}

void wxObjectLibraryBrowser::UpdateDetails()
{
    const int selection = entries_->GetSelection();
    if (selection == wxNOT_FOUND || static_cast<std::size_t>(selection) >= visible_.size()) return;
    const auto& e = *visible_[selection];
    std::ostringstream out;
    out << "Reference ID: " << e.referenceId << "\nOriginal Name: " << e.originalReference
        << "\nCategory: " << (e.category.empty() ? "(root)" : e.category)
        << "\nSource Relative File: " << e.sourceRelativeFile
        << "\nVersion: 0x" << std::hex << e.version << std::dec
        << "\nObject Kind: " << ToString(e.kind)
        << "\nBounds: unavailable without mesh payload decoding"
        << "\nMesh Count: " << e.meshCount << "\nSurface Count: " << e.surfaceCount
        << "\nMotion Present: " << (e.motionPresent ? "yes" : "no")
        << "\nTexture References: " << e.textureReferences.size()
        << "\nShader References: " << e.shaderReferences.size()
        << "\nMaterial References: " << e.materialReferences.size()
        << "\nParse Status: " << ToString(e.parseStatus)
        << "\nUnknown Chunks: " << e.unknownChunkCount
        << "\nRead-Only: yes";
    for (const auto& diagnostic : e.diagnostics) out << "\nDiagnostic: " << diagnostic;
    details_->SetValue(wxString::FromUTF8(out.str()));
}

void wxObjectLibraryBrowser::OnFilter(wxCommandEvent&) { RefreshEntries(); }
void wxObjectLibraryBrowser::OnSelection(wxCommandEvent&) { UpdateDetails(); }
void wxObjectLibraryBrowser::OnActivate(wxCommandEvent&)
{
    UpdateDetails();
    wxMessageBox("Object Library placement is deferred until the asset/render bridge is implemented.",
        "Object Library", wxOK | wxICON_INFORMATION, this);
}
