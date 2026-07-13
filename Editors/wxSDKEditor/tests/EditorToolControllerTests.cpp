#include "editor_app/EditorToolController.h"
#include "editor_assets/EditorAssetSelectionModel.h"
#include "editor_view/EditorPreviewPicking.h"

#include <cmath>
#include <iostream>

int RunEditorToolControllerTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (!condition)
        {
            ++failures;
            std::cerr << "FAIL: tools " << message << '\n';
        }
    };

    EditorToolController tools;
    check(tools.GetMode() == EditorToolMode::Select &&
        !tools.IsPlacementMode(), "default mode is Select");
    check(tools.SetMode(EditorToolMode::Move) &&
        tools.GetMode() == EditorToolMode::Move,
        "mode switches without external state");
    tools.SetMode(EditorToolMode::PlaceObject);
    check(tools.IsPlacementMode(), "object mode is placement");
    tools.SetMode(EditorToolMode::PlaceLight);
    check(tools.IsPlacementMode(), "light mode is placement");
    check(tools.CancelCurrentOperation() &&
        tools.GetMode() == EditorToolMode::Select,
        "cancel returns to Select");
    check(!tools.CancelCurrentOperation(), "cancel in Select is inert");

    const EditorAssetCatalog catalog = EditorAssetCatalog::CreateBuiltIn();
    check(catalog.Entries().size() == 6 &&
        catalog.FindById("DEMO.ACTOR") &&
        !catalog.FindById("missing.asset"),
        "built-in catalog has deterministic unique entries and lookup");
    check(catalog.CategoryPaths().size() == 5 &&
        catalog.FilterByCategory("objects").size() == 2,
        "catalog enumerates and filters categories");
    check(catalog.Search("ACTOR").size() == 1 &&
        catalog.Search("demo.").size() == 6,
        "catalog search is case-insensitive across descriptor fields");
    EditorAssetSelectionModel assetSelection;
    check(assetSelection.Select(catalog, "demo.point_light") &&
        assetSelection.HasPlaceableSelection(catalog) &&
        assetSelection.Resolve(catalog)->defaultTransform.y == 1.0f,
        "asset selection resolves placeable descriptor defaults");
    check(!assetSelection.Select(catalog, "missing.asset") &&
        assetSelection.SelectedId() == "demo.point_light",
        "unknown asset selection is rejected without losing valid selection");
    assetSelection.Clear();
    check(!assetSelection.HasPlaceableSelection(catalog),
        "asset selection clears explicitly");

    EditorPreviewProjectionContext projection;
    projection.width = 800;
    projection.height = 600;
    projection.cameraX = 3.0f;
    projection.cameraZ = -2.0f;
    projection.scale = 40.0f;
    const EditorPreviewWorldPoint center =
        UnprojectEditorPreviewToGround(400.0f, 300.0f, projection);
    check(center.valid && std::fabs(center.x - 3.0f) < 0.001f &&
        std::fabs(center.z + 2.0f) < 0.001f,
        "viewport center maps to camera X/Z");
    const EditorPreviewWorldPoint offset =
        UnprojectEditorPreviewToGround(480.0f, 340.0f, projection, 1.0f);
    check(offset.valid && std::fabs(offset.x - 5.0f) < 0.001f &&
        std::fabs(offset.y - 1.0f) < 0.001f &&
        std::fabs(offset.z + 1.0f) < 0.001f,
        "screen offsets and placement height map consistently");
    const EditorPreviewWorldPoint snapped =
        UnprojectEditorPreviewToGround(451.0f, 329.0f, projection,
            0.0f, true, 1.0f);
    check(snapped.valid && snapped.x == 4.0f && snapped.z == -1.0f,
        "placement snap rounds X/Z to fixed grid");
    projection.width = 0;
    check(!UnprojectEditorPreviewToGround(
        0.0f, 0.0f, projection).valid,
        "zero-size viewport rejects placement mapping");
    return failures;
}
