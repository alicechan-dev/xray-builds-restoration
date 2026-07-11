#ifndef XR_WX_SDK_EDITOR_EDITOR_TREE_SNAPSHOT_H
#define XR_WX_SDK_EDITOR_EDITOR_TREE_SNAPSHOT_H

#include <filesystem>
#include <string>

class EditorTreeModel;

bool SerializeEditorTreeSnapshot(const EditorTreeModel& model,
    std::string& output, std::string* reason = nullptr);
bool DeserializeEditorTreeSnapshot(EditorTreeModel& model,
    const std::string& input, std::string* reason = nullptr);
bool SaveEditorTreeSnapshot(const EditorTreeModel& model,
    const std::filesystem::path& path, std::string* reason = nullptr);
bool LoadEditorTreeSnapshot(EditorTreeModel& model,
    const std::filesystem::path& path, std::string* reason = nullptr);
bool RunEditorTreeSnapshotSelfCheck(std::string* failureReason = nullptr);

#endif
