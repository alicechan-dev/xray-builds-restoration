# Local Editor Dependency Inventory

This inventory records local dependency candidates for the historical SDK
editors. Paths are examples from one lawful research environment. They are not
CMake defaults, and none of these files belong in the repository.

## Results

| Dependency | Result | Local evidence | CMake direction |
|---|---|---|---|
| DirectX 9.0 Summer 2004 | Found | `D:\Projects\Others\DXGarbage` contains the complete Summer 2004 SDK. | Existing opt-in `XR_LEGACY_DX_ROOT`. |
| ElPack | Not found | No `ElTree.hpp`, `elpackB6.lib`, or ElPack directory under searched roots. | Existing opt-in `XR_ELPACK_ROOT`; keep editor targets blocked when absent. |
| AlexMX Borland controls | Not found | No `ExtBtn.hpp`, `MXCtrls.hpp`, `MxMenus.hpp`, or `mxPlacemnt.hpp`. The repository's `AlexMX/` tree is GemVIPM geometry code, not the control package. | Future editor-only `XR_ALEXMX_ROOT` after a lawful package is found. |
| MagicFM SDK | Incomplete | Runtime has `MagicFM.dll` and `MagicFMDLLB.dll`; repository has historical `MagicFM.lib` files, but no matching SDK headers were found. | Future `XR_MAGICFM_ROOT` only after header/library provenance and ABI are matched. |
| ColorPicker | Found | Runtime `D:\Projects\Github\stalker-dream\ColorPicker.dll`; import library `Editor/Tools/ETools/ColorPicker.lib`; source exports and callers agree on `FSColorPickerExecute`. | Existing `ETools` target already uses the historical import library. Keep runtime DLL deployment external. |
| Boost 1.30.x | Not found | Historical projects request `P:\boost_1_30_0`. | Do not substitute silently. |
| Boost 1.33.1 | Found | `D:\Projects\Toolchains\boost_1_33_1`; `boost/version.hpp` reports `BOOST_VERSION 103301`. | Existing `XRAY_BOOST_ROOT` can be used for an explicit compatibility probe. |
| Borland C++ Builder/VCL | Not found | No Borland/Embarcadero component root under the targeted installed-program paths; no `R:` drive exists. | A future toolchain/package root needs separate design; do not fake VCL headers. |

## Search Scope

The targeted search covered:

- `D:\Projects`, including `D:\Projects\Others` and
  `D:\Projects\Github`;
- likely Borland directories under `C:\`, `C:\Program Files`, and
  `C:\Program Files (x86)`;
- `R:\Borland` when available.

Exact searches covered `ElTree.hpp`, `elpackB6.lib`, MagicFM and ColorPicker
libraries/DLLs, named ElPack/AlexMX/MagicFM/Boost directories, and the Borland
control headers referenced by the editor projects.

## Safety and Use

- Do not copy third-party headers, libraries, packages, or runtime DLLs into
  this repository.
- Do not add machine-specific absolute paths as committed defaults.
- Keep every external root opt-in and limited to SDK/editor targets.
- Treat a matching filename as a candidate, not proof of compiler or ABI
  compatibility.
- Keep extracted game data and runtime archives outside the repository.

## Configure Diagnostics

When `BUILD_XR_ECORE`, `BUILD_XR_EPROPS`, or `BUILD_XR_LEVEL_EDITOR` is
enabled, CMake prints one SDK/editor dependency summary. It lists the enabled
targets and reports:

- whether `XR_ELPACK_ROOT` is unset, valid, or invalid;
- `XR_LEGACY_DX_ROOT` status for targets that use DirectX;
- `XRAY_BOOST_ROOT` status when LevelEditor is enabled.

Missing or invalid ElPack produces a warning but does not fail generation.
The warning states that editor builds may stop at `ElTree.hpp`. A configured
ElPack root is valid only when either `Code/Source/ElTree.hpp` or
`ElTree.hpp` exists.

No equivalent AlexMX or MagicFM cache variables are declared yet because no
complete, ABI-matched local SDK packages were found.

## Next Probe

The strongest available next dependency probe is Boost 1.33.1 through
`XRAY_BOOST_ROOT`. It is newer than the requested 1.30.x and must be tested
against one isolated SDK target rather than enabled globally.

ElPack remains the blocking dependency for `xrECore`, `xrEProps`, and the
LevelEditor shell.
