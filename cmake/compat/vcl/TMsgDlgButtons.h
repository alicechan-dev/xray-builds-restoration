#ifndef XRAY_VCL_COMPAT_TMSGDLGBUTTONS_H
#define XRAY_VCL_COMPAT_TMSGDLGBUTTONS_H

enum TMsgDlgBtn
{
    mbYes,
    mbNo,
    mbOK,
    mbCancel,
    mbAbort,
    mbRetry,
    mbIgnore,
    mbAll,
    mbNoToAll,
    mbYesToAll,
    mbHelp
};

class TMsgDlgButtons
{
public:
    TMsgDlgButtons()
        : buttons_(0)
    {
    }

    TMsgDlgButtons& operator<<(TMsgDlgBtn button)
    {
        buttons_ |= (1u << static_cast<unsigned int>(button));
        return *this;
    }

    bool Contains(TMsgDlgBtn button) const
    {
        return (buttons_ & (1u << static_cast<unsigned int>(button))) != 0;
    }

private:
    unsigned int buttons_;
};

#endif
