#include "AnsiString.h"

#include <cstring>
#include <iostream>
#include <string>

namespace
{
int failures = 0;

void check(bool condition, const char* description)
{
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}
}

int main()
{
    AnsiString empty;
    check(empty.IsEmpty(), "default construction is empty");
    check(empty.Length() == 0, "empty length is zero");

    AnsiString text("Stalker");
    check(std::strcmp(text.c_str(), "Stalker") == 0, "construction from const char*");
    check(text.Length() == 7, "Length reports byte length");
    check(text[1] == 'S' && text[7] == 'r', "indexing is one-based");

    text = "X-Ray";
    const char* immediate = text.c_str();
    check(std::strcmp(immediate, "X-Ray") == 0, "c_str supports immediate use");

    AnsiString from_std_string(std::string("SDK"));
    check(from_std_string == "SDK", "construction and comparison with std::string data");

    AnsiString number(1935);
    check(number == "1935", "signed integer construction");
    check(AnsiString(42u) == "42", "unsigned integer construction");

    AnsiString combined = AnsiString("build ") + number;
    combined += " restored";
    check(combined == "build 1935 restored", "concatenation and append");
    check(AnsiString("chunk ") + 7u == "chunk 7", "direct numeric concatenation");
    check(AnsiString("alpha") < AnsiString("beta"), "lexical ordering");

    check(AnsiString("MiXeD").LowerCase() == "mixed", "LowerCase returns a converted value");
    check(AnsiString("MiXeD").UpperCase() == "MIXED", "UpperCase returns a converted value");

    AnsiString formatted;
    AnsiString& formatted_result = formatted.sprintf("%s %d %.1f", "build", 1935, 1.5);
    check(&formatted_result == &formatted, "sprintf is chainable");
    check(formatted == "build 1935 1.5", "sprintf formats audited argument types");

    bool rejected_zero_index = false;
    try {
        (void)text[0];
    }
    catch (const std::out_of_range&) {
        rejected_zero_index = true;
    }
    check(rejected_zero_index, "zero index is rejected");

    if (failures != 0)
        return 1;

    std::cout << "AnsiString compatibility contract passed\n";
    return 0;
}
