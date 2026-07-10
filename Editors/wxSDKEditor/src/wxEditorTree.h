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
    void SetItemUserData(ItemHandle item, UserData userData) override;
    UserData GetSelectedUserData() const override;
    void ExpandAllItems() override;
    void SelectFirst() override;

private:
    ItemHandle StoreItem(const wxTreeItemId& item);

    ItemHandle nextHandle_ = 1;
    ItemHandle firstHandle_ = InvalidItem;
    std::unordered_map<ItemHandle, wxTreeItemId> items_;
};

#endif
