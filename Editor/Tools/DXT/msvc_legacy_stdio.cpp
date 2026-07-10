#include <cstdio>

#if defined(_MSC_VER) && _MSC_VER >= 1900
extern "C" FILE* __cdecl __acrt_iob_func(unsigned index);

// nvDXTlib.lib is a VC6-era static library that references the old _iob array
// for fprintf(stderr, ...) diagnostics. Keep this compatibility symbol local
// to the DXT helper instead of changing global CRT/link settings.
extern "C" FILE _iob[3] = {
    *__acrt_iob_func(0),
    *__acrt_iob_func(1),
    *__acrt_iob_func(2),
};
#endif
