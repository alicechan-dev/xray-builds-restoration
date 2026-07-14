#include "editor_scene/EditorHistoricalConversionPolicy.h"

EditorHistoricalConversionDisposition DetermineHistoricalConversionDisposition(
    EditorHistoricalObjectDecodeStatus status, bool hasSpecializedRecord,
    const EditorHistoricalConversionOptions& options)
{
    if (status == EditorHistoricalObjectDecodeStatus::Malformed)
        return EditorHistoricalConversionDisposition::Skipped;
    if (status == EditorHistoricalObjectDecodeStatus::Supported &&
        hasSpecializedRecord && options.includeSupported)
        return EditorHistoricalConversionDisposition::FullyConverted;
    if (status == EditorHistoricalObjectDecodeStatus::Partial &&
        hasSpecializedRecord && options.includePartial)
        return EditorHistoricalConversionDisposition::PartiallyConverted;
    if (status == EditorHistoricalObjectDecodeStatus::Unsupported &&
        options.includeGenericPlaceholders)
        return EditorHistoricalConversionDisposition::PlaceholderConverted;
    return EditorHistoricalConversionDisposition::Skipped;
}
