#ifndef XR_WX_SDK_EDITOR_EDITOR_VIEWPORT_DRAW_LIST_H
#define XR_WX_SDK_EDITOR_EDITOR_VIEWPORT_DRAW_LIST_H

#include <string>
#include <vector>

enum class EditorViewportPrimitiveType { Line, Rectangle, Circle, Text };
enum class EditorViewportStyle { Object, Marker, Light, Spawn, Selected, Label };

struct EditorViewportPrimitive
{
    EditorViewportPrimitiveType type = EditorViewportPrimitiveType::Line;
    EditorViewportStyle style = EditorViewportStyle::Object;
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    float radius = 0.0f;
    std::string text;
};

class EditorViewportDrawList
{
public:
    void Clear() { primitives_.clear(); }
    void Add(EditorViewportPrimitive primitive);
    const std::vector<EditorViewportPrimitive>& Primitives() const
    { return primitives_; }

private:
    std::vector<EditorViewportPrimitive> primitives_;
};

#endif
