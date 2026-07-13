#include "editor_assets/EditorMetadataCatalogAdapter.h"
#include "editor_assets/EditorImportedPrototype.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
std::string Summary(const EditorMetadataSection& section)
{
    std::ostringstream output;
    output << "Read-only build-1935 metadata section.";
    for (const char* key : {"$spawn", "class", "visual"})
    {
        if (const EditorMetadataEntry* entry = section.FindFirst(key))
            output << "\n" << key << " = " << entry->value;
    }
    return output.str();
}
}

EditorMetadataCatalogResult BuildEditorMetadataCatalog(
    const EditorImportedMetadata& metadata)
{
    EditorMetadataCatalogResult result;
    for (const EditorMetadataSection& section : metadata.sections)
    {
        if (!section.FindFirst("$spawn"))
        {
            ++result.unsupportedSections;
            continue;
        }
        const EditorImportedPrototype prototype =
            ClassifyImportedSpawnPrototype(section);
        EditorAssetDescriptor descriptor;
        descriptor.id = prototype.assetId;
        descriptor.displayName = prototype.displayName.empty()
            ? section.name : prototype.displayName;
        descriptor.categoryPath = prototype.categoryPath.empty()
            ? "Imported/Invalid Spawn Metadata" : prototype.categoryPath;
        descriptor.description = Summary(section);
        descriptor.baseNodeName = prototype.baseNodeName;
        descriptor.itemKind = EditorItemKind::Object;
        descriptor.nodeCategory = "imported spawn";
        descriptor.previewKind = EditorPreviewKind::Spawn;
        descriptor.placementType = EditorAssetPlacementType::Spawn;
        descriptor.placeable = prototype.placeableAsSynthetic;
        descriptor.sourceFile = section.sourceFile;
        descriptor.sourceSection = section.name;
        descriptor.sourceLine = section.sourceLine;
        descriptor.sourceKind = EditorAssetSourceKind::ImportedSpawnMetadata;
        descriptor.rawSpawnValue = prototype.rawSpawnValue;
        descriptor.placeabilityReason = prototype.placeabilityReason;
        if (!prototype.placeableAsSynthetic)
            result.diagnostics.push_back({
                EditorMetadataDiagnosticSeverity::Warning,
                section.sourceFile, section.sourceLine,
                "imported prototype is read-only: " +
                    prototype.placeabilityReason});
        if (!result.catalog.Add(std::move(descriptor)))
            result.diagnostics.push_back({
                EditorMetadataDiagnosticSeverity::Warning,
                section.sourceFile, section.sourceLine,
                "duplicate imported catalog id was skipped"});
    }
    return result;
}
