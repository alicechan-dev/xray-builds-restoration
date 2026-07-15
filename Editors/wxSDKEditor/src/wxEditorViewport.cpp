#include "wxEditorViewport.h"

#include "editor_view/IEditorViewportRenderer.h"
#include "editor_view/EditorTreePreviewAdapter.h"
#include "editor_render/EditorRenderAssetRegistry.h"
#include "editor_render/EditorRenderGeometryCache.h"

#include <algorithm>
#include <utility>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>

namespace
{
bool MapKey(int keyCode, EditorViewportKey& key)
{
    switch (keyCode)
    {
    case 'W': key = EditorViewportKey::Forward; return true;
    case 'S': key = EditorViewportKey::Backward; return true;
    case 'A': key = EditorViewportKey::Left; return true;
    case 'D': key = EditorViewportKey::Right; return true;
    case 'E': key = EditorViewportKey::Up; return true;
    case 'Q': key = EditorViewportKey::Down; return true;
    default: return false;
    }
}

EditorViewportMouseButton MapButton(int button)
{
    if (button == wxMOUSE_BTN_RIGHT)
        return EditorViewportMouseButton::Right;
    if (button == wxMOUSE_BTN_MIDDLE)
        return EditorViewportMouseButton::Middle;
    return EditorViewportMouseButton::Left;
}
}

wxEditorViewport::wxEditorViewport(wxWindow* parent) :
    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxWANTS_CHARS | wxBORDER_NONE),
    controller_(&renderer_), timer_(this)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    controller_.SetPickHandler([this](int x, int y) {
        return renderer_.Pick(static_cast<float>(x), static_cast<float>(y))
            .logicalPath;
    });
    Bind(wxEVT_PAINT, &wxEditorViewport::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &wxEditorViewport::OnEraseBackground, this);
    Bind(wxEVT_DESTROY, &wxEditorViewport::OnDestroy, this);
    Bind(wxEVT_SIZE, &wxEditorViewport::OnSize, this);
    Bind(wxEVT_SET_FOCUS, &wxEditorViewport::OnFocus, this);
    Bind(wxEVT_KILL_FOCUS, &wxEditorViewport::OnFocus, this);
    Bind(wxEVT_ENTER_WINDOW, &wxEditorViewport::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &wxEditorViewport::OnMouseLeave, this);
    Bind(wxEVT_MOTION, &wxEditorViewport::OnMouseMove, this);
    Bind(wxEVT_LEFT_DOWN, &wxEditorViewport::OnMouseButton, this);
    Bind(wxEVT_LEFT_UP, &wxEditorViewport::OnMouseButton, this);
    Bind(wxEVT_RIGHT_DOWN, &wxEditorViewport::OnMouseButton, this);
    Bind(wxEVT_RIGHT_UP, &wxEditorViewport::OnMouseButton, this);
    Bind(wxEVT_MIDDLE_DOWN, &wxEditorViewport::OnMouseButton, this);
    Bind(wxEVT_MIDDLE_UP, &wxEditorViewport::OnMouseButton, this);
    Bind(wxEVT_MOUSEWHEEL, &wxEditorViewport::OnMouseWheel, this);
    Bind(wxEVT_KEY_DOWN, &wxEditorViewport::OnKeyDown, this);
    Bind(wxEVT_KEY_UP, &wxEditorViewport::OnKeyUp, this);
    Bind(wxEVT_TIMER, &wxEditorViewport::OnTimer, this);
    timer_.Start(33);
}

wxEditorViewport::~wxEditorViewport()
{
    timer_.Stop();
    d3dRenderer_.Shutdown();
}

void wxEditorViewport::ToggleGrid()
{
    controller_.ToggleGrid();
    Refresh(false);
}

void wxEditorViewport::ResetCamera()
{
    controller_.ResetCamera();
    Refresh(false);
}

void wxEditorViewport::FocusViewport()
{
    SetFocus();
}

bool wxEditorViewport::IsGridVisible() const
{
    return controller_.State().gridVisible;
}

