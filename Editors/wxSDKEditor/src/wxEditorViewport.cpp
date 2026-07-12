#include "wxEditorViewport.h"

#include "editor_view/IEditorViewportRenderer.h"
#include "editor_view/EditorTreePreviewAdapter.h"

#include <wx/dcbuffer.h>

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

void wxEditorViewport::TogglePreviewLabels()
{
    renderer_.SetLabelsVisible(!renderer_.LabelsVisible());
    Refresh(false);
}

bool wxEditorViewport::ArePreviewLabelsVisible() const
{
    return renderer_.LabelsVisible();
}

bool wxEditorViewport::FrameSelected()
{
    const EditorPreviewObject* selected =
        previewScene_.FindByLogicalPath(previewScene_.SelectedPath());
    if (!selected)
        return false;
    controller_.FrameCameraOn(selected->x, selected->y, selected->z);
    Refresh(false);
    return true;
}

void wxEditorViewport::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    const EditorViewportState& state = controller_.State();
    dc.SetBackground(wxBrush(wxColour(34, 38, 42)));
    dc.Clear();

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
    dc.DrawText("Renderer is not connected", 12, 12);
    dc.DrawText(wxString::Format("Size: %d x %d", state.width, state.height),
        12, 34);
    dc.DrawText(wxString::Format("Mouse: %d, %d  Focus: %s",
        state.mouseX, state.mouseY, state.focused ? "yes" : "no"), 12, 54);
    dc.DrawText(wxString::Format(
        "Camera: (%.2f, %.2f, %.2f) yaw %.1f pitch %.1f speed %.1f",
        state.camera.x, state.camera.y, state.camera.z, state.camera.yaw,
        state.camera.pitch, state.camera.movementSpeed), 12, 74);
}

void wxEditorViewport::OnSize(wxSizeEvent& event)
{
    const wxSize size = event.GetSize();
    controller_.OnResize(size.GetWidth(), size.GetHeight());
    Refresh(false);
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
    Refresh(false);
    event.Skip();
}

void wxEditorViewport::OnMouseButton(wxMouseEvent& event)
{
    SetFocus();
    const bool pressed = event.ButtonDown();
    controller_.OnMouseButton(MapButton(event.GetButton()), pressed);
    if (pressed && event.GetButton() == wxMOUSE_BTN_LEFT && selectionHandler_)
        selectionHandler_(controller_.OnPrimaryClick(event.GetX(), event.GetY()));
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
