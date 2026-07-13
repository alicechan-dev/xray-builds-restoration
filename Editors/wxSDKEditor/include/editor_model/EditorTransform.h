#ifndef XR_WX_SDK_EDITOR_EDITOR_TRANSFORM_H
#define XR_WX_SDK_EDITOR_EDITOR_TRANSFORM_H

#include <cmath>

struct EditorTransform
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
    float sx = 1.0f;
    float sy = 1.0f;
    float sz = 1.0f;

    bool IsFinite() const;
    bool NearlyEquals(const EditorTransform& other,
        float epsilon = 0.0001f) const;
    void Translate(float dx, float dy, float dz);
};

#endif
