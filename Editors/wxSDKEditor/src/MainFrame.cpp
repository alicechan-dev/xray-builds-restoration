#include "MainFrame.h"

#include "editor_app/EditorTreePresenter.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_assets/EditorMetadataCatalogAdapter.h"
#include "editor_assets/EditorMetadataLoader.h"
#include "editor_scene/EditorHistoricalSceneProbe.h"
#include "editor_scene/EditorHistoricalConversionReport.h"
#include "wxEditorTree.h"
#include "wxAssetBrowser.h"
#include "wxObjectLibraryBrowser.h"
#include "wxEditorViewport.h"
#include "wxPropertyPanel.h"
#include "wxSceneInspector.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include <wx/artprov.h>
#include <wx/choicdlg.h>
#include <wx/config.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/toolbar.h>
#include <wx/treectrl.h>

namespace
{
enum
{
    IdAdapterStatus = wxID_HIGHEST + 1,
    IdAddDemoObject,
    IdAddDemoGroup,
    IdDeleteSelected,
    IdMoveSelected,
    IdImportPathList,
    IdInspectHistoricalScene,
    IdOpenHistoricalScene,
    IdConvertHistoricalScene,
    IdHistoricalConversionSummary,
    IdFindItem,
    IdClearSelection,
    IdShowSelection,
    IdViewSceneTree,
    IdViewProperties,
    IdViewOutput,
    IdViewAssetBrowser,
    IdViewObjectLibrary,
    IdViewSceneInspector,
    IdResetLayout,
    IdToggleViewportGrid,
    IdResetViewportCamera,
    IdFocusViewport,
    IdRebuildPreview,
    IdTogglePreviewLabels,
    IdFrameSelected,
    IdToggleMoveSnap,
    IdToolSelect,
    IdToolMove,
    IdToolPlaceObject,
    IdToolPlaceLight,
    IdObjectLibrarySummary
};

const char* SceneTreePane = "scene_tree";
const char* PropertiesPane = "properties";
const char* OutputPane = "output";
const char* AssetBrowserPane = "asset_browser";
const char* ObjectLibraryPane = "object_library";
const char* SceneInspectorPane = "scene_inspector";
const char* ViewportPane = "viewport";
const char* PerspectiveKey = "/layout/aui_perspective";

const char* SnapshotWildcard =
    "wxSDKEditor snapshots (*.wx_tree_snapshot)|*.wx_tree_snapshot|All files (*.*)|*.*";
const char* PathListWildcard =
    "wxSDKEditor path lists (*.wx_tree_paths)|*.wx_tree_paths|Text files (*.txt)|*.txt|All files (*.*)|*.*";

}

wxSDKEditorFrame::wxSDKEditorFrame() :
    wxFrame(nullptr, wxID_ANY, "wxSDKEditor", wxDefaultPosition, wxSize(1280, 850)),
    dialogService_(this)
{
    CreateMenus();
    CreateEditorToolbar();
    CreateWorkspace();
    std::string selfCheckFailure;
    if (!RunEditorTreeModelSelfCheck(&selfCheckFailure))
        dialogService_.Error("Editor tree model self-check failed", selfCheckFailure.c_str());
    if (!RunEditorTreeSnapshotSelfCheck(&selfCheckFailure))
        dialogService_.Error("Editor tree snapshot self-check failed", selfCheckFailure.c_str());
    CreateStatusBar();
    SetStatusText("wxSDKEditor experimental shell");
    Centre();
}

