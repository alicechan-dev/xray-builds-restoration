#include "editor_assets/EditorObjectLibraryLoader.h"
#include "editor_assets/EditorStaticMeshDecoder.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_render/EditorRenderAssetRegistry.h"
#include "editor_render/EditorRenderAssetWorkingSet.h"
#include "editor_render/EditorRenderGeometryCache.h"
#include "editor_render/EditorRenderScene.h"
#include "editor_render/EditorSoftwareWireframeRenderer.h"
#include "editor_view/EditorViewportController.h"

#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

namespace
{
using Bytes = std::vector<unsigned char>;
struct Position { float x, y, z; };
struct Triangle { std::uint32_t a, b, c; };
struct MeshSpec
{
    std::vector<Position> positions;
    std::vector<Triangle> triangles;
};

void U16(Bytes& out, std::uint16_t value)
{ out.push_back(value & 255); out.push_back(value >> 8); }
void U32(Bytes& out, std::uint32_t value)
{ for (int i = 0; i < 4; ++i) out.push_back((value >> (i * 8)) & 255); }
void F32(Bytes& out, float value)
{ std::uint32_t bits = 0; std::memcpy(&bits, &value, 4); U32(out, bits); }
void Z(Bytes& out, const char* value)
{ while (*value) out.push_back(*value++); out.push_back(0); }
void Chunk(Bytes& out, std::uint32_t id, const Bytes& payload)
{ U32(out, id); U32(out, static_cast<std::uint32_t>(payload.size())); out.insert(out.end(), payload.begin(), payload.end()); }

Bytes BuildObject(const std::vector<MeshSpec>& specs, bool skeletal = false,
    std::uint16_t meshVersion = 0x0011)
{
    Bytes body, payload;
    U16(payload, 0x0010); Chunk(body, 0x0900, payload); payload.clear();
    U32(payload, skeletal ? 1u : 0u); Chunk(body, 0x0903, payload); payload.clear();
    U32(payload, 1); U32(payload, 0); Chunk(body, 0x0911, payload); payload.clear();
    U32(payload, 1); Z(payload, "surface"); Z(payload, "default");
    Z(payload, "default"); Z(payload, "materials\\default");
    Z(payload, "textures\\default"); Z(payload, "Texture");
    U32(payload, 0); U32(payload, 0); U32(payload, 1);
    Chunk(body, 0x0907, payload); payload.clear();

    Bytes meshContainer;
    for (std::size_t meshIndex = 0; meshIndex < specs.size(); ++meshIndex)
    {
        const auto& spec = specs[meshIndex];
        Bytes mesh, value;
        U16(value, meshVersion); Chunk(mesh, 0x1000, value); value.clear();
        Z(value, ("mesh" + std::to_string(meshIndex)).c_str());
        Chunk(mesh, 0x1001, value); value.clear();
        Position minimum{}, maximum{};
        if (!spec.positions.empty()) minimum = maximum = spec.positions.front();
        for (const auto& p : spec.positions)
        {
            minimum.x = (std::min)(minimum.x, p.x); minimum.y = (std::min)(minimum.y, p.y); minimum.z = (std::min)(minimum.z, p.z);
            maximum.x = (std::max)(maximum.x, p.x); maximum.y = (std::max)(maximum.y, p.y); maximum.z = (std::max)(maximum.z, p.z);
        }
        F32(value, minimum.x); F32(value, minimum.y); F32(value, minimum.z);
        F32(value, maximum.x); F32(value, maximum.y); F32(value, maximum.z);
        Chunk(mesh, 0x1004, value); value.clear();
        U32(value, static_cast<std::uint32_t>(spec.positions.size()));
        for (const auto& p : spec.positions) { F32(value, p.x); F32(value, p.y); F32(value, p.z); }
        for (std::size_t i = 0; i < spec.positions.size(); ++i) value.push_back(0);
        Chunk(mesh, 0x1005, value); value.clear();
        U32(value, static_cast<std::uint32_t>(spec.triangles.size()));
        for (const auto& face : spec.triangles)
        {
            U32(value, face.a); U32(value, 0); U32(value, face.b);
            U32(value, 0); U32(value, face.c); U32(value, 0);
        }
        Chunk(mesh, 0x1006, value); value.clear();
        U32(value, static_cast<std::uint32_t>(spec.positions.size()));
        for (std::size_t i = 0; i < spec.positions.size(); ++i) value.push_back(0);
        Chunk(mesh, 0x1008, value); value.clear();
        U16(value, 1); Z(value, "surface");
        U32(value, static_cast<std::uint32_t>(spec.triangles.size()));
        for (std::uint32_t i = 0; i < spec.triangles.size(); ++i) U32(value, i);
        Chunk(mesh, 0x1009, value); value.clear();
        U32(value, 1); Z(value, "Texture"); value.push_back(2);
        value.push_back(0); value.push_back(0);
        U32(value, static_cast<std::uint32_t>(spec.positions.size()));
        for (std::size_t i = 0; i < spec.positions.size() * 2; ++i) F32(value, 0.0f);
        for (std::uint32_t i = 0; i < spec.positions.size(); ++i) U32(value, i);
        Chunk(mesh, 0x1012, value);
        Bytes wrapper; Chunk(wrapper, static_cast<std::uint32_t>(meshIndex), mesh);
        meshContainer.insert(meshContainer.end(), wrapper.begin(), wrapper.end());
    }
    Chunk(body, 0x0910, meshContainer);
    if (skeletal) { Bytes bone; Chunk(body, 0x0921, bone); }
    Bytes file; Chunk(file, 0x7777, body); return file;
}

bool Write(const std::filesystem::path& path, const Bytes& bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(stream);
}

int Check(bool condition, const char* message)
{
    if (condition) return 0;
    std::cerr << "Static geometry test failed: " << message << '\n';
    return 1;
}

EditorRenderObjectAsset Asset(const char* id, const char* file,
    const std::vector<MeshSpec>& specs)
{
    EditorRenderObjectAsset asset;
    asset.assetId = id;
    asset.sourceRelativeFile = file;
    asset.objectKind = EditorObjectKind::Static;
    asset.readiness = EditorRenderAssetReadiness::StaticGeometryDecodeCandidate;
    asset.meshCount = specs.size();
    for (const auto& spec : specs)
    {
        asset.totalVertices += spec.positions.size();
        asset.totalTriangles += spec.triangles.size();
    }
    return asset;
}

EditorWireframeCamera Camera()
{
    EditorWireframeCamera camera;
    camera.y = 0.0f;
    camera.viewportWidth = 800;
    camera.viewportHeight = 600;
    return camera;
}

EditorRenderInstance Instance(const EditorRenderObjectAsset& asset)
{
    EditorRenderInstance instance;
    instance.assetId = asset.assetId;
    instance.objectBounds = asset.bounds;
    instance.readiness = asset.readiness;
    instance.fallback = false;
    return instance;
}
}

