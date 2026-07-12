#include "editor_model/EditorTreeModel.h"
#include "editor_view/EditorPreviewRenderer.h"
#include "editor_view/EditorPreviewPicking.h"
#include "editor_view/EditorPreviewScene.h"
#include "editor_view/EditorTreePreviewAdapter.h"
#include "editor_view/EditorViewportState.h"

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

    EditorPreviewScene adapted = BuildEditorPreviewScene(model, light.Path());
    check(adapted.GetObjects().size() == 4 &&
        adapted.FindByLogicalPath(folder.Path()) == nullptr,
        "adapter excludes structural folders");
    check(adapted.FindByLogicalPath(object.Path())->kind == EditorPreviewKind::Box &&
        adapted.FindByLogicalPath(light.Path())->kind == EditorPreviewKind::Light &&
        adapted.FindByLogicalPath(spawn.Path())->kind == EditorPreviewKind::Spawn &&
        adapted.FindByLogicalPath(marker.Path())->kind == EditorPreviewKind::Marker,
        "adapter maps preview kinds");
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
    viewport.camera.z = -5.0f;
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
    check(projected.visible &&
        projected.x == 320.0f + projectedLight->x * 40.0f &&
        projected.y == 240.0f + (projectedLight->z + 5.0f) * 40.0f,
        "world-to-screen projection is deterministic");
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
    return failures;
}
