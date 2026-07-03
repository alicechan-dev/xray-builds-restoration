#pragma once

// Borland C++ Builder provided <fastmath.h>.  MSVC does not, and the ECore
// sources only need the standard math declarations. Some old stdafx headers
// define sqrtf before including fastmath.h, so keep that macro out of the CRT
// declarations and restore it afterwards.
#ifdef sqrtf
#  define XRAY_RESTORE_SQRTF_MACRO
#  undef sqrtf
#endif

#include <math.h>

#ifdef XRAY_RESTORE_SQRTF_MACRO
#  define sqrtf(a) sqrt(a)
#  undef XRAY_RESTORE_SQRTF_MACRO
#endif
