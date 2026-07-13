#ifndef XR_WX_SDK_EDITOR_EDITOR_ASSET_CATALOG_H
#define XR_WX_SDK_EDITOR_EDITOR_ASSET_CATALOG_H

#include "editor_assets/EditorAssetDescriptor.h"

#include <string>
#include <string_view>
#include <vector>

class EditorAssetCatalog
{
public:
    bool Add(EditorAssetDescriptor descriptor);
    const EditorAssetDescriptor* FindById(std::string_view id) const;
    const std::vector<EditorAssetDescriptor>& Entries() const { return entries_; }
    std::vector<std::string> CategoryPaths() const;
    std::vector<const EditorAssetDescriptor*> FilterByCategory(
        std::string_view category) const;
    std::vector<const EditorAssetDescriptor*> Search(std::string_view text) const;
    static EditorAssetCatalog CreateBuiltIn();

private:
    std::vector<EditorAssetDescriptor> entries_;
};

#endif
