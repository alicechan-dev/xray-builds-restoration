#ifndef XRAY_VCL_COMPAT_TSHIFTSTATE_H
#define XRAY_VCL_COMPAT_TSHIFTSTATE_H

enum TShiftStateEnum
{
    ssShift,
    ssAlt,
    ssCtrl,
    ssLeft,
    ssRight,
    ssMiddle,
    ssDouble
};

class TShiftState
{
public:
    TShiftState()
        : states_(0)
    {
    }

    TShiftState& operator<<(TShiftStateEnum state)
    {
        states_ |= mask(state);
        return *this;
    }

    TShiftState& operator>>(TShiftStateEnum state)
    {
        states_ &= ~mask(state);
        return *this;
    }

    bool Contains(TShiftStateEnum state) const
    {
        return (states_ & mask(state)) != 0;
    }

private:
    static unsigned int mask(TShiftStateEnum state)
    {
        return 1u << static_cast<unsigned int>(state);
    }

    unsigned int states_;
};

#endif
