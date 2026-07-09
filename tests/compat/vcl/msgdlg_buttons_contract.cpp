#include "TMsgDlgButtons.h"

#include <iostream>

static_assert(mbYes == 0, "mbYes value changed");
static_assert(mbNo == 1, "mbNo value changed");
static_assert(mbOK == 2, "mbOK value changed");
static_assert(mbCancel == 3, "mbCancel value changed");
static_assert(mbAbort == 4, "mbAbort value changed");
static_assert(mbRetry == 5, "mbRetry value changed");
static_assert(mbIgnore == 6, "mbIgnore value changed");
static_assert(mbAll == 7, "mbAll value changed");
static_assert(mbNoToAll == 8, "mbNoToAll value changed");
static_assert(mbYesToAll == 9, "mbYesToAll value changed");
static_assert(mbHelp == 10, "mbHelp value changed");

int main()
{
    TMsgDlgButtons empty;
    if (empty.Contains(mbYes) || empty.Contains(mbOK))
        return 1;

    TMsgDlgButtons confirmation = TMsgDlgButtons() << mbYes << mbNo << mbCancel;
    if (!confirmation.Contains(mbYes))
        return 1;
    if (!confirmation.Contains(mbNo))
        return 1;
    if (!confirmation.Contains(mbCancel))
        return 1;
    if (confirmation.Contains(mbOK))
        return 1;

    TMsgDlgButtons ok = TMsgDlgButtons() << mbOK;
    if (!ok.Contains(mbOK) || ok.Contains(mbCancel))
        return 1;

    std::cout << "TMsgDlgButtons compatibility contract passed\n";
    return 0;
}
