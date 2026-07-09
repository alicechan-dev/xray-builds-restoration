#include "TMsgDlgType.h"

#include <iostream>

static_assert(mtWarning == 0, "mtWarning value changed");
static_assert(mtError == 1, "mtError value changed");
static_assert(mtInformation == 2, "mtInformation value changed");
static_assert(mtConfirmation == 3, "mtConfirmation value changed");
static_assert(mtCustom == 4, "mtCustom value changed");

int main()
{
    const TMsgDlgType values[] = {
        mtWarning,
        mtError,
        mtInformation,
        mtConfirmation,
        mtCustom
    };

    for (int i = 0; i != 5; ++i) {
        if (values[i] != i)
            return 1;
    }

    std::cout << "TMsgDlgType compatibility contract passed\n";
    return 0;
}
