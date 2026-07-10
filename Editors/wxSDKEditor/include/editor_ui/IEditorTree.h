#ifndef XR_WX_SDK_EDITOR_I_EDITOR_TREE_H
#define XR_WX_SDK_EDITOR_I_EDITOR_TREE_H

#include <cstdint>
#include <string>

class IEditorTree
{
public:
    using ItemHandle = std::uintptr_t;
    static const ItemHandle InvalidItem = 0;

    virtual ~IEditorTree() = default;

    virtual void Clear() = 0;
    virtual ItemHandle AddRoot(const char* label) = 0;
    virtual ItemHandle AddChild(ItemHandle parent, const char* label) = 0;
    virtual std::string GetSelectedLabel() const = 0;
};

#endif
