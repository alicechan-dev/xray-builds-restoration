#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_CONVERSION_VALIDATION_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_CONVERSION_VALIDATION_H

#include <string>
#include <vector>

class EditorTreeModel;
struct EditorHistoricalConversionReport;

bool VerifyHistoricalSceneConversion(const EditorTreeModel& model,
    const EditorHistoricalConversionReport& report,
    std::vector<std::string>* diagnostics = nullptr,
    std::string* reason = nullptr);

#endif
