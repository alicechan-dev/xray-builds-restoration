#ifndef XR_WX_SDK_EDITOR_WX_ASSET_BROWSER_H
#define XR_WX_SDK_EDITOR_WX_ASSET_BROWSER_H

#include "editor_assets/EditorAssetCatalog.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <wx/panel.h>

class wxButton;
class wxChoice;
class wxCommandEvent;
class wxListBox;
class wxSearchCtrl;
class wxStaticText;

class wxAssetBrowser final : public wxPanel
{
public:
    using ActivateHandler = std::function<void(const std::string&)>;

    explicit wxAssetBrowser(wxWindow* parent);
    void SetActivateHandler(ActivateHandler handler)
    { activateHandler_ = std::move(handler); }
    const EditorAssetCatalog& Catalog() const { return catalog_; }

private:
    void RefreshAssets();
    void UpdateDetails();
    void ActivateSelected();
    void OnFilterChanged(wxCommandEvent& event);
    void OnAssetSelected(wxCommandEvent& event);
    void OnAssetActivated(wxCommandEvent& event);

    EditorAssetCatalog catalog_ = EditorAssetCatalog::CreateBuiltIn();
    wxSearchCtrl* search_ = nullptr;
    wxChoice* category_ = nullptr;
    wxListBox* assets_ = nullptr;
    wxStaticText* details_ = nullptr;
    std::vector<std::string> visibleIds_;
    ActivateHandler activateHandler_;
};

#endif
