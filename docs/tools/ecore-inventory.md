# ECore CMake Port Inventory

This document records the first modern CMake restoration pass for `Editors/ECore`, the shared editor-core DLL used by the historical LevelEditor and related SDK tools.

## Summary

`Editors/ECore` is a Borland C++ Builder 6 package/DLL project. The original project file is `Editors/ECore/xrECoreB.bpr`, with `xrECoreB.bpf` as the Borland project-manager main source and `resource.res` plus three `.dfm` form resources.

A modern opt-in CMake target named `xrECore` now exists behind `BUILD_XR_ECORE`. This target is intentionally experimental and partial. It maps the old source tree into a Visual Studio 2022 Win32 shared-library target, excludes the VCL form units, and keeps the rest of the editor architecture intact for build-probe work.

## Project Files Found

| Path | Type | Likely output | Notes |
|---|---|---|---|
| `Editors/ECore/xrECoreB.bpr` | Borland C++ Builder 6 project | `x:\xrECoreB.dll` | Main historical project. |
| `Editors/ECore/xrECoreB.bpf` | Borland project-manager file | DLL entry/project helper | Lists VCL forms and `DllEntryPoint`. |
| `Editors/ECore/resource.res` | Compiled resource | Linked resource | Kept in the CMake target for parity. |
| `Editors/ECore/Editor/*.dfm` | VCL form resources | GUI dialogs | Excluded from the first MSVC target. |

## CMake Target

| Field | Value |
|---|---|
| Target name | `xrECore` |
| Opt-in option | `BUILD_XR_ECORE` |
| Target type | `SHARED` DLL |
| Added from | `tools/CMakeLists.txt` via `add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/ECore" ...)` |
| Output directory | Existing global `XRAY_RUNTIME_DIR` / `XRAY_LIBRARY_DIR` layout |
| Runtime-heavy targets touched | None |

## Dependency Map

| Dependency | Evidence | Status |
|---|---|---|
| `xrCore` | `stdafx.h` includes `<xrCore.h>` and old pragmas request `xrCoreB.lib`. | Linked through existing CMake target `xrCore`. |
| `xrCDB` | `stdafx.h` includes `<xrCDB.h>`. | Linked through existing CMake target `xrCDB`. |
| `xrParticles` | `stdafx.h` includes `<PSystem.h>`. | Linked through existing CMake target `xrParticles`. |
| `ETools` | `stdafx.h` includes `ETools.h` and old pragmas request `EToolsB.lib`. | `BUILD_XR_ECORE` also adds the `ETools` target. |
| `xrEProps` | `stdafx.h` includes `xrEProps.h` and old pragmas request `xrEPropsB.lib`. | Header include path is present, but no modern `xrEProps` CMake target exists yet. |
| DirectX 9 / D3DX | Includes `d3d9.h`, `d3dx9.h`, `dinput.h`, and `dsound.h`. | Uses existing DirectX SDK discovery variables. |
| Borland/VCL packages | `.bpr` lists `rtl.bpi`, `vcl.bpi`, `vclx.bpi`, `bcbsmp.bpi`; `.bpf` uses `USEFORM`. | VCL form units are excluded from the initial CMake target. |
| ElPack / AlexMX / MagicFM | `.bpr` include/lib paths and source pragmas reference these packages. | Not restored; expected later editor GUI boundary. |

## Excluded Source Set

The following VCL form units are excluded from the initial target:

* `Editor/ImageEditor.cpp` and `Editor/ImageEditor.dfm`
* `Editor/LogForm.cpp` and `Editor/LogForm.dfm`
* `Editor/SoundEditor.cpp` and `Editor/SoundEditor.dfm`

Reason: these are Borland/VCL forms with external component packages such as ElPack, AlexMX, and MagicFM. They should be restored with the editor UI boundary, not hidden inside the first library scaffold.

## Build Probe

Configure command:

```bat
cmake -S . -B build-ecore-check -G "Visual Studio 17 2022" -A Win32 -DBUILD_XR_ECORE=ON
```

Result: passed after running outside the sandbox so VS2022 could read the user-local Windows SDK cache.

Build command:

```bat
cmake --build build-ecore-check --config Release --target xrECore -- /m:1 /v:minimal /clp:ErrorsOnly
```

First result: failed on missing Borland `fastmath.h`.

Fix applied: added `cmake/compat/msvc/fastmath.h`, a narrow MSVC compatibility shim that includes the standard math declarations while preserving the old `sqrtf` macro behavior expected by `Editors/ECore/stdafx.h`.

Second blocker fixed: added `cmake/compat/msvc/utime.h`, a narrow MSVC compatibility shim that forwards Borland-style `<utime.h>` includes to MSVC's `<sys/utime.h>`.

Current result after the fix: build advances to the next Borland/MSVC CRT wrapper blocker:

```text
Editors\ECore\stdafx.h(38,14): error C2373: '_eof': redefinition; different type modifiers
Editors\ECore\stdafx.h(42,14): error C2373: '_access': redefinition; different type modifiers
Editors\ECore\stdafx.h(46,15): error C2373: '_lseek': redefinition; different type modifiers
Editors\ECore\stdafx.h(51,14): error C2373: '_dup': redefinition; different type modifiers
```

## Recommended Next Step

Fix the old Borland compatibility wrapper block in `Editors/ECore/stdafx.h` narrowly for MSVC, without changing runtime targets or deleting the historical wrappers. Then rebuild only `xrECore` and stop at the next real blocker.

Do not proceed to `LevelEditor.exe` until `xrECore`, `xrEProps`, `ETools`, and the Borland/VCL package boundary are clearer.
