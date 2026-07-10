#ifndef XR_WX_SDK_EDITOR_WX_EDITOR_TREE_H
#define XR_WX_SDK_EDITOR_WX_EDITOR_TREE_H

#include "editor_ui/IEditorTree.h"

#include <unordered_map>

#include <wx/treectrl.h>

class wxEditorTree final : public wxTreeCtrl, public IEditorTree
{
public:
    explicit wxEditorTree(wxWindow* parent);

    void Clear() override;
    ItemHandle AddRoot(const char* label) override;
    ItemHandle AddChild(ItemHandle parent, const char* label) override;
    std::string GetSelectedLabel() const override;

private:
    ItemHandle StoreItem(const wxTreeItemId& item);

    ItemHandle nextHandle_ = 1;
    std::unordered_map<ItemHandle, wxTreeItemId> items_;
};

#endif
