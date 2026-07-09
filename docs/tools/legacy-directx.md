# Legacy DirectX SDK Support

## Inventory Result

The local `D:\Projects\Others\DXGarbage` tree is a complete DirectX 9.0 SDK
Update (Summer 2004), Final Release. Its readme identifies the included runtime
as DirectX 9.0c and documents Visual Studio .NET 2003 support.

This provenance matches the historical LevelEditor include path recorded in
the project files as `R:\SDK\DirectX9.0_sum_2004\Include`.

The local tree contains:

- `Include/` with `d3d9.h`, `d3dx9.h`, `dinput.h`, `ddraw.h`, `dsound.h`,
  `dplay8.h`, and `dxerr9.h`;
- Win32 libraries directly under `Lib/`, including `d3d9.lib`, `d3dx9.lib`,
  `d3dx9d.lib`, `dinput.lib`, `dinput8.lib`, `ddraw.lib`, `dsound.lib`,
  `dxguid.lib`, and `DxErr9.lib`;
- x64 libraries under `Lib/x64/`;
- developer runtime, redistributable, documentation, samples, and utilities.

No part of that SDK is stored in this repository.

## Intended Use

`XR_LEGACY_DX_ROOT` is an optional CMake cache path for restored SDK/editor
targets. It is empty by default and currently applies only to `ETools` and
`xrECore`. Those targets continue to use the repository's existing DirectX
selection when this option is unset.

Example:

```powershell
cmake -S . -B build-sdk-check -G "Visual Studio 17 2022" -A Win32 `
  -DXR_LEGACY_DX_ROOT=D:\Projects\Others\DXGarbage
```

The expected layout includes:

```text
<root>/Include/d3dx9.h
<root>/Include/dsound.h
<root>/Lib/d3dx9.lib
<root>/Lib/dsound.lib
<root>/Lib/dxguid.lib
```

Runtime targets do not read `XR_LEGACY_DX_ROOT`. Do not put this SDK ahead of
modern headers globally: its libraries and headers carry Visual Studio 2003-era
CRT, linker, and compatibility assumptions. Do not copy or commit the SDK.
