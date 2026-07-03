# ETools CMake Port Inventory

This document records the first CMake restoration pass for `Editors/Tools/ETools`, a low-level SDK/editor helper used by the historical LevelEditor toolchain.

## Summary

`Editors/Tools/ETools` is a Visual Studio .NET 2003 (`.vcproj`) Win32 DLL project, not a Borland/VCL GUI project. It is a better early SDK dependency target than `LevelEditor.exe` because it has no `.dfm` forms and `UseOfMFC="0"` in the old project file.

A modern opt-in CMake target named `ETools` now exists. It is intentionally partial: it builds the non-QSlim DLL wrapper source set and excludes the VIPM/progressive-mesh sources that require the old `x:/xrQSlim.lib` dependency until `xrQSlim` has its own CMake target.

Configure succeeds with `BUILD_XR_ETOOLS=ON`. The current build probe compiles and reaches link, then fails on the first remaining editor-tool dependency: `CreateDX.cpp` imports `FSColorPickerDoModal`, but `ColorPicker.lib` is not present beside `Editors/Tools/ETools/ETools.vcproj`.

## Project Files Found

| Path | Type | Likely output | Notes |
|---|---|---|---|
| `Editors/Tools/ETools/ETools.vcproj` | VS2003 Win32 DLL project | `x:\ETools.dll`, `x:\ETools.lib` | Main project converted to CMake scaffold. |
| `Editors/Tools/ETools/tools.sln` | VS2003 solution | Mixed tool solution | Paths inside the solution point to sibling projects as if the solution belonged one directory higher. It includes DXT, LWO, Gauss, xrProgressive, ETools, and xrHemisphere. |
| `Editor/Tools/ETools/CreateDX.dsp` | Older VC6-style project | Unknown helper output | Older generation contains `ColorPicker.lib`, but it was not linked into the current `Editors/Tools/ETools` target to avoid silently mixing generations. |

## CMake Target

| Field | Value |
|---|---|
| Target name | `ETools` |
| Opt-in option | `BUILD_XR_ETOOLS` |
| Target type | `SHARED` DLL |
| Added from | `tools/CMakeLists.txt` via `add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/Tools/ETools" ...)` |
| Output directory | Existing global `XRAY_RUNTIME_DIR` / `XRAY_LIBRARY_DIR` layout |
| Runtime-heavy targets touched | None |

## Included Source Set

The initial CMake target includes:

* `CreateDX.cpp`
* `ETools.cpp`
* `cmake/compat/msvc/d3dx9_snprintf_compat.cpp`
* public/private headers for IDE visibility

## Excluded Source Set

The following old project sources are intentionally excluded for now:

* `object.cpp`
* `object_sliding.cpp`
* `optimize_vertex_order.cpp`
* `PropSlimTools.cpp`

Reason: these form the VIPM/progressive-mesh path and depend on `MxQMetric.h`, `QSMesh`, `QSContraction`, and the historical `x:/xrQSlim.lib`. Restore `xrQSlim` as a CMake target before enabling these sources.

## Dependency Map

| Dependency | Evidence | Status |
|---|---|---|
| `xrCore` | `stdafx.h` includes `<xrCore.h>` and the old project linked `x:/xrCore.lib`. | Linked through existing CMake target `xrCore`. Historical `x:/xrCore.lib` pragma is ignored in this target. |
| DirectX 9 / D3DX | `stdafx.h` includes `d3dx9.h`; `CreateDX.cpp` wraps many D3DX calls. | Uses existing DirectX SDK discovery variables and links `d3dx9`. |
| `dxerr9` | `CreateDX.cpp` includes `dxerr9.h`; old project linked `dxerr9.lib`. | Uses local compatibility header and ignores `dxerr9.lib`, matching other modern targets. |
| ColorPicker | `ETools.vcproj` references `ColorPicker.lib`; `CreateDX.cpp` imports `FSColorPickerDoModal`. | First build blocker: missing beside `Editors/Tools/ETools`. Older `Editor/Tools/ETools/ColorPicker.lib` exists but was not mixed in. |
| `xrQSlim` | `stdafx.h` had `x:/xrQSlim.lib`; VIPM sources include `MxQMetric.h` and use QSlim types. | Excluded pending a separate `xrQSlim` CMake port. |
| MFC/VCL | `UseOfMFC="0"`; no `.dfm` or VCL forms in this folder. | Not a blocker for this target. |

## Build Probe

Configure command:

```bat
cmake -S . -B build-etools-check -G "Visual Studio 17 2022" -A Win32 -DBUILD_XR_ETOOLS=ON
```

Result: passed after running outside the sandbox so VS2022 could read the user-local Windows SDK cache.

Build command:

```bat
cmake --build build-etools-check --config Release --target ETools -- /m:1 /v:minimal /clp:ErrorsOnly
```

Result: failed at link with:

```text
CreateDX.obj : error LNK2019: unresolved external symbol "__declspec(dllimport) bool __stdcall FSColorPickerDoModal(unsigned int *,unsigned int *,int)"
```

## First Blocker

`CreateDX.cpp` exports `FSColorPickerExecute` by forwarding to imported `FSColorPickerDoModal`. The current `Editors/Tools/ETools` folder does not contain the old `ColorPicker.lib` that `ETools.vcproj` lists.

The older `Editor/Tools/ETools` tree does contain a small `ColorPicker.lib`, but this pass intentionally avoids linking that cross-generation binary until provenance and compatibility are checked.

## Recommended Next Step

Pick one of these narrow follow-ups:

1. Inventory the older `Editor/Tools/ETools/ColorPicker.lib` and any source/project that produced it, then decide whether it is safe to use as an imported library for this target.
2. Split `CreateDX.cpp` behind a CMake option and build an `ETools` core DLL containing only ray-triangle helpers while D3DX/color picker integration remains disabled.
3. Restore `xrQSlim` as a separate CMake static library, then re-enable the excluded VIPM/progressive-mesh sources.

Do not proceed to `LevelEditor.exe` until `ETools`, `xrQSlim`, `ECore`, and `xrEProps` boundaries are clearer.
