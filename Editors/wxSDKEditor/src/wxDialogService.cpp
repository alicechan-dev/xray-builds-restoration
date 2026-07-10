#include "wxDialogService.h"

#include <wx/msgdlg.h>
#include <wx/string.h>

wxDialogService::wxDialogService(wxWindow* parent) : parent_(parent)
{
}

void wxDialogService::Info(const char* title, const char* message)
{
    Show(title, message, wxOK | wxICON_INFORMATION);
}

void wxDialogService::Warning(const char* title, const char* message)
{
    Show(title, message, wxOK | wxICON_WARNING);
}

void wxDialogService::Error(const char* title, const char* message)
{
    Show(title, message, wxOK | wxICON_ERROR);
}

void wxDialogService::Show(const char* title, const char* message, long style)
{
    wxMessageBox(wxString::FromUTF8(message ? message : ""),
        wxString::FromUTF8(title ? title : ""), style, parent_);
}
