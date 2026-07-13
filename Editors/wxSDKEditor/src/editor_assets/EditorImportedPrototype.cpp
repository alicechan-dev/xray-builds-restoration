#include "editor_assets/EditorImportedPrototype.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace
{
constexpr std::string_view ImportedPrefix = "imported.section.";

std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return value;
}

std::string ImportedId(std::string name)
{
    return std::string(ImportedPrefix) + MakeSafeImportedNodeName(
        Lower(std::move(name)));
}

bool IsSafeSectionIdentity(const std::string& section)
{
    return !section.empty() && std::all_of(section.begin(), section.end(),
        [](unsigned char character) {
            return std::isalnum(character) || character == '_' ||
                character == '-' || character == '.';
        });
}

bool DecodeSpawnLabel(const std::string& raw, std::string& decoded,
    std::string& reason)
{
    if (raw.size() < 2 || raw.front() != '"' || raw.back() != '"')
    {
        reason = "$spawn must be a non-empty quoted editor label.";
        return false;
    }
    decoded = raw.substr(1, raw.size() - 2);
    if (decoded.empty() || decoded.front() == '\\' ||
        decoded.find('/') != std::string::npos ||
        decoded.find(':') != std::string::npos)
    {
        reason = "$spawn contains an empty, rooted, or unsupported label path.";
        return false;
    }
    std::size_t start = 0;
    while (start <= decoded.size())
    {
        const std::size_t end = decoded.find('\\', start);
        const std::string segment = decoded.substr(start,
            end == std::string::npos ? std::string::npos : end - start);
        if (segment.empty() || segment == "." || segment == ".." ||
            std::any_of(segment.begin(), segment.end(),
                [](unsigned char character) {
                    return std::iscntrl(character) != 0;
                }))
        {
            reason = "$spawn contains an unsafe or empty label segment.";
            return false;
        }
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return true;
}
}

std::string MakeSafeImportedNodeName(std::string value)
{
    std::string result;
    result.reserve(value.size());
    bool pendingSeparator = false;
    for (const unsigned char character : value)
    {
        if (std::isalnum(character) || character == '_' || character == '-')
        {
            if (pendingSeparator && !result.empty() && result.back() != '_')
                result += '_';
            result += static_cast<char>(std::tolower(character));
            pendingSeparator = false;
        }
        else
            pendingSeparator = true;
    }
    while (!result.empty() && result.back() == '_')
        result.pop_back();
    return result.empty() ? "imported_spawn" : result;
}

bool IsImportedAssetId(std::string_view assetId)
{
    return assetId.size() > ImportedPrefix.size() &&
        std::equal(ImportedPrefix.begin(), ImportedPrefix.end(),
            assetId.begin(), [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
}

std::string ImportedSectionFromAssetId(std::string_view assetId)
{
    return IsImportedAssetId(assetId)
        ? std::string(assetId.substr(ImportedPrefix.size())) : std::string();
}

EditorImportedPrototype ClassifyImportedSpawnPrototype(
    const EditorMetadataSection& section)
{
    EditorImportedPrototype prototype;
    prototype.sectionName = section.name;
    prototype.sourceFile = section.sourceFile;
    prototype.sourceLine = section.sourceLine;
    prototype.assetId = ImportedId(section.name);
    prototype.baseNodeName = MakeSafeImportedNodeName(section.name);

    std::vector<const EditorMetadataEntry*> spawnEntries;
    for (const EditorMetadataEntry& entry : section.entries)
    {
        if (entry.key == "$spawn")
            spawnEntries.push_back(&entry);
    }
    if (spawnEntries.empty())
    {
        prototype.placeabilityReason = "Section has no $spawn editor label.";
        return prototype;
    }
    prototype.rawSpawnValue = spawnEntries.front()->value;
    if (spawnEntries.size() != 1)
    {
        prototype.placeabilityReason =
            "Section has ambiguous duplicate $spawn records.";
        return prototype;
    }
    if (!IsSafeSectionIdentity(section.name))
    {
        prototype.placeabilityReason =
            "Section name is not a stable safe prototype identity.";
        return prototype;
    }
    if (!DecodeSpawnLabel(prototype.rawSpawnValue, prototype.spawnLabel,
        prototype.placeabilityReason))
        return prototype;

    const std::size_t firstSeparator = prototype.spawnLabel.find('\\');
    const std::size_t lastSeparator = prototype.spawnLabel.find_last_of('\\');
    const std::string group = prototype.spawnLabel.substr(0, firstSeparator);
    const std::string leaf = prototype.spawnLabel.substr(
        lastSeparator == std::string::npos ? 0 : lastSeparator + 1);
    prototype.displayName = leaf + " [" + section.name + "]";
    prototype.categoryPath = "Imported/" + group;
    prototype.placeableAsSynthetic = true;
    prototype.placeabilityReason =
        "Placeable as an inert synthetic Spawn marker.";
    return prototype;
}
