#ifndef XR_WX_SDK_EDITOR_MAIN_FRAME_H
#define XR_WX_SDK_EDITOR_MAIN_FRAME_H

#include "editor_model/EditorTreeModel.h"
#include "editor_ui/IEditorTree.h"
#include "wxDialogService.h"

#include <wx/frame.h>

class wxEditorTree;
class wxKeyEvent;
class wxPropertyPanel;
class wxTreeEvent;

class wxSDKEditorFrame final : public wxFrame
{
public:
    wxSDKEditorFrame();
    ~wxSDKEditorFrame() override;

private:
    void CreateMenus();
    void CreateWorkspace();
    void PopulateDemoTree();
    void RebuildTree(EditorTreeNode* selectedNode);
    void PopulateTreeNode(const EditorTreeNode& node,
        IEditorTree::ItemHandle parentItem);
    EditorTreeNode* GetSelectedModelNode() const;
    void AddDemoNode(const char* baseName, const char* category);
    void UpdateSelectionProperties();
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnAddDemoGroup(wxCommandEvent& event);
    void OnAddDemoObject(wxCommandEvent& event);
    void OnAdapterStatus(wxCommandEvent& event);
    void OnDeleteSelected(wxCommandEvent& event);
    void OnTreeEndLabelEdit(wxTreeEvent& event);
    void OnTreeKeyDown(wxKeyEvent& event);
    void OnTreeSelectionChanged(wxTreeEvent& event);

    wxEditorTree* editorTree_ = nullptr;
    wxPropertyPanel* propertyPanel_ = nullptr;
    wxDialogService dialogService_;
    EditorTreeModel treeModel_;
};

#endif
