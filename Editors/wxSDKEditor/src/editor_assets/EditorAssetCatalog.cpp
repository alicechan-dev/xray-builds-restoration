#include "editor_assets/EditorAssetCatalog.h"

#include <algorithm>
#include <cctype>

namespace
{
std::string Lower(std::string_view value)
{
    std::string lowered(value);
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return lowered;
}

bool Contains(std::string_view value, const std::string& loweredNeedle)
{
    return Lower(value).find(loweredNeedle) != std::string::npos;
}
}

bool EditorAssetCatalog::Add(EditorAssetDescriptor descriptor)
{
    if (descriptor.id.empty() || descriptor.displayName.empty() ||
        descriptor.baseNodeName.empty() || FindById(descriptor.id))
        return false;
    entries_.push_back(std::move(descriptor));
    return true;
}

const EditorAssetDescriptor* EditorAssetCatalog::FindById(
    std::string_view id) const
{
    const std::string key = Lower(id);
    const auto found = std::find_if(entries_.begin(), entries_.end(),
        [&key](const EditorAssetDescriptor& entry) {
            return Lower(entry.id) == key;
        });
    return found == entries_.end() ? nullptr : &*found;
}

std::vector<std::string> EditorAssetCatalog::CategoryPaths() const
{
    std::vector<std::string> categories;
    for (const EditorAssetDescriptor& entry : entries_)
    {
        if (std::find(categories.begin(), categories.end(), entry.categoryPath) ==
            categories.end())
            categories.push_back(entry.categoryPath);
    }
    return categories;
}

std::vector<const EditorAssetDescriptor*> EditorAssetCatalog::FilterByCategory(
    std::string_view category) const
{
    std::vector<const EditorAssetDescriptor*> result;
    const std::string key = Lower(category);
    for (const EditorAssetDescriptor& entry : entries_)
    {
        if (Lower(entry.categoryPath) == key)
            result.push_back(&entry);
    }
    return result;
}

std::vector<const EditorAssetDescriptor*> EditorAssetCatalog::Search(
    std::string_view text) const
{
    std::vector<const EditorAssetDescriptor*> result;
    const std::string needle = Lower(text);
    for (const EditorAssetDescriptor& entry : entries_)
    {
        if (needle.empty() || Contains(entry.displayName, needle) ||
            Contains(entry.id, needle) || Contains(entry.categoryPath, needle))
            result.push_back(&entry);
    }
    return result;
}

EditorAssetCatalog EditorAssetCatalog::CreateBuiltIn()
{
    EditorAssetCatalog catalog;
    catalog.Add({"demo.actor", "Actor Marker", "Objects",
        "Synthetic actor-shaped preview prototype.", "actor_marker",
        EditorItemKind::Object, "demo scene object", {},
        EditorPreviewKind::Box, EditorAssetPlacementType::Object});
    catalog.Add({"demo.physic_object", "Physics Object", "Objects",
        "Synthetic physics-object preview prototype.", "physic_object",
        EditorItemKind::Object, "demo scene object", {},
        EditorPreviewKind::Box, EditorAssetPlacementType::Object});
    catalog.Add({"demo.level_changer", "Level Changer", "Logic",
        "Synthetic level-change marker prototype.", "level_changer",
        EditorItemKind::Object, "demo scene object", {},
        EditorPreviewKind::Marker, EditorAssetPlacementType::Marker});
    EditorTransform lightTransform;
    lightTransform.y = 1.0f;
    catalog.Add({"demo.point_light", "Point Light", "Lights",
        "Synthetic point-light preview prototype.", "point_light",
        EditorItemKind::Object, "demo light", lightTransform,
        EditorPreviewKind::Light, EditorAssetPlacementType::Light});
    catalog.Add({"demo.ambient_sound", "Ambient Sound", "Sounds",
        "Synthetic ambient-sound marker prototype.", "ambient_sound",
        EditorItemKind::Object, "demo sound", {},
        EditorPreviewKind::Marker, EditorAssetPlacementType::Marker});
    catalog.Add({"demo.spawn", "Spawn Element", "Spawn",
        "Synthetic spawn preview prototype.", "spawn_element",
        EditorItemKind::Object, "demo spawn", {},
        EditorPreviewKind::Spawn, EditorAssetPlacementType::Spawn});
    return catalog;
}
