#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_CONVERSION_POLICY_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_CONVERSION_POLICY_H

#include "editor_scene/EditorHistoricalOriginMetadata.h"
#include "editor_scene/objects/EditorHistoricalObjectBodyRecord.h"

struct EditorHistoricalConversionOptions
{
    bool includeSupported = true;
    bool includePartial = true;
    bool includeGenericPlaceholders = true;
    bool preserveSourceProvenance = true;
};

EditorHistoricalConversionDisposition DetermineHistoricalConversionDisposition(
    EditorHistoricalObjectDecodeStatus status, bool hasSpecializedRecord,
    const EditorHistoricalConversionOptions& options);

#endif