void wxEditorViewport::RebuildPreview(
    const EditorTreeModel& model, const std::string& selectedPath)
{
    previewScene_ = BuildEditorPreviewScene(model, selectedPath);
    renderer_.SetScene(&previewScene_);
    controller_.Render();
    Refresh(false);
}

void wxEditorViewport::SetPreviewScene(EditorPreviewScene scene)
{
    previewScene_ = std::move(scene);
    renderer_.SetScene(&previewScene_);
    controller_.Render();
    Refresh(false);
}

void wxEditorViewport::SetRenderScene(EditorRenderScene scene,
    EditorRenderAssetRegistry* assets,
    EditorRenderGeometryCache* geometryCache)
{
    renderScene_ = std::move(scene);
    const std::size_t generation = assets ? assets->Generation() : 0;
    if (generation != renderAssetGeneration_)
    {
        d3dRenderer_.ClearGeometryCache();
        renderAssetGeneration_ = generation;
    }
    renderAssets_ = assets;
    geometryCache_ = geometryCache;
    PrepareActiveSceneGeometry();
    d3dRenderer_.Bind(renderAssets_, geometryCache_);
    d3dRenderer_.SetScene(renderScene_);
    Refresh(false);
}

void wxEditorViewport::TogglePreviewLabels()
{
    renderer_.SetLabelsVisible(!renderer_.LabelsVisible());
    Refresh(false);
}

bool wxEditorViewport::ArePreviewLabelsVisible() const
{
    return renderer_.LabelsVisible();
}

void wxEditorViewport::ToggleObjectBounds()
{ renderer_.SetObjectBoundsVisible(!renderer_.ObjectBoundsVisible()); controller_.Render(); Refresh(false); }
bool wxEditorViewport::AreObjectBoundsVisible() const { return renderer_.ObjectBoundsVisible(); }
void wxEditorViewport::ToggleRenderAssetDiagnostics()
{ renderer_.SetAssetDiagnosticsVisible(!renderer_.AssetDiagnosticsVisible()); controller_.Render(); Refresh(false); }
bool wxEditorViewport::AreRenderAssetDiagnosticsVisible() const { return renderer_.AssetDiagnosticsVisible(); }
void wxEditorViewport::ToggleRealMeshWireframe()
{ wireframeVisible_ = !wireframeVisible_; Refresh(false); }
void wxEditorViewport::ToggleBackfaceCulling()
{ backfaceCulling_ = !backfaceCulling_; d3dOptions_.backfaceCulling = backfaceCulling_; Refresh(false); }

void wxEditorViewport::SetBackend(EditorViewportBackend backend)
{
    if (backend == EditorViewportBackend::SoftwareDiagnostic)
    {
        d3dRenderer_.Shutdown();
        d3dAttempted_ = false;
        d3dAvailable_ = false;
    }
    backend_ = backend;
    if (backend_ == EditorViewportBackend::Direct3D11 && !EnsureD3D11())
        backend_ = EditorViewportBackend::SoftwareDiagnostic;
    Refresh(false);
}

void wxEditorViewport::ToggleFilledMeshes()
{ d3dOptions_.filledMeshes = !d3dOptions_.filledMeshes; Refresh(false); }
void wxEditorViewport::ToggleWireframeOverlay()
{ d3dOptions_.wireframeOverlay = !d3dOptions_.wireframeOverlay; Refresh(false); }
void wxEditorViewport::ToggleIsolateSelected()
{ d3dOptions_.isolateSelected = !d3dOptions_.isolateSelected; Refresh(false); }

bool wxEditorViewport::EnsureD3D11()
{
    if (d3dRenderer_.IsInitialized()) return true;
    if (d3dAttempted_) return false;
    d3dAttempted_ = true;
    d3dAvailable_ = d3dRenderer_.Initialize(GetHandle(),
        controller_.State().width, controller_.State().height, &d3dFailure_);
    if (d3dAvailable_)
    {
        d3dRenderer_.Bind(renderAssets_, geometryCache_);
        d3dRenderer_.SetScene(renderScene_);
    }
    return d3dAvailable_;
}

