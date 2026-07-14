#include "editor_scene/EditorConvertedDocumentStatistics.h"

#include "editor_model/EditorTreeModel.h"

#include <sstream>

namespace
{
void CollectNode(const EditorTreeNode& node,
    EditorConvertedDocumentStatistics& result)
{
    if (node.HistoricalOrigin())
    {
        const EditorHistoricalOriginMetadata& origin = *node.HistoricalOrigin();
        if (result.sourceSceneName.empty())
            result.sourceSceneName = origin.sourceSceneName;
        ++result.classCounts[origin.sourceClassId];
        if (!origin.retainedWarningSummary.empty())
            ++result.warningNodes;
        if (!origin.sourceTransformConfirmed)
            ++result.missingTransforms;
        switch (origin.disposition)
        {
        case EditorHistoricalConversionDisposition::FullyConverted:
            ++result.fullyConverted; break;
        case EditorHistoricalConversionDisposition::PartiallyConverted:
            ++result.partiallyConverted; break;
        case EditorHistoricalConversionDisposition::PlaceholderConverted:
            ++result.placeholderConverted; break;
        case EditorHistoricalConversionDisposition::Skipped:
            break;
        }
    }
    for (const auto& child : node.ChildrenView())
        CollectNode(*child, result);
}
}

EditorConvertedDocumentStatistics CollectEditorConvertedDocumentStatistics(
    const EditorTreeModel& model)
{
    EditorConvertedDocumentStatistics result;
    VerifyEditorHistoricalOrigins(model, result.integrity, nullptr);
    result.totalNodes = result.integrity.nodeCount;
    result.historicalNodes = result.integrity.originNodeCount;
    result.newlyCreatedNodes = result.integrity.newObjectNodeCount;
    if (model.Root())
        CollectNode(*model.Root(), result);
    return result;
}

std::string EditorConvertedDocumentStatistics::BuildSummary() const
{
    std::ostringstream output;
    output << "Original scene: " <<
        (sourceSceneName.empty() ? "unknown" : sourceSceneName) << '\n'
        << "Current nodes: " << totalNodes << '\n'
        << "Historical-origin nodes: " << historicalNodes << '\n'
        << "Newly added objects: " << newlyCreatedNodes << '\n'
        << "Fully converted: " << fullyConverted << '\n'
        << "Partially converted: " << partiallyConverted << '\n'
        << "Placeholders: " << placeholderConverted << '\n'
        << "Nodes with warnings: " << warningNodes << '\n'
        << "Missing confirmed transforms: " << missingTransforms << '\n'
        << "Deleted historical nodes: not tracked (no tombstone log)\n"
        << "Origin integrity: " << (integrity.valid ? "ok" : "failed") << '\n'
        << "Per-class counts:";
    for (const auto& item : classCounts)
        output << "\n  class " << item.first << ": " << item.second;
    if (!integrity.diagnostics.empty())
    {
        output << "\nDiagnostics:";
        for (const std::string& diagnostic : integrity.diagnostics)
            output << "\n  " << diagnostic;
    }
    output << "\n\nThis document cannot be exported back to historical .level format.";
    return output.str();
}
