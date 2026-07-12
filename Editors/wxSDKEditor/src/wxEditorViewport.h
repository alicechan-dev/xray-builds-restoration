#ifndef XR_WX_SDK_EDITOR_WX_EDITOR_VIEWPORT_H
#define XR_WX_SDK_EDITOR_WX_EDITOR_VIEWPORT_H

#include "editor_view/EditorViewportController.h"
#include "editor_view/EditorPreviewRenderer.h"
#include "editor_view/EditorPreviewScene.h"

#include <memory>
#include <functional>
#include <string>
#include <wx/panel.h>
#include <wx/timer.h>

class EditorTreeModel;

class wxEditorViewport final : public wxPanel
{
public:
    explicit wxEditorViewport(wxWindow* parent);

    void ToggleGrid();
    void ResetCamera();
    void FocusViewport();
    bool IsGridVisible() const;
    void RebuildPreview(const EditorTreeModel& model,
        const std::string& selectedPath);
    void TogglePreviewLabels();
    bool ArePreviewLabelsVisible() const;
    bool FrameSelected();
    void SetSelectionHandler(std::function<void(const std::string&)> handler)
    { selectionHandler_ = std::move(handler); }

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

    EditorPreviewScene previewScene_;
    EditorPreviewRenderer renderer_;
    EditorViewportController controller_;
    wxTimer timer_;
    std::function<void(const std::string&)> selectionHandler_;
};

#endif
