#ifndef XR_WX_SDK_EDITOR_MAIN_FRAME_H
#define XR_WX_SDK_EDITOR_MAIN_FRAME_H

#include "wxDialogService.h"
#include "editor_app/EditorDocument.h"
#include "editor_app/EditorToolMode.h"
#include "editor_assets/EditorImportedMetadata.h"

#include <memory>
#include <string>
#include <wx/aui/aui.h>
#include <wx/frame.h>

class EditorTreePresenter;
class wxEditorTree;
class wxAssetBrowser;
class wxEditorViewport;
class wxKeyEvent;
class wxPropertyPanel;
class wxTextCtrl;
class wxTreeEvent;
class wxCloseEvent;

class wxSDKEditorFrame final : public wxFrame
{
public:
    wxSDKEditorFrame();
    ~wxSDKEditorFrame() override;

private:
    void CreateMenus();
    void CreateEditorToolbar();
    void CreateWorkspace();
    void SetToolMode(EditorToolMode mode);
    bool ConfirmSaveChanges();
    bool SaveDocument();
    bool SaveDocumentAs();
    void UpdateDocumentTitle();
    void RestoreLayout();
    void SaveLayout();
    void ResetLayout();
    void TogglePane(const char* paneName);
    void UpdatePaneMenu(wxUpdateUIEvent& event, const char* paneName);
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnNewDocument(wxCommandEvent& event);
    void OnOpenDocument(wxCommandEvent& event);
    void OnSaveDocument(wxCommandEvent& event);
    void OnSaveDocumentAs(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnUpdateUndo(wxUpdateUIEvent& event);
    void OnUpdateRedo(wxUpdateUIEvent& event);
    void OnToggleSceneTree(wxCommandEvent& event);
    void OnToggleProperties(wxCommandEvent& event);
    void OnToggleOutput(wxCommandEvent& event);
    void OnToggleAssetBrowser(wxCommandEvent& event);
    void OnResetLayout(wxCommandEvent& event);
    void OnUpdateSceneTree(wxUpdateUIEvent& event);
    void OnUpdateProperties(wxUpdateUIEvent& event);
    void OnUpdateOutput(wxUpdateUIEvent& event);
    void OnUpdateAssetBrowser(wxUpdateUIEvent& event);
    void OnLoadMetadata();
    void OnClearImportedMetadata();
    void OnToggleViewportGrid(wxCommandEvent& event);
    void OnResetViewportCamera(wxCommandEvent& event);
    void OnFocusViewport(wxCommandEvent& event);
    void OnUpdateViewportGrid(wxUpdateUIEvent& event);
    void OnRebuildPreview(wxCommandEvent& event);
    void OnTogglePreviewLabels(wxCommandEvent& event);
    void OnFrameSelected(wxCommandEvent& event);
    void OnUpdatePreviewLabels(wxUpdateUIEvent& event);
    void OnToggleMoveSnap(wxCommandEvent& event);
    void OnUpdateMoveSnap(wxUpdateUIEvent& event);
    void OnSelectTool(wxCommandEvent& event);
    void OnMoveTool(wxCommandEvent& event);
    void OnPlaceObjectTool(wxCommandEvent& event);
    void OnPlaceLightTool(wxCommandEvent& event);
    void OnUpdateToolMode(wxUpdateUIEvent& event);
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
    void OnTreeEndLabelEdit(wxTreeEvent& event);
    void OnTreeKeyDown(wxKeyEvent& event);
    void OnTreeSelectionChanged(wxTreeEvent& event);

    wxEditorTree* editorTree_ = nullptr;
    wxAssetBrowser* assetBrowser_ = nullptr;
    wxPropertyPanel* propertyPanel_ = nullptr;
    wxTextCtrl* output_ = nullptr;
    wxEditorViewport* viewport_ = nullptr;
    wxAuiManager auiManager_;
    wxString defaultPerspective_;
    EditorDocument document_;
    EditorImportedMetadata importedMetadata_;
    wxDialogService dialogService_;
    std::unique_ptr<EditorTreePresenter> treePresenter_;
};

#endif
