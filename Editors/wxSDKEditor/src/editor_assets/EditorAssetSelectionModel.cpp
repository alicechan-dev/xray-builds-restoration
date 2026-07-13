#include "editor_assets/EditorAssetSelectionModel.h"

bool EditorAssetSelectionModel::Select(
    const EditorAssetCatalog& catalog, const std::string& id)
{
    const EditorAssetDescriptor* descriptor = catalog.FindById(id);
    if (!descriptor || !descriptor->placeable)
        return false;
    selectedId_ = descriptor->id;
    return true;
}

const EditorAssetDescriptor* EditorAssetSelectionModel::Resolve(
    const EditorAssetCatalog& catalog) const
{
    return catalog.FindById(selectedId_);
}

bool EditorAssetSelectionModel::HasPlaceableSelection(
    const EditorAssetCatalog& catalog) const
{
    const EditorAssetDescriptor* descriptor = Resolve(catalog);
    return descriptor && descriptor->placeable;
}
