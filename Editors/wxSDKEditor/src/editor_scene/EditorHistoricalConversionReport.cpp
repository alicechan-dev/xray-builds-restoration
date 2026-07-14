#include "editor_scene/EditorHistoricalConversionReport.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace
{
constexpr std::size_t MaximumReportItems = 64;
constexpr std::size_t MaximumReportText = 512;

void AddBoundedUnique(std::vector<std::string>& destination, std::string value)
{
    if (value.empty() || destination.size() >= MaximumReportItems)
        return;
    if (value.size() > MaximumReportText)
        value.resize(MaximumReportText);
    if (std::find(destination.begin(), destination.end(), value) ==
        destination.end())
        destination.push_back(std::move(value));
}
}

std::size_t EditorHistoricalConversionReport::ConvertedCount() const
{
    return fullyConverted + partiallyConverted + placeholderConverted;
}

void EditorHistoricalConversionReport::AddWarning(std::string warning)
{
    AddBoundedUnique(warnings, std::move(warning));
}

void EditorHistoricalConversionReport::AddOpaqueSummary(std::string summary)
{
    AddBoundedUnique(retainedOpaqueDataSummaries, std::move(summary));
}

void EditorHistoricalConversionReport::AddUnsupportedField(std::string field)
{
    AddBoundedUnique(unsupportedFields, std::move(field));
}

std::string EditorHistoricalConversionReport::BuildSummary() const
{
    std::ostringstream output;
    output << "Historical conversion report: source="
        << sourceSceneDisplayName << ", version=" << sourceVersion
        << ", total=" << totalHistoricalRecords
        << ", full=" << fullyConverted
        << ", partial=" << partiallyConverted
        << ", placeholders=" << placeholderConverted
        << ", skipped=" << skipped
        << ", without_transform=" << recordsWithoutTransforms
        << ", estimated_snapshot_bytes=" << estimatedSnapshotBytes << ".\n";
    for (const EditorHistoricalConversionClassCount& item : perClass)
        output << "  class " << item.classId << ": total=" << item.total
            << ", full=" << item.fullyConverted
            << ", partial=" << item.partiallyConverted
            << ", placeholders=" << item.placeholderConverted
            << ", skipped=" << item.skipped << ".\n";
    if (!warnings.empty())
    {
        output << "Warnings:\n";
        for (const std::string& warning : warnings)
            output << "  - " << warning << '\n';
    }
    output << "This editable copy cannot be saved back to historical .level format.";
    return output.str();
}
