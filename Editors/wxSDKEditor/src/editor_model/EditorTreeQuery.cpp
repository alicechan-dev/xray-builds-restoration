#include "editor_model/EditorTreeQuery.h"

#include "editor_model/EditorTreeModel.h"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace
{
std::string Normalize(std::string_view value, bool caseSensitive)
{
    std::string normalized(value);
    if (!caseSensitive)
    {
        std::transform(normalized.begin(), normalized.end(), normalized.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
    }
    return normalized;
}

bool MatchesField(const std::string& field, const std::string& query,
    const EditorTreeQueryOptions& options)
{
    const std::string candidate = Normalize(field, options.caseSensitive);
    return options.exactMatch
        ? candidate == query
        : candidate.find(query) != std::string::npos;
}

void QueryNode(const EditorTreeNode& node, const EditorTreeQueryOptions& options,
    const std::string& query, EditorTreeQueryResult& results)
{
    const bool kindMatches = !options.kind || node.Kind() == *options.kind;
    const bool emptyQuery = options.text.empty();
    const bool textMatches = emptyQuery ||
        (options.matchLabel && MatchesField(node.Label(), query, options)) ||
        (options.matchPath && MatchesField(node.Path(), query, options));
    if (kindMatches && textMatches)
        results.push_back(&node);

    for (const auto& child : node.ChildrenView())
        QueryNode(*child, options, query, results);
}
}

EditorTreeQueryResult QueryEditorTree(
    const EditorTreeModel& model, const EditorTreeQueryOptions& options)
{
    EditorTreeQueryResult results;
    if (!model.Root())
        return results;

    const std::string query = Normalize(options.text, options.caseSensitive);
    QueryNode(*model.Root(), options, query, results);
    return results;
}
