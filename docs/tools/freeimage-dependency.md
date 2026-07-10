# FreeImage Dependency

## Current State

`xrDO_Light` uses the historical FreeImage 2 image-loading API while rebuilding
detail-object lighting data. The source snapshot contains several copies of
`FreeImage.h`, but only one import library:

- `Editor/ShaderEditor/Lib/FreeImage.lib`

MSVC does not accept that library:

```powershell
dumpbin /headers Editor\ShaderEditor\Lib\FreeImage.lib
```

reports:

```text
warning LNK4048: Invalid format file; ignored
```

The file also does not start with the COFF archive signature `!<arch>`, so it is
not a modern MSVC COFF import library. It is likely a legacy non-COFF import
library from the historical editor toolchain.

The local runtime folder contains a matching runtime candidate:

- `D:\Projects\Github\stalker-dream\FreeImage.dll`

`dumpbin /headers` identifies it as a 32-bit PE DLL, and `dumpbin /exports`
shows decorated x86 exports such as `_FreeImage_Allocate@24`.

## Generating A Local Import Library

Do not commit generated FreeImage import libraries, runtime DLLs, or third-party
binaries. Generate them into a local toolchain folder outside this repository,
for example:

```text
D:\Projects\Toolchains\freeimage-msvc-x86\lib
```

From a Visual Studio developer prompt, or from PowerShell with Visual Studio
tools discoverable, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\generate-freeimage-import-lib.ps1 `
  -DllPath D:\Projects\Github\stalker-dream\FreeImage.dll `
  -OutDir D:\Projects\Toolchains\freeimage-msvc-x86\lib
```

The helper performs the same underlying steps:

```powershell
dumpbin /exports D:\Projects\Github\stalker-dream\FreeImage.dll
```

then writes a `FreeImage.def` that preserves the decorated export names, for
example:

```def
LIBRARY "FreeImage.dll"
EXPORTS
    FreeImage_Allocate@24=_FreeImage_Allocate@24
    FreeImage_Free@4=_FreeImage_Free@4
```

Do not quote the decorated names. The public side of each alias intentionally
drops the leading underscore so MSVC `lib.exe` creates import symbols such as
`__imp__FreeImage_Free@4`, while the right-hand side still names the DLL export
`_FreeImage_Free@4`.

and runs:

```powershell
lib /def:D:\Projects\Toolchains\freeimage-msvc-x86\lib\FreeImage.def `
  /machine:x86 `
  /out:D:\Projects\Toolchains\freeimage-msvc-x86\lib\FreeImage.lib
```

The script refuses to write generated files inside the repository when run from
a Git checkout.

## CMake Usage

Point `xrDO_Light` at the generated import library explicitly:

```powershell
cmake -S . -B build-xrdolight-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_DO_LIGHT=ON `
  -DXR_LEGACY_DX_ROOT=D:\Projects\Others\DXGarbage `
  -DXR_DO_LIGHT_FREEIMAGE_LIB=D:\Projects\Toolchains\freeimage-msvc-x86\lib\FreeImage.lib
```

Alternatively, use `XR_FREEIMAGE_ROOT` with this layout:

```text
<root>/include/FreeImage.h
<root>/lib/FreeImage.lib
<root>/bin/FreeImage.dll
```

The DLL must still be available next to `xrDO_Light.exe` or on `PATH` when the
tool runs. The generated import library only satisfies the link step.

With the generated import library at
`D:\Projects\Toolchains\freeimage-msvc-x86\lib\FreeImage.lib`, the Release
`xrDO_Light` build links successfully. `dumpbin /dependents` on the resulting
`xrDO_Light.exe` confirms `FreeImage.dll` is a runtime dependency.

## Policy

- Do not vendor FreeImage into this repository.
- Do not replace the bundled historical import library with generated output.
- Do not commit `FreeImage.lib`, `FreeImage.exp`, `FreeImage.def`, or
  `FreeImage.dll`.
- Keep FreeImage paths opt-in and target-local to the SDK/tool targets that
  actually need them.
