#include "editor_model/EditorTransform.h"

bool EditorTransform::IsFinite() const
{
    return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) &&
        std::isfinite(yaw) && std::isfinite(pitch) && std::isfinite(roll) &&
        std::isfinite(sx) && std::isfinite(sy) && std::isfinite(sz);
}

bool EditorTransform::NearlyEquals(
    const EditorTransform& other, float epsilon) const
{
    return std::fabs(x - other.x) < epsilon &&
        std::fabs(y - other.y) < epsilon &&
        std::fabs(z - other.z) < epsilon &&
        std::fabs(yaw - other.yaw) < epsilon &&
        std::fabs(pitch - other.pitch) < epsilon &&
        std::fabs(roll - other.roll) < epsilon &&
        std::fabs(sx - other.sx) < epsilon &&
        std::fabs(sy - other.sy) < epsilon &&
        std::fabs(sz - other.sz) < epsilon;
}

void EditorTransform::Translate(float dx, float dy, float dz)
{
    x += dx;
    y += dy;
    z += dz;
}
