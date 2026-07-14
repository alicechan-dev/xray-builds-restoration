#ifndef XR_WX_SDK_EDITOR_EDITOR_CONVERTED_DOCUMENT_STATISTICS_H
#define XR_WX_SDK_EDITOR_EDITOR_CONVERTED_DOCUMENT_STATISTICS_H

#include "editor_scene/EditorHistoricalOriginVerifier.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

class EditorTreeModel;

struct EditorConvertedDocumentStatistics
{
    std::string sourceSceneName;
    std::size_t totalNodes = 0;
    std::size_t historicalNodes = 0;
    std::size_t newlyCreatedNodes = 0;
    std::size_t fullyConverted = 0;
    std::size_t partiallyConverted = 0;
    std::size_t placeholderConverted = 0;
    std::size_t warningNodes = 0;
    std::size_t missingTransforms = 0;
    std::map<std::uint32_t, std::size_t> classCounts;
    EditorHistoricalOriginIntegrityResult integrity;

    bool HasHistoricalOrigins() const { return historicalNodes != 0; }
    std::string BuildSummary() const;
};

EditorConvertedDocumentStatistics CollectEditorConvertedDocumentStatistics(
    const EditorTreeModel& model);

#endif
