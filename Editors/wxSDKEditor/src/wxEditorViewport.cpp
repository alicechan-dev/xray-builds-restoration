#include "wxEditorViewport.h"

#include "editor_view/IEditorViewportRenderer.h"

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
    renderer_(std::make_unique<NullEditorViewportRenderer>()),
    controller_(renderer_.get()), timer_(this)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
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
