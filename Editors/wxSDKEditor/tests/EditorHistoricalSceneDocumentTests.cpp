#include "editor_scene/EditorHistoricalSceneDocument.h"
#include "editor_scene/EditorHistoricalScenePreviewAdapter.h"
#include "editor_scene/EditorHistoricalSceneProperties.h"

#include <iostream>
#include <string>

namespace
{
EditorSceneObjectRecord Object(std::size_t recordIndex, std::size_t offset,
    const char* name, bool transformed)
{
    EditorSceneObjectRecord object;
    object.recordIndex = recordIndex;
    object.sourceOffset = offset;
    object.classId = 3;
    object.hasClassId = true;
    object.name = name ? name : "";
    object.hasName = name != nullptr;
    object.chunkPath = "0x00008003/0x00000003/0x" +
        std::to_string(recordIndex);
    object.hasTransform = transformed;
    if (transformed)
    {
        object.position = {1.0f + static_cast<float>(recordIndex), 2.0f, 3.0f};
        object.rotation = {0.1f, 0.2f, 0.3f};
        object.scale = {1.0f, 2.0f, 3.0f};
    }
    return object;
}

EditorSceneManifest Manifest()
{
    EditorSceneManifest manifest;
    manifest.sourceFile = "C:/fixtures/test.level";
    manifest.totalSize = 512;
    manifest.format = "Build 1935 LevelEditor scene v5";
    manifest.version = 5;
    manifest.hasVersion = true;
    manifest.declaredObjectCount = 4;
    manifest.hasDeclaredObjectCount = true;
    manifest.objects.push_back(Object(7, 100, "crate", true));
    manifest.objects.push_back(Object(8, 200, "crate", true));
    manifest.objects.push_back(Object(9, 300, nullptr, false));
    manifest.objects.push_back(Object(10, 400, "marker", false));
    manifest.chunks.push_back({0x1234, 0x80001234, 12, 0, 8, 0, true,
        "0x00001234", "Unknown"});
    return manifest;
}
}

int RunEditorHistoricalSceneDocumentTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition)
            return;
        ++failures;
        std::cerr << "FAIL: historical document " << message << '\n';
    };

    EditorHistoricalSceneDocument empty;
    EditorSceneManifest emptyManifest;
    emptyManifest.sourceFile = "empty.level";
    emptyManifest.version = 5;
    emptyManifest.hasVersion = true;
    std::string reason;
    check(empty.BuildFromManifest(emptyManifest, &reason) && empty.IsLoaded() &&
        empty.IsReadOnly() && !empty.IsModified() &&
        empty.GetConfirmedObjectCount() == 0,
        "empty version-5 scene converts read-only");

    EditorHistoricalSceneDocument document;
    check(document.BuildFromManifest(Manifest(), &reason),
        "manifest conversion succeeds");
    check(document.GetDisplayName() == "test.level" &&
        document.GetSceneVersion() == 5 &&
        document.GetConfirmedObjectCount() == 4 &&
        document.GetNamedObjectCount() == 3 &&
        document.GetTransformedObjectCount() == 2 &&
        document.GetUnsupportedCompressedChunkCount() == 1 &&
        document.GetUnsupportedBodyCount() == 4,
        "summary counts are preserved");

    const auto& objects = document.Objects();
    check(objects.size() == 4 &&
        objects[0].stableRecordId == "historical.object.7.100" &&
        objects[1].stableRecordId == "historical.object.8.200" &&
        objects[0].stableRecordId != objects[1].stableRecordId,
        "stable identity uses index and source offset, not name");
    check(objects[0].sourceName == "crate" &&
        objects[1].sourceName == "crate" &&
        objects[0].nodePath.find("crate") != std::string::npos &&
        objects[1].nodePath.find("crate [#2]") != std::string::npos,
        "duplicate source names remain unchanged with display suffix");
    check(objects[2].sourceName.empty() &&
        objects[2].nodePath.find("unnamed_object_9") != std::string::npos,
        "unnamed records receive deterministic display fallback");
    check(objects[0].sourceOffset == 100 &&
        objects[0].chunkPath.find("0x7") != std::string::npos &&
        objects[0].transform.NearlyEquals(
            {8.0f, 2.0f, 3.0f, 0.2f, 0.1f, 0.3f, 1.0f, 2.0f, 3.0f}),
        "provenance and confirmed transform map exactly");
    check(!objects[2].transformConfirmed && !objects[2].bodySupported,
        "unsupported transform and body metadata remain explicit");

    const EditorPropertySet properties =
        BuildHistoricalSceneObjectPropertySet(objects[0]);
    check(properties.Find("stable_record_id") &&
        properties.Find("source_chunk_path") &&
        properties.Find("source_offset") &&
        properties.Find("read_only") &&
        properties.Find("read_only")->value == "true",
        "historical provenance properties are present");
    bool allReadOnly = true;
    for (const EditorProperty& property : properties.Properties())
        allReadOnly = allReadOnly && property.readOnly;
    check(allReadOnly, "all historical properties are read-only");

    const EditorPreviewScene preview = BuildHistoricalScenePreview(
        document, objects[1].stableRecordId);
    check(preview.GetObjects().size() == 2 &&
        preview.FindByLogicalPath(objects[0].stableRecordId) &&
        preview.FindByLogicalPath(objects[1].stableRecordId) &&
        !preview.FindByLogicalPath(objects[2].stableRecordId) &&
        preview.SelectedPath() == objects[1].stableRecordId,
        "preview includes only confirmed transforms with stable identities");

    const std::string preservedId = document.Objects()[0].stableRecordId;
    EditorSceneManifest invalid = Manifest();
    invalid.version = 4;
    check(!document.BuildFromManifest(std::move(invalid), &reason) &&
        document.Objects()[0].stableRecordId == preservedId,
        "failed conversion preserves prior historical document atomically");

    return failures;
}
