# ShaderEditor CMake Shell

## Scope

`BUILD_XR_SHADER_EDITOR` adds an experimental `ShaderEditor` Win32 executable
target from `Editors/ShaderEditor`. This is a dependency and source-ownership
shell, not a VCL GUI port.

The target does not fake ElPack, AlexMX, MagicFM SDK headers, Borland/VCL
classes, form behavior, or package libraries.

## Canonical Source

`Editors/ShaderEditor` is the canonical first target:

- it is in the active `Editors` tree;
- its project is Borland C++ Builder 6 (`BCB.06.00`);
- its historical output is `x:\ShaderEditor.exe`;
- its source list is explicit in `Editors/ShaderEditor/ShaderEditor.bpr`;
- `Editor/ShaderEditor` is an older generation that also contains a historical
  `Lib/MagicFM.lib` import library.

The CMake shell imports only `CCompiler` entries from `ShaderEditor.bpr` and
keeps `.dfm`, `.bpr`, `.todo`, and `.res` files visible as metadata.

The active `.bpr` references `Splash.cpp` and `resource.res`, but those files
are not present in this source snapshot. `ShaderEditor.cpp` still includes the
matching `splash.h`, so this is a real source snapshot gap, not a CMake
omission.

## Dependency Chain

Enabling `BUILD_XR_SHADER_EDITOR` also enables:

- `ETools`;
- `xrECore`;
- `xrEProps`.

The shell also declares the historical engine/editor libraries used by the
tool: `xrCore`, `xrCDB`, `xrParticles`, and `xrSound`. DirectX remains
target-local through `XR_LEGACY_DX_ROOT`.

The active Borland project references:

- ElPack (`ElTree.hpp`, `ElTreeAdvEdit.hpp`, `ElXPThemedControl.hpp`);
- AlexMX controls (`ExtBtn.hpp`, `mxPlacemnt.hpp`);
- Borland/VCL forms and frames (`Application`, `TForm`, `TFrame`, `.dfm`);
- MagicFM SDK include path `P:\MagicFM_Borland\Sdk\Include`;
- `MagicFMDLLB.lib` through the shared editor-style `stdafx.h`;
- old Boost/Luabind paths in history entries, but not as proven active source
  requirements yet.

The repository contains a historical `Editor/ShaderEditor/Lib/MagicFM.lib`.
The CMake shell links it when present, but this does not replace missing
MagicFM SDK headers or prove compiler/linker ABI compatibility.

## Build Probe

```powershell
cmake -S . -B build-shadereditor-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_SHADER_EDITOR=ON

cmake --build build-shadereditor-check --config Release `
  --target ShaderEditor -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configuration is expected to pass. With `XR_ELPACK_ROOT` unset, dependency-aware
builds may stop first in the required `xrECore` target at the known ElPack
boundary:

```text
Editors/ECore/Engine/GameMtlLib.h(...): fatal error C1083:
Cannot open include file: 'ElTree.hpp'
```

A direct ShaderEditor compile probe gets past the local Borland CRT alias
blocker, DirectSound `WAVEFORMATEX` include-order issue, and the audited
non-GUI VCL value shims. It then stops at the missing `splash.h` form from the
active `.bpr` plus real GUI/package dependencies such as `ElTree.hpp`,
`Classes.hpp`, and `RenderWindow.hpp`.

Do not fake those blockers. `splash.h`/`Splash.cpp` should be resolved by
locating the matching historical source, while `ElTree.hpp`, `Classes.hpp`, and
`RenderWindow.hpp` belong to the ElPack/Borland/VCL/editor package boundary.

## Next Honest Step

Obtain and validate lawful ElPack, Borland/VCL, AlexMX, and MagicFM SDK
dependencies before attempting a real GUI compile. If those packages remain
unavailable, pause ShaderEditor and continue with archive tools, format
validators, compiler inventories, or runtime restoration.
