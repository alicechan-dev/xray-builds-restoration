#include "MainFrame.h"

#include "editor_app/EditorTreePresenter.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "wxEditorTree.h"
#include "wxEditorViewport.h"
#include "wxPropertyPanel.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include <wx/choicdlg.h>
#include <wx/config.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
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
    IdFindItem,
    IdClearSelection,
    IdShowSelection,
    IdViewSceneTree,
    IdViewProperties,
    IdViewOutput,
    IdResetLayout,
    IdToggleViewportGrid,
    IdResetViewportCamera,
    IdFocusViewport,
    IdRebuildPreview,
    IdTogglePreviewLabels,
    IdFrameSelected
};

const char* SceneTreePane = "scene_tree";
const char* PropertiesPane = "properties";
const char* OutputPane = "output";
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
    menuBar->Append(viewMenu, "&View");

    auto* toolsMenu = new wxMenu();
    toolsMenu->Append(IdAddDemoObject, "Add Demo &Object");
    toolsMenu->Append(IdAddDemoGroup, "Add Demo &Group");
    toolsMenu->Append(IdDeleteSelected, "&Delete Selected");
    toolsMenu->Append(IdMoveSelected, "&Move Selected To...");
    toolsMenu->Append(IdFindItem, "&Find Item...");
    toolsMenu->Append(IdShowSelection, "Show Selected &Path");
    toolsMenu->Append(IdClearSelection, "&Clear Selection");
    toolsMenu->AppendSeparator();
    toolsMenu->Append(IdAdapterStatus, "&Adapter Status");
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
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnSaveDocument, this, wxID_SAVE);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnSaveDocumentAs, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnUndo, this, wxID_UNDO);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnRedo, this, wxID_REDO);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateUndo, this, wxID_UNDO);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateRedo, this, wxID_REDO);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleSceneTree,
        this, IdViewSceneTree);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleProperties,
        this, IdViewProperties);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnToggleOutput,
        this, IdViewOutput);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnResetLayout,
        this, IdResetLayout);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateSceneTree,
        this, IdViewSceneTree);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateProperties,
        this, IdViewProperties);
    Bind(wxEVT_UPDATE_UI, &wxSDKEditorFrame::OnUpdateOutput,
        this, IdViewOutput);
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
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAddDemoObject, this, IdAddDemoObject);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAddDemoGroup, this, IdAddDemoGroup);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnDeleteSelected, this, IdDeleteSelected);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnMoveSelected, this, IdMoveSelected);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAdapterStatus, this, IdAdapterStatus);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnImportPathList, this, IdImportPathList);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnFindItem, this, IdFindItem);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnShowSelection, this, IdShowSelection);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnClearSelection, this, IdClearSelection);
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
        [this](const EditorTreeModel& model, const std::string& selectedPath) {
            viewport_->RebuildPreview(model, selectedPath);
        });
    treePresenter_->InitializeDemo();
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
}

void wxSDKEditorFrame::OnOpenDocument(wxCommandEvent&)
{
    if (!ConfirmSaveChanges())
        return;

    wxFileDialog dialog(this, "Open development tree snapshot", wxEmptyString,
        wxEmptyString, SnapshotWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;
    treePresenter_->LoadSnapshot(
        std::filesystem::path(dialog.GetPath().ToStdWstring()));
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
    if (!document_.HasFilePath())
        return SaveDocumentAs();
    return treePresenter_->SaveSnapshot(document_.GetFilePath());
}

bool wxSDKEditorFrame::SaveDocumentAs()
{
    wxFileDialog dialog(this, "Save development tree snapshot", wxEmptyString,
        document_.GetDisplayName() == "Untitled"
            ? "untitled.wx_tree_snapshot" : document_.GetDisplayName(),
        SnapshotWildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK)
        return false;
    return treePresenter_->SaveSnapshot(
        std::filesystem::path(dialog.GetPath().ToStdWstring()));
}

void wxSDKEditorFrame::UpdateDocumentTitle()
{
    std::string title = "wxSDKEditor - " + document_.GetDisplayName();
    if (document_.IsModified())
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
    treePresenter_->ImportPathList(text, dialog.GetPath().ToStdString());
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
