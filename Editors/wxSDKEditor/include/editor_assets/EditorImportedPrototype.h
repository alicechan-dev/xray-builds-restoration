#ifndef XR_WX_SDK_EDITOR_EDITOR_IMPORTED_PROTOTYPE_H
#define XR_WX_SDK_EDITOR_EDITOR_IMPORTED_PROTOTYPE_H

#include "editor_assets/EditorImportedMetadata.h"

#include <string>
#include <string_view>

enum class EditorImportedPrototypeKind { SpawnMetadata };

struct EditorImportedPrototype
{
    std::string assetId;
    std::string sectionName;
    std::string sourceFile;
    std::size_t sourceLine = 0;
    std::string rawSpawnValue;
    std::string spawnLabel;
    std::string displayName;
    std::string categoryPath;
    std::string baseNodeName;
    EditorImportedPrototypeKind kind =
        EditorImportedPrototypeKind::SpawnMetadata;
    bool placeableAsSynthetic = false;
    std::string placeabilityReason;
};

EditorImportedPrototype ClassifyImportedSpawnPrototype(
    const EditorMetadataSection& section);
std::string MakeSafeImportedNodeName(std::string value);
bool IsImportedAssetId(std::string_view assetId);
std::string ImportedSectionFromAssetId(std::string_view assetId);

#endif
