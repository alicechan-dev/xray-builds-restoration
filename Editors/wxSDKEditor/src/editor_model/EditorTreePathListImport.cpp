#include "editor_model/EditorTreePathListImport.h"

#include "editor_model/EditorTreeModel.h"

#include <cctype>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
constexpr const char* ImportedGroupCategory = "imported group";
constexpr const char* ImportedItemCategory = "imported item";

std::string Trim(std::string_view value)
{
    std::size_t first = 0;
    while (first < value.size() &&
        std::isspace(static_cast<unsigned char>(value[first])))
        ++first;

    std::size_t last = value.size();
    while (last > first &&
        std::isspace(static_cast<unsigned char>(value[last - 1])))
        --last;
    return std::string(value.substr(first, last - first));
}

bool Fail(std::string* reason, std::size_t line, const std::string& message)
{
    if (reason)
        *reason = "Line " + std::to_string(line) + ": " + message;
    return false;
}

std::string Lowercase(std::string value)
{
    for (char& character : value)
    {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character)));
    }
    return value;
}

bool SplitPath(const std::string& path, std::vector<std::string>& components,
    std::size_t line, std::string* reason)
{
    if (path.empty() || path.front() != '/')
        return Fail(reason, line, "path must begin with '/'.");
    if (path.size() == 1)
        return Fail(reason, line, "path must contain at least one component.");

    std::size_t start = 1;
    while (start <= path.size())
    {
        const std::size_t separator = path.find('/', start);
        const std::size_t end = separator == std::string::npos
            ? path.size() : separator;
        if (end == start)
            return Fail(reason, line, "path contains an empty component.");
        components.push_back(path.substr(start, end - start));
        if (separator == std::string::npos)
            break;
        start = separator + 1;
    }
    return true;
}
}

bool ImportEditorTreePathList(
    EditorTreeModel& output, std::string_view text, std::string* reason)
{
    EditorTreeModel parsed;
    std::istringstream input{std::string(text)};
    std::string lineText;
    std::unordered_set<std::string> importedPaths;
    std::size_t lineNumber = 0;
    std::size_t entryCount = 0;

    while (std::getline(input, lineText))
    {
        ++lineNumber;
        if (!lineText.empty() && lineText.back() == '\r')
            lineText.pop_back();

        const std::string line = Trim(lineText);
        if (line.empty() || line.front() == '#')
            continue;

        const std::size_t categorySeparator = line.find('|');
        const std::string path = Trim(line.substr(0, categorySeparator));
        const std::string category = categorySeparator == std::string::npos
            ? ImportedItemCategory : Trim(line.substr(categorySeparator + 1));
        if (category.empty())
            return Fail(reason, lineNumber, "category after '|' cannot be empty.");

        std::vector<std::string> components;
        if (!SplitPath(path, components, lineNumber, reason))
            return false;
        if (!importedPaths.insert(Lowercase(path)).second)
            return Fail(reason, lineNumber,
                "duplicate full path (comparison ignores case).");

        EditorTreeNode* current = parsed.Root();
        if (!current)
        {
            current = &parsed.CreateRoot(components.front(),
                components.size() == 1 ? category : ImportedGroupCategory);
        }
        else if (Lowercase(current->Label()) != Lowercase(components.front()))
        {
            return Fail(reason, lineNumber,
                "all paths must share the same root component.");
        }

        for (std::size_t index = 1; index < components.size(); ++index)
        {
            EditorTreeNode* child =
                parsed.FindChildCaseInsensitive(*current, components[index]);
            const bool leaf = index + 1 == components.size();
            if (child)
            {
                current = child;
                continue;
            }

            current = &parsed.AddChild(*current, components[index],
                leaf ? category : ImportedGroupCategory);
        }

        ++entryCount;
    }

    if (entryCount == 0)
    {
        if (reason)
            *reason = "Path list contains no entries.";
        return false;
    }

    output = std::move(parsed);
    if (reason)
        reason->clear();
    return true;
}
