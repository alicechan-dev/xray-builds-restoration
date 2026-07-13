#ifndef XR_WX_SDK_EDITOR_EDITOR_METADATA_CATALOG_ADAPTER_H
#define XR_WX_SDK_EDITOR_EDITOR_METADATA_CATALOG_ADAPTER_H

#include "editor_assets/EditorAssetCatalog.h"
#include "editor_assets/EditorImportedMetadata.h"

#include <cstddef>
#include <vector>

struct EditorMetadataCatalogResult
{
    EditorAssetCatalog catalog;
    std::vector<EditorMetadataDiagnostic> diagnostics;
    std::size_t unsupportedSections = 0;
};

EditorMetadataCatalogResult BuildEditorMetadataCatalog(
    const EditorImportedMetadata& metadata);

#endif
