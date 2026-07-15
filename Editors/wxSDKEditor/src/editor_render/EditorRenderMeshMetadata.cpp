#include "editor_render/EditorRenderMeshMetadata.h"

#include <algorithm>
#include <cmath>

bool MergeEditorRenderBounds(EditorRenderBounds& target,
    const EditorRenderBounds& source)
{
    if (!source.valid) return false;
    if (!target.valid) { target = source; return true; }
    target.minX = (std::min)(target.minX, source.minX);
    target.minY = (std::min)(target.minY, source.minY);
    target.minZ = (std::min)(target.minZ, source.minZ);
    target.maxX = (std::max)(target.maxX, source.maxX);
    target.maxY = (std::max)(target.maxY, source.maxY);
    target.maxZ = (std::max)(target.maxZ, source.maxZ);
    return true;
}
