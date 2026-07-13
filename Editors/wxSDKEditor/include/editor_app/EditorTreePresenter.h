#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_PRESENTER_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_PRESENTER_H

#include "editor_app/EditorDocument.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

class IDialogService;
class IEditorTree;
class IPropertyPanel;

class EditorTreePresenter
{
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using PreviewCallback = std::function<void(
        const EditorTreeModel&, const std::string&)>;

    EditorTreePresenter(EditorDocument& document, IEditorTree& tree,
        IPropertyPanel& properties,
        IDialogService& dialogs, MessageCallback status,
        MessageCallback output, MessageCallback documentChanged = {},
        PreviewCallback previewChanged = {});

    void InitializeDemo();
    void NewDocument();
    void RefreshSelection();
    void AddDemoNode(const char* baseName, const char* category);
    void DeleteSelected();
    std::vector<std::string> GetMoveDestinations() const;
    bool MoveSelectedTo(const std::string& newParentPath);
    bool ApplySelectedProperty(const std::string& key, std::string value);
    bool RenameNode(EditorTreeNode& node, std::string newName,
        std::string* reason = nullptr);
    bool LoadSnapshot(const std::filesystem::path& path);
    bool SaveSnapshot(const std::filesystem::path& path);
    bool ImportPathList(std::string_view text, const std::string& sourceName);
    std::size_t FindFirst(std::string text);
    void ClearSelection();
    void ReportSelection();
    void RefreshPreview() const;
    bool SelectLogicalPath(const std::string& logicalPath);
    bool SetLogicalTransform(const std::string& path,
        const EditorTransform& transform);
    bool Undo();
    bool Redo();
    bool CanUndo() const { return history_.CanUndo(); }
    bool CanRedo() const { return history_.CanRedo(); }

private:
    EditorTreeNode* SelectedNode() const;
    void Rebuild(EditorTreeNode* selectedNode = nullptr, bool selectFirst = false);
    void RebuildByPath(const std::string& selectedPath, bool selectFirst = false);
    void PopulateNode(const EditorTreeNode& node,
        std::uintptr_t parentItem);
    void SetStatus(const std::string& message) const;
    void NotifyDocumentChanged() const;

    IEditorTree& tree_;
    IPropertyPanel& properties_;
    IDialogService& dialogs_;
    MessageCallback status_;
    MessageCallback output_;
    MessageCallback documentChanged_;
    PreviewCallback previewChanged_;
    EditorDocument& document_;
    EditorTreeModel& model_;
    EditorSelectionModel& selection_;
    EditorCommandHistory& history_;
};

#endif