void wxEditorViewport::PrepareActiveSceneGeometry()
{
    if (!renderAssets_ || !geometryCache_ || !geometryCache_->IsBound()) return;
    EditorRenderAssetWorkingSet set;
    set.Rebuild(renderScene_, *renderAssets_);
    for (const auto& entry : set.Entries())
    {
        if (!entry.staticGeometry) continue;
        if (const EditorRenderObjectAsset* asset = renderAssets_->Find(entry.assetId))
        {
            std::string ignored;
            geometryCache_->Request(*asset, &ignored);
        }
    }
}

bool wxEditorViewport::FrameSelected()
{
    const EditorPreviewObject* selected =
        previewScene_.FindByLogicalPath(previewScene_.SelectedPath());
    if (!selected)
        return false;
    const EditorRenderInstance* renderInstance = nullptr;
    for (const EditorRenderInstance& candidate : renderScene_.Instances())
        if (candidate.logicalPath == previewScene_.SelectedPath())
        {
            renderInstance = &candidate;
            break;
        }
    const EditorWireframeWorldBounds bounds = renderInstance
        ? ComputeEditorWireframeWorldBounds(*renderInstance)
        : EditorWireframeWorldBounds{};
    if (bounds.valid)
        controller_.FrameCameraOn(bounds.center.x, bounds.center.y,
            bounds.center.z, bounds.radius);
    else
        controller_.FrameCameraOn(selected->x, selected->y, selected->z,
            (std::max)({selected->sizeX, selected->sizeY, selected->sizeZ}) * 0.5f);
    Refresh(false);
    return true;
}

void wxEditorViewport::ToggleMoveSnap()
{
    moveSnapEnabled_ = !moveSnapEnabled_;
}

bool wxEditorViewport::CancelTransientOperation()
{
    if (!moveGizmo_.Active())
        return false;
    EditorPreviewObject* selected =
        previewScene_.FindByLogicalPath(previewScene_.SelectedPath());
    if (selected)
    {
        const EditorTransform& start = moveGizmo_.Start();
        selected->x = start.x;
        selected->y = start.y;
        selected->z = start.z;
    }
    moveGizmo_.Cancel();
    renderer_.SetActiveGizmoAxis(EditorGizmoAxis::None);
    controller_.Render();
    Refresh(false);
    return true;
}

void wxEditorViewport::SetToolMode(EditorToolMode mode)
{
    CancelTransientOperation();
    toolMode_ = mode;
    placementPreview_ = {};
    renderer_.SetPlacementPreview(placementPreview_);
    renderer_.SetGizmoVisible(mode == EditorToolMode::Move);
    SetCursor(mode == EditorToolMode::PlaceObject ||
        mode == EditorToolMode::PlaceLight || mode == EditorToolMode::PlaceAsset
        ? wxCursor(wxCURSOR_CROSS) : wxNullCursor);
    controller_.Render();
    Refresh(false);
}

void wxEditorViewport::SetPlacementDescriptor(
    const EditorAssetDescriptor* descriptor)
{
    placementAssetId_ = descriptor ? descriptor->id : std::string();
    placementAssetName_ = descriptor ? descriptor->displayName : std::string();
    placementDefaults_ = descriptor
        ? descriptor->defaultTransform : EditorTransform{};
    placementPreviewKind_ = descriptor
        ? descriptor->previewKind : EditorPreviewKind::Marker;
    renderer_.SetPlacementPreviewKind(placementPreviewKind_);
}

