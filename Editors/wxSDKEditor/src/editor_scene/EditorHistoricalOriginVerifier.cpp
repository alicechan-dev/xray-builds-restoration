#include "editor_scene/EditorHistoricalOriginVerifier.h"

#include "editor_model/EditorItemType.h"
#include "editor_model/EditorTreeModel.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <set>

namespace
{
constexpr std::size_t MaximumDiagnostics = 32;
constexpr std::size_t MaximumSceneName = 255;
constexpr std::size_t MaximumIdentity = 4096;
constexpr std::size_t MaximumSummary = 1024;
constexpr std::size_t MaximumOpaqueSummary = 512;

bool Fail(EditorHistoricalOriginIntegrityResult& result, std::string* reason,
    const std::string& message)
{
    result.valid = false;
    if (result.diagnostics.size() < MaximumDiagnostics)
        result.diagnostics.push_back(message);
    if (reason)
        *reason = message;
    return false;
}

bool IsSafeSceneName(const std::string& value)
{
    return !value.empty() && value.size() <= MaximumSceneName &&
        value.find('/') == std::string::npos &&
        value.find('\\') == std::string::npos &&
        value.find(':') == std::string::npos;
}

bool IsDigits(const std::string& value)
{
    return !value.empty() && std::all_of(value.begin(), value.end(),
        [](unsigned char character) { return std::isdigit(character) != 0; });
}

bool IsAllowedOpaqueSummary(const std::string& value)
{
    if (value.empty())
        return true;
    std::size_t begin = 0;
    while (begin < value.size())
    {
        const std::size_t end = value.find(';', begin);
        const std::string item = value.substr(begin,
            end == std::string::npos ? std::string::npos : end - begin);
        const std::size_t separator = item.find('=');
        if (separator == std::string::npos)
            return false;
        const std::string key = item.substr(0, separator);
        if (key != "runtime_packet_bytes" && key != "attached_object_bytes")
            return false;
        if (!IsDigits(item.substr(separator + 1)))
            return false;
        if (end == std::string::npos)
            return true;
        begin = end + 1;
        if (begin < value.size() && value[begin] == ' ')
            ++begin;
    }
    return true;
}

bool VerifyNode(const EditorTreeNode& node, std::set<std::string>& paths,
    std::set<std::string>& originIds,
    EditorHistoricalOriginIntegrityResult& result, std::string* reason)
{
    ++result.nodeCount;
    if (!paths.insert(node.Path()).second)
        return Fail(result, reason, "Editor node paths are not unique.");
    if (!node.Transform().IsFinite())
        return Fail(result, reason, "Editor node transform is not finite: " +
            node.Path());

    if (!node.HistoricalOrigin())
    {
        if (!IsGroupKind(node.Kind()))
            ++result.newObjectNodeCount;
    }
    else
    {
        ++result.originNodeCount;
        const EditorHistoricalOriginMetadata& origin = *node.HistoricalOrigin();
        if (!IsSafeSceneName(origin.sourceSceneName))
            return Fail(result, reason,
                "Historical origin has an invalid source scene filename.");
        if (origin.sourceClassId > 0xffffu)
            return Fail(result, reason,
                "Historical origin class ID exceeds the 16-bit scene range.");
        if (origin.sourceObjectIndex > 0xffffffffu ||
            origin.sourceOffset > 0xffffffffu)
            return Fail(result, reason,
                "Historical source index or offset exceeds the 32-bit scene range.");
        if (origin.sourceName.size() > MaximumIdentity ||
            origin.sourceStableRecordId.size() > MaximumIdentity ||
            origin.referenceName.size() > MaximumIdentity)
            return Fail(result, reason,
                "Historical origin identity field exceeds its bound.");
        if (origin.retainedFieldSummary.size() > MaximumSummary ||
            origin.retainedWarningSummary.size() > MaximumSummary ||
            origin.opaqueDataSummary.size() > MaximumOpaqueSummary)
            return Fail(result, reason,
                "Historical origin summary exceeds its bound.");
        if (!IsAllowedOpaqueSummary(origin.opaqueDataSummary))
            return Fail(result, reason,
                "Historical origin contains a forbidden opaque-data marker.");
        if (!origin.sourceStableRecordId.empty() &&
            !originIds.insert(origin.sourceStableRecordId).second)
            return Fail(result, reason,
                "Historical source record IDs are duplicated.");
        if (origin.decodeStatus != "Supported" &&
            origin.decodeStatus != "Partial" &&
            origin.decodeStatus != "Unsupported")
            return Fail(result, reason,
                "Historical origin has an invalid decode status.");
        switch (origin.disposition)
        {
        case EditorHistoricalConversionDisposition::FullyConverted:
        case EditorHistoricalConversionDisposition::PartiallyConverted:
        case EditorHistoricalConversionDisposition::PlaceholderConverted:
            break;
        case EditorHistoricalConversionDisposition::Skipped:
            return Fail(result, reason,
                "Skipped historical records cannot own editable nodes.");
        default:
            return Fail(result, reason,
                "Historical origin has an invalid conversion disposition.");
        }
        if (origin.placeholder !=
            (origin.disposition ==
                EditorHistoricalConversionDisposition::PlaceholderConverted))
            return Fail(result, reason,
                "Historical placeholder state and disposition disagree.");
        if (origin.hasPreviewSize &&
            (!std::isfinite(origin.previewSize) || origin.previewSize < 0.0f))
            return Fail(result, reason,
                "Historical preview size is invalid.");
    }

    for (const auto& child : node.ChildrenView())
        if (!VerifyNode(*child, paths, originIds, result, reason))
            return false;
    return true;
}
}

bool VerifyEditorHistoricalOrigins(const EditorTreeModel& model,
    EditorHistoricalOriginIntegrityResult& result, std::string* reason)
{
    result = {};
    if (!model.Root())
        return Fail(result, reason, "Editor model has no root node.");
    std::set<std::string> paths;
    std::set<std::string> originIds;
    if (!VerifyNode(*model.Root(), paths, originIds, result, reason))
        return false;
    result.valid = true;
    if (reason)
        reason->clear();
    return true;
}
