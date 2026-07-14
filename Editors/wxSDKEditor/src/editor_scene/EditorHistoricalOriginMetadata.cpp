#include "editor_scene/EditorHistoricalOriginMetadata.h"

const char* ToString(EditorHistoricalConversionDisposition disposition)
{
    switch (disposition)
    {
    case EditorHistoricalConversionDisposition::FullyConverted:
        return "Fully Converted";
    case EditorHistoricalConversionDisposition::PartiallyConverted:
        return "Partially Converted";
    case EditorHistoricalConversionDisposition::PlaceholderConverted:
        return "Placeholder Converted";
    case EditorHistoricalConversionDisposition::Skipped:
        return "Skipped";
    }
    return "Skipped";
}

bool ParseEditorHistoricalConversionDisposition(const std::string& text,
    EditorHistoricalConversionDisposition& disposition)
{
    if (text == "Fully Converted")
        disposition = EditorHistoricalConversionDisposition::FullyConverted;
    else if (text == "Partially Converted")
        disposition = EditorHistoricalConversionDisposition::PartiallyConverted;
    else if (text == "Placeholder Converted")
        disposition = EditorHistoricalConversionDisposition::PlaceholderConverted;
    else if (text == "Skipped")
        disposition = EditorHistoricalConversionDisposition::Skipped;
    else
        return false;
    return true;
}
