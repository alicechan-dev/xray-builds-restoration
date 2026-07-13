#include "editor_model/EditorTreeSnapshot.h"

#include "editor_model/EditorTreeModel.h"

#include <fstream>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
constexpr const char* SnapshotHeaderV1 = "# wxSDKEditor tree snapshot v1";
constexpr const char* SnapshotHeaderV2 = "# wxSDKEditor tree snapshot v2";
constexpr const char* SnapshotHeaderV3 = "# wxSDKEditor tree snapshot v3";
constexpr std::streamoff MaximumSnapshotSize = 8 * 1024 * 1024;

bool Fail(std::string* reason, const std::string& message)
{
    if (reason)
        *reason = message;
    return false;
}

std::string Escape(const std::string& value)
{
    std::string escaped;
    for (const char character : value)
    {
        switch (character)
        {
        case '\\': escaped += "\\\\"; break;
        case '"': escaped += "\\\""; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default: escaped += character; break;
        }
    }
    return escaped;
}

void SerializeNode(const EditorTreeNode& node, unsigned int depth,
    std::string& output)
{
    output += "node depth=" + std::to_string(depth) +
        " kind=\"" + std::string(ToString(node.Kind())) +
        "\" label=\"" + Escape(node.Label()) +
        "\" category=\"" + Escape(node.Category()) +
        "\" path=\"" + Escape(node.Path()) + "\" transform=\"";
    const EditorTransform& t=node.Transform();
    output += std::to_string(t.x)+" "+std::to_string(t.y)+" "+std::to_string(t.z)+" "+std::to_string(t.yaw)+" "+std::to_string(t.pitch)+" "+std::to_string(t.roll)+" "+std::to_string(t.sx)+" "+std::to_string(t.sy)+" "+std::to_string(t.sz)+"\"\n";
    for (const auto& child : node.ChildrenView())
        SerializeNode(*child, depth + 1, output);
}

bool Consume(const std::string& line, std::size_t& position, const char* text)
{
    const std::string expected(text);
    if (line.compare(position, expected.size(), expected) != 0)
        return false;
    position += expected.size();
    return true;
}

bool ParseDepth(const std::string& line, std::size_t& position, unsigned int& depth)
{
    if (position >= line.size() || line[position] < '0' || line[position] > '9')
        return false;

    depth = 0;
    while (position < line.size() && line[position] >= '0' && line[position] <= '9')
    {
        const unsigned int digit = static_cast<unsigned int>(line[position] - '0');
        if (depth > (std::numeric_limits<unsigned int>::max() - digit) / 10)
            return false;
        depth = depth * 10 + digit;
        ++position;
    }
    return true;
}

bool ParseQuoted(const std::string& line, std::size_t& position, std::string& value)
{
    if (position >= line.size() || line[position++] != '"')
        return false;

    value.clear();
    while (position < line.size())
    {
        const char character = line[position++];
        if (character == '"')
            return true;
        if (character != '\\')
        {
            value += character;
            continue;
        }
        if (position >= line.size())
            return false;

        switch (line[position++])
        {
        case '\\': value += '\\'; break;
        case '"': value += '"'; break;
        case 'n': value += '\n'; break;
        case 'r': value += '\r'; break;
        case 't': value += '\t'; break;
        default: return false;
        }
    }
    return false;
}

bool ParseNodeLine(const std::string& line, unsigned int& depth,
    bool hasKind, bool hasTransform, EditorItemKind& kind, std::string& label,
    std::string& category, std::string& path, EditorTransform& transform)
{
    std::size_t position = 0;
    if (!Consume(line, position, "node depth=") ||
        !ParseDepth(line, position, depth))
        return false;
    if (hasKind)
    {
        std::string kindText;
        if (!Consume(line, position, " kind=") ||
            !ParseQuoted(line, position, kindText) ||
            !ParseEditorItemKind(kindText, kind))
            return false;
    }
    if (!(
        Consume(line, position, " label=") &&
        ParseQuoted(line, position, label) &&
        Consume(line, position, " category=") &&
        ParseQuoted(line, position, category) &&
        Consume(line, position, " path=") &&
        ParseQuoted(line, position, path))) return false;
    if (!hasTransform) return position == line.size();
    std::string values;
    if (!Consume(line,position," transform=")||!ParseQuoted(line,position,values)||position!=line.size()) return false;
    std::istringstream input(values); input>>transform.x>>transform.y>>transform.z>>transform.yaw>>transform.pitch>>transform.roll>>transform.sx>>transform.sy>>transform.sz;
    return input && input.peek()==std::char_traits<char>::eof() && transform.IsFinite();
}
}

bool SerializeEditorTreeSnapshot(
    const EditorTreeModel& model, std::string& output, std::string* reason)
{
    if (!model.Root())
        return Fail(reason, "The tree model has no root node.");

    output = std::string(SnapshotHeaderV3) + "\n";
    SerializeNode(*model.Root(), 0, output);
    if (reason)
        reason->clear();
    return true;
}