void wxEditorViewport::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);
    const EditorViewportState& state = controller_.State();
    bool d3dFrame = backend_ == EditorViewportBackend::Direct3D11 &&
        EnsureD3D11();
    if (d3dFrame)
    {
        d3dOptions_.backfaceCulling = backfaceCulling_;
        if (!d3dRenderer_.Render(state, d3dOptions_))
        {
            d3dRenderer_.Shutdown();
            d3dAvailable_ = false;
            backend_ = EditorViewportBackend::SoftwareDiagnostic;
            d3dFrame = false;
        }
    }
    if (!d3dFrame)
    {
        dc.SetBackground(wxBrush(wxColour(34, 38, 42)));
        dc.Clear();
    }

    if (state.gridVisible)
    {
        dc.SetPen(wxPen(wxColour(52, 58, 63)));
        constexpr int spacing = 32;
        for (int x = state.width / 2 % spacing; x < state.width; x += spacing)
            dc.DrawLine(x, 0, x, state.height);
        for (int y = state.height / 2 % spacing; y < state.height; y += spacing)
            dc.DrawLine(0, y, state.width, y);
        dc.SetPen(wxPen(wxColour(72, 80, 86)));
        dc.DrawLine(state.width / 2, 0, state.width / 2, state.height);
        dc.DrawLine(0, state.height / 2, state.width, state.height / 2);
    }

    controller_.Render();
    if (!d3dFrame && wireframeVisible_ && renderAssets_ && geometryCache_)
        wireframeFrame_ = wireframeRenderer_.Render(renderScene_,
            *renderAssets_, *geometryCache_,
            MakeEditorWireframeCamera(state), backfaceCulling_);
    else
        wireframeFrame_ = {};
    for (const EditorWireframeLine& line : wireframeFrame_.lines)
    {
        dc.SetPen(wxPen(line.selected ? wxColour(255, 145, 55)
            : wxColour(90, 205, 220), line.selected ? 2 : 1));
        dc.DrawLine(static_cast<int>(line.x1), static_cast<int>(line.y1),
            static_cast<int>(line.x2), static_cast<int>(line.y2));
    }
    for (const EditorViewportPrimitive& primitive :
        renderer_.DrawList().Primitives())
    {
        wxColour colour(160, 175, 185);
        if (primitive.style == EditorViewportStyle::Light)
            colour = wxColour(245, 210, 90);
        else if (primitive.style == EditorViewportStyle::Spawn)
            colour = wxColour(100, 210, 145);
        else if (primitive.style == EditorViewportStyle::Selected)
            colour = wxColour(255, 145, 55);
        else if (primitive.style == EditorViewportStyle::Label)
            colour = wxColour(215, 220, 225);
        else if (primitive.style == EditorViewportStyle::GizmoX)
            colour = wxColour(225, 70, 70);
        else if (primitive.style == EditorViewportStyle::GizmoZ)
            colour = wxColour(70, 145, 235);
        else if (primitive.style == EditorViewportStyle::GizmoActive)
            colour = wxColour(255, 225, 70);
        else if (primitive.style == EditorViewportStyle::Placement)
            colour = wxColour(90, 230, 180);
        dc.SetPen(wxPen(colour,
            primitive.style == EditorViewportStyle::Selected ? 2 : 1));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        switch (primitive.type)
        {
        case EditorViewportPrimitiveType::Line:
            dc.DrawLine(static_cast<int>(primitive.x1),
                static_cast<int>(primitive.y1), static_cast<int>(primitive.x2),
                static_cast<int>(primitive.y2));
            break;
        case EditorViewportPrimitiveType::Rectangle:
            dc.DrawRectangle(static_cast<int>(primitive.x1),
                static_cast<int>(primitive.y1),
                static_cast<int>(primitive.x2 - primitive.x1),
                static_cast<int>(primitive.y2 - primitive.y1));
            break;
        case EditorViewportPrimitiveType::Circle:
            dc.DrawCircle(static_cast<int>(primitive.x1),
                static_cast<int>(primitive.y1),
                static_cast<int>(primitive.radius));
            break;
        case EditorViewportPrimitiveType::Text:
            dc.SetTextForeground(colour);
            dc.DrawText(wxString::FromUTF8(primitive.text),
                static_cast<int>(primitive.x1), static_cast<int>(primitive.y1));
            break;
        }
    }
    dc.SetTextForeground(wxColour(205, 213, 220));
    dc.DrawText(d3dFrame ? "Direct3D 11 renderer" :
        (wireframeVisible_ ? "Software wireframe renderer" :
            "Real mesh wireframe disabled"), 12, 12);
    dc.DrawText(wxString::Format("Size: %d x %d", state.width, state.height),
        12, 34);
    dc.DrawText(wxString::Format("Mouse: %d, %d  Focus: %s",
        state.mouseX, state.mouseY, state.focused ? "yes" : "no"), 12, 54);
    dc.DrawText(wxString::Format(
        "Camera: (%.2f, %.2f, %.2f) yaw %.1f pitch %.1f speed %.1f",
        state.camera.x, state.camera.y, state.camera.z, state.camera.yaw,
        state.camera.pitch, state.camera.movementSpeed), 12, 74);
    dc.DrawText("Tool: " + wxString::FromUTF8(EditorToolModeName(toolMode_)),
        12, 94);
    if (d3dFrame)
    {
        const auto& value = d3dRenderer_.Diagnostics();
        dc.DrawText(wxString::Format(
            "D3D11: %s frame=%.2fms workset=%zu resident=%zu uploads=%zu draws=%zu instances=%zu triangles=%zu fallback=%zu",
            wxString::FromUTF8(value.status), value.cpuFrameMilliseconds,
            value.workingSetAssets, value.residentAssets,
            value.uploadsThisFrame, value.drawCalls, value.instancesDrawn,
            value.trianglesSubmitted, value.fallbackBounds), 12, 114);
    }
    else if (wireframeVisible_)
        dc.DrawText(wxString::Format(
            "Wireframe: visible=%zu decoded=%zu triangles=%zu lines=%zu culled=%zu fallback=%zu budget_skip=%zu failures=%zu",
            wireframeFrame_.statistics.visibleInstances,
            wireframeFrame_.statistics.decodedAssets,
            wireframeFrame_.statistics.trianglesSubmitted,
            wireframeFrame_.statistics.linesDrawn,
            wireframeFrame_.statistics.culledInstances,
            wireframeFrame_.statistics.fallbackBounds,
            wireframeFrame_.statistics.budgetSkippedObjects,
            wireframeFrame_.statistics.decodeFailures), 12, 114);
    if (wireframeVisible_ && wireframeFrame_.selectedDiagnostic.present)
    {
        const auto& diagnostic = wireframeFrame_.selectedDiagnostic;
        const auto& camera = MakeEditorWireframeCamera(state);
        dc.DrawText(wxString::Format(
            "Selected: %s asset=%s resolved=%d ready=%s cull=%s",
            wxString::FromUTF8(diagnostic.logicalPath),
            wxString::FromUTF8(diagnostic.assetId), diagnostic.assetResolved,
            ToString(diagnostic.readiness), ToString(diagnostic.cullReason)),
            12, 134);
        dc.DrawText(wxString::Format(
            "Object pos=(%.2f,%.2f,%.2f) local=[(%.2f,%.2f,%.2f)-(%.2f,%.2f,%.2f)]",
            diagnostic.transform.x, diagnostic.transform.y, diagnostic.transform.z,
            diagnostic.objectBounds.minX, diagnostic.objectBounds.minY,
            diagnostic.objectBounds.minZ, diagnostic.objectBounds.maxX,
            diagnostic.objectBounds.maxY, diagnostic.objectBounds.maxZ), 12, 154);
        dc.DrawText(wxString::Format(
            "World center=(%.2f,%.2f,%.2f) bounds=[(%.2f,%.2f,%.2f)-(%.2f,%.2f,%.2f)]",
            diagnostic.worldBounds.center.x, diagnostic.worldBounds.center.y,
            diagnostic.worldBounds.center.z, diagnostic.worldBounds.minimum.x,
            diagnostic.worldBounds.minimum.y, diagnostic.worldBounds.minimum.z,
            diagnostic.worldBounds.maximum.x, diagnostic.worldBounds.maximum.y,
            diagnostic.worldBounds.maximum.z), 12, 174);
        dc.DrawText(wxString::Format(
            "Camera=(%.2f,%.2f,%.2f) yaw=%.1f pitch=%.1f view-center=(%.2f,%.2f,%.2f) near/far=%.2f/%.0f",
            camera.x, camera.y, camera.z, camera.yawDegrees,
            camera.pitchDegrees, diagnostic.cameraSpaceCenter.x,
            diagnostic.cameraSpaceCenter.y, diagnostic.cameraSpaceCenter.z,
            camera.nearPlane, camera.farPlane), 12, 194);
        dc.DrawText(wxString::Format(
            "Basis R=(%.2f,%.2f,%.2f) U=(%.2f,%.2f,%.2f) F=(%.2f,%.2f,%.2f)",
            diagnostic.cameraBasis.right.x, diagnostic.cameraBasis.right.y,
            diagnostic.cameraBasis.right.z, diagnostic.cameraBasis.up.x,
            diagnostic.cameraBasis.up.y, diagnostic.cameraBasis.up.z,
            diagnostic.cameraBasis.forward.x, diagnostic.cameraBasis.forward.y,
            diagnostic.cameraBasis.forward.z), 12, 214);
    }
    if (placementPreview_.valid)
    {
        dc.DrawText(wxString::Format("Place: %.2f, %.2f, %.2f",
            placementPreview_.x, placementPreview_.y, placementPreview_.z),
            12, wireframeVisible_ ? 234 : 114);
        if (!placementAssetName_.empty())
            dc.DrawText("Asset: " + wxString::FromUTF8(placementAssetName_ +
                " (" + placementAssetId_ + ")"), 12,
                wireframeVisible_ ? 254 : 134);
    }
}

