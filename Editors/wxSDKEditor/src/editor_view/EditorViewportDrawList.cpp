#include "editor_view/EditorViewportDrawList.h"

#include <utility>

void EditorViewportDrawList::Add(EditorViewportPrimitive primitive)
{
    primitives_.push_back(std::move(primitive));
}
