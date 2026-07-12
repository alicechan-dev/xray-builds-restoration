#ifndef XR_WX_SDK_EDITOR_EDITOR_PREVIEW_SCENE_H
#define XR_WX_SDK_EDITOR_EDITOR_PREVIEW_SCENE_H

#include <string>
#include <vector>

enum class EditorPreviewKind
{
    Box,
    Marker,
    Light,
    Spawn,
    Unknown
};

struct EditorPreviewObject
{
    std::string logicalPath;
    std::string label;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float sizeZ = 1.0f;
    EditorPreviewKind kind = EditorPreviewKind::Unknown;
    bool selected = false;
    bool visible = true;
};

class EditorPreviewScene
{
public:
    void Clear();
    EditorPreviewObject& AddObject(EditorPreviewObject object);
    bool RemoveObject(const std::string& logicalPath);
    EditorPreviewObject* FindByLogicalPath(const std::string& logicalPath);
    const EditorPreviewObject* FindByLogicalPath(
        const std::string& logicalPath) const;
    void SetSelectedPath(const std::string& logicalPath);
    const std::string& SelectedPath() const { return selectedPath_; }
    const std::vector<EditorPreviewObject>& GetObjects() const { return objects_; }
    static EditorPreviewScene BuildDemoScene();

private:
    std::vector<EditorPreviewObject> objects_;
    std::string selectedPath_;
};

#endif
