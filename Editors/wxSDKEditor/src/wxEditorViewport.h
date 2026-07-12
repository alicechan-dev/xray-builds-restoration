#ifndef XR_WX_SDK_EDITOR_WX_EDITOR_VIEWPORT_H
#define XR_WX_SDK_EDITOR_WX_EDITOR_VIEWPORT_H

#include "editor_view/EditorViewportController.h"

#include <memory>
#include <wx/panel.h>
#include <wx/timer.h>

class IEditorViewportRenderer;

class wxEditorViewport final : public wxPanel
{
public:
    explicit wxEditorViewport(wxWindow* parent);

    void ToggleGrid();
    void ResetCamera();
    void FocusViewport();
    bool IsGridVisible() const;

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnFocus(wxFocusEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseButton(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnTimer(wxTimerEvent& event);

    std::unique_ptr<IEditorViewportRenderer> renderer_;
    EditorViewportController controller_;
    wxTimer timer_;
};

#endif
