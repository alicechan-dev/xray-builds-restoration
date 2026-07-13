#include "editor_view/EditorMoveGizmo.h"

#include <cmath>

EditorGizmoAxis EditorMoveGizmo::Hit(
    float originX, float originY, float mouseX, float mouseY) const
{
    if (std::fabs(mouseY - originY) <= 5.0f && mouseX >= originX &&
        mouseX <= originX + 45.0f)
        return EditorGizmoAxis::X;
    if (std::fabs(mouseX - originX) <= 5.0f && mouseY >= originY - 45.0f &&
        mouseY <= originY)
        return EditorGizmoAxis::Z;
    return EditorGizmoAxis::None;
}

void EditorMoveGizmo::Begin(EditorGizmoAxis axis, int mouseX, int mouseY,
    const EditorTransform& transform)
{
    active_ = axis;
    mouseX_ = mouseX;
    mouseY_ = mouseY;
    start_ = transform;
}

EditorTransform EditorMoveGizmo::Update(
    int mouseX, int mouseY, bool snap) const
{
    EditorTransform transform = start_;
    if (active_ == EditorGizmoAxis::X)
        transform.x += static_cast<float>(mouseX - mouseX_) / 40.0f;
    if (active_ == EditorGizmoAxis::Z)
        transform.z += static_cast<float>(mouseY - mouseY_) / 40.0f;
    if (snap)
    {
        transform.x = std::round(transform.x);
        transform.z = std::round(transform.z);
    }
    return transform;
}
