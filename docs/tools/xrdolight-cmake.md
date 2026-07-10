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
  supplied through `XR_DO_LIGHT_FREEIMAGE_LIB`
- Win32 libraries: `comctl32`, `winmm`, and standard system libraries

The target uses `XR_LEGACY_DX_ROOT` first, then the existing DirectX SDK
selection (`XRAY_DXSDK_ROOT`, `DXSDK_DIR`, `D3DXSDK_DIR`, or
`DIRECTX_SDK_DIR`). The old source contains absolute `X:\...` pragma libraries;
the CMake target links local targets/import libraries and suppresses only those
stale default-library requests.

The bundled `Editor/ShaderEditor/Lib/FreeImage.lib` is the only FreeImage import
library currently found in this source snapshot. The CMake target exposes
`XR_DO_LIGHT_FREEIMAGE_LIB` so a lawful MSVC-compatible FreeImage import library
can be supplied later without changing runtime targets or vendoring binaries.

## Build Probe

```powershell
cmake -S . -B build-xrdolight-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_DO_LIGHT=ON `
  -DXR_LEGACY_DX_ROOT=D:\Projects\Others\DXGarbage

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
- Release linking currently stops at `LNK1136` because the in-repo
  `Editor/ShaderEditor/Lib/FreeImage.lib` is not accepted by MSVC as a valid
  COFF import library.
