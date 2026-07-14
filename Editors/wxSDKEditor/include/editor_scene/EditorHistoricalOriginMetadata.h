#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_ORIGIN_METADATA_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_ORIGIN_METADATA_H

#include <cstddef>
#include <cstdint>
#include <string>

enum class EditorHistoricalConversionDisposition
{
    FullyConverted,
    PartiallyConverted,
    PlaceholderConverted,
    Skipped
};

const char* ToString(EditorHistoricalConversionDisposition disposition);
bool ParseEditorHistoricalConversionDisposition(const std::string& text,
    EditorHistoricalConversionDisposition& disposition);

struct EditorHistoricalOriginMetadata
{
    std::uint32_t sourceSceneVersion = 0;
    std::uint32_t sourceClassId = 0;
    std::size_t sourceObjectIndex = 0;
    std::size_t sourceOffset = 0;
    std::string sourceName;
    std::string sourceStableRecordId;
    std::string decodeStatus;
    EditorHistoricalConversionDisposition disposition =
        EditorHistoricalConversionDisposition::FullyConverted;
    std::string referenceName;
    std::string retainedFieldSummary;
    std::string retainedWarningSummary;
    std::string opaqueDataSummary;
    bool sourceTransformConfirmed = false;
    bool placeholder = false;
    bool hasPreviewSize = false;
    float previewSize = 1.0f;
};

#endif
