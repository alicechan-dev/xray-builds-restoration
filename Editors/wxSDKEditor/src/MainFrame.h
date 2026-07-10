#ifndef XR_WX_SDK_EDITOR_MAIN_FRAME_H
#define XR_WX_SDK_EDITOR_MAIN_FRAME_H

#include "wxDialogService.h"

#include <wx/frame.h>

class wxEditorTree;
class wxPropertyPanel;

class wxSDKEditorFrame final : public wxFrame
{
public:
    wxSDKEditorFrame();

private:
    void CreateMenus();
    void CreateWorkspace();
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    wxEditorTree* editorTree_ = nullptr;
    wxPropertyPanel* propertyPanel_ = nullptr;
    wxDialogService dialogService_;
};

#endif