int RunEditorStaticMeshTests()
{
    int failures = 0;
    const auto unique = std::chrono::high_resolution_clock::now()
        .time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() /
        ("xr-wx-static-geometry-" + std::to_string(unique));
    const MeshSpec triangle{{{-1, -1, 0}, {1, -1, 0}, {0, 1, 0}}, {{0, 1, 2}}};
    const MeshSpec second{{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}, {{2, 1, 0}}};
    const MeshSpec quad{{{-2, -2, 0}, {2, -2, 0}, {2, 2, 0}, {-2, 2, 0}},
        {{0, 1, 2}, {0, 2, 3}}};
    const MeshSpec nearCross{{{-0.2f, -0.2f, -0.1f},
        {0.2f, -0.2f, 0.1f}, {0.0f, 0.2f, 0.1f}}, {{0, 1, 2}}};
    const MeshSpec reversed{{{-1, -1, 0}, {1, -1, 0}, {0, 1, 0}},
        {{0, 2, 1}}};
    Write(root / "triangle.object", BuildObject({triangle}));
    Write(root / "multiple.object", BuildObject({triangle, second}));
    Write(root / "quad.object", BuildObject({quad}));
    Write(root / "near.object", BuildObject({nearCross}));
    Write(root / "reversed.object", BuildObject({reversed}));
    Write(root / "skeletal.object", BuildObject({triangle}, true));

    EditorStaticMeshDecoder decoder;
    EditorStaticAssetGeometry geometry;
    std::string reason;
    auto triangleAsset = Asset("triangle", "triangle.object", {triangle});
    failures += Check(decoder.Decode(root, triangleAsset, geometry, &reason) ==
        EditorStaticGeometryDecodeStatus::Decoded, "one triangle decodes");
    failures += Check(geometry.meshes.size() == 1 &&
        geometry.meshes[0].buffer.positions[1].x == 1.0f &&
        geometry.meshes[0].buffer.triangles[0].a == 0 &&
        geometry.meshes[0].buffer.triangles[0].b == 1 &&
        geometry.meshes[0].buffer.triangles[0].c == 2,
        "positions, direct indices, and winding are exact");
    failures += Check(geometry.normalsGeneratedMeshes == 1 &&
        geometry.meshes[0].buffer.normalStatus ==
            EditorGeometryNormalStatus::NormalsGenerated &&
        geometry.meshes[0].buffer.normals.size() == 3 &&
        geometry.meshes[0].buffer.normals[0].z > 0.99f,
        "single triangle receives deterministic generated normals");
    EditorGeometryBuffer flatQuad;
    flatQuad.positions = {{0,0,0},{1,0,0},{1,1,0},{0,1,0}};
    flatQuad.triangles = {{0,1,2},{0,2,3}};
    failures += Check(GenerateEditorGeometryNormals(flatQuad) &&
        flatQuad.normals.size() == 4 && flatQuad.normals[2].z > 0.99f,
        "flat quad shares a stable area-weighted normal");
    EditorGeometryBuffer degenerate;
    degenerate.positions = {{0,0,0},{1,0,0},{2,0,0}};
    degenerate.triangles = {{0,1,2}};
    std::size_t ignored = 0;
    failures += Check(!GenerateEditorGeometryNormals(degenerate, &ignored) &&
        ignored == 1 && degenerate.normalStatus ==
            EditorGeometryNormalStatus::NormalGenerationFailed,
        "zero-area-only geometry fails normals safely");
    EditorGeometryBuffer invalidNormals;
    invalidNormals.positions = {{0,0,0}};
    invalidNormals.triangles = {{0,1,2}};
    failures += Check(!GenerateEditorGeometryNormals(invalidNormals),
        "normal generation rejects invalid indices");
    auto multipleAsset = Asset("multiple", "multiple.object", {triangle, second});
    failures += Check(decoder.Decode(root, multipleAsset, geometry, &reason) ==
        EditorStaticGeometryDecodeStatus::Decoded && geometry.meshes.size() == 2 &&
        geometry.totalTriangles == 2, "multiple meshes decode deterministically");

    MeshSpec invalid = triangle; invalid.triangles[0].c = 7;
    Write(root / "invalid.object", BuildObject({invalid}));
    auto invalidAsset = Asset("invalid", "invalid.object", {invalid});
    geometry.assetId = "atomic-sentinel";
    failures += Check(decoder.Decode(root, invalidAsset, geometry, &reason) ==
        EditorStaticGeometryDecodeStatus::Malformed &&
        geometry.assetId == "atomic-sentinel", "out-of-range index fails atomically");
    Bytes truncated = BuildObject({triangle}); truncated.resize(truncated.size() - 5);
    Write(root / "truncated.object", truncated);
    auto truncatedAsset = Asset("truncated", "truncated.object", {triangle});
    failures += Check(decoder.Decode(root, truncatedAsset, geometry, &reason) ==
        EditorStaticGeometryDecodeStatus::Malformed, "truncated face or vertex data fails");
    EditorStaticMeshDecoderLimits tiny; tiny.maximumVerticesPerMesh = 2;
    failures += Check(EditorStaticMeshDecoder(tiny).Decode(root, triangleAsset,
        geometry, &reason) == EditorStaticGeometryDecodeStatus::Malformed,
        "excessive vertex count is bounded");
    MeshSpec nonFinite = triangle;
    nonFinite.positions[0].x = std::numeric_limits<float>::quiet_NaN();
    Write(root / "nonfinite.object", BuildObject({nonFinite}));
    auto nonFiniteAsset = Asset("nonfinite", "nonfinite.object", {nonFinite});
    failures += Check(decoder.Decode(root, nonFiniteAsset, geometry, &reason) ==
        EditorStaticGeometryDecodeStatus::Malformed, "non-finite position fails");
    MeshSpec empty;
    Write(root / "empty.object", BuildObject({empty}));
    auto emptyAsset = Asset("empty", "empty.object", {empty});
    failures += Check(decoder.Decode(root, emptyAsset, geometry, &reason) ==
        EditorStaticGeometryDecodeStatus::Malformed, "empty mesh fails");

    EditorObjectLibrary library;
    EditorObjectLibraryLoadStatistics load;
    failures += Check(EditorObjectLibraryLoader().Load(root, library, load, &reason),
        "synthetic Object Library loads");
    EditorRenderAssetRegistry registry;
    failures += Check(registry.Build(library, &reason), "synthetic registry builds");
    EditorRenderGeometryCache cache;
    cache.Bind(library.Root(), &registry);
    auto* registeredTriangle = registry.Find("triangle");
    failures += Check(registeredTriangle != nullptr, "triangle registry entry exists");
    const auto* first = cache.Request(*registeredTriangle, &reason);
    const auto* secondRequest = cache.Request(*registeredTriangle, &reason);
    failures += Check(first && first == secondRequest &&
        cache.Statistics().decoded == 1 && cache.Statistics().hits == 1 &&
        registeredTriangle->readiness == EditorRenderAssetReadiness::StaticGeometryDecoded,
        "cache decodes once and transitions readiness");

    EditorTreeModel model;
    auto& modelRoot = model.CreateRoot("Scene");
    model.AddChild(modelRoot, "node", "object", EditorItemKind::Object);
    std::string before, after;
    SerializeEditorTreeSnapshot(model, before, &reason);
    cache.Request(*registeredTriangle, &reason);
    SerializeEditorTreeSnapshot(model, after, &reason);
    failures += Check(before == after, "geometry cache does not mutate document");

    EditorRenderScene scene;
    EditorRenderInstance instance = Instance(*registeredTriangle);
    instance.selected = true;
    scene.Add(instance);
    EditorRenderInstance duplicateInstance = instance;
    duplicateInstance.selected = false;
    duplicateInstance.transform.x = 2.0f;
    scene.Add(duplicateInstance);
    EditorRenderInstance missingWorkingSetInstance;
    missingWorkingSetInstance.assetId = "not-in-library";
    scene.Add(missingWorkingSetInstance);
    EditorRenderAssetWorkingSet workingSet;
    workingSet.Rebuild(scene, registry);
    failures += Check(workingSet.Statistics().uniqueAssets == 2 &&
        workingSet.Statistics().referencedInstances == 3 &&
        workingSet.Statistics().staticAssets == 1 &&
        workingSet.Statistics().missingAssets == 1 &&
        workingSet.Entries()[0].assetId == "triangle" &&
        workingSet.Entries()[0].instanceCount == 2 &&
        workingSet.Entries()[0].selected,
        "active-scene working set deduplicates assets and prioritizes selection");
    scene = {};
    scene.Add(instance);
    EditorSoftwareWireframeRenderer renderer;
    auto frame = renderer.Render(scene, registry, cache, Camera(), false);
    failures += Check(frame.validCamera && frame.statistics.trianglesSubmitted == 1 &&
        frame.lines.size() == 3 && frame.lines[0].selected,
        "triangle emits three selected semantic segments");
    const EditorWireframeCameraBasis zeroYawBasis =
        ComputeEditorWireframeCameraBasis(Camera());
    failures += Check(zeroYawBasis.right.x == 1.0f &&
        zeroYawBasis.up.y == 1.0f && zeroYawBasis.forward.z == 1.0f,
        "X-Ray editor mapping is Y-up with yaw zero looking along positive Z");

    const EditorWireframeWorldBounds framedBounds =
        ComputeEditorWireframeWorldBounds(instance);
    EditorViewportController framedController;
    framedController.OnResize(800, 600);
    framedController.FrameCameraOn(framedBounds.center.x,
        framedBounds.center.y, framedBounds.center.z, framedBounds.radius);
    const EditorWireframeCamera framedCamera =
        MakeEditorWireframeCamera(framedController.State());
    const auto framed = renderer.Render(scene, registry, cache,
        framedCamera, false);
    failures += Check(framed.selectedDiagnostic.present &&
        framed.selectedDiagnostic.cameraSpaceCenter.z > framedCamera.nearPlane &&
        framed.selectedDiagnostic.cullReason == EditorWireframeCullReason::None &&
        framed.statistics.visibleInstances == 1 &&
        framed.statistics.decodedAssets == 1 && !framed.lines.empty(),
        "framed selected target is in front, survives culling, and renders");

    EditorRenderScene behindScene;
    EditorRenderInstance behind = instance;
    behind.transform.z = -7.0f;
    behindScene.Add(behind);
    const auto behindFrame = renderer.Render(behindScene, registry, cache,
        Camera(), false);
    failures += Check(behindFrame.statistics.culledInstances == 1 &&
        behindFrame.selectedDiagnostic.cullReason ==
            EditorWireframeCullReason::BehindNearPlane,
        "object wholly behind camera is coarsely culled with a reason");
    const float originalX = frame.lines[0].x1;
    EditorRenderScene translatedScene;
    instance.transform.x = 1.0f;
    translatedScene.Add(instance);
    const auto translated = renderer.Render(translatedScene, registry, cache,
        Camera(), false);
    failures += Check(!translated.lines.empty() &&
        translated.lines[0].x1 > originalX, "translation affects projection");
    EditorRenderScene transformedScene;
    instance.transform.x = 0.0f;
    instance.transform.yaw = 0.5f;
    instance.transform.sx = 2.0f;
    transformedScene.Add(instance);
    const auto transformed = renderer.Render(transformedScene, registry, cache,
        Camera(), false);
    failures += Check(!transformed.lines.empty() &&
        transformed.lines[0].x1 != originalX &&
        transformed.statistics.visibleInstances == 1,
        "rotated and scaled bounds remain visible and affect projection");

    auto* quadAsset = registry.Find("quad");
    EditorRenderScene quadScene;
    quadScene.Add(Instance(*quadAsset));
    EditorWireframeBudget oneTriangle; oneTriangle.maximumTriangles = 1;
    oneTriangle.maximumLines = 3;
    const auto budgeted = renderer.Render(quadScene, registry, cache,
        Camera(), false, oneTriangle);
    failures += Check(budgeted.budgetExceeded && budgeted.lines.size() == 3,
        "triangle and line budgets truncate deterministically");
    const auto repeatedEdges = renderer.Render(quadScene, registry, cache,
        Camera(), false);
    failures += Check(repeatedEdges.lines.size() == 6,
        "wireframe emits three segments per triangle and retains shared edges");
    auto* reversedAsset = registry.Find("reversed");
    EditorRenderScene reversedScene;
    reversedScene.Add(Instance(*reversedAsset));
    const auto unculled = renderer.Render(reversedScene, registry, cache,
        Camera(), false);
    const auto culled = renderer.Render(reversedScene, registry, cache,
        Camera(), true);
    failures += Check(!unculled.lines.empty() && culled.lines.empty(),
        "backface culling follows preserved winding");

    auto* nearAsset = registry.Find("near");
    EditorRenderScene nearScene;
    EditorRenderInstance nearInstance = Instance(*nearAsset);
    nearInstance.transform.z = -4.98f;
    nearScene.Add(nearInstance);
    const auto nearFrame = renderer.Render(nearScene, registry, cache,
        Camera(), false);
    failures += Check(!nearFrame.lines.empty(),
        "segments crossing the near plane are clipped and retained");

    EditorWireframeCamera zeroCamera = Camera(); zeroCamera.viewportWidth = 0;
    failures += Check(!renderer.Render(scene, registry, cache, zeroCamera, false)
        .validCamera, "zero viewport is safe");
    EditorWireframeCamera badCamera = Camera();
    badCamera.x = std::numeric_limits<float>::infinity();
    failures += Check(!renderer.Render(scene, registry, cache, badCamera, false)
        .validCamera, "non-finite camera is rejected");
    EditorWireframeCamera clippedCamera = Camera(); clippedCamera.viewportWidth = 80;
    clippedCamera.viewportHeight = 60;
    const auto clipped = renderer.Render(quadScene, registry, cache,
        clippedCamera, false);
    bool inside = !clipped.lines.empty();
    for (const auto& line : clipped.lines)
        inside &= line.x1 >= 0 && line.x1 <= 79 && line.x2 >= 0 && line.x2 <= 79 &&
            line.y1 >= 0 && line.y1 <= 59 && line.y2 >= 0 && line.y2 <= 59;
    failures += Check(inside, "projected lines are clipped to viewport");

    EditorRenderScene missingScene;
    EditorRenderInstance missing; missing.assetId = "missing";
    missing.objectBounds = registeredTriangle->bounds;
    missingScene.Add(missing);
    failures += Check(renderer.Render(missingScene, registry, cache, Camera(), false)
        .statistics.fallbackBounds == 1, "missing asset keeps bounds fallback");
    EditorRenderScene partialScene;
    partialScene.Add(Instance(*registeredTriangle));
    partialScene.Add(missing);
    const auto partialFrame = renderer.Render(partialScene, registry, cache,
        Camera(), false);
    failures += Check(!partialFrame.lines.empty() &&
        partialFrame.statistics.decodedAssets == 1 &&
        partialFrame.statistics.fallbackBounds == 1,
        "partial Object Library renders matching references and falls back for missing ones");
    auto* skeletalAsset = registry.Find("skeletal");
    EditorRenderScene skeletalScene; skeletalScene.Add(Instance(*skeletalAsset));
    failures += Check(renderer.Render(skeletalScene, registry, cache, Camera(), false)
        .statistics.fallbackBounds == 1, "skeletal asset stays deferred fallback");

    cache.Clear();
    failures += Check(registeredTriangle->readiness ==
        EditorRenderAssetReadiness::StaticGeometryDecodeCandidate,
        "cache clear restores candidate readiness");
    const auto cleared = renderer.Render(scene, registry, cache, Camera(), false);
    failures += Check(cleared.statistics.fallbackBounds == 1,
        "cache clear safely restores fallback");
    cache.Bind(library.Root(), &registry);
    const auto reloaded = renderer.Render(scene, registry, cache, Camera(), false);
    failures += Check(reloaded.lines.size() == 3,
        "cache rebind restores wireframe");

    EditorRenderGeometryCacheLimits oneByte;
    oneByte.maximumTotalBytes = 1;
    EditorRenderGeometryCache bounded(oneByte);
    bounded.Bind(library.Root(), &registry);
    registry.UpdateReadiness("triangle",
        EditorRenderAssetReadiness::StaticGeometryDecodeCandidate);
    failures += Check(!bounded.Request(*registeredTriangle, &reason) &&
        bounded.Statistics().unsupported == 1,
        "cache memory accounting refuses over-budget geometry");
    bounded.Clear();

    std::error_code error;
    std::filesystem::remove_all(root, error);
    return failures;
}
