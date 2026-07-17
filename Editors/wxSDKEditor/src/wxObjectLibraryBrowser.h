#ifndef XR_WX_SDK_EDITOR_WX_OBJECT_LIBRARY_BROWSER_H
#define XR_WX_SDK_EDITOR_WX_OBJECT_LIBRARY_BROWSER_H

#include "editor_assets/EditorObjectLibrary.h"

#include <functional>
#include <vector>
#include <unordered_set>
#include <wx/panel.h>

class wxButton;
class wxCommandEvent;
class wxListBox;
class wxTextCtrl;
class wxChoice;

class wxObjectLibraryBrowser final : public wxPanel
{
public:
    explicit wxObjectLibraryBrowser(wxWindow* parent);
    void SetLibrary(const EditorObjectLibrary* library);
    void SetCurrentSceneAssets(const std::vector<std::string>& assetIds);
    void SetLoadHandler(std::function<void()> handler) { load_ = std::move(handler); }
    void SetClearHandler(std::function<void()> handler) { clear_ = std::move(handler); }

private:
    void RefreshEntries();
    void UpdateDetails();
    void OnFilter(wxCommandEvent&);
    void OnSelection(wxCommandEvent&);
    void OnActivate(wxCommandEvent&);

    const EditorObjectLibrary* library_ = nullptr;
    std::vector<const EditorObjectLibraryEntry*> visible_;
    wxTextCtrl* search_ = nullptr;
    wxListBox* entries_ = nullptr;
    wxTextCtrl* details_ = nullptr;
    wxChoice* scope_ = nullptr;
    std::unordered_set<std::string> currentSceneAssets_;
    std::function<void()> load_;
    std::function<void()> clear_;
};

#endif
