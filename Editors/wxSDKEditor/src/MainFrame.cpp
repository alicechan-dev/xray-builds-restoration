#include "MainFrame.h"

#include "editor_app/EditorTreePresenter.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "wxEditorTree.h"
#include "wxPropertyPanel.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
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
    IdSaveSnapshot,
    IdLoadSnapshot,
    IdImportPathList,
    IdFindItem,
    IdClearSelection,
    IdShowSelection
};

const char* SnapshotWildcard =
    "wxSDKEditor snapshots (*.wx_tree_snapshot)|*.wx_tree_snapshot|All files (*.*)|*.*";
const char* PathListWildcard =
    "wxSDKEditor path lists (*.wx_tree_paths)|*.wx_tree_paths|Text files (*.txt)|*.txt|All files (*.*)|*.*";

wxPanel* CreateViewportPanel(wxWindow* parent)
{
    auto* panel = new wxPanel(parent);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddStretchSpacer();
    sizer->Add(new wxStaticText(panel, wxID_ANY, "Viewport placeholder"),
        0, wxALIGN_CENTER_HORIZONTAL);
    sizer->AddStretchSpacer();
    panel->SetSizer(sizer);
    return panel;
}
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
    fileMenu->Append(IdSaveSnapshot, "&Save Demo Snapshot...");
    fileMenu->Append(IdLoadSnapshot, "&Load Demo Snapshot...");
    fileMenu->Append(IdImportPathList, "&Import Demo Path List...");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X");
    menuBar->Append(fileMenu, "&File");

    auto* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "&Undo")->Enable(false);
    editMenu->Append(wxID_REDO, "&Redo")->Enable(false);
    menuBar->Append(editMenu, "&Edit");

    auto* viewMenu = new wxMenu();
    viewMenu->Append(wxID_ANY, "Reset layout")->Enable(false);
    menuBar->Append(viewMenu, "&View");

    auto* toolsMenu = new wxMenu();
    toolsMenu->Append(IdAddDemoObject, "Add Demo &Object");
    toolsMenu->Append(IdAddDemoGroup, "Add Demo &Group");
    toolsMenu->Append(IdDeleteSelected, "&Delete Selected");
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
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAddDemoObject, this, IdAddDemoObject);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAddDemoGroup, this, IdAddDemoGroup);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnDeleteSelected, this, IdDeleteSelected);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAdapterStatus, this, IdAdapterStatus);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnSaveSnapshot, this, IdSaveSnapshot);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnLoadSnapshot, this, IdLoadSnapshot);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnImportPathList, this, IdImportPathList);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnFindItem, this, IdFindItem);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnShowSelection, this, IdShowSelection);
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnClearSelection, this, IdClearSelection);
}

void wxSDKEditorFrame::CreateWorkspace()
{
    auto* outerSplitter = new wxSplitterWindow(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);
    auto* workspaceSplitter = new wxSplitterWindow(outerSplitter, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);
    auto* contentSplitter = new wxSplitterWindow(workspaceSplitter, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);

    auto* treePanel = new wxPanel(workspaceSplitter);
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

    wxPanel* viewportPanel = CreateViewportPanel(contentSplitter);
    propertyPanel_ = new wxPropertyPanel(contentSplitter);
    propertyPanel_->ShowPlaceholder("Properties placeholder");

    contentSplitter->SetMinimumPaneSize(180);
    contentSplitter->SplitVertically(viewportPanel, propertyPanel_, 700);
    workspaceSplitter->SetMinimumPaneSize(180);
    workspaceSplitter->SplitVertically(treePanel, contentSplitter, 250);

    output_ = new wxTextCtrl(outerSplitter, wxID_ANY,
        "wxSDKEditor experimental shell ready.", wxDefaultPosition,
        wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    outerSplitter->SetMinimumPaneSize(100);
    outerSplitter->SetSashGravity(1.0);
    outerSplitter->SplitHorizontally(workspaceSplitter, output_, 680);

    treePresenter_ = std::make_unique<EditorTreePresenter>(
        *editorTree_, *propertyPanel_, dialogService_,
        [this](const std::string& message) { SetStatusText(message); },
        [this](const std::string& message) {
            output_->AppendText("\n" + wxString::FromUTF8(message) + "\n");
        });
    treePresenter_->InitializeDemo();
}

wxSDKEditorFrame::~wxSDKEditorFrame() = default;

void wxSDKEditorFrame::OnExit(wxCommandEvent&)
{
    Close(true);
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

void wxSDKEditorFrame::OnLoadSnapshot(wxCommandEvent&)
{
    wxFileDialog dialog(this, "Load demo tree snapshot", wxEmptyString,
        wxEmptyString, SnapshotWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;

    const std::filesystem::path path(dialog.GetPath().ToStdWstring());
    treePresenter_->LoadSnapshot(path);
}

void wxSDKEditorFrame::OnImportPathList(wxCommandEvent&)
{
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

void wxSDKEditorFrame::OnSaveSnapshot(wxCommandEvent&)
{
    wxFileDialog dialog(this, "Save demo tree snapshot", wxEmptyString,
        "demo.wx_tree_snapshot", SnapshotWildcard,
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK)
        return;

    const std::filesystem::path path(dialog.GetPath().ToStdWstring());
    treePresenter_->SaveSnapshot(path);
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
