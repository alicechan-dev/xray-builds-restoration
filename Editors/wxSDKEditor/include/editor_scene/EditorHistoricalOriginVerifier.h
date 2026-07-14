#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_ORIGIN_VERIFIER_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_ORIGIN_VERIFIER_H

#include <cstddef>
#include <string>
#include <vector>

class EditorTreeModel;

struct EditorHistoricalOriginIntegrityResult
{
    bool valid = false;
    std::size_t nodeCount = 0;
    std::size_t originNodeCount = 0;
    std::size_t newObjectNodeCount = 0;
    std::vector<std::string> diagnostics;
};

bool VerifyEditorHistoricalOrigins(const EditorTreeModel& model,
    EditorHistoricalOriginIntegrityResult& result,
    std::string* reason = nullptr);

#endif
