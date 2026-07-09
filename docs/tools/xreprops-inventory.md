# xrEProps CMake Port Inventory

## Summary

`Editors/xrEProps` is the historical editor property/UI DLL. The Borland C++
Builder 6 project `xrEPropsB.bpr` outputs `x:\xrEPropsB.dll` and links VCL
runtime packages plus `elpackB6.lib`.

An experimental opt-in CMake target named `xrEProps` now exists behind
`BUILD_XR_EPROPS`. It is a full-source build probe, not a portable property
library: no VCL forms or ElPack-dependent units are silently excluded.

## Historical Project

| Item | Evidence |
|---|---|
| Project type | Borland C++ Builder 6 DLL |
| Output | `x:\xrEPropsB.dll` |
| Main source | `xrEPropsB.bpf` |
| Runtime packages | `vcl.bpi`, `rtl.bpi`, `vclx.bpi`, `bcbsmp.bpi` |
| Libraries | `editor.lib`, `elpackB6.lib` |
| Defines | `_EDITOR`, `XR_EPROPS_EXPORTS` |

The project contains seven `.dfm` form units:

- `ChoseForm`
- `ItemDialog`
- `ItemList`
- `NumericVector`
- `PropertiesList`
- `ShaderFunction`
- `TextForm`

## Dependencies

- `xrCore`;
- `ETools` / `FSColorPickerExecute`;
- Borland VCL runtime and form support;
- ElPack tree, editors, themed-control, button, and status-bar components;
- AlexMX controls such as `ExtBtn`, `MXCtrls`, `MxMenus`, and placement tools;
- Borland form resources and `USEFORM` registration.

The old project mentions `xrSoundB.lib`, but no direct xrSound symbol use was
found during this scaffold pass, so the modern target does not pull that
runtime library into the probe.

## CMake Target

```powershell
cmake -S . -B build-xreprops-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_EPROPS=ON

cmake --build build-xreprops-check --config Release `
  --target xrEProps -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configuration succeeds. With `XR_ELPACK_ROOT` unset, CMake reports the expected
dependency boundary and compilation stops at:

```text
Editors/xrEProps/FolderLib.h(6): error C1083:
Cannot open include file: 'ElTree.hpp'
```

This is the correct first blocker. `FolderLib`, item lists, property lists, and
public helper APIs use real `TElTree` and `TElTreeItem` behavior. ElPack is
required, not an optional form-only feature.

## Next Safe Step

Obtain and inventory a lawful local ElPack/Borland package tree through
`XR_ELPACK_ROOT`, then determine whether its generated C++ headers can be
consumed by MSVC at all. Do not create substitute widgets or exclude the core
property/list implementation to obtain a nominal DLL.

## Checkpoint

The xrEProps CMake scaffold faithfully retains the historical DLL source and
form set, configures successfully, and stops at `FolderLib.h` requiring
`ElTree.hpp`. That is the intended checkpoint: property and item-list APIs use
real `TElTree`/`TElTreeItem` behavior throughout.

Continuing requires ElPack and Borland/VCL plus the related AlexMX component
set. Do not fake those types, strip out the property UI to claim a successful
DLL, or vendor third-party packages. Work may resume after dependency
acquisition, or remain paused while archive/runtime tasks continue.
