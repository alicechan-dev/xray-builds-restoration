#ifndef XR_WX_SDK_EDITOR_EDITOR_MOVE_GIZMO_H
#define XR_WX_SDK_EDITOR_EDITOR_MOVE_GIZMO_H

#include "editor_model/EditorTransform.h"

enum class EditorGizmoAxis { None, X, Z };

class EditorMoveGizmo
{
public:
    EditorGizmoAxis Hit(float originX, float originY,
        float mouseX, float mouseY) const;
    void Begin(EditorGizmoAxis axis, int mouseX, int mouseY,
        const EditorTransform& transform);
    EditorTransform Update(int mouseX, int mouseY, bool snap) const;
    void Cancel() { active_ = EditorGizmoAxis::None; }
    bool Active() const { return active_ != EditorGizmoAxis::None; }
    EditorGizmoAxis Axis() const { return active_; }
    const EditorTransform& Start() const { return start_; }

private:
    EditorGizmoAxis active_ = EditorGizmoAxis::None;
    int mouseX_ = 0;
    int mouseY_ = 0;
    EditorTransform start_;
};

#endif
