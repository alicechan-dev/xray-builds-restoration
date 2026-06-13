#pragma once

// VC2003 provided <typeinfo.h>; modern MSVC provides the standard <typeinfo>.
// Keep this as a CMake-side compatibility shim so historical sources stay intact.
#include <typeinfo>
