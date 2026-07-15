#ifndef XR_WX_SDK_EDITOR_EDITOR_RENDER_MESH_METADATA_H
#define XR_WX_SDK_EDITOR_EDITOR_RENDER_MESH_METADATA_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct EditorRenderBounds
{
    float minX = 0, minY = 0, minZ = 0;
    float maxX = 0, maxY = 0, maxZ = 0;
    bool valid = false;
};

struct EditorRenderMeshMetadata
{
    std::size_t meshIndex = 0;
    std::string meshId;
    std::string name;
    std::uint16_t version = 0;
    std::size_t vertexCount = 0;
    std::size_t triangleCount = 0;
    std::size_t surfaceSlotCount = 0;
    std::size_t vmapCount = 0;
    std::size_t uvMapCount = 0;
    std::size_t weightMapCount = 0;
    std::uint8_t vmapFormat = 0;
    bool smoothingGroupsPresent = false;
    std::uint64_t sourcePayloadSize = 0;
    EditorRenderBounds bounds;
    bool countsValidated = false;
    bool skeletal = false;
    bool supported = false;
    bool malformed = false;
    std::size_t unknownChunkCount = 0;
    std::vector<std::uint32_t> unknownChunkIds;
    std::vector<std::string> diagnostics;
};

bool MergeEditorRenderBounds(EditorRenderBounds& target,
    const EditorRenderBounds& source);

#endif
