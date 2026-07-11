#include "MainFrame.h"

#include "editor_model/EditorTreeSnapshot.h"
#include "wxEditorTree.h"
#include "wxPropertyPanel.h"

#include <filesystem>
#include <utility>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
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
    IdLoadSnapshot
};

const char* SnapshotWildcard =
    "wxSDKEditor snapshots (*.wx_tree_snapshot)|*.wx_tree_snapshot|All files (*.*)|*.*";

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

    auto* output = new wxTextCtrl(outerSplitter, wxID_ANY,
        "wxSDKEditor experimental shell ready.", wxDefaultPosition,
        wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    outerSplitter->SetMinimumPaneSize(100);
    outerSplitter->SetSashGravity(1.0);
    outerSplitter->SplitHorizontally(workspaceSplitter, output, 680);

    PopulateDemoTree();
    CallAfter([this]() { editorTree_->SelectFirst(); });
}

wxSDKEditorFrame::~wxSDKEditorFrame()
{
    if (editorTree_)
        editorTree_->Clear();
}

void wxSDKEditorFrame::PopulateDemoTree()
{
    treeModel_ = EditorTreeModel::CreateDemoScene();
    RebuildTree(nullptr);
}

void wxSDKEditorFrame::RebuildTree(EditorTreeNode* selectedNode)
{
    IEditorTree& tree = *editorTree_;
    tree.Clear();
    PopulateTreeNode(*treeModel_.Root(), IEditorTree::InvalidItem);
    tree.ExpandAllItems();
    if (selectedNode)
    {
        tree.SelectByUserData(
            reinterpret_cast<IEditorTree::UserData>(selectedNode));
        UpdateSelectionProperties();
    }
}

void wxSDKEditorFrame::PopulateTreeNode(
    const EditorTreeNode& node, IEditorTree::ItemHandle parentItem)
{
    IEditorTree& tree = *editorTree_;
    const auto item = parentItem == IEditorTree::InvalidItem
        ? tree.AddRoot(node.Label().c_str())
        : tree.AddChild(parentItem, node.Label().c_str());
    tree.SetItemUserData(item,
        reinterpret_cast<IEditorTree::UserData>(&node));

    for (const auto& child : node.ChildrenView())
        PopulateTreeNode(*child, item);
}

EditorTreeNode* wxSDKEditorFrame::GetSelectedModelNode() const
{
    return reinterpret_cast<EditorTreeNode*>(editorTree_->GetSelectedUserData());
}

void wxSDKEditorFrame::AddDemoNode(const char* baseName, const char* category)
{
    EditorTreeNode* parent = GetSelectedModelNode();
    if (!parent)
        parent = treeModel_.Root();

    const std::string name = treeModel_.MakeUniqueChildName(*parent, baseName);
    EditorTreeNode& added = treeModel_.AddChild(*parent, name, category);
    RebuildTree(&added);
    SetStatusText("Added '" + added.Label() + "' under '" + parent->Label() + "'.");
}

void wxSDKEditorFrame::UpdateSelectionProperties()
{
    const auto* node = reinterpret_cast<const EditorTreeNode*>(
        editorTree_->GetSelectedUserData());
    if (!node)
    {
        propertyPanel_->Clear();
        return;
    }

    const std::string text = "Selected: " + node->Label() +
        "\nType: " + node->Category() +
        "\nPath: " + node->Path() +
        "\nProperties: placeholder only"
        "\n\nNo real SDK data is loaded.";
    propertyPanel_->ShowPlaceholder(text.c_str());
}

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
    AddDemoNode("new_group", "demo group");
}

void wxSDKEditorFrame::OnAddDemoObject(wxCommandEvent&)
{
    AddDemoNode("new_object", "demo scene object");
}

void wxSDKEditorFrame::OnAdapterStatus(wxCommandEvent&)
{
    dialogService_.Info("Adapter Status",
        "IEditorTree is the first active SDK UI adapter boundary.\n\n"
        "The current scene tree and properties are demo placeholders only.");
}

void wxSDKEditorFrame::OnDeleteSelected(wxCommandEvent&)
{
    EditorTreeNode* selected = GetSelectedModelNode();
    if (!selected)
    {
        dialogService_.Warning("Delete rejected", "No tree node is selected.");
        return;
    }

    std::string reason;
    if (!treeModel_.CanDeleteNode(*selected, &reason))
    {
        dialogService_.Warning("Delete rejected", reason.c_str());
        SetStatusText("Delete rejected: " + reason);
        return;
    }

    const std::string label = selected->Label();
    const std::string prompt = "Delete '" + label + "' and all of its children?";
    if (!dialogService_.Confirm("Delete demo node", prompt.c_str()))
        return;

    EditorTreeNode* parent = selected->Parent();
    editorTree_->Clear();
    if (!treeModel_.DeleteNode(*selected, &reason))
    {
        RebuildTree(selected);
        dialogService_.Warning("Delete rejected", reason.c_str());
        return;
    }

    RebuildTree(parent ? parent : treeModel_.Root());
    SetStatusText("Deleted '" + label + "'.");
}

void wxSDKEditorFrame::OnLoadSnapshot(wxCommandEvent&)
{
    wxFileDialog dialog(this, "Load demo tree snapshot", wxEmptyString,
        wxEmptyString, SnapshotWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;

    EditorTreeModel loaded;
    std::string reason;
    const std::filesystem::path path(dialog.GetPath().ToStdWstring());
    if (!LoadEditorTreeSnapshot(loaded, path, &reason))
    {
        dialogService_.Error("Snapshot load failed", reason.c_str());
        SetStatusText("Snapshot load failed: " + reason);
        return;
    }

    editorTree_->Clear();
    treeModel_ = std::move(loaded);
    RebuildTree(nullptr);
    editorTree_->SelectFirst();
    UpdateSelectionProperties();
    SetStatusText("Loaded demo tree snapshot.");
}

void wxSDKEditorFrame::OnSaveSnapshot(wxCommandEvent&)
{
    wxFileDialog dialog(this, "Save demo tree snapshot", wxEmptyString,
        "demo.wx_tree_snapshot", SnapshotWildcard,
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK)
        return;

    std::string reason;
    const std::filesystem::path path(dialog.GetPath().ToStdWstring());
    if (!SaveEditorTreeSnapshot(treeModel_, path, &reason))
    {
        dialogService_.Error("Snapshot save failed", reason.c_str());
        SetStatusText("Snapshot save failed: " + reason);
        return;
    }
    SetStatusText("Saved demo tree snapshot.");
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
    const std::string previousLabel = node->Label();
    if (!treeModel_.RenameNode(*node, event.GetLabel().ToStdString(), &reason))
    {
        event.Veto();
        SetStatusText("Rename rejected: " + reason);
        CallAfter([this, reason]() {
            dialogService_.Warning("Rename rejected", reason.c_str());
            UpdateSelectionProperties();
        });
        return;
    }

    SetStatusText("Renamed '" + previousLabel + "' to '" + node->Label() + "'.");
    UpdateSelectionProperties();
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
    UpdateSelectionProperties();
    event.Skip();
}
