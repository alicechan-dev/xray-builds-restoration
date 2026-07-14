#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_CONVERTER_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_CONVERTER_H

#include "editor_scene/EditorHistoricalConversionPolicy.h"

#include <string>

class EditorDocument;
class EditorHistoricalSceneDocument;
struct EditorHistoricalConversionReport;

bool DryRunHistoricalSceneConversion(
    const EditorHistoricalSceneDocument& source,
    const EditorHistoricalConversionOptions& options,
    EditorHistoricalConversionReport& report,
    std::string* reason = nullptr);

bool ConvertHistoricalSceneToEditableDocument(
    const EditorHistoricalSceneDocument& source,
    const EditorHistoricalConversionOptions& options,
    EditorDocument& destination,
    EditorHistoricalConversionReport& report,
    std::string* reason = nullptr);

#endif
