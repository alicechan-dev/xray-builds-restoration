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

bool wxDialogService::Confirm(const char* title, const char* message)
{
    return wxMessageBox(wxString::FromUTF8(message ? message : ""),
        wxString::FromUTF8(title ? title : ""),
        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, parent_) == wxYES;
}

void wxDialogService::Show(const char* title, const char* message, long style)
{
    wxMessageBox(wxString::FromUTF8(message ? message : ""),
        wxString::FromUTF8(title ? title : ""), style, parent_);
}
