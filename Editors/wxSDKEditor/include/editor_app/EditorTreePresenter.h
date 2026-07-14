#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_PRESENTER_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_PRESENTER_H

#include "editor_app/EditorDocument.h"
#include "editor_app/EditorDocumentMode.h"
#include "editor_app/EditorToolController.h"
#include "editor_assets/EditorAssetCatalog.h"
#include "editor_assets/EditorAssetSelectionModel.h"

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
class EditorHistoricalSceneDocument;
struct EditorSceneManifest;
class EditorPreviewScene;

class EditorTreePresenter
{
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using PreviewCallback = std::function<void(const EditorPreviewScene&)>;

    EditorTreePresenter(EditorDocument& document, IEditorTree& tree,
        IPropertyPanel& properties,
        IDialogService& dialogs, MessageCallback status,
        MessageCallback output, MessageCallback documentChanged = {},
        PreviewCallback previewChanged = {});

    void InitializeDemo();
    void AttachHistoricalDocument(EditorHistoricalSceneDocument& document);
    bool OpenHistoricalScene(EditorSceneManifest manifest,
        std::string* reason = nullptr);
    EditorDocumentMode GetDocumentMode() const { return mode_; }
    bool IsReadOnly() const
    { return mode_ == EditorDocumentMode::HistoricalSceneReadOnly; }
    std::string GetActiveDisplayName() const;
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
    bool SetToolMode(EditorToolMode mode);
    EditorToolMode GetToolMode() const { return tools_.GetMode(); }
    bool CancelActiveTool();
    bool PlaceAt(const EditorTransform& transform);
    bool SelectAsset(const std::string& assetId);
    void SetImportedAssetCatalog(EditorAssetCatalog catalog);
    void ClearImportedAssetCatalog();
    const EditorAssetCatalog& AssetCatalog() const { return assetCatalog_; }
    const EditorAssetDescriptor* SelectedAsset() const;
    std::string ResolvePlacementParentPath() const;
    bool Undo();
    bool Redo();
    bool CanUndo() const { return !IsReadOnly() && history_.CanUndo(); }
    bool CanRedo() const { return !IsReadOnly() && history_.CanRedo(); }

private:
    EditorTreeNode* SelectedNode() const;
    void Rebuild(EditorTreeNode* selectedNode = nullptr, bool selectFirst = false);
    void RebuildByPath(const std::string& selectedPath, bool selectFirst = false);
    void PopulateNode(const EditorTreeNode& node,
        std::uintptr_t parentItem);
    void SetStatus(const std::string& message) const;
    void NotifyDocumentChanged() const;
    const EditorAssetDescriptor* FindAsset(const std::string& assetId) const;
    EditorTreeModel& ActiveModel();
    const EditorTreeModel& ActiveModel() const;
    EditorSelectionModel& ActiveSelection();
    const EditorSelectionModel& ActiveSelection() const;
    bool RejectReadOnly(const char* operation) const;
    void UseEditableDocument();

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
    EditorHistoricalSceneDocument* historicalDocument_ = nullptr;
    EditorDocumentMode mode_ = EditorDocumentMode::EditableSnapshot;
    EditorToolController tools_;
    EditorAssetCatalog assetCatalog_ = EditorAssetCatalog::CreateBuiltIn();
    EditorAssetCatalog importedAssetCatalog_;
    EditorAssetSelectionModel assetSelection_;
};

#endif
