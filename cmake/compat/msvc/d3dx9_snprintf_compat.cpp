// Legacy D3DX9 static libraries were built against old MSVC CRT symbols.
// Modern MSVC keeps these implementations out of the default CRT import set,
// so provide the narrow compatibility entry points for XR_3DA only. Debug
// already gets _vsnprintf/_vsprintf from existing engine objects, but still
// needs _snprintf for D3DX shader compiler objects.

#include <vadefs.h>

#ifdef _DEBUG
extern "C" int __cdecl _vsnprintf(char* buffer, unsigned int count, const char* format, va_list args);
#else
extern "C" int __cdecl vsnprintf(char* buffer, unsigned int count, const char* format, va_list args);

extern "C" int __cdecl vsprintf(char* buffer, const char* format, va_list args)
{
    return vsnprintf(buffer, ~0u, format, args);
}

extern "C" int __cdecl _vsnprintf(char* buffer, unsigned int count, const char* format, va_list args)
{
    return vsnprintf(buffer, count, format, args);
}

extern "C" int __cdecl _vsprintf(char* buffer, const char* format, va_list args)
{
    return vsprintf(buffer, format, args);
}
#endif

extern "C" int __cdecl _snprintf(char* buffer, unsigned int count, const char* format, ...)
{
    va_list args;
    __crt_va_start(args, format);
    int result = _vsnprintf(buffer, count, format, args);
    __crt_va_end(args);
    return result;
}
