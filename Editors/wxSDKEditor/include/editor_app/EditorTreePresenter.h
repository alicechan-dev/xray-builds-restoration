#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_PRESENTER_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_PRESENTER_H

#include "editor_model/EditorTreeModel.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>

class IDialogService;
class IEditorTree;
class IPropertyPanel;

class EditorTreePresenter
{
public:
    using MessageCallback = std::function<void(const std::string&)>;

    EditorTreePresenter(IEditorTree& tree, IPropertyPanel& properties,
        IDialogService& dialogs, MessageCallback status,
        MessageCallback output);

    void InitializeDemo();
    void RefreshSelection();
    void AddDemoNode(const char* baseName, const char* category);
    void DeleteSelected();
    bool RenameNode(EditorTreeNode& node, std::string newName,
        std::string* reason = nullptr);
    bool LoadSnapshot(const std::filesystem::path& path);
    bool SaveSnapshot(const std::filesystem::path& path);
    bool ImportPathList(std::string_view text, const std::string& sourceName);
    std::size_t FindFirst(std::string text);

private:
    EditorTreeNode* SelectedNode() const;
    void Rebuild(EditorTreeNode* selectedNode = nullptr, bool selectFirst = false);
    void PopulateNode(const EditorTreeNode& node,
        std::uintptr_t parentItem);
    void SetStatus(const std::string& message) const;

    IEditorTree& tree_;
    IPropertyPanel& properties_;
    IDialogService& dialogs_;
    MessageCallback status_;
    MessageCallback output_;
    EditorTreeModel model_;
};

#endif
