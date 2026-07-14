#include "editor_scene/EditorHistoricalConversionValidation.h"

#include "editor_model/EditorTreeModel.h"
#include "editor_scene/EditorHistoricalConversionReport.h"
#include "editor_scene/EditorHistoricalOriginVerifier.h"

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

}

bool VerifyHistoricalSceneConversion(const EditorTreeModel& model,
    const EditorHistoricalConversionReport& report,
    std::vector<std::string>* diagnostics, std::string* reason)
{
    EditorHistoricalOriginIntegrityResult integrity;
    if (!VerifyEditorHistoricalOrigins(model, integrity, reason))
    {
        if (diagnostics)
            *diagnostics = integrity.diagnostics;
        return false;
    }
    if (integrity.originNodeCount != report.ConvertedCount())
        return Fail(reason, "Converted origin count does not match the report.",
            diagnostics);
    if (reason)
        reason->clear();
    return true;
}