bool DeserializeEditorTreeSnapshot(
    EditorTreeModel& model, const std::string& input, std::string* reason)
{
    if (input.empty())
        return Fail(reason, "The snapshot is empty.");

    std::istringstream stream(input);
    std::string line;
    if (!std::getline(stream, line))
        return Fail(reason, "The snapshot header is missing.");
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    const bool hasTransform = line == SnapshotHeaderV3;
    const bool hasKind = hasTransform || line == SnapshotHeaderV2;
    if (!hasKind && line != SnapshotHeaderV1)
        return Fail(reason, "Unsupported or malformed snapshot header.");

    EditorTreeModel parsed;
    std::vector<EditorTreeNode*> depthStack;
    unsigned int lineNumber = 1;
    while (std::getline(stream, line))
    {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            return Fail(reason, "Malformed empty line at " + std::to_string(lineNumber) + ".");

        unsigned int depth = 0;
        std::string label;
        std::string category;
        std::string storedPath;
        EditorItemKind kind = EditorItemKind::Unknown;
        EditorTransform transform;
        if (!ParseNodeLine(
            line, depth, hasKind, hasTransform, kind, label, category, storedPath, transform))
            return Fail(reason, "Malformed node record at line " + std::to_string(lineNumber) + ".");
        if (label.empty())
            return Fail(reason, "Empty node label at line " + std::to_string(lineNumber) + ".");

        EditorTreeNode* node = nullptr;
        if (depth == 0)
        {
            if (parsed.Root() || !depthStack.empty())
                return Fail(reason, "Multiple root nodes are not allowed.");
            node = &parsed.CreateRoot(label, category,
                hasKind ? kind : EditorItemKind::Root);
            depthStack.push_back(node);
        }
        else
        {
            if (!parsed.Root() || depth > depthStack.size())
                return Fail(reason, "Invalid depth at line " + std::to_string(lineNumber) + ".");
            depthStack.resize(depth);
            EditorTreeNode* parent = depthStack[depth - 1];
            if (parsed.FindChildCaseInsensitive(*parent, label))
                return Fail(reason, "Duplicate sibling label at line " + std::to_string(lineNumber) + ".");
            node = &parsed.AddChild(*parent, label, category,
                hasKind ? kind : InferEditorItemKind(category));
            depthStack.push_back(node);
        }

        if (node->Path() != storedPath)
            return Fail(reason, "Stored path mismatch at line " + std::to_string(lineNumber) + ".");
        if (!parsed.SetNodeTransform(*node, transform, reason))
            return false;
    }

    if (!parsed.Root())
        return Fail(reason, "The snapshot contains no nodes.");

    model = std::move(parsed);
    if (reason)
        reason->clear();
    return true;
}

bool SaveEditorTreeSnapshot(const EditorTreeModel& model,
    const std::filesystem::path& path, std::string* reason)
{
    std::string snapshot;
    if (!SerializeEditorTreeSnapshot(model, snapshot, reason))
        return false;

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
        return Fail(reason, "Could not open the snapshot for writing.");
    file.write(snapshot.data(), static_cast<std::streamsize>(snapshot.size()));
    if (!file)
        return Fail(reason, "Failed while writing the snapshot.");
    if (reason)
        reason->clear();
    return true;
}

bool LoadEditorTreeSnapshot(EditorTreeModel& model,
    const std::filesystem::path& path, std::string* reason)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        return Fail(reason, "Could not open the snapshot for reading.");
    const std::streamoff size = file.tellg();
    if (size < 0 || size > MaximumSnapshotSize)
        return Fail(reason, "The snapshot size is invalid or exceeds 8 MiB.");
    file.seekg(0);

    std::string snapshot(static_cast<std::size_t>(size), '\0');
    if (size > 0)
        file.read(&snapshot[0], static_cast<std::streamsize>(size));
    if (!file && size > 0)
        return Fail(reason, "Failed while reading the snapshot.");
    return DeserializeEditorTreeSnapshot(model, snapshot, reason);
}

bool RunEditorTreeSnapshotSelfCheck(std::string* failureReason)
{
    EditorTreeModel source = EditorTreeModel::CreateDemoScene();
    const std::string escapedLabel = "quoted\"\\line\nitem";
    source.AddChild(*source.Root(), escapedLabel, "category\tvalue");
    std::string snapshot;
    if (!SerializeEditorTreeSnapshot(source, snapshot, failureReason))
        return false;

    EditorTreeModel loaded;
    if (!DeserializeEditorTreeSnapshot(loaded, snapshot, failureReason))
        return false;
    if (!loaded.Root() || !loaded.FindByPath("Scene (demo data)/Objects/actor"))
        return Fail(failureReason, "Snapshot round trip lost the demo hierarchy.");
    EditorTreeNode* escaped = loaded.FindByLabel(escapedLabel);
    if (!escaped || escaped->Category() != "category\tvalue")
        return Fail(failureReason, "Snapshot escaping round trip failed.");

    const std::string duplicate =
        std::string(SnapshotHeaderV1) + "\n"
        "node depth=0 label=\"Root\" category=\"root\" path=\"Root\"\n"
        "node depth=1 label=\"child\" category=\"item\" path=\"Root/child\"\n"
        "node depth=1 label=\"CHILD\" category=\"item\" path=\"Root/CHILD\"\n";
    EditorTreeModel rejected = EditorTreeModel::CreateDemoScene();
    if (DeserializeEditorTreeSnapshot(rejected, duplicate, failureReason))
        return Fail(failureReason, "Duplicate sibling snapshot was accepted.");
    if (!rejected.FindByPath("Scene (demo data)/Objects/actor"))
        return Fail(failureReason, "Failed load changed the existing model.");
    if (DeserializeEditorTreeSnapshot(rejected, "not a snapshot\n", failureReason))
        return Fail(failureReason, "Malformed snapshot was accepted.");

    if (failureReason)
        failureReason->clear();
    return true;
}