void wxEditorViewport::OnSize(wxSizeEvent& event)
{
    const wxSize size = event.GetSize();
    controller_.OnResize(size.GetWidth(), size.GetHeight());
    if (d3dRenderer_.IsInitialized())
    {
        std::string reason;
        if (!d3dRenderer_.Resize(size.GetWidth(), size.GetHeight(), &reason))
        {
            d3dFailure_ = reason;
            d3dRenderer_.Shutdown();
            backend_ = EditorViewportBackend::SoftwareDiagnostic;
            d3dAvailable_ = false;
        }
    }
    Refresh(false);
    event.Skip();
}

void wxEditorViewport::OnEraseBackground(wxEraseEvent&)
{
    // D3D clears its swap-chain target; suppress the native erase to avoid flicker.
}

void wxEditorViewport::OnDestroy(wxWindowDestroyEvent& event)
{
    timer_.Stop();
    d3dRenderer_.Shutdown();
    event.Skip();
}

void wxEditorViewport::OnFocus(wxFocusEvent& event)
{
    controller_.OnFocusChanged(event.GetEventType() == wxEVT_SET_FOCUS);
    Refresh(false);
    event.Skip();
}

void wxEditorViewport::OnMouseEnter(wxMouseEvent& event)
{
    controller_.OnMouseEnter();
    event.Skip();
}

