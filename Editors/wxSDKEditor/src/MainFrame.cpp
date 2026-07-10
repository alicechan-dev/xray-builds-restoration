#include "MainFrame.h"

#include "wxEditorTree.h"
#include "wxPropertyPanel.h"

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
    IdAdapterStatus = wxID_HIGHEST + 1
};

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
    CreateStatusBar();
    SetStatusText("wxSDKEditor experimental shell");
    Centre();
}

void wxSDKEditorFrame::CreateMenus()
{
    auto* menuBar = new wxMenuBar();

    auto* fileMenu = new wxMenu();
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
    Bind(wxEVT_MENU, &wxSDKEditorFrame::OnAdapterStatus, this, IdAdapterStatus);
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
    IEditorTree& tree = *editorTree_;
    tree.Clear();
    treeModel_ = EditorTreeModel::CreateDemoScene();
    PopulateTreeNode(*treeModel_.Root(), IEditorTree::InvalidItem);
    tree.ExpandAllItems();
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

void wxSDKEditorFrame::OnAdapterStatus(wxCommandEvent&)
{
    dialogService_.Info("Adapter Status",
        "IEditorTree is the first active SDK UI adapter boundary.\n\n"
        "The current scene tree and properties are demo placeholders only.");
}

void wxSDKEditorFrame::OnTreeSelectionChanged(wxTreeEvent& event)
{
    UpdateSelectionProperties();
    event.Skip();
}
