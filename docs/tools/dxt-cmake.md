# DXT CMake Shell

## Scope

`BUILD_XR_DXT` adds an experimental `DXT` shared-library target from
`Editor/Tools/DXT`. This is a non-GUI SDK/tool probe for the historical texture
compression helper DLL.

The target does not use or fake ElPack, VCL, AlexMX, or MagicFM dependencies.

## Historical Project

The source is represented by:

- `Editor/Tools/DXT/DXT.dsp`
- historical output: `x:\dxt.dll`
- project type: Visual C++ 6.0 Win32 DLL

The project marks `nvdxt.cpp` as excluded from build and links the bundled
historical `nvDXTlib.lib` instead. The CMake target preserves that split rather
than rebuilding NVIDIA's converter source directly.

Because `nvDXTlib.lib` is a VC6-era static library, the target keeps its CRT
compatibility local:

- disables SAFESEH for this DLL only;
- ignores obsolete `libcp.lib` / `LIBC.lib` default-library requests;
- links MSVC's `legacy_stdio_definitions.lib`;
- provides a tiny local `_iob` compatibility symbol used by the library's old
  `fprintf(stderr, ...)` diagnostic paths.

The DLL exports the texture compression entry point from `DXT.cpp`:

- `DXTCompress`

## Dependencies

`DXT.cpp` includes the legacy DirectX header `d3dx.h`. The CMake target uses
target-local DirectX include paths only:

- `XR_LEGACY_DX_ROOT`, if set;
- otherwise the existing global DirectX SDK selection
  (`XRAY_DXSDK_ROOT`, `DXSDK_DIR`, `D3DXSDK_DIR`, or `DIRECTX_SDK_DIR`).

Do not add DirectX include paths globally for this target.

## Build Probe

```powershell
cmake -S . -B build-dxt-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_DXT=ON

cmake --build build-dxt-check --config Release `
  --target DXT -- /m:1 /v:minimal /clp:ErrorsOnly
```

If the active Windows SDK does not provide `d3dx.h`, configure with a lawful
legacy DirectX SDK root:

```powershell
cmake -S . -B build-dxt-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_DXT=ON `
  -DXR_LEGACY_DX_ROOT=D:\Projects\Others\DXGarbage
```

## Verification

After a successful build, inspect the DLL shape with Visual Studio `dumpbin`:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /headers build-dxt-check\bin\DXT.dll

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /exports build-dxt-check\bin\DXT.dll

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /dependents build-dxt-check\bin\DXT.dll
```

## Current Status

The shell is intentionally narrow: it represents the historical DLL target and
uses the repository's existing `Editor/Tools/DXT` source snapshot without
inventing a new DDS/DXT implementation. Validate `DXTCompress` only with lawful
synthetic or user-owned texture data, and do not commit converted assets.

The Release build currently produces:

- `build-dxt-check/bin/DXT.dll`
- `build-dxt-check/lib/DXT.lib`
- `build-dxt-check/lib/DXT.exp`

Current verification result:

- PE machine type: `14C machine (x86)` / 32-bit DLL.
- exports:
  - `DXTCompress`
- dependents:
  - `USER32.dll`
  - `VCRUNTIME140.dll`
  - Universal CRT API-set DLLs for heap/stdio/runtime/math/utility
  - `KERNEL32.dll`

This checkpoint verifies the helper DLL shape and export, not texture-output
compatibility. Do not commit build folders, logs, converted textures, or other
generated test data.