void wxEditorViewport::OnMouseLeave(wxMouseEvent& event)
{
    controller_.OnMouseLeave();
    Refresh(false);
    event.Skip();
}

void wxEditorViewport::OnMouseMove(wxMouseEvent& event)
{
    controller_.OnMouseMove(event.GetX(), event.GetY());
    if (moveGizmo_.Active())
    {
        EditorPreviewObject* selected =
            previewScene_.FindByLogicalPath(previewScene_.SelectedPath());
        if (selected)
        {
            const EditorTransform preview = moveGizmo_.Update(
                event.GetX(), event.GetY(), moveSnapEnabled_);
            selected->x = preview.x;
            selected->y = preview.y;
            selected->z = preview.z;
            controller_.Render();
        }
    }
    else if (toolMode_ == EditorToolMode::PlaceObject ||
        toolMode_ == EditorToolMode::PlaceLight ||
        toolMode_ == EditorToolMode::PlaceAsset)
    {
        const EditorPreviewProjectionContext projection =
            MakeEditorPreviewProjectionContext(controller_.State(),
                controller_.State().width, controller_.State().height);
        placementPreview_ = UnprojectEditorPreviewToGround(
            static_cast<float>(event.GetX()), static_cast<float>(event.GetY()),
            projection, placementDefaults_.y,
            moveSnapEnabled_);
        renderer_.SetPlacementPreview(placementPreview_);
        controller_.Render();
    }
    Refresh(false);
    event.Skip();
}

