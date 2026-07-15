#include "editor_model/EditorTreeModel.h"
#include "editor_view/EditorPreviewRenderer.h"
#include "editor_view/EditorPreviewPicking.h"
#include "editor_view/EditorPreviewScene.h"
#include "editor_view/EditorTreePreviewAdapter.h"
#include "editor_view/EditorViewportState.h"
#include "editor_render/EditorRenderOverlay.h"
#include "editor_render/EditorRenderScene.h"

#include <algorithm>
#include <iostream>

namespace
{
std::size_t CountStyle(const EditorViewportDrawList& drawList,
    EditorViewportStyle style)
{
    return static_cast<std::size_t>(std::count_if(
        drawList.Primitives().begin(), drawList.Primitives().end(),
        [style](const EditorViewportPrimitive& primitive) {
            return primitive.style == style;
        }));
}
}

int RunEditorPreviewSceneTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition) return;
        ++failures;
        std::cerr << "FAIL: preview " << message << '\n';
    };

    EditorPreviewScene scene = EditorPreviewScene::BuildDemoScene();
    check(scene.GetObjects().size() == 2 &&
        scene.FindByLogicalPath("Demo/box") != nullptr,
        "demo scene and logical lookup");
    scene.SetSelectedPath("Demo/light");
    check(scene.FindByLogicalPath("Demo/light")->selected &&
        !scene.FindByLogicalPath("Demo/box")->selected,
        "selected path marks exactly one object");
    scene.FindByLogicalPath("Demo/box")->visible = false;
    check(!scene.FindByLogicalPath("Demo/box")->visible,
        "visibility remains preview-local state");
    check(scene.RemoveObject("Demo/light") && scene.SelectedPath().empty() &&
        !scene.RemoveObject("Demo/missing"),
        "remove clears selection and missing removal is safe");
    scene.Clear();
    check(scene.GetObjects().empty(), "clear removes preview objects");

    EditorTreeModel model;
    EditorTreeNode& root = model.CreateRoot("Root", "root", EditorItemKind::Root);
    EditorTreeNode& folder = model.AddChild(
        root, "Folder", "folder", EditorItemKind::Folder);
    EditorTreeNode& object = model.AddChild(
        folder, "object", "demo object", EditorItemKind::Object);
    EditorTreeNode& light = model.AddChild(
        folder, "light", "demo light", EditorItemKind::Object);
    EditorTreeNode& spawn = model.AddChild(
        folder, "spawn", "spawn element", EditorItemKind::Object);
    EditorTreeNode& marker = model.AddChild(
        folder, "marker", "custom", EditorItemKind::Unknown);
    EditorTreeNode& assetMapped = model.AddChild(
        folder, "asset-mapped", "custom", EditorItemKind::Object);
    EditorTreeNode& unknownAsset = model.AddChild(
        folder, "unknown-asset", "demo spawn", EditorItemKind::Object);
    EditorTreeNode& importedAsset = model.AddChild(
        folder, "imported-asset", "custom", EditorItemKind::Object);
    EditorTransform transform; transform.x=-4; transform.z=-5; model.SetNodeTransform(object,transform);
    transform.x=-2; transform.y=2; model.SetNodeTransform(light,transform);
    transform.x=0; transform.y=0; model.SetNodeTransform(spawn,transform);
    transform.x=2; model.SetNodeTransform(marker,transform);
    model.SetNodeAssetId(assetMapped, "demo.point_light");
    model.SetNodeAssetId(unknownAsset, "future.unknown");
    model.SetNodeAssetId(importedAsset, "imported.section.wpn_ak74");

    EditorPreviewScene adapted = BuildEditorPreviewScene(model, light.Path());
    check(adapted.GetObjects().size() == 7 &&
        adapted.FindByLogicalPath(folder.Path()) == nullptr,
        "adapter excludes structural folders");
    check(adapted.FindByLogicalPath(object.Path())->kind == EditorPreviewKind::Box &&
        adapted.FindByLogicalPath(light.Path())->kind == EditorPreviewKind::Light &&
        adapted.FindByLogicalPath(spawn.Path())->kind == EditorPreviewKind::Spawn &&
        adapted.FindByLogicalPath(marker.Path())->kind == EditorPreviewKind::Marker,
        "adapter maps preview kinds");
    check(adapted.FindByLogicalPath(assetMapped.Path())->kind ==
            EditorPreviewKind::Light,
        "known asset id takes precedence over category fallback");
    check(adapted.FindByLogicalPath(unknownAsset.Path())->kind ==
            EditorPreviewKind::Spawn,
        "unknown asset id retains category fallback behavior");
    check(adapted.FindByLogicalPath(importedAsset.Path())->kind ==
            EditorPreviewKind::Spawn,
        "imported prototype id retains inert Spawn preview without catalog");
    check(adapted.FindByLogicalPath(light.Path())->selected &&
        adapted.FindByLogicalPath(light.Path())->y == 2.0f,
        "light selection and elevated synthetic placement");
    check(adapted.GetObjects()[0].x == -4.0f &&
        adapted.GetObjects()[1].x == -2.0f &&
        adapted.GetObjects()[0].z == -5.0f,
        "adapter placement follows deterministic traversal grid");

    std::string reason;
    check(model.RenameNode(object, "renamed", &reason), "rename fixture succeeds");
    adapted = BuildEditorPreviewScene(model, object.Path());
    check(adapted.FindByLogicalPath("Root/Folder/object") == nullptr &&
        adapted.FindByLogicalPath("Root/Folder/renamed") != nullptr,
        "adapter rebuild uses current renamed path");
    check(model.MoveNode(object, root, &reason), "move fixture succeeds");
    adapted = BuildEditorPreviewScene(model, object.Path());
    check(adapted.FindByLogicalPath("Root/renamed") != nullptr,
        "adapter rebuild uses current moved path");
    check(model.DeleteNode(object, &reason), "delete fixture succeeds");
    adapted = BuildEditorPreviewScene(model);
    check(adapted.FindByLogicalPath("Root/renamed") == nullptr,
        "adapter rebuild removes deleted object");

    EditorPreviewRenderer renderer;
    renderer.SetScene(&adapted);
    renderer.Resize(640, 480);
    EditorViewportState viewport;
    viewport.width = 640;
    viewport.height = 480;
    viewport.camera.z = -10.0f;
    adapted.SetSelectedPath(light.Path());
    renderer.Render(viewport);
    check(!renderer.DrawList().Primitives().empty() &&
        CountStyle(renderer.DrawList(), EditorViewportStyle::Selected) == 1 &&
        CountStyle(renderer.DrawList(), EditorViewportStyle::Label) ==
            adapted.GetObjects().size(),
        "draw list includes labels and selected highlight");
    renderer.SetLabelsVisible(false);
    renderer.Render(viewport);
    check(CountStyle(renderer.DrawList(), EditorViewportStyle::Label) == 0,
        "label toggle removes text primitives");
    EditorPreviewScene empty;
    renderer.SetScene(&empty);
    renderer.Render(viewport);
    check(renderer.DrawList().Primitives().empty(),
        "empty scene produces empty draw list");
    renderer.Resize(0, 0);
    renderer.SetScene(&adapted);
    renderer.Render(viewport);
    check(renderer.DrawList().Primitives().empty(),
        "zero viewport dimensions are safe");

    const EditorPreviewProjectionContext context =
        MakeEditorPreviewProjectionContext(viewport, 640, 480);
    const EditorPreviewProjectedPoint projected =
        ProjectEditorPreviewObject(*adapted.FindByLogicalPath(light.Path()), context);
    const EditorPreviewObject* projectedLight =
        adapted.FindByLogicalPath(light.Path());
    check(projectedLight && projected.visible,
        "world-to-screen projection uses the perspective camera");
    EditorViewportState movedCamera = viewport;
    movedCamera.camera.x = 1.0f;
    const EditorPreviewProjectedPoint cameraProjected =
        ProjectEditorPreviewObject(*adapted.FindByLogicalPath(light.Path()),
            MakeEditorPreviewProjectionContext(movedCamera, 800, 480));
    check(cameraProjected.x != projected.x,
        "resize and camera offset affect projection");
    check(!ProjectEditorPreviewObject(*adapted.FindByLogicalPath(light.Path()),
        MakeEditorPreviewProjectionContext(viewport, 0, 0)).visible,
        "zero-size projection is safe");

    std::vector<EditorPreviewPickShape> shapes =
        BuildEditorPreviewPickShapes(adapted, context);
    EditorPreviewPickResult pick = PickEditorPreview(shapes,
        projected.x, projected.y);
    check(pick.hit && pick.logicalPath == light.Path() &&
        pick.kind == EditorPreviewKind::Light,
        "light-circle hit returns logical identity");
    check(!PickEditorPreview(shapes, 5.0f, 5.0f).hit,
        "empty-space miss is safe");
    EditorPreviewScene glowScene;
    glowScene.AddObject({"glow", "glow", 0.0f, 0.0f, 0.0f,
        2.5f, 2.5f, 2.5f, EditorPreviewKind::Glow});
    renderer.Resize(640, 480);
    renderer.SetScene(&glowScene);
    renderer.Render(viewport);
    const auto& glowPrimitives = renderer.DrawList().Primitives();
    check(!glowPrimitives.empty() &&
        glowPrimitives[0].type == EditorViewportPrimitiveType::Circle &&
        glowPrimitives[0].radius == 9.0f,
        "glow produces bounded diagnostic circle");
    const std::vector<EditorPreviewPickShape> glowShapes =
        BuildEditorPreviewPickShapes(glowScene, context);
    check(glowShapes.size() == 1 && glowShapes[0].circular &&
        glowShapes[0].radius == 9.0f &&
        PickEditorPreview(glowShapes,glowShapes[0].centerX,
            glowShapes[0].centerY).hit,
        "glow diagnostic circle uses matching pick radius");
    EditorPreviewScene historicalLightScene;
    historicalLightScene.AddObject({"historical-light", "historical light",
        0.0f, 0.0f, 0.0f, 15.0f, 15.0f, 15.0f,
        EditorPreviewKind::HistoricalLight});
    renderer.SetScene(&historicalLightScene);
    renderer.Render(viewport);
    const auto& historicalLightPrimitives = renderer.DrawList().Primitives();
    check(!historicalLightPrimitives.empty() &&
        historicalLightPrimitives[0].type ==
            EditorViewportPrimitiveType::Circle &&
        historicalLightPrimitives[0].style == EditorViewportStyle::Light &&
        historicalLightPrimitives[0].radius == 9.0f,
        "historical light range produces a bounded semantic diagnostic ring");
    const std::vector<EditorPreviewPickShape> historicalLightShapes =
        BuildEditorPreviewPickShapes(historicalLightScene, context);
    check(historicalLightShapes.size() == 1 &&
        historicalLightShapes[0].circular &&
        historicalLightShapes[0].radius == 9.0f &&
        PickEditorPreview(historicalLightShapes,historicalLightShapes[0].centerX,
            historicalLightShapes[0].centerY).hit,
        "historical light range ring uses matching bounded pick radius");
    EditorPreviewPickShape markerShape;
    markerShape.logicalPath = "marker";
    markerShape.kind = EditorPreviewKind::Marker;
    markerShape.centerX = 100.0f;
    markerShape.centerY = 100.0f;
    markerShape.halfWidth = markerShape.halfHeight = 2.0f;
    check(PickEditorPreview({markerShape}, 105.0f, 100.0f).hit,
        "marker pick tolerance is applied");
    EditorPreviewPickShape boxShape = markerShape;
    boxShape.logicalPath = "box";
    boxShape.kind = EditorPreviewKind::Box;
    boxShape.halfWidth = 14.0f;
    boxShape.halfHeight = 10.0f;
    check(PickEditorPreview({boxShape}, 113.0f, 109.0f).hit &&
        !PickEditorPreview({boxShape}, 115.0f, 100.0f).hit,
        "box rectangle hit and miss use exact bounds");
    EditorPreviewPickShape spawnShape = markerShape;
    spawnShape.logicalPath = "spawn";
    spawnShape.kind = EditorPreviewKind::Spawn;
    check(PickEditorPreview({spawnShape}, 103.0f, 103.0f).logicalPath == "spawn",
        "spawn marker uses tolerant rectangle picking");
    EditorPreviewPickShape first = markerShape;
    first.logicalPath = "first";
    EditorPreviewPickShape last = markerShape;
    last.logicalPath = "last";
    check(PickEditorPreview({first, last}, 100.0f, 100.0f).logicalPath == "last",
        "overlap resolves to last drawn object");
    markerShape.selectable = false;
    check(!PickEditorPreview({markerShape}, 100.0f, 100.0f).hit,
        "non-selectable shape is ignored");

    EditorRenderFrameContext frame;
    frame.viewportWidth=800; frame.viewportHeight=600;
    frame.cameraPosition={0.0f,1.0f,-5.0f};
    const EditorProjectedPoint ahead=ProjectEditorWorldPoint({0.0f,1.0f,0.0f},frame);
    check(ahead.finite&&ahead.inFront&&ahead.insideDepth&&ahead.insideViewport&&
        std::fabs(ahead.screenX-400.0f)<0.01f&&
        std::fabs(ahead.screenY-300.0f)<0.01f,
        "canonical projection centers a point directly ahead");
    EditorRenderFrameContext raised=frame;
    raised.cameraPosition.y=2.0f;
    check(ProjectEditorWorldPoint({0.0f,1.0f,0.0f},raised).screenY>
        ahead.screenY,"camera height moves a world marker vertically");
    EditorRenderFrameContext pitched=frame;
    pitched.pitchDegrees=20.0f;
    check(ProjectEditorWorldPoint({0.0f,1.0f,0.0f},pitched).screenY!=
        ahead.screenY,"camera pitch moves a world marker vertically");
    check(!ProjectEditorWorldPoint({0.0f,1.0f,-10.0f},frame).inFront,
        "canonical projection rejects behind-camera points");
    EditorRenderFrameContext emptyFrame=frame;
    emptyFrame.viewportWidth=0;
    check(!ProjectEditorWorldPoint({0.0f,1.0f,0.0f},emptyFrame).finite,
        "canonical projection rejects zero-size viewports");

    EditorPreviewScene overlayScene;
    overlayScene.AddObject({"selected","selected",0,1,0,
        1,1,1,EditorPreviewKind::Spawn});
    overlayScene.SetSelectedPath("selected");
    EditorRenderOverlayOptions overlayOptions;
    overlayOptions.grid=false;
    EditorRenderScene emptyRenderScene;
    auto overlay=BuildEditorRenderOverlay(overlayScene,emptyRenderScene,frame,
        overlayOptions);
    check(!overlay.lines.empty()&&overlay.labels.size()==1&&
        !overlay.lines.front().depthTest,
        "selected marker receives priority label and no-depth highlight");
    overlayOptions.labels=EditorPreviewLabelPolicy::Off;
    check(BuildEditorRenderOverlay(overlayScene,emptyRenderScene,frame,
        overlayOptions).labels.empty(),"label Off policy submits no labels");
    overlayOptions.labels=EditorPreviewLabelPolicy::All;
    overlayOptions.maximumLines=1;
    overlay=BuildEditorRenderOverlay(overlayScene,emptyRenderScene,frame,
        overlayOptions);
    check(overlay.lines.size()==1&&overlay.skippedLines>0,
        "overlay line capacity is bounded deterministically");
    return failures;
}
