// Legacy d3dx9.lib was built against old MSVC CRT entry points that are not
// exported by modern MSVC CRT import libraries. Keep these narrow shims local
// to xrRender_R1, which links the legacy D3DX shader/texture helpers directly.

#include <vadefs.h>

extern "C" int __cdecl vsnprintf(char* buffer, unsigned int count, const char* format, va_list args);
extern "C" unsigned __int64* __cdecl __local_stdio_scanf_options();
extern "C" int __cdecl __stdio_common_vsscanf(
    unsigned __int64 options,
    const char* buffer,
    unsigned int buffer_count,
    const char* format,
    void* locale,
    va_list args);

extern "C" int __cdecl vsprintf(char* buffer, const char* format, va_list args)
{
    return vsnprintf(buffer, ~0u, format, args);
}

extern "C" int __cdecl _vsnprintf(char* buffer, unsigned int count, const char* format, va_list args)
{
    return vsnprintf(buffer, count, format, args);
}

extern "C" int __cdecl sprintf(char* buffer, const char* format, ...)
{
    va_list args;
    __crt_va_start(args, format);
    int result = vsprintf(buffer, format, args);
    __crt_va_end(args);
    return result;
}

extern "C" int __cdecl _snprintf(char* buffer, unsigned int count, const char* format, ...)
{
    va_list args;
    __crt_va_start(args, format);
    int result = _vsnprintf(buffer, count, format, args);
    __crt_va_end(args);
    return result;
}

extern "C" int __cdecl sscanf(const char* buffer, const char* format, ...)
{
    va_list args;
    __crt_va_start(args, format);
    int result = __stdio_common_vsscanf(*__local_stdio_scanf_options(), buffer, ~0u, format, 0, args);
    __crt_va_end(args);
    return result;
}
