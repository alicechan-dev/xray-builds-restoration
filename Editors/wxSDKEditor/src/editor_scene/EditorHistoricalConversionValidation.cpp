#include "editor_scene/EditorHistoricalConversionValidation.h"

#include "editor_model/EditorTreeModel.h"
#include "editor_scene/EditorHistoricalConversionReport.h"

#include <set>

namespace
{
bool Fail(std::string* reason, const std::string& message,
    std::vector<std::string>* diagnostics)
{
    if (reason)
        *reason = message;
    if (diagnostics && diagnostics->size() < 32)
        diagnostics->push_back(message);
    return false;
}

bool VerifyNode(const EditorTreeNode& node, std::set<std::string>& paths,
    std::set<std::string>& originIds, std::size_t& origins,
    std::vector<std::string>* diagnostics, std::string* reason)
{
    if (!paths.insert(node.Path()).second)
        return Fail(reason, "Converted node paths are not unique.", diagnostics);
    if (!node.Transform().IsFinite())
        return Fail(reason, "Converted node transform is not finite.", diagnostics);

    if (node.HistoricalOrigin())
    {
        const EditorHistoricalOriginMetadata& origin = *node.HistoricalOrigin();
        ++origins;
        if (!origin.sourceStableRecordId.empty() &&
            !originIds.insert(origin.sourceStableRecordId).second)
            return Fail(reason, "Converted historical origin IDs are not unique.",
                diagnostics);
        if (origin.placeholder &&
            node.Category().find("historical unsupported class") ==
                std::string::npos)
            return Fail(reason,
                "Historical placeholder is not clearly marked unsupported.",
                diagnostics);
        if (origin.opaqueDataSummary.size() > 512 ||
            origin.retainedFieldSummary.size() > 1024 ||
            origin.retainedWarningSummary.size() > 1024)
            return Fail(reason, "Converted origin summary exceeds its bound.",
                diagnostics);
    }

    for (const auto& child : node.ChildrenView())
    {
        if (!VerifyNode(*child, paths, originIds, origins, diagnostics, reason))
            return false;
    }
    return true;
}
}

bool VerifyHistoricalSceneConversion(const EditorTreeModel& model,
    const EditorHistoricalConversionReport& report,
    std::vector<std::string>* diagnostics, std::string* reason)
{
    if (!model.Root())
        return Fail(reason, "Converted model has no root.", diagnostics);
    std::set<std::string> paths;
    std::set<std::string> originIds;
    std::size_t origins = 0;
    if (!VerifyNode(*model.Root(), paths, originIds, origins, diagnostics, reason))
        return false;
    if (origins != report.ConvertedCount())
        return Fail(reason, "Converted origin count does not match the report.",
            diagnostics);
    if (reason)
        reason->clear();
    return true;
}
