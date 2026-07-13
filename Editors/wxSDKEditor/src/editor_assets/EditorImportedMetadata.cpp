#include "editor_assets/EditorImportedMetadata.h"

#include <algorithm>
#include <cctype>

namespace
{
bool EqualInsensitive(std::string_view left, std::string_view right)
{
    return left.size() == right.size() &&
        std::equal(left.begin(), left.end(), right.begin(),
            [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
}
}

const EditorMetadataEntry* EditorMetadataSection::FindFirst(
    std::string_view key) const
{
    const auto found = std::find_if(entries.begin(), entries.end(),
        [key](const EditorMetadataEntry& entry) {
            return EqualInsensitive(entry.key, key);
        });
    return found == entries.end() ? nullptr : &*found;
}
