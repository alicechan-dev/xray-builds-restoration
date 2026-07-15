#include "editor_view/EditorTreePreviewAdapter.h"

#include "editor_assets/EditorAssetCatalog.h"
#include "editor_assets/EditorImportedPrototype.h"
#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_assets/EditorObjectLibrary.h"
#include "editor_render/EditorRenderAssetRegistry.h"

#include <algorithm>
#include <cctype>
#include <cmath>

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
    if (node.HistoricalOrigin())
    {
        const EditorHistoricalOriginMetadata& origin = *node.HistoricalOrigin();
        if (origin.placeholder)
            return EditorPreviewKind::Unknown;
        switch (origin.sourceClassId)
        {
        case 1u: return EditorPreviewKind::Glow;
        case 2u: return EditorPreviewKind::Box;
        case 3u: return EditorPreviewKind::Light;
        case 6u: return EditorPreviewKind::Spawn;
        default: return EditorPreviewKind::Unknown;
        }
    }
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
    std::size_t& renderedIndex, const EditorRenderAssetRegistry* assets)
{
    if (node.Kind() != EditorItemKind::Root &&
        node.Kind() != EditorItemKind::Folder)
    {
        if (node.HistoricalOrigin() &&
            !node.HistoricalOrigin()->sourceTransformConfirmed)
        {
            for (const auto& child : node.ChildrenView())
                AddNodes(*child, scene, renderedIndex, assets);
            return;
        }
        const float x = node.Transform().x;
        const float z = node.Transform().z;
        const EditorPreviewKind kind = KindFor(node);
        const float y = node.Transform().y;
        float size = node.HistoricalOrigin() &&
                node.HistoricalOrigin()->hasPreviewSize
            ? node.HistoricalOrigin()->previewSize : 1.0f;
        std::string label = node.Label();
        if (assets && node.HistoricalOrigin() &&
            !node.HistoricalOrigin()->referenceName.empty()) {
            std::string normalized;
            if (NormalizeHistoricalObjectReference(
                node.HistoricalOrigin()->referenceName, normalized)) {
                if (const auto* asset = assets->Find(normalized); asset && asset->bounds.valid) {
                    const auto& b=asset->bounds; const auto& t=node.Transform();
                    const float rx=(std::max)(std::fabs(b.minX*t.sx),std::fabs(b.maxX*t.sx));
                    const float ry=(std::max)(std::fabs(b.minY*t.sy),std::fabs(b.maxY*t.sy));
                    const float rz=(std::max)(std::fabs(b.minZ*t.sz),std::fabs(b.maxZ*t.sz));
                    size=2.0f*std::sqrt(rx*rx+ry*ry+rz*rz);
                    label += " [" + normalized + "]";
                }
            }
        }
        EditorPreviewObject preview{node.Path(), label, x, y, z,
            size, size, size, kind};
        if (assets && node.HistoricalOrigin() && !node.HistoricalOrigin()->referenceName.empty()) {
            std::string id;if(NormalizeHistoricalObjectReference(node.HistoricalOrigin()->referenceName,id))
                if(const auto* asset=assets->Find(id)){preview.realBounds=asset->bounds.valid;
                    preview.renderAssetDiagnostic=std::string(ToString(asset->readiness))+"; meshes="+
                        std::to_string(asset->meshCount)+"; vertices="+std::to_string(asset->totalVertices)+
                        "; triangles="+std::to_string(asset->totalTriangles);}
        }
        scene.AddObject(std::move(preview));
        ++renderedIndex;
    }
    for (const auto& child : node.ChildrenView())
        AddNodes(*child, scene, renderedIndex, assets);
}
}

EditorPreviewScene BuildEditorPreviewScene(
    const EditorTreeModel& model, const std::string& selectedPath,
    const EditorRenderAssetRegistry* assets)
{
    EditorPreviewScene scene;
    if (model.Root())
    {
        std::size_t renderedIndex = 0;
        AddNodes(*model.Root(), scene, renderedIndex, assets);
    }
    scene.SetSelectedPath(selectedPath);
    return scene;
}
