#ifndef XR_WX_SDK_EDITOR_MAIN_FRAME_H
#define XR_WX_SDK_EDITOR_MAIN_FRAME_H

#include "wxDialogService.h"
#include "editor_app/EditorDocument.h"
#include "editor_app/EditorToolMode.h"
#include "editor_assets/EditorImportedMetadata.h"
#include "editor_scene/EditorSceneManifest.h"
#include "editor_scene/EditorHistoricalSceneDocument.h"

#include <memory>
#include <string>
#include <wx/aui/aui.h>
#include <wx/frame.h>

class EditorTreePresenter;
class wxEditorTree;
class wxAssetBrowser;
class wxObjectLibraryBrowser;
class wxEditorViewport;
class wxKeyEvent;
class wxPropertyPanel;
class wxSceneInspector;
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
    void OnOpenHistoricalScene(wxCommandEvent& event);
    void OnConvertHistoricalScene(wxCommandEvent& event);
    void OnUpdateConvertHistoricalScene(wxUpdateUIEvent& event);
    void OnSaveDocument(wxCommandEvent& event);
    void OnSaveDocumentAs(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnUpdateUndo(wxUpdateUIEvent& event);
    void OnUpdateRedo(wxUpdateUIEvent& event);
    void OnUpdateEditableAction(wxUpdateUIEvent& event);
    void OnToggleSceneTree(wxCommandEvent& event);
    void OnToggleProperties(wxCommandEvent& event);
    void OnToggleOutput(wxCommandEvent& event);
    void OnToggleAssetBrowser(wxCommandEvent& event);
    void OnToggleObjectLibrary(wxCommandEvent& event);
    void OnResetLayout(wxCommandEvent& event);
    void OnUpdateSceneTree(wxUpdateUIEvent& event);
    void OnUpdateProperties(wxUpdateUIEvent& event);
    void OnUpdateOutput(wxUpdateUIEvent& event);
    void OnUpdateAssetBrowser(wxUpdateUIEvent& event);
    void OnUpdateObjectLibrary(wxUpdateUIEvent& event);
    void OnLoadObjectLibrary();
    void OnClearObjectLibrary();
    void OnObjectLibrarySummary(wxCommandEvent& event);
    bool ConfigureObjectLibraryRoot(const std::filesystem::path& dataRoot,
        bool persist, bool reportErrors = true);
    void RestoreObjectLibraryConfiguration();
    void EnsureObjectLibraryForScene(const std::filesystem::path& scenePath);
    void OnLoadMetadata();
    void OnClearImportedMetadata();
    void OnToggleViewportGrid(wxCommandEvent& event);
    void OnResetViewportCamera(wxCommandEvent& event);
    void OnFocusViewport(wxCommandEvent& event);
    void OnUpdateViewportGrid(wxUpdateUIEvent& event);
    void OnRebuildPreview(wxCommandEvent& event);
    void OnTogglePreviewLabels(wxCommandEvent& event);
    void OnToggleObjectBounds(wxCommandEvent& event);
    void OnToggleRenderAssetDiagnostics(wxCommandEvent& event);
    void OnToggleRealMeshWireframe(wxCommandEvent& event);
    void OnToggleBackfaceCulling(wxCommandEvent& event);
    void OnSelectDirect3D11(wxCommandEvent& event);
    void OnSelectSoftwareDiagnostic(wxCommandEvent& event);
    void OnToggleFilledMeshes(wxCommandEvent& event);
    void OnToggleWireframeOverlay(wxCommandEvent& event);
    void OnToggleIsolateSelected(wxCommandEvent& event);
    void OnFrameSelected(wxCommandEvent& event);
    void OnUpdatePreviewLabels(wxUpdateUIEvent& event);
    void OnUpdateObjectBounds(wxUpdateUIEvent& event);
    void OnUpdateRenderAssetDiagnostics(wxUpdateUIEvent& event);
    void OnUpdateRealMeshWireframe(wxUpdateUIEvent& event);
    void OnUpdateBackfaceCulling(wxUpdateUIEvent& event);
    void OnUpdateRendererBackend(wxUpdateUIEvent& event);
    void OnUpdateD3DViewOption(wxUpdateUIEvent& event);
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
    void OnHistoricalConversionSummary(wxCommandEvent& event);
    void OnUpdateHistoricalConversionSummary(wxUpdateUIEvent& event);
    void OnDeleteSelected(wxCommandEvent& event);
    void OnMoveSelected(wxCommandEvent& event);
    void OnImportPathList(wxCommandEvent& event);
    void OnInspectHistoricalScene(wxCommandEvent& event);
    void OnToggleSceneInspector(wxCommandEvent& event);
    void OnUpdateSceneInspector(wxUpdateUIEvent& event);
    void OnFindItem(wxCommandEvent& event);
    void OnClearSelection(wxCommandEvent& event);
    void OnShowSelection(wxCommandEvent& event);
    void OnTreeEndLabelEdit(wxTreeEvent& event);
    void OnTreeKeyDown(wxKeyEvent& event);
    void OnTreeSelectionChanged(wxTreeEvent& event);

    wxEditorTree* editorTree_ = nullptr;
    wxAssetBrowser* assetBrowser_ = nullptr;
    wxObjectLibraryBrowser* objectLibraryBrowser_ = nullptr;
    wxPropertyPanel* propertyPanel_ = nullptr;
    wxTextCtrl* output_ = nullptr;
    wxEditorViewport* viewport_ = nullptr;
    wxSceneInspector* sceneInspector_ = nullptr;
    wxAuiManager auiManager_;
    wxString defaultPerspective_;
    EditorDocument document_;
    EditorHistoricalSceneDocument historicalDocument_;
    EditorImportedMetadata importedMetadata_;
    EditorSceneManifest inspectedScene_;
    wxDialogService dialogService_;
    std::unique_ptr<EditorTreePresenter> treePresenter_;
};

#endif
