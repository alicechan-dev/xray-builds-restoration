#include "editor_view/EditorTreePreviewAdapter.h"

#include "editor_assets/EditorAssetCatalog.h"
#include "editor_assets/EditorImportedPrototype.h"
#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"

#include <algorithm>
#include <cctype>

namespace
{
std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    return value;
}

EditorPreviewKind KindFor(const EditorTreeNode& node)
{
    if (!node.AssetId().empty())
    {
        if (IsImportedAssetId(node.AssetId()))
            return EditorPreviewKind::Spawn;
        static const EditorAssetCatalog catalog =
            EditorAssetCatalog::CreateBuiltIn();
        if (const EditorAssetDescriptor* descriptor =
            catalog.FindById(node.AssetId()))
            return descriptor->previewKind;
    }
    const std::string category = Lower(node.Category());
    if (category.find("unsupported") != std::string::npos)
        return EditorPreviewKind::Unknown;
    if (category.find("light") != std::string::npos)
        return EditorPreviewKind::Light;
    if (category.find("glow") != std::string::npos)
        return EditorPreviewKind::Glow;
    if (category.find("spawn") != std::string::npos)
        return EditorPreviewKind::Spawn;
    if (node.Kind() == EditorItemKind::Object)
        return EditorPreviewKind::Box;
    return EditorPreviewKind::Marker;
}

void AddNodes(const EditorTreeNode& node, EditorPreviewScene& scene,
    std::size_t& renderedIndex)
{
    if (node.Kind() != EditorItemKind::Root &&
        node.Kind() != EditorItemKind::Folder)
    {
        if (node.HistoricalOrigin() &&
            !node.HistoricalOrigin()->sourceTransformConfirmed)
        {
            for (const auto& child : node.ChildrenView())
                AddNodes(*child, scene, renderedIndex);
            return;
        }
        const float x = node.Transform().x;
        const float z = node.Transform().z;
        const EditorPreviewKind kind = KindFor(node);
        const float y = node.Transform().y;
        const float size = node.HistoricalOrigin() &&
                node.HistoricalOrigin()->hasPreviewSize
            ? node.HistoricalOrigin()->previewSize : 1.0f;
        scene.AddObject({node.Path(), node.Label(), x, y, z,
            size, size, size, kind});
        ++renderedIndex;
    }
    for (const auto& child : node.ChildrenView())
        AddNodes(*child, scene, renderedIndex);
}
}

EditorPreviewScene BuildEditorPreviewScene(
    const EditorTreeModel& model, const std::string& selectedPath)
{
    EditorPreviewScene scene;
    if (model.Root())
    {
        std::size_t renderedIndex = 0;
        AddNodes(*model.Root(), scene, renderedIndex);
    }
    scene.SetSelectedPath(selectedPath);
    return scene;
}
