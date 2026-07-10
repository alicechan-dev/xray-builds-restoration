#include "wxEditorTree.h"

#include <wx/string.h>

namespace
{
class EditorTreeItemData final : public wxTreeItemData
{
public:
    explicit EditorTreeItemData(IEditorTree::UserData value) : value_(value) {}
    IEditorTree::UserData Value() const { return value_; }

private:
    IEditorTree::UserData value_;
};

wxString FromUtf8(const char* text)
{
    return wxString::FromUTF8(text ? text : "");
}
}

wxEditorTree::wxEditorTree(wxWindow* parent) :
    wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT)
{
}

void wxEditorTree::Clear()
{
    DeleteAllItems();
    items_.clear();
    nextHandle_ = 1;
    firstHandle_ = InvalidItem;
}

IEditorTree::ItemHandle wxEditorTree::AddRoot(const char* label)
{
    const ItemHandle root = StoreItem(wxTreeCtrl::AddRoot(FromUtf8(label)));
    firstHandle_ = InvalidItem;
    return root;
}

IEditorTree::ItemHandle wxEditorTree::AddChild(ItemHandle parent, const char* label)
{
    const auto parentItem = items_.find(parent);
    if (parentItem == items_.end())
        return InvalidItem;

    return StoreItem(AppendItem(parentItem->second, FromUtf8(label)));
}

std::string wxEditorTree::GetSelectedLabel() const
{
    const wxTreeItemId selected = GetSelection();
    return selected.IsOk() ? GetItemText(selected).ToStdString() : std::string();
}

void wxEditorTree::SetItemUserData(ItemHandle item, UserData userData)
{
    const auto storedItem = items_.find(item);
    if (storedItem != items_.end())
        SetItemData(storedItem->second, new EditorTreeItemData(userData));
}

IEditorTree::UserData wxEditorTree::GetSelectedUserData() const
{
    const wxTreeItemId selected = GetSelection();
    if (!selected.IsOk())
        return 0;

    const auto* data = dynamic_cast<EditorTreeItemData*>(GetItemData(selected));
    return data ? data->Value() : 0;
}

void wxEditorTree::ExpandAllItems()
{
    ExpandAll();
}

void wxEditorTree::SelectFirst()
{
    const auto firstItem = items_.find(firstHandle_);
    if (firstItem != items_.end())
        SelectItem(firstItem->second);
}

IEditorTree::ItemHandle wxEditorTree::StoreItem(const wxTreeItemId& item)
{
    if (!item.IsOk())
        return InvalidItem;

    const ItemHandle handle = nextHandle_++;
    items_.emplace(handle, item);
    if (firstHandle_ == InvalidItem)
        firstHandle_ = handle;
    return handle;
}