void wxEditorViewport::OnMouseButton(wxMouseEvent& event)
{
    SetFocus();
    const bool pressed = event.ButtonDown();
    const bool left = event.GetButton() == wxMOUSE_BTN_LEFT;
    if (pressed && left)
    {
        if (toolMode_ == EditorToolMode::PlaceObject ||
            toolMode_ == EditorToolMode::PlaceLight ||
            toolMode_ == EditorToolMode::PlaceAsset)
        {
            const EditorViewportState& state = controller_.State();
            const EditorPreviewProjectionContext projection =
                MakeEditorPreviewProjectionContext(
                    state, state.width, state.height);
            placementPreview_ = UnprojectEditorPreviewToGround(
                static_cast<float>(event.GetX()),
                static_cast<float>(event.GetY()), projection,
                placementDefaults_.y,
                moveSnapEnabled_);
            renderer_.SetPlacementPreview(placementPreview_);
            if (placementPreview_.valid && placementHandler_)
            {
                EditorTransform transform;
                transform.x = placementPreview_.x;
                transform.y = placementPreview_.y;
                transform.z = placementPreview_.z;
                placementHandler_(toolMode_, transform);
            }
        }
        else
        {
        const EditorPreviewProjectedPoint origin = renderer_.SelectedPoint();
        const EditorPreviewObject* selected =
            previewScene_.FindByLogicalPath(previewScene_.SelectedPath());
        const EditorGizmoAxis axis = origin.visible
            ? moveGizmo_.Hit(origin.x, origin.y,
                static_cast<float>(event.GetX()),
                static_cast<float>(event.GetY()))
            : EditorGizmoAxis::None;
        if (toolMode_ == EditorToolMode::Move &&
            axis != EditorGizmoAxis::None && selected)
        {
            EditorTransform start;
            start.x = selected->x;
            start.y = selected->y;
            start.z = selected->z;
            moveGizmo_.Begin(axis, event.GetX(), event.GetY(), start);
            renderer_.SetActiveGizmoAxis(axis);
        }
        else if (selectionHandler_)
            selectionHandler_(controller_.OnPrimaryClick(
                event.GetX(), event.GetY()));
        }
    }
    else if (!pressed && left && moveGizmo_.Active())
    {
        const std::string path = previewScene_.SelectedPath();
        const EditorPreviewObject* selected =
            previewScene_.FindByLogicalPath(path);
        EditorTransform result = moveGizmo_.Start();
        if (selected)
        {
            result.x = selected->x;
            result.y = selected->y;
            result.z = selected->z;
        }
        moveGizmo_.Cancel();
        renderer_.SetActiveGizmoAxis(EditorGizmoAxis::None);
        if (transformHandler_ && !result.NearlyEquals(moveGizmo_.Start()))
            transformHandler_(path, result);
    }
    controller_.OnMouseButton(MapButton(event.GetButton()), pressed);
    if (pressed && !HasCapture())
        CaptureMouse();
    else if (!event.LeftIsDown() && !event.RightIsDown() &&
        !event.MiddleIsDown() && HasCapture())
        ReleaseMouse();
    Refresh(false);
    event.Skip();
}

void wxEditorViewport::OnMouseWheel(wxMouseEvent& event)
{
    controller_.OnMouseWheel(event.GetWheelRotation());
    Refresh(false);
    event.Skip();
}

void wxEditorViewport::OnKeyDown(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_ESCAPE)
    {
        if (CancelTransientOperation())
            return;
        if (toolMode_ != EditorToolMode::Select && cancelToolHandler_)
        {
            cancelToolHandler_();
            return;
        }
    }
    EditorViewportKey key;
    if (MapKey(event.GetKeyCode(), key))
    {
        controller_.OnKeyDown(key);
        return;
    }
    event.Skip();
}

void wxEditorViewport::OnKeyUp(wxKeyEvent& event)
{
    EditorViewportKey key;
    if (MapKey(event.GetKeyCode(), key))
    {
        controller_.OnKeyUp(key);
        return;
    }
    event.Skip();
}

void wxEditorViewport::OnTimer(wxTimerEvent&)
{
    controller_.Tick(0.033);
    Refresh(false);
}
