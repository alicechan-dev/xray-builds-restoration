#include "editor_app/EditorDocument.h"
#include "editor_model/EditorPropertySet.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_scene/EditorHistoricalConversionReport.h"
#include "editor_scene/EditorHistoricalConversionValidation.h"
#include "editor_scene/EditorHistoricalSceneConverter.h"
#include "editor_scene/EditorHistoricalSceneDocument.h"
#include "editor_view/EditorTreePreviewAdapter.h"

#include <iostream>
#include <string>

namespace
{
EditorSceneObjectRecord BaseObject(std::size_t index, std::uint32_t classId,
    const char* name, bool transform = true)
{
    EditorSceneObjectRecord object;
    object.recordIndex = index;
    object.sourceOffset = 100 + index * 32;
    object.classId = classId;
    object.hasClassId = true;
    object.name = name;
    object.hasName = true;
    object.hasTransform = transform;
    if (transform)
    {
        object.position = {static_cast<float>(index), 2.0f, 3.0f};
        object.rotation = {0.1f, 0.2f, 0.3f};
        object.scale = {1.0f, 1.0f, 1.0f};
    }
    return object;
}

EditorHistoricalSceneDocument ConversionFixture()
{
    EditorSceneManifest manifest;
    manifest.sourceFile = "C:/lawful/fixture.level";
    manifest.version = 5;
    manifest.hasVersion = true;

    EditorSceneObjectRecord scene = BaseObject(1, 2, "scene_object");
    scene.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Supported;
    scene.bodyDecode.typeName = "Scene Object";
    scene.bodyDecode.hasSceneObject = true;
    scene.bodyDecode.sceneObject.referenceName = "objects\\crate";
    scene.bodyDecode.sceneObject.hasFlags = true;
    scene.bodyDecode.sceneObject.flags = 3;
    manifest.objects.push_back(scene);

    EditorSceneObjectRecord glow = BaseObject(2, 1, "glow");
    glow.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Supported;
    glow.bodyDecode.typeName = "Glow";
    glow.bodyDecode.hasGlow = true;
    glow.bodyDecode.glow.radius = 4.5f;
    glow.bodyDecode.glow.shaderName = "effects\\glow";
    glow.bodyDecode.glow.textureName = "glow_texture";
    glow.bodyDecode.glow.hasFlags = true;
    glow.bodyDecode.glow.flags = 2;
    manifest.objects.push_back(glow);

    EditorSceneObjectRecord light = BaseObject(3, 3, "light");
    light.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Partial;
    light.bodyDecode.typeName = "Light";
    light.bodyDecode.hasLight = true;
    light.bodyDecode.light.type = 1;
    light.bodyDecode.light.brightness = 2.0f;
    light.bodyDecode.light.range = 12.0f;
    light.bodyDecode.light.cone = 0.5f;
    light.bodyDecode.light.virtualSize = 0.25f;
    light.bodyDecode.light.hasFuzzyData = true;
    light.bodyDecode.light.fuzzyPayloadSize = 48;
    manifest.objects.push_back(light);

    EditorSceneObjectRecord spawn = BaseObject(4, 6, "spawn");
    spawn.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Partial;
    spawn.bodyDecode.typeName = "Spawn Point";
    spawn.bodyDecode.hasSpawnPoint = true;
    spawn.bodyDecode.spawnPoint.type = 2;
    spawn.bodyDecode.spawnPoint.hasEntityReference = true;
    spawn.bodyDecode.spawnPoint.entityReference = "actor";
    spawn.bodyDecode.spawnPoint.hasRuntimePacket = true;
    spawn.bodyDecode.spawnPoint.runtimePacketSize = 128;
    spawn.bodyDecode.spawnPoint.hasAttachedObject = true;
    spawn.bodyDecode.spawnPoint.attachedObjectSize = 64;
    manifest.objects.push_back(spawn);

    EditorSceneObjectRecord generic = BaseObject(5, 9, "way", false);
    generic.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Unsupported;
    generic.bodyDecode.typeName = "Unknown historical object";
    manifest.objects.push_back(generic);

    EditorSceneObjectRecord malformed = BaseObject(6, 1, "bad_glow");
    malformed.bodyDecode.status = EditorHistoricalObjectDecodeStatus::Malformed;
    malformed.bodyDecode.typeName = "Glow";
    manifest.objects.push_back(malformed);

    EditorHistoricalSceneDocument document;
    std::string reason;
    document.BuildFromManifest(std::move(manifest), &reason);
    return document;
}
}

int RunEditorHistoricalSceneConversionTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition) return;
        ++failures;
        std::cerr << "FAIL: historical conversion " << message << '\n';
    };

    const EditorHistoricalSceneDocument source = ConversionFixture();
    EditorHistoricalConversionOptions options;
    EditorHistoricalConversionReport dryRun;
    std::string reason;
    check(source.IsLoaded() && DryRunHistoricalSceneConversion(
        source, options, dryRun, &reason), "default dry-run succeeds");
    check(dryRun.totalHistoricalRecords == 6 && dryRun.fullyConverted == 2 &&
        dryRun.partiallyConverted == 2 && dryRun.placeholderConverted == 1 &&
        dryRun.skipped == 1 && dryRun.recordsWithoutTransforms == 1 &&
        dryRun.estimatedSnapshotBytes > 0,
        "default policy counts full partial placeholder malformed and transform");

    EditorHistoricalConversionOptions strict;
    strict.includePartial = false;
    strict.includeGenericPlaceholders = false;
    EditorHistoricalConversionReport strictReport;
    check(DryRunHistoricalSceneConversion(source, strict, strictReport, &reason) &&
        strictReport.fullyConverted == 2 && strictReport.partiallyConverted == 0 &&
        strictReport.placeholderConverted == 0 && strictReport.skipped == 4,
        "policy overrides exclude partial and placeholder records");

    EditorDocument destination;
    EditorTreeNode* oldActor = destination.Model().FindByLabel("actor");
    check(ConvertHistoricalSceneToEditableDocument(
        source, options, destination, dryRun, &reason),
        "atomic conversion succeeds");
    check(destination.IsModified() && !destination.HasFilePath() &&
        !destination.History().CanUndo() &&
        destination.Selection().SelectedCount() == 0 && oldActor !=
            destination.Model().FindByLabel("actor"),
        "converted document is dirty pathless unselected with empty history");
    check(source.Objects().size() == 6 && source.IsReadOnly(),
        "source historical document remains unchanged and read-only");

    const EditorTreeNode* scene = destination.Model().FindByLabel("scene_object");
    const EditorTreeNode* glow = destination.Model().FindByLabel("glow");
    const EditorTreeNode* light = destination.Model().FindByLabel("light");
    const EditorTreeNode* spawn = destination.Model().FindByLabel("spawn");
    const EditorTreeNode* placeholder = destination.Model().FindByLabel("way");
    check(scene && scene->HistoricalOrigin() &&
        scene->HistoricalOrigin()->sourceSceneName == "fixture.level" &&
        scene->HistoricalOrigin()->referenceName == "objects\\crate" &&
        scene->HistoricalOrigin()->retainedFieldSummary.find("flags=0x3") !=
            std::string::npos,
        "SceneObject reference flags and transform are preserved inertly");
    check(glow && glow->HistoricalOrigin() &&
        glow->HistoricalOrigin()->hasPreviewSize &&
        glow->HistoricalOrigin()->previewSize == 4.5f &&
        glow->HistoricalOrigin()->retainedFieldSummary.find("glow_texture") !=
            std::string::npos,
        "Glow fields and diagnostic radius are preserved without resolution");
    check(light && light->HistoricalOrigin() &&
        light->HistoricalOrigin()->disposition ==
            EditorHistoricalConversionDisposition::PartiallyConverted &&
        light->HistoricalOrigin()->retainedWarningSummary.find("Fuzzy") !=
            std::string::npos,
        "Light scalar fields retain an explicit fuzzy warning");
    check(spawn && spawn->HistoricalOrigin() &&
        spawn->HistoricalOrigin()->referenceName == "actor" &&
        spawn->HistoricalOrigin()->opaqueDataSummary.find("128") !=
            std::string::npos &&
        spawn->HistoricalOrigin()->opaqueDataSummary.find("64") !=
            std::string::npos,
        "SpawnPoint keeps section and opaque sizes without packet bytes");
    check(placeholder && placeholder->HistoricalOrigin() &&
        placeholder->HistoricalOrigin()->placeholder &&
        placeholder->Category().find("unsupported") != std::string::npos,
        "generic record becomes a clearly marked placeholder");
    check(!destination.Model().FindByLabel("bad_glow"),
        "malformed specialized record is always skipped");

    const EditorPropertySet properties = BuildEditorNodePropertySet(*spawn);
    check(properties.Find("label") && !properties.Find("label")->readOnly &&
        properties.Find("label")->section == "Editable" &&
        properties.Find("historical.class_id") &&
        properties.Find("historical.class_id")->readOnly &&
        properties.Find("historical.class_id")->section ==
            "Historical Origin - Read-Only" &&
        properties.Find("historical.opaque") &&
        properties.Find("historical.opaque")->readOnly,
        "ordinary fields stay editable while origin and opaque summaries are read-only");

    const EditorPreviewScene preview = BuildEditorPreviewScene(
        destination.Model(), spawn->Path());
    check(preview.GetObjects().size() == 4 &&
        preview.FindByLogicalPath(glow->Path()) &&
        preview.FindByLogicalPath(glow->Path())->kind == EditorPreviewKind::Glow &&
        preview.FindByLogicalPath(light->Path())->kind == EditorPreviewKind::Light &&
        preview.FindByLogicalPath(spawn->Path())->kind == EditorPreviewKind::Spawn &&
        !preview.FindByLogicalPath(placeholder->Path()),
        "converted preview preserves semantics and omits unconfirmed placement");

    std::vector<std::string> diagnostics;
    check(VerifyHistoricalSceneConversion(destination.Model(), dryRun,
        &diagnostics, &reason) && diagnostics.empty(),
        "converted model verification passes");

    std::string snapshot;
    EditorTreeModel roundTrip;
    check(SerializeEditorTreeSnapshot(destination.Model(), snapshot, &reason) &&
        snapshot.find("snapshot v5") != std::string::npos &&
        DeserializeEditorTreeSnapshot(roundTrip, snapshot, &reason),
        "origin metadata snapshot v5 round-trip succeeds");
    const EditorTreeNode* loadedSpawn = roundTrip.FindByLabel("spawn");
    check(loadedSpawn && loadedSpawn->HistoricalOrigin() &&
        loadedSpawn->HistoricalOrigin()->sourceClassId == 6 &&
        loadedSpawn->HistoricalOrigin()->opaqueDataSummary ==
            spawn->HistoricalOrigin()->opaqueDataSummary,
        "origin and opaque size summary persist across snapshot round-trip");

    EditorTreeModel atomicSnapshot = EditorTreeModel::CreateDemoScene();
    std::string malformedSnapshot = snapshot;
    const std::size_t disposition = malformedSnapshot.find(
        "disposition=\"Partially Converted\"");
    if (disposition != std::string::npos)
        malformedSnapshot.replace(disposition,
            std::string("disposition=\"Partially Converted\"").size(),
            "disposition=\"Invented\"");
    check(!DeserializeEditorTreeSnapshot(
        atomicSnapshot, malformedSnapshot, &reason) &&
        atomicSnapshot.FindByLabel("actor"),
        "malformed origin metadata fails atomically");

    EditorHistoricalSceneDocument emptySource;
    EditorDocument preserved;
    std::string before;
    std::string after;
    SerializeEditorTreeSnapshot(preserved.Model(), before, nullptr);
    check(!ConvertHistoricalSceneToEditableDocument(
        emptySource, options, preserved, dryRun, &reason),
        "conversion rejects an unloaded source");
    SerializeEditorTreeSnapshot(preserved.Model(), after, nullptr);
    check(before == after && !preserved.IsModified(),
        "failed conversion preserves the destination document atomically");

    const std::string oldV4 =
        "# wxSDKEditor tree snapshot v4\n"
        "node depth=0 kind=\"root\" label=\"Root\" category=\"root\" "
        "path=\"Root\" asset=\"\" transform=\"0 0 0 0 0 0 1 1 1\"\n";
    EditorTreeModel oldSnapshot;
    check(DeserializeEditorTreeSnapshot(oldSnapshot, oldV4, &reason) &&
        oldSnapshot.Root() && !oldSnapshot.Root()->HistoricalOrigin(),
        "snapshot v4 remains readable without origin metadata");

    return failures;
}
