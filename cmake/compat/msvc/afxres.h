#ifndef XRAY_COMPAT_AFXRES_H_INCLUDED
#define XRAY_COMPAT_AFXRES_H_INCLUDED

// xrCore.rc only uses standard Win32 dialog/style resource symbols. The
// original VC2003 include came from MFC, but MFC is not otherwise required.
#include <winres.h>

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

#endif