void wxSDKEditorFrame::CreateMenus()
{
    auto* menuBar = new wxMenuBar();

    auto* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "&New\tCtrl+N");
    fileMenu->Append(wxID_OPEN, "&Open Snapshot...\tCtrl+O");
    fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S");
    fileMenu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
    fileMenu->AppendSeparator();
    fileMenu->Append(IdImportPathList, "&Import Demo Path List...");
    fileMenu->Append(IdInspectHistoricalScene,
        "Inspect &Historical Scene...");
    fileMenu->Append(IdOpenHistoricalScene,
        "Open Historical Scene &Read-Only...");
    fileMenu->Append(IdConvertHistoricalScene,
        "&Convert Historical Scene to Editable Copy...");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X");
    menuBar->Append(fileMenu, "&File");

    auto* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
    editMenu->Append(wxID_REDO, "&Redo\tCtrl+Y");
    menuBar->Append(editMenu, "&Edit");

    auto* viewMenu = new wxMenu();
    viewMenu->AppendCheckItem(IdViewSceneTree, "Scene &Tree");
    viewMenu->AppendCheckItem(IdViewProperties, "&Properties");
    viewMenu->AppendCheckItem(IdViewOutput, "&Output");
    viewMenu->AppendCheckItem(IdViewAssetBrowser, "&Asset Browser");
    viewMenu->AppendCheckItem(IdViewObjectLibrary, "Object &Library");
    viewMenu->AppendCheckItem(IdViewSceneInspector, "Scene &Inspector");
    viewMenu->AppendSeparator();
    viewMenu->Append(IdResetLayout, "&Reset Layout");
    viewMenu->AppendSeparator();
    viewMenu->AppendCheckItem(IdToggleViewportGrid, "Viewport &Grid");
    viewMenu->Append(IdResetViewportCamera, "Reset Viewport &Camera");
    viewMenu->Append(IdFocusViewport, "&Focus Viewport");
    viewMenu->AppendSeparator();
    viewMenu->Append(IdRebuildPreview, "&Rebuild Preview Scene");
    viewMenu->AppendCheckItem(IdTogglePreviewLabels, "Preview &Labels");
    viewMenu->Append(IdFrameSelected, "Frame &Selected");
    viewMenu->AppendCheckItem(IdToggleMoveSnap, "Snap Move To &Grid");
    menuBar->Append(viewMenu, "&View");

    auto* toolsMenu = new wxMenu();
    toolsMenu->AppendRadioItem(IdToolSelect, "&Select Tool");
    toolsMenu->AppendRadioItem(IdToolMove, "&Move Tool");
    toolsMenu->AppendRadioItem(IdToolPlaceObject, "Place &Object Tool");
    toolsMenu->AppendRadioItem(IdToolPlaceLight, "Place &Light Tool");
    toolsMenu->AppendSeparator();
    toolsMenu->Append(IdAddDemoObject, "Add Demo &Object");
    toolsMenu->Append(IdAddDemoGroup, "Add Demo &Group");
    toolsMenu->Append(IdDeleteSelected, "&Delete Selected");
    toolsMenu->Append(IdMoveSelected, "&Move Selected To...");
    toolsMenu->Append(IdFindItem, "&Find Item...");
    toolsMenu->Append(IdShowSelection, "Show Selected &Path");
    toolsMenu->Append(IdClearSelection, "&Clear Selection");
    toolsMenu->AppendSeparator();
    toolsMenu->Append(IdAdapterStatus, "&Adapter Status");
    toolsMenu->Append(IdHistoricalConversionSummary,
        "Historical Conversion &Summary...");
    toolsMenu->Append(IdObjectLibrarySummary, "Object Library &Summary...");
    toolsMenu->AppendSeparator();
    toolsMenu->Append(wxID_PREFERENCES, "&Options")->Enable(false);
    menuBar->Append(toolsMenu, "&Tools");

    auto* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_CLOSE_WINDOW, &wxSDKEditorFrame::OnClose, this);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnNewDocument, this, wxID_NEW);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnOpenDocument, this, wxID_OPEN);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnOpenHistoricalScene,
        this, IdOpenHistoricalScene);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnConvertHistoricalScene,
        this, IdConvertHistoricalScene);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateConvertHistoricalScene,
        this, IdConvertHistoricalScene);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnSaveDocument, this, wxID_SAVE);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnSaveDocumentAs, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnUndo, this, wxID_UNDO);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnRedo, this, wxID_REDO);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateUndo, this, wxID_UNDO);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateRedo, this, wxID_REDO);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateEditableAction,
        this, wxID_SAVE);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateEditableAction,
        this, wxID_SAVEAS);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateEditableAction,
        this, IdAddDemoObject);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateEditableAction,
        this, IdAddDemoGroup);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateEditableAction,
        this, IdDeleteSelected);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateEditableAction,
        this, IdMoveSelected);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateEditableAction,
        this, IdImportPathList);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleSceneTree,
        this, IdViewSceneTree);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleProperties,
        this, IdViewProperties);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleOutput,
        this, IdViewOutput);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleAssetBrowser,
        this, IdViewAssetBrowser);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleObjectLibrary,
        this, IdViewObjectLibrary);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleSceneInspector,
        this, IdViewSceneInspector);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnResetLayout,
        this, IdResetLayout);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateSceneTree,
        this, IdViewSceneTree);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateProperties,
        this, IdViewProperties);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateOutput,
        this, IdViewOutput);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateAssetBrowser,
        this, IdViewAssetBrowser);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateObjectLibrary,
        this, IdViewObjectLibrary);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateSceneInspector,
        this, IdViewSceneInspector);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleViewportGrid,
        this, IdToggleViewportGrid);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnResetViewportCamera,
        this, IdResetViewportCamera);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnFocusViewport,
        this, IdFocusViewport);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateViewportGrid,
        this, IdToggleViewportGrid);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnRebuildPreview,
        this, IdRebuildPreview);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnTogglePreviewLabels,
        this, IdTogglePreviewLabels);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnFrameSelected,
        this, IdFrameSelected);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdatePreviewLabels,
        this, IdTogglePreviewLabels);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleMoveSnap,
        this, IdToggleMoveSnap);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateMoveSnap,
        this, IdToggleMoveSnap);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAddDemoObject, this, IdAddDemoObject);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAddDemoGroup, this, IdAddDemoGroup);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnDeleteSelected, this, IdDeleteSelected);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnMoveSelected, this, IdMoveSelected);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAdapterStatus, this, IdAdapterStatus);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnHistoricalConversionSummary,
        this, IdHistoricalConversionSummary);
    Bind(wxEVT_UPDATE_UI,
        &wxSDKEditorFrame::OnUpdateHistoricalConversionSummary,
        this, IdHistoricalConversionSummary);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnObjectLibrarySummary,
        this, IdObjectLibrarySummary);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnImportPathList, this, IdImportPathList);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnInspectHistoricalScene,
        this, IdInspectHistoricalScene);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnFindItem, this, IdFindItem);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnShowSelection, this, IdShowSelection);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnClearSelection, this, IdClearSelection);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnSelectTool, this, IdToolSelect);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnMoveTool, this, IdToolMove);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnPlaceObjectTool,
        this, IdToolPlaceObject);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnPlaceLightTool,
        this, IdToolPlaceLight);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateToolMode,
        this, IdToolSelect, IdToolPlaceLight);
}

void wxSDKEditorFrame::CreateEditorToolbar()
{
    wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT);
    toolbar->AddTool(IdToolSelect, "Select",
        wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_TOOLBAR),
        "Select preview objects", wxITEM_RADIO);
    toolbar->AddTool(IdToolMove, "Move",
        wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR),
        "Move selected preview object", wxITEM_RADIO);
    toolbar->AddTool(IdToolPlaceObject, "Object",
        wxArtProvider::GetBitmap(wxART_PLUS, wxART_TOOLBAR),
        "Place synthetic object", wxITEM_RADIO);
    toolbar->AddTool(IdToolPlaceLight, "Light",
        wxArtProvider::GetBitmap(wxART_TIP, wxART_TOOLBAR),
        "Place synthetic light", wxITEM_RADIO);
    toolbar->AddSeparator();
    toolbar->AddTool(wxID_UNDO, "Undo",
        wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR));
    toolbar->AddTool(wxID_REDO, "Redo",
        wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR));
    toolbar->AddSeparator();
    toolbar->AddTool(IdDeleteSelected, "Delete",
        wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR));
    toolbar->AddTool(IdFrameSelected, "Frame",
        wxArtProvider::GetBitmap(wxART_FIND, wxART_TOOLBAR));
    toolbar->AddCheckTool(IdToggleMoveSnap, "Snap",
        wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_TOOLBAR),
        wxNullBitmap, "Snap placement and move to 1.0-unit grid");
    toolbar->Realize();
    toolbar->ToggleTool(IdToolSelect, true);
}

