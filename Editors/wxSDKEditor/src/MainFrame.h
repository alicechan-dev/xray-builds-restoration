#ifndef XR_WX_SDK_EDITOR_MAIN_FRAME_H
#define XR_WX_SDK_EDITOR_MAIN_FRAME_H

#include "wxDialogService.h"

#include <memory>
#include <wx/frame.h>

class EditorTreePresenter;
class wxEditorTree;
class wxKeyEvent;
class wxPropertyPanel;
class wxTextCtrl;
class wxTreeEvent;

class wxSDKEditorFrame final : public wxFrame
{
public:
    wxSDKEditorFrame();
    ~wxSDKEditorFrame() override;

private:
    void CreateMenus();
    void CreateWorkspace();
    void OnExit(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnUpdateUndo(wxUpdateUIEvent& event);
    void OnUpdateRedo(wxUpdateUIEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnAddDemoGroup(wxCommandEvent& event);
    void OnAddDemoObject(wxCommandEvent& event);
    void OnAdapterStatus(wxCommandEvent& event);
    void OnDeleteSelected(wxCommandEvent& event);
    void OnMoveSelected(wxCommandEvent& event);
    void OnImportPathList(wxCommandEvent& event);
    void OnFindItem(wxCommandEvent& event);
    void OnClearSelection(wxCommandEvent& event);
    void OnShowSelection(wxCommandEvent& event);
    void OnLoadSnapshot(wxCommandEvent& event);
    void OnSaveSnapshot(wxCommandEvent& event);
    void OnTreeEndLabelEdit(wxTreeEvent& event);
    void OnTreeKeyDown(wxKeyEvent& event);
    void OnTreeSelectionChanged(wxTreeEvent& event);

    wxEditorTree* editorTree_ = nullptr;
    wxPropertyPanel* propertyPanel_ = nullptr;
    wxTextCtrl* output_ = nullptr;
    wxDialogService dialogService_;
    std::unique_ptr<EditorTreePresenter> treePresenter_;
};

#endif
