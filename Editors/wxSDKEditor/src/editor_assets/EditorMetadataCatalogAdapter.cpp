#include "editor_assets/EditorMetadataCatalogAdapter.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
std::string ImportedId(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(),
        [](unsigned char character) {
            if (std::isalnum(character) || character == '.' ||
                character == '_' || character == '-')
                return static_cast<char>(std::tolower(character));
            return '_';
        });
    return "imported.section." + name;
}

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
        const EditorMetadataEntry* spawn = section.FindFirst("$spawn");
        if (!spawn || spawn->value.empty())
        {
            ++result.unsupportedSections;
            continue;
        }
        EditorAssetDescriptor descriptor;
        descriptor.id = ImportedId(section.name);
        descriptor.displayName = section.name;
        descriptor.categoryPath = "Imported/Spawn Metadata";
        descriptor.description = Summary(section);
        descriptor.baseNodeName = section.name;
        descriptor.itemKind = EditorItemKind::Object;
        descriptor.nodeCategory = "imported metadata";
        descriptor.previewKind = EditorPreviewKind::Unknown;
        descriptor.placeable = false;
        descriptor.sourceFile = section.sourceFile;
        descriptor.sourceSection = section.name;
        descriptor.sourceLine = section.sourceLine;
        if (!result.catalog.Add(std::move(descriptor)))
            result.diagnostics.push_back({
                EditorMetadataDiagnosticSeverity::Warning,
                section.sourceFile, section.sourceLine,
                "duplicate imported catalog id was skipped"});
    }
    return result;
}
