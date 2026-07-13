#ifndef XR_WX_SDK_EDITOR_WX_SCENE_INSPECTOR_H
#define XR_WX_SDK_EDITOR_WX_SCENE_INSPECTOR_H

#include "editor_scene/EditorSceneManifest.h"

#include <wx/panel.h>

class wxStaticText;
class wxTextCtrl;
class wxTreeCtrl;
class wxTreeEvent;

class wxSceneInspector final : public wxPanel
{
public:
    explicit wxSceneInspector(wxWindow* parent);

    void SetManifest(const EditorSceneManifest& manifest);
    void ClearManifest();
    bool HasManifest() const { return hasManifest_; }

private:
    void OnSelectionChanged(wxTreeEvent& event);

    wxStaticText* summary_ = nullptr;
    wxTreeCtrl* tree_ = nullptr;
    wxTextCtrl* details_ = nullptr;
    EditorSceneManifest manifest_;
    bool hasManifest_ = false;
};

#endif
