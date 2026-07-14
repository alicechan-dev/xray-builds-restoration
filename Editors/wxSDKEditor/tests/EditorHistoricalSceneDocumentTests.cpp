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
    EditorSceneChunkRecord compressed;
    compressed.id = 0x1234;
    compressed.rawId = 0x80001234;
    compressed.size = 12;
    compressed.dataOffset = 8;
    compressed.compressed = true;
    compressed.compressedSize = 12;
    compressed.path = "0x00001234";
    compressed.label = "Unknown";
    manifest.chunks.push_back(compressed);
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

    EditorSceneManifest decodedManifest;
    decodedManifest.sourceFile = "decoded.level";
    decodedManifest.version = 5;
    decodedManifest.hasVersion = true;
    EditorSceneObjectRecord decoded = Object(11, 500, "decoded_crate", true);
    decoded.classId = 2;
    decoded.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Supported;
    decoded.bodyDecode.typeName = "Scene Object";
    decoded.bodyDecode.hasBodyVersion = true;
    decoded.bodyDecode.bodyVersion = 0x0011;
    decoded.bodyDecode.hasSceneObject = true;
    decoded.bodyDecode.sceneObject.version = 0x0011;
    decoded.bodyDecode.sceneObject.referenceVersion = 17;
    decoded.bodyDecode.sceneObject.referenceName = "objects\\crate";
    decodedManifest.objects.push_back(decoded);
    EditorSceneObjectRecord malformed = Object(12, 600, "damaged", true);
    malformed.classId = 2;
    malformed.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Malformed;
    malformed.bodyDecode.typeName = "Scene Object";
    malformed.bodyDecode.diagnostics.push_back("truncated reference");
    decodedManifest.objects.push_back(malformed);

    EditorHistoricalSceneDocument decodedDocument;
    check(decodedDocument.BuildFromManifest(decodedManifest, &reason) &&
        decodedDocument.Objects().size() == 2 &&
        decodedDocument.Objects()[0].bodySupported &&
        !decodedDocument.Objects()[1].bodySupported &&
        decodedDocument.Objects()[1].diagnosticsSummary.find("truncated") !=
            std::string::npos,
        "decoded and malformed specialized bodies preserve generic records");
    const EditorPropertySet decodedProperties =
        BuildHistoricalSceneObjectPropertySet(decodedDocument.Objects()[0]);
    check(decodedProperties.Find("decode_status") &&
        decodedProperties.Find("decode_status")->value == "Supported" &&
        decodedProperties.Find("scene_object.reference") &&
        decodedProperties.Find("scene_object.reference")->value ==
            "objects\\crate",
        "scene-object fields appear as read-only historical properties");
    const EditorPreviewScene decodedPreview = BuildHistoricalScenePreview(
        decodedDocument, {});
    check(decodedPreview.GetObjects().size() == 2 &&
        decodedPreview.GetObjects()[0].label.find("objects\\crate") !=
            std::string::npos,
        "scene-object reference augments the conservative generic preview label");

    EditorSceneManifest glowManifest;
    glowManifest.sourceFile = "glow.level";
    glowManifest.version = 5;
    glowManifest.hasVersion = true;
    EditorSceneObjectRecord glow = Object(13, 700, "lamp_glow", true);
    glow.classId = 1;
    glow.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Supported;
    glow.bodyDecode.typeName = "Glow";
    glow.bodyDecode.hasBodyVersion = true;
    glow.bodyDecode.bodyVersion = 0x0012;
    glow.bodyDecode.hasGlow = true;
    glow.bodyDecode.glow.version = 0x0012;
    glow.bodyDecode.glow.hasShader = true;
    glow.bodyDecode.glow.shaderName = "effects\\glow";
    glow.bodyDecode.glow.textureName = "glow\\lamp";
    glow.bodyDecode.glow.radius = 2.5f;
    glow.bodyDecode.glow.radiusProvenance.sourceOffset = 740;
    glow.bodyDecode.glow.textureProvenance.sourceOffset = 760;
    glowManifest.objects.push_back(glow);
    EditorHistoricalSceneDocument glowDocument;
    check(glowDocument.BuildFromManifest(glowManifest, &reason) &&
        glowDocument.Objects()[0].bodySupported,
        "decoded glow attaches to historical document inertly");
    const EditorPropertySet glowProperties =
        BuildHistoricalSceneObjectPropertySet(glowDocument.Objects()[0]);
    check(glowProperties.Find("glow.texture") &&
        glowProperties.Find("glow.texture")->value == "glow\\lamp" &&
        glowProperties.Find("glow.radius") &&
        glowProperties.Find("glow.radius")->readOnly &&
        glowProperties.Find("glow.radius_source_offset")->value == "740",
        "glow fields and provenance appear as read-only properties");
    const EditorPreviewScene glowPreview = BuildHistoricalScenePreview(
        glowDocument, glowDocument.Objects()[0].stableRecordId);
    check(glowPreview.GetObjects().size() == 1 &&
        glowPreview.GetObjects()[0].kind == EditorPreviewKind::Glow &&
        glowPreview.GetObjects()[0].sizeX == 2.5f &&
        glowPreview.SelectedPath() ==
            glowDocument.Objects()[0].stableRecordId,
        "glow radius drives conservative diagnostic preview ring");

    EditorSceneManifest lightManifest;
    lightManifest.sourceFile = "light.level";
    lightManifest.version = 5;
    lightManifest.hasVersion = true;
    EditorSceneObjectRecord light = Object(14, 800, "lamp_light", true);
    light.classId = 3;
    light.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Partial;
    light.bodyDecode.typeName = "Light";
    light.bodyDecode.hasBodyVersion = true;
    light.bodyDecode.bodyVersion = 0x0011;
    light.bodyDecode.hasLight = true;
    light.bodyDecode.light.version = 0x0011;
    light.bodyDecode.light.type = 1;
    light.bodyDecode.light.color = {1.0f, 0.5f, 0.25f, 0.0f};
    light.bodyDecode.light.brightness = 2.0f;
    light.bodyDecode.light.range = 15.0f;
    light.bodyDecode.light.attenuation = {1.0f, 0.1f, 0.01f};
    light.bodyDecode.light.useInD3D = 1;
    light.bodyDecode.light.hasFuzzyData = true;
    light.bodyDecode.light.fuzzyPointCount = 4;
    light.bodyDecode.light.paramsProvenance.sourceOffset = 840;
    lightManifest.objects.push_back(light);
    EditorHistoricalSceneDocument lightDocument;
    check(lightDocument.BuildFromManifest(lightManifest, &reason) &&
        lightDocument.Objects()[0].bodySupported,
        "partial decoded light attaches to historical document inertly");
    const EditorPropertySet lightProperties =
        BuildHistoricalSceneObjectPropertySet(lightDocument.Objects()[0]);
    check(lightProperties.Find("light.type") &&
        lightProperties.Find("light.type")->value == "Point" &&
        lightProperties.Find("light.range") &&
        lightProperties.Find("light.range")->readOnly &&
        lightProperties.Find("light.params_source_offset")->value == "840" &&
        lightProperties.Find("light.fuzzy")->value == "sphere, 4 points",
        "light fields and provenance appear as read-only properties");
    const EditorPreviewScene lightPreview = BuildHistoricalScenePreview(
        lightDocument, lightDocument.Objects()[0].stableRecordId);
    check(lightPreview.GetObjects().size() == 1 &&
        lightPreview.GetObjects()[0].kind ==
            EditorPreviewKind::HistoricalLight &&
        lightPreview.GetObjects()[0].sizeX == 15.0f &&
        lightPreview.GetObjects()[0].label.find("[Point]") !=
            std::string::npos,
        "light range drives conservative historical diagnostic ring");

    const std::string preservedId = document.Objects()[0].stableRecordId;
    EditorSceneManifest invalid = Manifest();
    invalid.version = 4;
    check(!document.BuildFromManifest(std::move(invalid), &reason) &&
        document.Objects()[0].stableRecordId == preservedId,
        "failed conversion preserves prior historical document atomically");

    return failures;
}
