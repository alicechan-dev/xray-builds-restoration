# xrDO_Light CMake Shell

## Scope

`BUILD_XR_DO_LIGHT` adds an experimental `xrDO_Light` executable target from
`xrLC/xrDO_Light`. This is a non-GUI level-compiler helper that recomputes
detail-object lighting data and shows its own small Win32 progress dialog.

The target does not use or fake ElPack, VCL, AlexMX, or MagicFM dependencies,
and it does not pull in the full `xrLC` compiler.

## Historical Project

The source is represented by:

- `xrLC/xrDO_Light/xrDO_Light.vcproj`
- `xrLC/xrDO_Light/xrDO_Light.dsp`
- historical output: `xrDO_Light.exe`
- project type: Visual C++ 7.1 Win32 executable

The historical project compiles only the helper sources listed in the VC2003
project. The copied legacy kernel/math files in the same folder are retained as
reference files, but they are not compiled by this CMake shell because the
original VC2003 project did not compile them.

## Dependencies

- `xrCore`
- `xrCDB`
- `xrHemisphere`
- DirectX 9 `d3dx9.h` / `d3dx9.lib`
- historical FreeImage import library:
  `Editor/ShaderEditor/Lib/FreeImage.lib`, or an MSVC-compatible replacement
  supplied through `XR_FREEIMAGE_ROOT` / `XR_DO_LIGHT_FREEIMAGE_LIB`
- Win32 libraries: `comctl32`, `winmm`, and standard system libraries

The target uses `XR_LEGACY_DX_ROOT` first, then the existing DirectX SDK
selection (`XRAY_DXSDK_ROOT`, `DXSDK_DIR`, `D3DXSDK_DIR`, or
`DIRECTX_SDK_DIR`). The old source contains absolute `X:\...` pragma libraries;
the CMake target links local targets/import libraries and suppresses only those
stale default-library requests.

The bundled `Editor/ShaderEditor/Lib/FreeImage.lib` is the only FreeImage import
library currently found in this source snapshot. The CMake target exposes
`XR_FREEIMAGE_ROOT` and `XR_DO_LIGHT_FREEIMAGE_LIB` so a lawful MSVC-compatible
FreeImage import library can be supplied later without changing runtime targets
or vendoring binaries.

See [FreeImage Dependency](freeimage-dependency.md) for the current
classification and an optional local import-library generation workflow.

Expected `XR_FREEIMAGE_ROOT` layouts include:

- `<root>/include/FreeImage.h`
- `<root>/lib/FreeImage.lib`
- optional `<root>/bin/FreeImage.dll`

Direct legacy layouts are also accepted:

- `<root>/FreeImage.h`
- `<root>/FreeImage.lib`
- optional `<root>/FreeImage.dll`

## Build Probe

```powershell
cmake -S . -B build-xrdolight-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_DO_LIGHT=ON `
  -DXR_LEGACY_DX_ROOT=D:\Projects\Others\DXGarbage `
  -DXR_DO_LIGHT_FREEIMAGE_LIB=D:\Projects\Toolchains\freeimage-msvc-x86\lib\FreeImage.lib

cmake --build build-xrdolight-check --config Release `
  --target xrDO_Light -- /m:1 /v:minimal /clp:ErrorsOnly
```

## Verification

After a successful build, inspect the executable shape with Visual Studio
`dumpbin`:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /headers build-xrdolight-check\bin\xrDO_Light.exe

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /dependents build-xrdolight-check\bin\xrDO_Light.exe
```

## Current Status

This shell is intentionally narrow. It restores the historical helper target
shape and dependency chain without changing the larger level compiler or editor
GUI targets.

Current probe result:

- configure passes with `BUILD_XR_DO_LIGHT=ON`;
- the source gets past the local Win32 process declaration blocker by defining
  the XP-era Win32 API level target-locally and including `windows.h` in the
  helper PCH;
- the source gets past the Windows `min` macro collision by defining `NOMINMAX`
  in the helper PCH;
- `D:\Projects\Github\stalker-dream\FreeImage.dll` exists and is a 32-bit PE
  DLL exporting `_FreeImage_*@N` functions;
- a local import library can be generated outside the repo from that DLL and
  passed through `XR_DO_LIGHT_FREEIMAGE_LIB`;
- Release `xrDO_Light` builds when pointed at that generated MSVC x86 import
  library;
- the verified output is `build-xrdolight-freeimage-check\bin\xrDO_Light.exe`,
  an x86 / 32-bit Windows GUI executable;
- `dumpbin /dependents` confirms the executable depends on `FreeImage.dll` at
  runtime, along with `xrCore.dll`, `xrCDB.dll`, Win32 system DLLs, and the MSVC
  runtime.

At this checkpoint the target is considered build-restored only with the
explicit external FreeImage import-library input. Do not commit the generated
`.lib`/`.def`, `FreeImage.dll`, build directory, logs, or tool output data.
