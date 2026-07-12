#include "editor_view/EditorPreviewScene.h"

#include <algorithm>
#include <utility>

void EditorPreviewScene::Clear()
{
    objects_.clear();
    selectedPath_.clear();
}

EditorPreviewObject& EditorPreviewScene::AddObject(EditorPreviewObject object)
{
    objects_.push_back(std::move(object));
    return objects_.back();
}

bool EditorPreviewScene::RemoveObject(const std::string& logicalPath)
{
    const auto found = std::find_if(objects_.begin(), objects_.end(),
        [&logicalPath](const EditorPreviewObject& object) {
            return object.logicalPath == logicalPath;
        });
    if (found == objects_.end())
        return false;
    objects_.erase(found);
    if (selectedPath_ == logicalPath)
        selectedPath_.clear();
    return true;
}

EditorPreviewObject* EditorPreviewScene::FindByLogicalPath(
    const std::string& logicalPath)
{
    const auto found = std::find_if(objects_.begin(), objects_.end(),
        [&logicalPath](const EditorPreviewObject& object) {
            return object.logicalPath == logicalPath;
        });
    return found == objects_.end() ? nullptr : &*found;
}

const EditorPreviewObject* EditorPreviewScene::FindByLogicalPath(
    const std::string& logicalPath) const
{
    const auto found = std::find_if(objects_.begin(), objects_.end(),
        [&logicalPath](const EditorPreviewObject& object) {
            return object.logicalPath == logicalPath;
        });
    return found == objects_.end() ? nullptr : &*found;
}

void EditorPreviewScene::SetSelectedPath(const std::string& logicalPath)
{
    selectedPath_ = logicalPath;
    for (EditorPreviewObject& object : objects_)
        object.selected = object.logicalPath == logicalPath;
}

EditorPreviewScene EditorPreviewScene::BuildDemoScene()
{
    EditorPreviewScene scene;
    scene.AddObject({"Demo/box", "box", -2.0f, 0.0f, -5.0f,
        1.0f, 1.0f, 1.0f, EditorPreviewKind::Box});
    scene.AddObject({"Demo/light", "light", 2.0f, 2.0f, -5.0f,
        1.0f, 1.0f, 1.0f, EditorPreviewKind::Light});
    return scene;
}
