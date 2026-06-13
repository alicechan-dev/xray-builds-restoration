#ifndef XRAY_COMPAT_DXERR9_H_INCLUDED
#define XRAY_COMPAT_DXERR9_H_INCLUDED

#include <windows.h>

// The legacy DirectX SDK provided dxerr9.h/dxerr9.lib for richer HRESULT text.
// When that SDK is unavailable, xrDebug.cpp already falls back to FormatMessage
// if this helper returns an empty string.
static __inline const char* DXGetErrorDescription9(HRESULT)
{
    return "";
}

#endif
