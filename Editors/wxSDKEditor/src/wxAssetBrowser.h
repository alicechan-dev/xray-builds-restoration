#ifndef XR_WX_SDK_EDITOR_WX_ASSET_BROWSER_H
#define XR_WX_SDK_EDITOR_WX_ASSET_BROWSER_H

#include "editor_assets/EditorAssetCatalog.h"
#include "editor_assets/EditorImportedMetadata.h"
#include "editor_assets/EditorMetadataCatalogAdapter.h"

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
    using CommandHandler = std::function<void()>;
    using StatusHandler = std::function<void(const std::string&)>;

    explicit wxAssetBrowser(wxWindow* parent);
    void SetActivateHandler(ActivateHandler handler)
    { activateHandler_ = std::move(handler); }
    void SetLoadHandler(CommandHandler handler)
    { loadHandler_ = std::move(handler); }
    void SetClearHandler(CommandHandler handler)
    { clearHandler_ = std::move(handler); }
    void SetStatusHandler(StatusHandler handler)
    { statusHandler_ = std::move(handler); }
    void SetImportedMetadata(const EditorImportedMetadata& metadata,
        EditorMetadataCatalogResult catalogResult);
    void ClearImportedMetadata();
    const EditorAssetCatalog& Catalog() const { return catalog_; }

private:
    void RefreshAssets();
    void UpdateDetails();
    void ActivateSelected();
    void OnFilterChanged(wxCommandEvent& event);
    void OnAssetSelected(wxCommandEvent& event);
    void OnAssetActivated(wxCommandEvent& event);
    void OnLoadMetadata(wxCommandEvent& event);
    void OnClearMetadata(wxCommandEvent& event);

    struct VisibleEntry
    {
        bool imported = false;
        std::string id;
    };

    const EditorAssetDescriptor* ResolveVisible(int selection) const;
    void RebuildCategories();

    EditorAssetCatalog catalog_ = EditorAssetCatalog::CreateBuiltIn();
    wxSearchCtrl* search_ = nullptr;
    wxChoice* category_ = nullptr;
    wxListBox* assets_ = nullptr;
    wxStaticText* details_ = nullptr;
    wxStaticText* sourceStatus_ = nullptr;
    wxButton* place_ = nullptr;
    wxButton* clear_ = nullptr;
    std::vector<VisibleEntry> visibleEntries_;
    EditorAssetCatalog importedCatalog_;
    EditorImportedMetadata importedMetadata_;
    std::size_t unsupportedSections_ = 0;
    ActivateHandler activateHandler_;
    CommandHandler loadHandler_;
    CommandHandler clearHandler_;
    StatusHandler statusHandler_;
};

#endif
