#ifndef XR_WX_SDK_EDITOR_I_DIALOG_SERVICE_H
#define XR_WX_SDK_EDITOR_I_DIALOG_SERVICE_H

class IDialogService
{
public:
    virtual ~IDialogService() = default;

    virtual void Info(const char* title, const char* message) = 0;
    virtual void Warning(const char* title, const char* message) = 0;
    virtual void Error(const char* title, const char* message) = 0;
    virtual bool Confirm(const char* title, const char* message) = 0;
};

#endif
