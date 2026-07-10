# xrHemisphere CMake Shell

## Scope

`BUILD_XR_HEMISPHERE` adds an experimental `xrHemisphere` shared-library target
from `xrLC/xrHemisphere`. This is a small non-GUI level-compiler helper DLL for
hemisphere sample generation.

The target does not use or fake ElPack, VCL, AlexMX, or MagicFM dependencies,
and it does not pull in the full `xrLC` compiler.

## Historical Project

The source is represented by:

- `xrLC/xrHemisphere/xrHemisphere.vcproj`
- historical output: `x:\xrHemisphere.dll`
- project type: Visual C++ 7.1 Win32 DLL

The DLL exposes the C entry point declared in `xrHemisphere.h`:

- `xrHemisphereBuild`

The historical project used the default `__stdcall` calling convention, so the
CMake target keeps `/Gz`.

## Build Probe

```powershell
cmake -S . -B build-xrhemisphere-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_HEMISPHERE=ON

cmake --build build-xrhemisphere-check --config Release `
  --target xrHemisphere -- /m:1 /v:minimal /clp:ErrorsOnly
```

## Verification

After a successful build, inspect the DLL shape with Visual Studio `dumpbin`:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /headers build-xrhemisphere-check\bin\xrHemisphere.dll

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /exports build-xrhemisphere-check\bin\xrHemisphere.dll

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /dependents build-xrhemisphere-check\bin\xrHemisphere.dll
```

## Current Status

The target is intentionally narrow. It restores the helper DLL shape and leaves
the larger `xrLC`, `xrDO_Light`, and `xrAI` compiler/toolchain restoration for
separate dependency-focused passes.

The Release build currently produces:

- `build-xrhemisphere-check/bin/xrHemisphere.dll`
- `build-xrhemisphere-check/lib/xrHemisphere.lib`
- `build-xrhemisphere-check/lib/xrHemisphere.exp`

Current verification result:

- PE machine type: `14C machine (x86)` / 32-bit DLL.
- exports:
  - `_xrHemisphereBuild@24`
- dependents:
  - `VCRUNTIME140.dll`
  - Universal CRT API-set DLLs for math/runtime
  - `KERNEL32.dll`

The decorated export name matches the target's historical `__stdcall` calling
convention.

This checkpoint verifies the helper DLL shape and export. Do not commit the
build directory, generated import library, logs, or generated sample data.
