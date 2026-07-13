#ifndef XR_WX_SDK_EDITOR_EDITOR_ASSET_SELECTION_MODEL_H
#define XR_WX_SDK_EDITOR_EDITOR_ASSET_SELECTION_MODEL_H

#include "editor_assets/EditorAssetCatalog.h"

#include <string>

class EditorAssetSelectionModel
{
public:
    bool Select(const EditorAssetCatalog& catalog, const std::string& id);
    void Clear() { selectedId_.clear(); }
    const std::string& SelectedId() const { return selectedId_; }
    const EditorAssetDescriptor* Resolve(const EditorAssetCatalog& catalog) const;
    bool HasPlaceableSelection(const EditorAssetCatalog& catalog) const;

private:
    std::string selectedId_;
};

#endif
