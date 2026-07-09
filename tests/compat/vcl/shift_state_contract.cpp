#include "TShiftState.h"

#include <iostream>

static_assert(ssShift == 0, "ssShift value changed");
static_assert(ssAlt == 1, "ssAlt value changed");
static_assert(ssCtrl == 2, "ssCtrl value changed");
static_assert(ssLeft == 3, "ssLeft value changed");
static_assert(ssRight == 4, "ssRight value changed");
static_assert(ssMiddle == 5, "ssMiddle value changed");
static_assert(ssDouble == 6, "ssDouble value changed");

int main()
{
    TShiftState empty;
    if (empty.Contains(ssShift) || empty.Contains(ssLeft))
        return 1;

    TShiftState input;
    input << ssCtrl << ssLeft << ssDouble;
    if (!input.Contains(ssCtrl) || !input.Contains(ssLeft) ||
        !input.Contains(ssDouble))
        return 1;
    if (input.Contains(ssAlt) || input.Contains(ssRight))
        return 1;

    TShiftState copied = input;
    copied >> ssLeft;
    if (copied.Contains(ssLeft) || !copied.Contains(ssCtrl))
        return 1;
    if (!input.Contains(ssLeft))
        return 1;

    std::cout << "TShiftState compatibility contract passed\n";
    return 0;
}
