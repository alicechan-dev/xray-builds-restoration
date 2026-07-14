#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_DOCUMENT_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_DOCUMENT_H

#include "editor_model/EditorSelectionModel.h"
#include "editor_model/EditorTransform.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_scene/EditorSceneManifest.h"
#include "editor_scene/objects/EditorHistoricalSceneObjectRecord.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct EditorHistoricalSceneObjectData
{
    std::string stableRecordId;
    std::string nodePath;
    std::size_t objectIndex = 0;
    std::uint32_t classId = 0;
    std::string sourceName;
    EditorTransform transform;
    bool transformConfirmed = false;
    bool bodySupported = false;
    EditorHistoricalObjectBodyDecodeResult bodyDecode;
    std::string chunkPath;
    std::size_t sourceOffset = 0;
    bool fromDecompressedPayload = false;
    std::size_t compressedSourceOffset = 0;
    std::size_t decompressedOffset = 0;
    std::uint32_t sceneVersion = 0;
    std::string diagnosticsSummary;
};

class EditorHistoricalSceneDocument
{
public:
    bool BuildFromManifest(EditorSceneManifest manifest,
        std::string* reason = nullptr);
    void Clear();

    bool IsLoaded() const { return loaded_; }
    bool IsReadOnly() const { return true; }
    bool IsModified() const { return false; }
    const EditorTreeModel& GetTreeModel() const { return model_; }
    EditorTreeModel& GetTreeModel() { return model_; }
    const EditorSelectionModel& GetSelectionModel() const { return selection_; }
    EditorSelectionModel& GetSelectionModel() { return selection_; }
    const EditorSceneManifest& GetManifest() const { return manifest_; }
    const std::filesystem::path& GetSourcePath() const { return sourcePath_; }
    std::string GetDisplayName() const;
    std::uint32_t GetSceneVersion() const { return manifest_.version; }
    std::size_t GetConfirmedObjectCount() const { return objects_.size(); }
    std::size_t GetNamedObjectCount() const;
    std::size_t GetTransformedObjectCount() const;
    std::size_t GetUnsupportedCompressedChunkCount() const;
    std::size_t GetUnsupportedBodyCount() const;
    const std::vector<EditorHistoricalSceneObjectData>& Objects() const
    { return objects_; }
    const EditorHistoricalSceneObjectData* FindByStableId(
        const std::string& stableId) const;
    const EditorHistoricalSceneObjectData* FindByNodePath(
        const std::string& nodePath) const;

private:
    EditorSceneManifest manifest_;
    EditorTreeModel model_;
    EditorSelectionModel selection_;
    std::filesystem::path sourcePath_;
    std::vector<EditorHistoricalSceneObjectData> objects_;
    bool loaded_ = false;
};

#endif
