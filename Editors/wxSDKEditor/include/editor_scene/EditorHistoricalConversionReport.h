#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_CONVERSION_REPORT_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_CONVERSION_REPORT_H

#include "editor_scene/EditorHistoricalOriginMetadata.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct EditorHistoricalConversionClassCount
{
    std::uint32_t classId = 0;
    std::size_t total = 0;
    std::size_t fullyConverted = 0;
    std::size_t partiallyConverted = 0;
    std::size_t placeholderConverted = 0;
    std::size_t skipped = 0;
};

struct EditorHistoricalConversionReport
{
    std::string sourceSceneDisplayName;
    std::uint32_t sourceVersion = 0;
    std::size_t totalHistoricalRecords = 0;
    std::size_t fullyConverted = 0;
    std::size_t partiallyConverted = 0;
    std::size_t placeholderConverted = 0;
    std::size_t skipped = 0;
    std::size_t recordsWithoutTransforms = 0;
    std::size_t estimatedSnapshotBytes = 0;
    std::vector<EditorHistoricalConversionClassCount> perClass;
    std::vector<std::string> warnings;
    std::vector<std::string> retainedOpaqueDataSummaries;
    std::vector<std::string> unsupportedFields;

    std::size_t ConvertedCount() const;
    void AddWarning(std::string warning);
    void AddOpaqueSummary(std::string summary);
    void AddUnsupportedField(std::string field);
    std::string BuildSummary() const;
};

#endif
