#ifndef XR_WX_SDK_EDITOR_EDITOR_DOCUMENT_H
#define XR_WX_SDK_EDITOR_EDITOR_DOCUMENT_H

#include "editor_app/EditorCommandHistory.h"
#include "editor_model/EditorSelectionModel.h"
#include "editor_model/EditorTreeModel.h"

#include <filesystem>
#include <string>
#include <string_view>

class EditorDocument
{
public:
    EditorDocument();

    void NewDocument();
    bool LoadFromSnapshot(const std::filesystem::path& path,
        std::string* reason = nullptr);
    bool Save(std::string* reason = nullptr);
    bool SaveAs(const std::filesystem::path& path,
        std::string* reason = nullptr);
    bool ImportPathList(std::string_view text, std::string* reason = nullptr);
    bool ReplaceWithConvertedModel(EditorTreeModel model,
        std::string* reason = nullptr);

    bool IsModified() const;
    bool HasFilePath() const { return !filePath_.empty(); }
    const std::filesystem::path& GetFilePath() const { return filePath_; }
    std::string GetDisplayName() const;

    EditorTreeModel& Model() { return model_; }
    const EditorTreeModel& Model() const { return model_; }
    EditorSelectionModel& Selection() { return selection_; }
    EditorCommandHistory& History() { return history_; }
    const EditorCommandHistory& History() const { return history_; }

private:
    bool CaptureSavedState(std::string* reason = nullptr);

    EditorTreeModel model_;
    EditorSelectionModel selection_;
    EditorCommandHistory history_;
    std::filesystem::path filePath_;
    std::string savedSnapshot_;
    bool importedDirty_ = false;
};

#endif
