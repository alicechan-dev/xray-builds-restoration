#include "editor_app/EditorCommandHistory.h"
#include "editor_app/EditorModelCommand.h"
#include "editor_model/EditorPropertySet.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorTreeQuery.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_scene/EditorConvertedDocumentStatistics.h"
#include "editor_scene/EditorHistoricalOriginVerifier.h"
#include "editor_scene/EditorHistoricalOriginMetadata.h"
#include "editor_view/EditorTreePreviewAdapter.h"

#include <iostream>
#include <memory>
#include <string>

namespace
{
EditorHistoricalOriginMetadata Origin(std::size_t index,
    std::uint32_t classId,
    EditorHistoricalConversionDisposition disposition =
        EditorHistoricalConversionDisposition::FullyConverted)
{
    EditorHistoricalOriginMetadata origin;
    origin.sourceSceneName = "fixture.level";
    origin.sourceSceneVersion = 5;
    origin.sourceClassId = classId;
    origin.sourceObjectIndex = index;
    origin.sourceOffset = 100 + index * 32;
    origin.sourceName = "source_" + std::to_string(index);
    origin.sourceStableRecordId = "historical.object." + std::to_string(index);
    origin.decodeStatus = disposition ==
        EditorHistoricalConversionDisposition::PlaceholderConverted
        ? "Unsupported" : disposition ==
            EditorHistoricalConversionDisposition::PartiallyConverted
            ? "Partial" : "Supported";
    origin.disposition = disposition;
    origin.sourceTransformConfirmed = true;
    origin.placeholder = disposition ==
        EditorHistoricalConversionDisposition::PlaceholderConverted;
    return origin;
}

EditorTreeModel BuildModel()
{
    EditorTreeModel model;
    EditorTreeNode& root = model.CreateRoot("Converted", "root",
        EditorItemKind::Root);
    EditorTreeNode& a = model.AddChild(root, "A", "folder",
        EditorItemKind::Folder);
    model.AddChild(root, "B", "folder", EditorItemKind::Folder);
    EditorTreeNode& scene = model.AddChild(a, "scene", "scene object",
        EditorItemKind::Object);
    model.SetNodeHistoricalOrigin(scene, Origin(1, 2));
    EditorTreeNode& light = model.AddChild(a, "light", "light",
        EditorItemKind::Object);
    model.SetNodeHistoricalOrigin(light, Origin(2, 3,
        EditorHistoricalConversionDisposition::PartiallyConverted));
    EditorTreeNode& spawn = model.AddChild(a, "spawn", "spawn",
        EditorItemKind::Object);
    EditorHistoricalOriginMetadata spawnOrigin = Origin(3, 6,
        EditorHistoricalConversionDisposition::PartiallyConverted);
    spawnOrigin.retainedWarningSummary = "Runtime packet was not copied.";
    spawnOrigin.opaqueDataSummary = "runtime_packet_bytes=64";
    model.SetNodeHistoricalOrigin(spawn, std::move(spawnOrigin));
    EditorTreeNode& placeholder = model.AddChild(a, "placeholder",
        "historical unsupported class 9", EditorItemKind::Object);
    model.SetNodeHistoricalOrigin(placeholder, Origin(4, 9,
        EditorHistoricalConversionDisposition::PlaceholderConverted));
    return model;
}

bool SameOrigin(const EditorHistoricalOriginMetadata& left,
    const EditorHistoricalOriginMetadata& right)
{
    return left.sourceSceneName == right.sourceSceneName &&
        left.sourceSceneVersion == right.sourceSceneVersion &&
        left.sourceClassId == right.sourceClassId &&
        left.sourceObjectIndex == right.sourceObjectIndex &&
        left.sourceOffset == right.sourceOffset &&
        left.sourceName == right.sourceName &&
        left.sourceStableRecordId == right.sourceStableRecordId &&
        left.decodeStatus == right.decodeStatus &&
        left.disposition == right.disposition &&
        left.retainedWarningSummary == right.retainedWarningSummary &&
        left.opaqueDataSummary == right.opaqueDataSummary;
}
}

int RunEditorHistoricalOriginIntegrityTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition) return;
        ++failures;
        std::cerr << "FAIL: historical origin " << message << '\n';
    };
    std::string reason;
    EditorTreeModel model = BuildModel();
    EditorHistoricalOriginIntegrityResult integrity;
    check(VerifyEditorHistoricalOrigins(model, integrity, &reason) &&
        integrity.nodeCount == 7 && integrity.originNodeCount == 4,
        "valid model verifies with deterministic ownership counts");

    const EditorConvertedDocumentStatistics statistics =
        CollectEditorConvertedDocumentStatistics(model);
    check(statistics.sourceSceneName == "fixture.level" &&
        statistics.fullyConverted == 1 && statistics.partiallyConverted == 2 &&
        statistics.placeholderConverted == 1 &&
        statistics.BuildSummary().find("cannot be exported") !=
            std::string::npos,
        "statistics summarize provenance without a tombstone log");

    std::string snapshotA;
    EditorTreeModel roundTrip;
    std::string snapshotB;
    check(SerializeEditorTreeSnapshot(model, snapshotA, &reason) &&
        DeserializeEditorTreeSnapshot(roundTrip, snapshotA, &reason) &&
        SerializeEditorTreeSnapshot(roundTrip, snapshotB, &reason) &&
        snapshotA == snapshotB,
        "v5 save load save is byte-for-byte deterministic");

    std::string extension = snapshotA;
    const std::size_t extensionAt = extension.find(" preview_size=\"");
    const std::size_t extensionEnd = extension.find('\n', extensionAt);
    extension.insert(extensionEnd, " x_future_summary=\"safe optional value\"");
    EditorTreeModel extensionLoaded;
    check(DeserializeEditorTreeSnapshot(extensionLoaded, extension, &reason),
        "bounded x_ optional origin field is ignored safely");
    std::string unknown = snapshotA;
    const std::size_t unknownEnd = unknown.find('\n',
        unknown.find(" preview_size=\""));
    unknown.insert(unknownEnd, " future_summary=\"unsafe unknown field\"");
    EditorTreeModel preserved = BuildModel();
    check(!DeserializeEditorTreeSnapshot(preserved, unknown, &reason) &&
        preserved.FindByPath("Converted/A/scene"),
        "unknown required-shape field rejects atomically");

    EditorTreeNode* scene = model.FindByPath("Converted/A/scene");
    const EditorHistoricalOriginMetadata original = *scene->HistoricalOrigin();
    EditorCommandHistory history;
    check(history.Execute(std::make_unique<EditorModelCommand>(model, "Rename",
        scene->Path(), [&model](std::string* selected, std::string* error) {
            EditorTreeNode* node = model.FindByPath("Converted/A/scene");
            if (!node || !model.RenameNode(*node, "renamed", error)) return false;
            *selected = node->Path(); return true;
        }), &reason), "rename command executes");
    check(history.Execute(std::make_unique<EditorModelCommand>(model, "Move",
        "Converted/A/renamed", [&model](std::string* selected,
            std::string* error) {
            EditorTreeNode* node = model.FindByPath("Converted/A/renamed");
            EditorTreeNode* parent = model.FindByPath("Converted/B");
            if (!node || !parent || !model.MoveNode(*node, *parent, error))
                return false;
            *selected = node->Path(); return true;
        }), &reason), "move command executes");
    check(history.Execute(std::make_unique<EditorModelCommand>(model,
        "Transform", "Converted/B/renamed", [&model](std::string* selected,
            std::string* error) {
            EditorTreeNode* node = model.FindByPath("Converted/B/renamed");
            if (!node) return false;
            EditorTransform transform = node->Transform(); transform.x = 42.0f;
            if (!model.SetNodeTransform(*node, transform, error)) return false;
            *selected = node->Path(); return true;
        }), &reason), "transform command executes");
    const EditorTreeNode* edited = model.FindByPath("Converted/B/renamed");
    check(edited && SameOrigin(*edited->HistoricalOrigin(), original),
        "rename move and transform preserve immutable origin");
    check(history.Undo(&reason) && history.Undo(&reason) && history.Undo(&reason) &&
        model.FindByPath("Converted/A/scene") &&
        SameOrigin(*model.FindByPath("Converted/A/scene")->HistoricalOrigin(),
            original),
        "undo sequence restores node and provenance");
    check(history.Redo(&reason) && history.Redo(&reason) && history.Redo(&reason) &&
        model.FindByPath("Converted/B/renamed") &&
        SameOrigin(*model.FindByPath("Converted/B/renamed")->HistoricalOrigin(),
            original),
        "redo sequence restores edits without rewriting provenance");

    EditorTreeNode* light = model.FindByPath("Converted/A/light");
    const EditorHistoricalOriginMetadata lightOrigin = *light->HistoricalOrigin();
    check(history.Execute(std::make_unique<EditorModelCommand>(model, "Delete",
        light->Path(), [&model](std::string* selected, std::string* error) {
            EditorTreeNode* node = model.FindByPath("Converted/A/light");
            if (!node || !model.DeleteNode(*node, error)) return false;
            selected->clear(); return true;
        }), &reason) && !model.FindByPath("Converted/A/light") &&
        history.Undo(&reason) && model.FindByPath("Converted/A/light") &&
        SameOrigin(*model.FindByPath("Converted/A/light")->HistoricalOrigin(),
            lightOrigin) && history.Redo(&reason) &&
        !model.FindByPath("Converted/A/light"),
        "delete undo redo removes and restores node-owned metadata");

    EditorTreeNode* spawn = model.FindByPath("Converted/A/spawn");
    const EditorHistoricalOriginMetadata spawnOrigin = *spawn->HistoricalOrigin();
    check(ApplyEditorNodeProperty(model, *spawn, "category", "custom edited").success &&
        SameOrigin(*spawn->HistoricalOrigin(), spawnOrigin) &&
        BuildEditorPreviewScene(model).FindByLogicalPath(spawn->Path())->kind ==
            EditorPreviewKind::Spawn,
        "category edit preserves origin and origin-first preview identity");
    EditorTreeNode& newNode = model.AddChild(*model.FindByPath("Converted/A"),
        "new_object", "custom", EditorItemKind::Object);
    check(!newNode.HistoricalOrigin(), "new synthetic object has no provenance");

    EditorTreeModel invalid = BuildModel();
    EditorTreeNode* invalidNode = invalid.FindByPath("Converted/A/spawn");
    EditorHistoricalOriginMetadata invalidOrigin = *invalidNode->HistoricalOrigin();
    invalidOrigin.retainedWarningSummary.assign(1025, 'x');
    invalid.SetNodeHistoricalOrigin(*invalidNode, invalidOrigin);
    check(!VerifyEditorHistoricalOrigins(invalid, integrity, &reason),
        "oversized warning is rejected");
    EditorTreeModel maximumBound = BuildModel();
    EditorTreeNode* maximumNode = maximumBound.FindByPath("Converted/A/spawn");
    EditorHistoricalOriginMetadata maximumOrigin = *maximumNode->HistoricalOrigin();
    maximumOrigin.retainedWarningSummary.assign(1024, 'x');
    maximumBound.SetNodeHistoricalOrigin(*maximumNode, maximumOrigin);
    check(VerifyEditorHistoricalOrigins(maximumBound, integrity, &reason),
        "maximum-size warning is accepted");
    invalid = BuildModel();
    invalidNode = invalid.FindByPath("Converted/A/spawn");
    invalidOrigin = *invalidNode->HistoricalOrigin();
    invalidOrigin.opaqueDataSummary = "raw_bytes=64";
    invalid.SetNodeHistoricalOrigin(*invalidNode, invalidOrigin);
    check(!VerifyEditorHistoricalOrigins(invalid, integrity, &reason),
        "forbidden opaque payload marker is rejected");
    invalid = BuildModel();
    invalidNode = invalid.FindByPath("Converted/A/spawn");
    invalidOrigin = *invalidNode->HistoricalOrigin();
    invalidOrigin.sourceStableRecordId =
        invalid.FindByPath("Converted/A/scene")->HistoricalOrigin()
            ->sourceStableRecordId;
    invalid.SetNodeHistoricalOrigin(*invalidNode, invalidOrigin);
    check(!VerifyEditorHistoricalOrigins(invalid, integrity, &reason),
        "duplicate historical record identity is rejected");
    invalid = BuildModel();
    invalidNode = invalid.FindByPath("Converted/A/spawn");
    invalidOrigin = *invalidNode->HistoricalOrigin();
    invalidOrigin.sourceClassId = 0x10000u;
    invalid.SetNodeHistoricalOrigin(*invalidNode, invalidOrigin);
    check(!VerifyEditorHistoricalOrigins(invalid, integrity, &reason),
        "out-of-range class identity is rejected");
    invalid = BuildModel();
    invalidNode = invalid.FindByPath("Converted/A/spawn");
    invalidOrigin = *invalidNode->HistoricalOrigin();
    invalidOrigin.disposition =
        static_cast<EditorHistoricalConversionDisposition>(999);
    invalid.SetNodeHistoricalOrigin(*invalidNode, invalidOrigin);
    check(!VerifyEditorHistoricalOrigins(invalid, integrity, &reason),
        "unknown conversion disposition is rejected");

    EditorTreeModel large;
    EditorTreeNode& largeRoot = large.CreateRoot("Large", "root",
        EditorItemKind::Root);
    EditorTreeNode& largeFolder = large.AddChild(largeRoot, "Objects", "folder",
        EditorItemKind::Folder);
    constexpr std::size_t LargeNodeCount = 20000;
    for (std::size_t index = 0; index < LargeNodeCount; ++index)
    {
        EditorTreeNode& node = large.AddChild(largeFolder,
            "object_" + std::to_string(index), "converted",
            EditorItemKind::Object);
        const auto disposition = index % 10 == 0
            ? EditorHistoricalConversionDisposition::PlaceholderConverted
            : index % 3 == 0
                ? EditorHistoricalConversionDisposition::PartiallyConverted
                : EditorHistoricalConversionDisposition::FullyConverted;
        large.SetNodeHistoricalOrigin(node, Origin(index, index % 4 + 1,
            disposition));
    }
    std::string largeSnapshot;
    EditorTreeModel largeLoaded;
    check(VerifyEditorHistoricalOrigins(large, integrity, &reason) &&
        integrity.originNodeCount == LargeNodeCount &&
        SerializeEditorTreeSnapshot(large, largeSnapshot, &reason) &&
        DeserializeEditorTreeSnapshot(largeLoaded, largeSnapshot, &reason) &&
        largeLoaded.FindByPath("Large/Objects/object_19999") &&
        BuildEditorPreviewScene(largeLoaded).GetObjects().size() == LargeNodeCount,
        "twenty-thousand-node verify save load lookup and preview rebuild");
    EditorCommandHistory largeHistory;
    check(largeHistory.Execute(std::make_unique<EditorModelCommand>(largeLoaded,
        "Large rename", "Large/Objects/object_19999",
        [&largeLoaded](std::string* selected, std::string* error) {
            EditorTreeNode* node =
                largeLoaded.FindByPath("Large/Objects/object_19999");
            if (!node || !largeLoaded.RenameNode(*node, "tail", error))
                return false;
            *selected = node->Path(); return true;
        }), &reason) && largeLoaded.FindByPath("Large/Objects/tail") &&
        largeHistory.Undo(&reason) &&
        largeLoaded.FindByPath("Large/Objects/object_19999"),
        "large document command and undo preserve model integrity");
    return failures;
}
