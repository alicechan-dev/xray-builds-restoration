#include "wxEditorTree.h"

#include <wx/string.h>

namespace
{
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
}

IEditorTree::ItemHandle wxEditorTree::AddRoot(const char* label)
{
    return StoreItem(wxTreeCtrl::AddRoot(FromUtf8(label)));
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

IEditorTree::ItemHandle wxEditorTree::StoreItem(const wxTreeItemId& item)
{
    if (!item.IsOk())
        return InvalidItem;

    const ItemHandle handle = nextHandle_++;
    items_.emplace(handle, item);
    return handle;
}
