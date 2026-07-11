#ifndef XR_WX_SDK_EDITOR_WX_DIALOG_SERVICE_H
#define XR_WX_SDK_EDITOR_WX_DIALOG_SERVICE_H

#include "editor_ui/IDialogService.h"

class wxWindow;

class wxDialogService final : public IDialogService
{
public:
    explicit wxDialogService(wxWindow* parent);

    void Info(const char* title, const char* message) override;
    void Warning(const char* title, const char* message) override;
    void Error(const char* title, const char* message) override;
    bool Confirm(const char* title, const char* message) override;

private:
    void Show(const char* title, const char* message, long style);

    wxWindow* parent_ = nullptr;
};

#endif