void wxSDKEditorFrame::SetToolMode(EditorToolMode mode)
{
    if (treePresenter_)
    {
        treePresenter_->SetToolMode(mode);
        mode = treePresenter_->GetToolMode();
    }
    if (viewport_)
    {
        const EditorAssetDescriptor* descriptor = nullptr;
        if (treePresenter_)
        {
            if (mode == EditorToolMode::PlaceObject)
                descriptor = treePresenter_->AssetCatalog().FindById(
                    "demo.physic_object");
            else if (mode == EditorToolMode::PlaceLight)
                descriptor = treePresenter_->AssetCatalog().FindById(
                    "demo.point_light");
            else if (mode == EditorToolMode::PlaceAsset)
                descriptor = treePresenter_->SelectedAsset();
        }
        viewport_->SetPlacementDescriptor(descriptor);
        viewport_->SetToolMode(mode);
    }
}

void wxSDKEditorFrame::CreateWorkspace()
{
    auiManager_.SetManagedWindow(this);

    auto* treePanel = new wxPanel(this);
    auto* treeSizer = new wxBoxSizer(wxVERTICAL);
    treeSizer->Add(new wxStaticText(treePanel, wxID_ANY, "Scene / Objects"),
        0, wxALL, 8);
    editorTree_ = new wxEditorTree(treePanel);
    editorTree_->Bind(wxEVT_KEY_DOWN,
        &wxSDKEditorFrame::OnTreeKeyDown, this);
    editorTree_->Bind(wxEVT_TREE_END_LABEL_EDIT,
        &wxSDKEditorFrame::OnTreeEndLabelEdit, this);
    editorTree_->Bind(wxEVT_TREE_SEL_CHANGED,
        &wxSDKEditorFrame::OnTreeSelectionChanged, this);
    treeSizer->Add(editorTree_, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    treePanel->SetSizer(treeSizer);

    viewport_ = new wxEditorViewport(this);
    viewport_->SetSelectionHandler([this](const std::string& logicalPath) {
        treePresenter_->SelectLogicalPath(logicalPath);
    });
    propertyPanel_ = new wxPropertyPanel(this);
    propertyPanel_->ShowPlaceholder("Properties placeholder");

    output_ = new wxTextCtrl(this, wxID_ANY,
        "wxSDKEditor experimental shell ready.", wxDefaultPosition,
        wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    assetBrowser_ = new wxAssetBrowser(this);
    objectLibraryBrowser_ = new wxObjectLibraryBrowser(this);
    sceneInspector_ = new wxSceneInspector(this);

    auiManager_.AddPane(treePanel, wxAuiPaneInfo().Name(SceneTreePane)
        .Caption("Scene / Objects").Left().Layer(1).Position(0)
        .BestSize(260, 600).MinSize(180, 180).CloseButton(true)
        .MaximizeButton(true).Resizable(true));
    auiManager_.AddPane(propertyPanel_, wxAuiPaneInfo().Name(PropertiesPane)
        .Caption("Properties").Right().Layer(1).Position(0)
        .BestSize(320, 600).MinSize(220, 180).CloseButton(true)
        .MaximizeButton(true).Resizable(true));
    auiManager_.AddPane(output_, wxAuiPaneInfo().Name(OutputPane)
        .Caption("Output").Bottom().Layer(1).Position(0)
        .BestSize(-1, 170).MinSize(240, 100).CloseButton(true)
        .MaximizeButton(true).Resizable(true));
    auiManager_.AddPane(assetBrowser_, wxAuiPaneInfo().Name(AssetBrowserPane)
        .Caption("Asset Browser").Left().Layer(1).Position(1)
        .BestSize(280, 420).MinSize(220, 220).CloseButton(true)
        .MaximizeButton(true).Resizable(true));
    auiManager_.AddPane(objectLibraryBrowser_,
        wxAuiPaneInfo().Name(ObjectLibraryPane).Caption("Object Library")
        .Left().Layer(2).Position(2).BestSize(310, 500).MinSize(240, 240)
        .CloseButton(true).MaximizeButton(true).Resizable(true));
    auiManager_.AddPane(sceneInspector_,
        wxAuiPaneInfo().Name(SceneInspectorPane).Caption("Scene Inspector")
        .Right().Layer(2).Position(1).BestSize(380, 500)
        .MinSize(260, 220).CloseButton(true).MaximizeButton(true)
        .Resizable(true).Hide());
    auiManager_.AddPane(viewport_, wxAuiPaneInfo().Name(ViewportPane)
        .Caption("Viewport").CenterPane().PaneBorder(false)
        .CloseButton(false).Floatable(false).Dockable(false));
    auiManager_.Update();
    defaultPerspective_ = auiManager_.SavePerspective();
    RestoreLayout();

    treePresenter_ = std::make_unique<EditorTreePresenter>(
        document_, *editorTree_, *propertyPanel_, dialogService_,
        [this](const std::string& message) { SetStatusText(message); },
        [this](const std::string& message) {
            output_->AppendText("\n" + wxString::FromUTF8(message) + "\n");
        }, [this](const std::string&) { UpdateDocumentTitle(); },
        [this](const EditorPreviewScene& scene) {
            viewport_->SetPreviewScene(scene);
        });
    treePresenter_->AttachHistoricalDocument(historicalDocument_);
    objectLibraryBrowser_->SetLoadHandler([this]() { OnLoadObjectLibrary(); });
    objectLibraryBrowser_->SetClearHandler([this]() { OnClearObjectLibrary(); });
    viewport_->SetTransformHandler(
        [this](const std::string& path, const EditorTransform& transform) {
            return treePresenter_->SetLogicalTransform(path, transform);
        });
    viewport_->SetPlacementHandler(
        [this](EditorToolMode mode, const EditorTransform& transform) {
            if (treePresenter_->GetToolMode() != mode)
                treePresenter_->SetToolMode(mode);
            return treePresenter_->PlaceAt(transform);
        });
    viewport_->SetCancelToolHandler([this]() {
        treePresenter_->CancelActiveTool();
        viewport_->SetToolMode(EditorToolMode::Select);
    });
    assetBrowser_->SetActivateHandler([this](const std::string& assetId) {
        if (treePresenter_->SelectAsset(assetId))
            SetToolMode(EditorToolMode::PlaceAsset);
    });
    assetBrowser_->SetLoadHandler([this]() { OnLoadMetadata(); });
    assetBrowser_->SetClearHandler([this]() { OnClearImportedMetadata(); });
    assetBrowser_->SetStatusHandler([this](const std::string& message) {
        SetStatusText(message);
        output_->AppendText("\n" + wxString::FromUTF8(message) + "\n");
    });
    treePresenter_->InitializeDemo();
    SetToolMode(EditorToolMode::Select);
}

wxSDKEditorFrame::~wxSDKEditorFrame()
{
    SaveLayout();
    if (editorTree_)
    {
        editorTree_->Unbind(wxEVT_KEY_DOWN,
            &wxSDKEditorFrame::OnTreeKeyDown, this);
        editorTree_->Unbind(wxEVT_TREE_END_LABEL_EDIT,
            &wxSDKEditorFrame::OnTreeEndLabelEdit, this);
        editorTree_->Unbind(wxEVT_TREE_SEL_CHANGED,
            &wxSDKEditorFrame::OnTreeSelectionChanged, this);
    }
    treePresenter_.reset();
    auiManager_.UnInit();
}

void wxSDKEditorFrame::RestoreLayout()
{
    wxConfig config("wxSDKEditor");
    wxString perspective;
    if (!config.Read(PerspectiveKey, &perspective) || perspective.empty())
        return;

    if (!auiManager_.LoadPerspective(perspective, true))
        auiManager_.LoadPerspective(defaultPerspective_, true);
    auiManager_.Update();
}

void wxSDKEditorFrame::SaveLayout()
{
    wxConfig config("wxSDKEditor");
    config.Write(PerspectiveKey, auiManager_.SavePerspective());
    config.Flush();
}

void wxSDKEditorFrame::ResetLayout()
{
    auiManager_.LoadPerspective(defaultPerspective_, true);
    auiManager_.Update();
    SaveLayout();
    SetStatusText("Editor layout reset.");
}

void wxSDKEditorFrame::TogglePane(const char* paneName)
{
    wxAuiPaneInfo& pane = auiManager_.GetPane(paneName);
    if (!pane.IsOk())
        return;
    pane.Show(!pane.IsShown());
    auiManager_.Update();
}

void wxSDKEditorFrame::UpdatePaneMenu(
    wxUpdateUIEvent& event, const char* paneName)
{
    const wxAuiPaneInfo& pane = auiManager_.GetPane(paneName);
    event.Check(pane.IsOk() && pane.IsShown());
}

void wxSDKEditorFrame::OnExit(wxCommandEvent&)
{
    Close(false);
}

void wxSDKEditorFrame::OnClose(wxCloseEvent& event)
{
    if (ConfirmSaveChanges())
        event.Skip();
    else
        event.Veto();
}

void wxSDKEditorFrame::OnNewDocument(wxCommandEvent&)
{
    if (!ConfirmSaveChanges())
        return;
    treePresenter_->NewDocument();
    SetToolMode(EditorToolMode::Select);
}

void wxSDKEditorFrame::OnOpenDocument(wxCommandEvent&)
{
    if (!ConfirmSaveChanges())
        return;

    wxFileDialog dialog(this, "Open development tree snapshot", wxEmptyString,
        wxEmptyString, SnapshotWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;
    if (treePresenter_->LoadSnapshot(
        std::filesystem::path(dialog.GetPath().ToStdWstring())))
        SetToolMode(EditorToolMode::Select);
}

void wxSDKEditorFrame::OnOpenHistoricalScene(wxCommandEvent&)
{
    if (!ConfirmSaveChanges())
        return;
    wxFileDialog dialog(this, "Open historical X-Ray scene read-only",
        wxEmptyString, wxEmptyString,
        "Historical X-Ray scenes (*.level)|*.level|All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;

    EditorSceneManifest manifest;
    std::string reason;
    EditorHistoricalSceneProbe probe;
    if (!probe.ProbeSceneFile(
        std::filesystem::path(dialog.GetPath().ToStdWstring()),
        manifest, &reason) ||
        !treePresenter_->OpenHistoricalScene(std::move(manifest), &reason))
    {
        dialogService_.Error("Historical scene open failed", reason.c_str());
        SetStatusText("Historical scene open failed: " + reason);
        return;
    }

    SetToolMode(EditorToolMode::Select);
    const EditorSceneManifest& source = historicalDocument_.GetManifest();
    const std::string summary = "Historical scene opened read-only: file=" +
        historicalDocument_.GetDisplayName() + ", version=" +
        std::to_string(source.version) + ", bytes=" +
        std::to_string(source.totalSize) + ", declared=" +
        (source.hasDeclaredObjectCount
            ? std::to_string(source.declaredObjectCount) : "unknown") +
        ", confirmed=" +
        std::to_string(historicalDocument_.GetConfirmedObjectCount()) +
        ", named=" +
        std::to_string(historicalDocument_.GetNamedObjectCount()) +
        ", transformed=" +
        std::to_string(historicalDocument_.GetTransformedObjectCount()) +
        ", compressed=" + std::to_string(source.compressedChunkCount) +
        ", decompressed=" + std::to_string(source.decompressedChunkCount) +
        ", decompression_failed=" +
        std::to_string(source.decompressionFailureCount) +
        ", bodies_unsupported=" +
        std::to_string(historicalDocument_.GetUnsupportedBodyCount()) +
        ", diagnostics=" + std::to_string(source.diagnostics.size()) + ".";
    output_->AppendText("\n" + wxString::FromUTF8(summary) + "\n");
    SetStatusText(summary);
    UpdateDocumentTitle();
}

void wxSDKEditorFrame::OnConvertHistoricalScene(wxCommandEvent&)
{
    if (!treePresenter_ || !treePresenter_->CanConvertHistoricalScene())
        return;

    EditorHistoricalConversionOptions options;
    EditorHistoricalConversionReport preview;
    std::string reason;
    if (!treePresenter_->PreviewHistoricalConversion(options, preview, &reason))
    {
        dialogService_.Error("Historical conversion unavailable", reason.c_str());
        return;
    }

    const wxString message = wxString::Format(
        "Source records: %zu\nFully convertible: %zu\nPartially convertible: %zu\n"
        "Placeholder candidates: %zu\nSkipped: %zu\n\n"
        "Choose which non-full records to include. The result is a separate "
        "editable snapshot and cannot be saved back to .level.",
        preview.totalHistoricalRecords, preview.fullyConverted,
        preview.partiallyConverted, preview.placeholderConverted,
        preview.skipped);
    wxArrayString choices;
    choices.Add("Include partially converted specialized records");
    choices.Add("Include generic unsupported placeholders");
    wxMultiChoiceDialog dialog(this, message,
        "Convert Historical Scene to Editable Copy", choices);
    wxArrayInt defaults;
    defaults.Add(0);
    defaults.Add(1);
    dialog.SetSelections(defaults);
    if (dialog.ShowModal() != wxID_OK)
        return;

    options.includePartial = false;
    options.includeGenericPlaceholders = false;
    for (const int selected : dialog.GetSelections())
    {
        if (selected == 0) options.includePartial = true;
        if (selected == 1) options.includeGenericPlaceholders = true;
    }

    EditorHistoricalConversionReport report;
    if (!treePresenter_->ConvertHistoricalSceneToEditableCopy(
        options, report, &reason))
    {
        dialogService_.Error("Historical conversion failed", reason.c_str());
        return;
    }
    SetToolMode(EditorToolMode::Select);
    auiManager_.GetPane(OutputPane).Show(true);
    auiManager_.Update();
    dialogService_.Info("Historical conversion complete",
        report.BuildSummary().c_str());
    UpdateDocumentTitle();
}

void wxSDKEditorFrame::OnUpdateConvertHistoricalScene(wxUpdateUIEvent& event)
{
    event.Enable(treePresenter_ && treePresenter_->CanConvertHistoricalScene());
}

void wxSDKEditorFrame::OnHistoricalConversionSummary(wxCommandEvent&)
{
    std::string summary;
    std::string reason;
    if (!treePresenter_ ||
        !treePresenter_->GetHistoricalConversionSummary(summary, &reason))
    {
        dialogService_.Warning("Historical Conversion Summary", reason.c_str());
        return;
    }
    output_->AppendText("\nHistorical Conversion Summary\n" +
        wxString::FromUTF8(summary) + "\n");
    dialogService_.Info("Historical Conversion Summary", summary.c_str());
}

void wxSDKEditorFrame::OnUpdateHistoricalConversionSummary(
    wxUpdateUIEvent& event)
{
    event.Enable(treePresenter_ &&
        treePresenter_->HasHistoricalConversionSummary());
}

void wxSDKEditorFrame::OnLoadObjectLibrary()
{
    wxDirDialog dialog(this, "Choose historical Object Library root",
        wxEmptyString, wxDD_DIR_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK) return;
    EditorObjectLibraryLoadStatistics statistics;
    std::string reason;
    if (!treePresenter_->LoadObjectLibrary(
        std::filesystem::path(dialog.GetPath().ToStdWstring()), statistics, &reason)) {
        dialogService_.Error("Object Library load failed", reason.c_str()); return;
    }
    objectLibraryBrowser_->SetLibrary(&treePresenter_->ObjectLibrary());
    output_->AppendText(wxString::Format(
        "\nObject Library: files=%zu entries=%zu supported=%zu partial=%zu malformed=%zu duplicates=%zu bytes_read=%llu\n",
        statistics.filesScanned, statistics.entriesLoaded, statistics.supported,
        statistics.partial, statistics.malformed, statistics.duplicateReferences,
        static_cast<unsigned long long>(statistics.bytesRead)));
}

void wxSDKEditorFrame::OnClearObjectLibrary()
{
    treePresenter_->ClearObjectLibrary();
    objectLibraryBrowser_->SetLibrary(nullptr);
}

void wxSDKEditorFrame::OnObjectLibrarySummary(wxCommandEvent&)
{
    if (!treePresenter_->ObjectLibrary().IsLoaded()) {
        dialogService_.Info("Object Library Summary", "No Object Library is loaded."); return;
    }
    const auto s = treePresenter_->ObjectLibraryStatistics();
    const std::string summary = "Entries: " + std::to_string(s.entries) +
        "\nSupported: " + std::to_string(s.supported) +
        "\nPartial: " + std::to_string(s.partial) +
        "\nMalformed: " + std::to_string(s.malformed) +
        "\nStatic: " + std::to_string(s.staticObjects) +
        "\nSkeletal: " + std::to_string(s.skeletalObjects) +
        "\nUnknown: " + std::to_string(s.unknownObjects) +
        "\nDuplicate references: " + std::to_string(s.duplicateReferences);
    dialogService_.Info("Object Library Summary", summary.c_str());
}

void wxSDKEditorFrame::OnSaveDocument(wxCommandEvent&)
{
    SaveDocument();
}

void wxSDKEditorFrame::OnSaveDocumentAs(wxCommandEvent&)
{
    SaveDocumentAs();
}

bool wxSDKEditorFrame::ConfirmSaveChanges()
{
    if (treePresenter_ && treePresenter_->IsReadOnly())
        return true;
    if (!document_.IsModified())
        return true;

    wxMessageDialog dialog(this,
        "Save changes to '" + document_.GetDisplayName() + "'?",
        "Unsaved development document",
        wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_WARNING);
    const int result = dialog.ShowModal();
    if (result == wxID_CANCEL)
        return false;
    if (result == wxID_NO)
        return true;
    return result == wxID_YES && SaveDocument();
}

bool wxSDKEditorFrame::SaveDocument()
{
    if (treePresenter_ && treePresenter_->IsReadOnly())
    {
        treePresenter_->SaveSnapshot({});
        return false;
    }
    if (!document_.HasFilePath())
        return SaveDocumentAs();
    const bool saved = treePresenter_->SaveSnapshot(document_.GetFilePath());
    if (saved)
        SetToolMode(EditorToolMode::Select);
    return saved;
}

bool wxSDKEditorFrame::SaveDocumentAs()
{
    if (treePresenter_ && treePresenter_->IsReadOnly())
    {
        treePresenter_->SaveSnapshot({});
        return false;
    }
    wxFileDialog dialog(this, "Save development tree snapshot", wxEmptyString,
        document_.GetDisplayName() == "Untitled"
            ? "untitled.wx_tree_snapshot" : document_.GetDisplayName(),
        SnapshotWildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK)
        return false;
    const bool saved = treePresenter_->SaveSnapshot(
        std::filesystem::path(dialog.GetPath().ToStdWstring()));
    if (saved)
        SetToolMode(EditorToolMode::Select);
    return saved;
}

void wxSDKEditorFrame::UpdateDocumentTitle()
{
    const bool readOnly = treePresenter_ && treePresenter_->IsReadOnly();
    std::string title = "wxSDKEditor - " + (treePresenter_
        ? treePresenter_->GetActiveDisplayName() : document_.GetDisplayName());
    if (readOnly)
        title += " [Read-Only]";
    else if (document_.IsModified())
        title += " *";
    SetTitle(wxString::FromUTF8(title));
}

void wxSDKEditorFrame::OnUndo(wxCommandEvent&)
{
    treePresenter_->Undo();
}

void wxSDKEditorFrame::OnRedo(wxCommandEvent&)
{
    treePresenter_->Redo();
}

void wxSDKEditorFrame::OnUpdateUndo(wxUpdateUIEvent& event)
{
    event.Enable(treePresenter_ && treePresenter_->CanUndo());
}

void wxSDKEditorFrame::OnUpdateRedo(wxUpdateUIEvent& event)
{
    event.Enable(treePresenter_ && treePresenter_->CanRedo());
}

void wxSDKEditorFrame::OnUpdateEditableAction(wxUpdateUIEvent& event)
{
    event.Enable(!treePresenter_ || !treePresenter_->IsReadOnly());
}

void wxSDKEditorFrame::OnToggleSceneTree(wxCommandEvent&)
{
    TogglePane(SceneTreePane);
}

void wxSDKEditorFrame::OnToggleProperties(wxCommandEvent&)
{
    TogglePane(PropertiesPane);
}

void wxSDKEditorFrame::OnToggleOutput(wxCommandEvent&)
{
    TogglePane(OutputPane);
}

void wxSDKEditorFrame::OnToggleAssetBrowser(wxCommandEvent&)
{
    TogglePane(AssetBrowserPane);
}

void wxSDKEditorFrame::OnToggleObjectLibrary(wxCommandEvent&)
{
    TogglePane(ObjectLibraryPane);
}

void wxSDKEditorFrame::OnToggleSceneInspector(wxCommandEvent&)
{
    TogglePane(SceneInspectorPane);
}

void wxSDKEditorFrame::OnResetLayout(wxCommandEvent&)
{
    ResetLayout();
}

void wxSDKEditorFrame::OnUpdateSceneTree(wxUpdateUIEvent& event)
{
    UpdatePaneMenu(event, SceneTreePane);
}

void wxSDKEditorFrame::OnUpdateProperties(wxUpdateUIEvent& event)
{
    UpdatePaneMenu(event, PropertiesPane);
}

void wxSDKEditorFrame::OnUpdateOutput(wxUpdateUIEvent& event)
{
    UpdatePaneMenu(event, OutputPane);
}

void wxSDKEditorFrame::OnUpdateAssetBrowser(wxUpdateUIEvent& event)
{
    UpdatePaneMenu(event, AssetBrowserPane);
}

void wxSDKEditorFrame::OnUpdateObjectLibrary(wxUpdateUIEvent& event)
{
    UpdatePaneMenu(event, ObjectLibraryPane);
}

void wxSDKEditorFrame::OnUpdateSceneInspector(wxUpdateUIEvent& event)
{
    UpdatePaneMenu(event, SceneInspectorPane);
}

void wxSDKEditorFrame::OnLoadMetadata()
{
    wxDirDialog rootDialog(this, "Choose metadata root", wxEmptyString,
        wxDD_DIR_MUST_EXIST);
    if (rootDialog.ShowModal() != wxID_OK)
        return;
    wxFileDialog entryDialog(this, "Choose entry LTX file",
        rootDialog.GetPath(), "system.ltx",
        "X-Ray metadata (*.ltx)|*.ltx|All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (entryDialog.ShowModal() != wxID_OK)
        return;

    EditorImportedMetadata loaded;
    std::string reason;
    EditorMetadataLoader loader;
    if (!loader.LoadMetadataRoot(
        std::filesystem::path(rootDialog.GetPath().ToStdWstring()),
        std::filesystem::path(entryDialog.GetPath().ToStdWstring()),
        loaded, &reason))
    {
        dialogService_.Error("Metadata import failed", reason.c_str());
        SetStatusText("Metadata import failed: " + reason);
        return;
    }

    EditorMetadataCatalogResult catalog = BuildEditorMetadataCatalog(loaded);
    const std::size_t catalogDiagnostics = catalog.diagnostics.size();
    std::vector<EditorMetadataDiagnostic> diagnostics = loaded.diagnostics;
    diagnostics.insert(diagnostics.end(), catalog.diagnostics.begin(),
        catalog.diagnostics.end());
    importedMetadata_ = loaded;
    treePresenter_->SetImportedAssetCatalog(catalog.catalog);
    assetBrowser_->SetImportedMetadata(importedMetadata_, std::move(catalog));
    const std::string summary = "Metadata loaded read-only: files=" +
        std::to_string(loaded.files.size()) + ", sections=" +
        std::to_string(loaded.sections.size()) + ", includes=" +
        std::to_string(loaded.includesFollowed) + ", warnings=" +
        std::to_string(loaded.diagnostics.size() + catalogDiagnostics) + ".";
    output_->AppendText("\n" + wxString::FromUTF8(summary) + "\n");
    constexpr std::size_t DiagnosticPreviewLimit = 20;
    for (std::size_t index = 0;
        index < diagnostics.size() && index < DiagnosticPreviewLimit; ++index)
    {
        const EditorMetadataDiagnostic& diagnostic = diagnostics[index];
        output_->AppendText(wxString::FromUTF8(
            diagnostic.sourceFile + ":" +
            std::to_string(diagnostic.sourceLine) + ": " +
            diagnostic.message + "\n"));
    }
    if (diagnostics.size() > DiagnosticPreviewLimit)
        output_->AppendText(wxString::Format(
            "... %zu additional metadata diagnostics omitted.\n",
            diagnostics.size() - DiagnosticPreviewLimit));
    SetStatusText(summary);
}

void wxSDKEditorFrame::OnClearImportedMetadata()
{
    importedMetadata_ = {};
    treePresenter_->ClearImportedAssetCatalog();
    assetBrowser_->ClearImportedMetadata();
    SetStatusText("Imported metadata cleared; synthetic assets retained.");
}

void wxSDKEditorFrame::OnToggleViewportGrid(wxCommandEvent&)
{
    viewport_->ToggleGrid();
}

void wxSDKEditorFrame::OnResetViewportCamera(wxCommandEvent&)
{
    viewport_->ResetCamera();
    SetStatusText("Placeholder viewport camera reset.");
}

void wxSDKEditorFrame::OnFocusViewport(wxCommandEvent&)
{
    viewport_->FocusViewport();
}

void wxSDKEditorFrame::OnUpdateViewportGrid(wxUpdateUIEvent& event)
{
    event.Check(viewport_ && viewport_->IsGridVisible());
}

void wxSDKEditorFrame::OnRebuildPreview(wxCommandEvent&)
{
    treePresenter_->RefreshPreview();
    SetStatusText("Preview scene rebuilt from the development model.");
}

void wxSDKEditorFrame::OnTogglePreviewLabels(wxCommandEvent&)
{
    viewport_->TogglePreviewLabels();
}

void wxSDKEditorFrame::OnFrameSelected(wxCommandEvent&)
{
    if (viewport_->FrameSelected())
        SetStatusText("Placeholder camera framed the selected preview object.");
    else
        SetStatusText("Selected tree item has no renderable preview object.");
}

void wxSDKEditorFrame::OnUpdatePreviewLabels(wxUpdateUIEvent& event)
{
    event.Check(viewport_ && viewport_->ArePreviewLabelsVisible());
}

void wxSDKEditorFrame::OnToggleMoveSnap(wxCommandEvent&)
{
    viewport_->ToggleMoveSnap();
    SetStatusText(viewport_->IsMoveSnapEnabled()
        ? "Move gizmo grid snapping enabled (1.0 unit)."
        : "Move gizmo grid snapping disabled.");
}

void wxSDKEditorFrame::OnUpdateMoveSnap(wxUpdateUIEvent& event)
{
    event.Enable(!treePresenter_ || !treePresenter_->IsReadOnly());
    event.Check(viewport_ && viewport_->IsMoveSnapEnabled());
}

void wxSDKEditorFrame::OnSelectTool(wxCommandEvent&)
{
    SetToolMode(EditorToolMode::Select);
}

void wxSDKEditorFrame::OnMoveTool(wxCommandEvent&)
{
    SetToolMode(EditorToolMode::Move);
}

void wxSDKEditorFrame::OnPlaceObjectTool(wxCommandEvent&)
{
    SetToolMode(EditorToolMode::PlaceObject);
}

void wxSDKEditorFrame::OnPlaceLightTool(wxCommandEvent&)
{
    SetToolMode(EditorToolMode::PlaceLight);
}

void wxSDKEditorFrame::OnUpdateToolMode(wxUpdateUIEvent& event)
{
    if (!treePresenter_)
        return;
    EditorToolMode expected = EditorToolMode::Select;
    if (event.GetId() == IdToolMove)
        expected = EditorToolMode::Move;
    else if (event.GetId() == IdToolPlaceObject)
        expected = EditorToolMode::PlaceObject;
    else if (event.GetId() == IdToolPlaceLight)
        expected = EditorToolMode::PlaceLight;
    event.Enable(expected == EditorToolMode::Select ||
        !treePresenter_->IsReadOnly());
    event.Check(treePresenter_->GetToolMode() == expected);
}

void wxSDKEditorFrame::OnAbout(wxCommandEvent&)
{
    dialogService_.Info("About wxSDKEditor",
        "Experimental wxSDKEditor for gradual X-Ray SDK UI restoration.\n\n"
        "This is the future replacement path for VCL/ElPack GUI dependencies.\n"
        "No real level editing or game-data loading is implemented yet.");
}

void wxSDKEditorFrame::OnAddDemoGroup(wxCommandEvent&)
{
    treePresenter_->AddDemoNode("new_group", "demo group");
}

void wxSDKEditorFrame::OnAddDemoObject(wxCommandEvent&)
{
    treePresenter_->AddDemoNode("new_object", "demo scene object");
}

void wxSDKEditorFrame::OnAdapterStatus(wxCommandEvent&)
{
    dialogService_.Info("Adapter Status",
        "IEditorTree is the first active SDK UI adapter boundary.\n\n"
        "The current scene tree and properties are demo placeholders only.");
}

void wxSDKEditorFrame::OnDeleteSelected(wxCommandEvent&)
{
    treePresenter_->DeleteSelected();
}

void wxSDKEditorFrame::OnMoveSelected(wxCommandEvent&)
{
    const std::vector<std::string> destinations =
        treePresenter_->GetMoveDestinations();
    if (destinations.empty())
    {
        dialogService_.Warning("Move rejected",
            "No valid destination folders are available for the selection.");
        return;
    }

    wxArrayString choices;
    for (const std::string& path : destinations)
        choices.Add(wxString::FromUTF8(path.c_str()));

    wxSingleChoiceDialog dialog(this, "Choose the new parent folder",
        "Move Selected To", choices);
    if (dialog.ShowModal() != wxID_OK)
        return;
    treePresenter_->MoveSelectedTo(
        destinations[static_cast<std::size_t>(dialog.GetSelection())]);
}

void wxSDKEditorFrame::OnImportPathList(wxCommandEvent&)
{
    if (!ConfirmSaveChanges())
        return;
    wxFileDialog dialog(this, "Import demo path list", wxEmptyString,
        wxEmptyString, PathListWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;

    const std::filesystem::path path(dialog.GetPath().ToStdWstring());
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        dialogService_.Error("Path-list import failed", "Could not open the selected file.");
        SetStatusText("Path-list import failed: could not open file.");
        return;
    }

    const std::string text{
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (treePresenter_->ImportPathList(text, dialog.GetPath().ToStdString()))
        SetToolMode(EditorToolMode::Select);
}

void wxSDKEditorFrame::OnInspectHistoricalScene(wxCommandEvent&)
{
    wxFileDialog dialog(this, "Inspect historical X-Ray scene", wxEmptyString,
        wxEmptyString,
        "Historical X-Ray scenes (*.level)|*.level|All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;

    EditorSceneManifest manifest;
    std::string reason;
    EditorHistoricalSceneProbe probe;
    if (!probe.ProbeSceneFile(
        std::filesystem::path(dialog.GetPath().ToStdWstring()),
        manifest, &reason))
    {
        dialogService_.Error("Historical scene inspection failed",
            reason.c_str());
        SetStatusText("Historical scene inspection failed: " + reason);
        return;
    }

    inspectedScene_ = std::move(manifest);
    sceneInspector_->SetManifest(inspectedScene_);
    wxAuiPaneInfo& pane = auiManager_.GetPane(SceneInspectorPane);
    if (pane.IsOk())
        pane.Show(true);
    auiManager_.Update();

    const std::string summary = "Historical scene inspected read-only: chunks=" +
        std::to_string(inspectedScene_.chunks.size()) + ", objects=" +
        std::to_string(inspectedScene_.objects.size()) + ", warnings=" +
        std::to_string(inspectedScene_.diagnostics.size()) + ".";
    output_->AppendText("\n" + wxString::FromUTF8(summary) + "\n");
    SetStatusText(summary);
}

void wxSDKEditorFrame::OnFindItem(wxCommandEvent&)
{
    wxTextEntryDialog dialog(this, "Search node labels", "Find Item");
    if (dialog.ShowModal() != wxID_OK)
        return;
    treePresenter_->FindFirst(dialog.GetValue().ToStdString());
}

void wxSDKEditorFrame::OnClearSelection(wxCommandEvent&)
{
    treePresenter_->ClearSelection();
}

void wxSDKEditorFrame::OnShowSelection(wxCommandEvent&)
{
    treePresenter_->ReportSelection();
}

void wxSDKEditorFrame::OnTreeEndLabelEdit(wxTreeEvent& event)
{
    if (treePresenter_->IsReadOnly())
    {
        event.Veto();
        SetStatusText("Rename rejected: Historical scene is read-only.");
        return;
    }
    if (event.IsEditCancelled())
    {
        event.Skip();
        return;
    }

    auto* node = reinterpret_cast<EditorTreeNode*>(
        editorTree_->GetItemUserData(event.GetItem()));
    if (!node)
    {
        event.Veto();
        return;
    }

    std::string reason;
    if (!treePresenter_->RenameNode(*node, event.GetLabel().ToStdString(), &reason))
    {
        event.Veto();
        CallAfter([this, reason]() {
            dialogService_.Warning("Rename rejected", reason.c_str());
            treePresenter_->RefreshSelection();
        });
        return;
    }

    event.Skip();
}

void wxSDKEditorFrame::OnTreeKeyDown(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_F2)
    {
        if (treePresenter_->IsReadOnly())
        {
            SetStatusText("Rename rejected: Historical scene is read-only.");
            return;
        }
        editorTree_->BeginEditSelectedLabel();
        return;
    }
    event.Skip();
}

void wxSDKEditorFrame::OnTreeSelectionChanged(wxTreeEvent& event)
{
    treePresenter_->RefreshSelection();
    event.Skip();
}
