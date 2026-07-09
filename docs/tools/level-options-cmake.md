# LevelOptions CMake Shell

## Scope

`BUILD_XR_LEVEL_OPTIONS` adds an experimental `LevelOptions` DLL target from
`Editors/LevelOptions`. The output name stays historical: `xrLC_Options`.

This is a dependency and source-ownership shell for the old `xrLC` build
options dialog. It is not a VCL GUI port, and it does not fake ElPack, AlexMX,
MagicFM, Borland package behavior, or form classes.

## Historical Project

The source is represented by `Editors/LevelOptions/xrLC_Options.bpr`, a Borland
C++ Builder 6 DLL project. The project lists:

- `stdafx.cpp`;
- `Editor/SceneProperties.cpp`;
- `xrLC_OptionsEntry.cpp`, which is not present in this snapshot;
- `Editor/SceneProperties.dfm`.

CMake keeps the `.bpr`, `.bpf`, and `.dfm` files as project metadata and warns
that `xrLC_OptionsEntry.cpp` is missing.

## Dependency Chain

Enabling `BUILD_XR_LEVEL_OPTIONS` also enables the existing editor dependency
chain:

- `ETools`;
- `xrECore`;
- `xrEProps`.

The shell declares the historical runtime/editor libraries used by the dialog:
`xrCore`, `xrCDB`, `xrParticles`, and `xrSound`. DirectX remains target-local
through `XR_LEGACY_DX_ROOT`.

The actual dialog source also requires:

- ElPack (`ElTree.hpp`, `ElXPThemedControl.hpp`, `TElTree`, `TElTreeItem`);
- AlexMX controls (`ExtBtn.hpp`, `multi_color.hpp`, `multi_edit.hpp`,
  `mxPlacemnt.hpp`);
- Borland/VCL headers such as `Forms.hpp`, `Controls.hpp`, `Classes.hpp`;
- ColorPicker and editor UI headers;
- likely MagicFM/Borland package libraries through the shared editor stack.

## Build Probe

```powershell
cmake -S . -B build-sdk-next-editor-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_LEVEL_OPTIONS=ON

cmake --build build-sdk-next-editor-check --config Release `
  --target LevelOptions -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configuration is expected to pass. A dependency-aware target build may spend
time walking the required editor chain first, where `xrECore` is already known
to stop at the ElPack boundary when `XR_ELPACK_ROOT` is unset:

```text
Editors/ECore/Engine/GameMtlLib.h(...): fatal error C1083:
Cannot open include file: 'ElTree.hpp'
```

This is the same honest blocker as `xrECore`, `xrEProps`, and LevelEditor.

A direct LevelOptions compile probe, using MSBuild with project references
disabled, now gets past the local Borland/MSVC CRT wrapper aliases,
DirectSound wave-format include ordering, and the existing non-GUI VCL value
shims. It stops at the same package boundary in the LevelOptions form itself:

```text
Editors/LevelOptions/Editor/SceneProperties.h(7): fatal error C1083:
Cannot open include file: 'ElTree.hpp'
```

That header owns the actual GUI dependencies for this dialog:
`ElTree.hpp`, `ElXPThemedControl.hpp`, AlexMX control headers, VCL headers, and
`.dfm` form behavior.

## Next Honest Step

Do not patch around the missing GUI packages. Locate lawful ElPack,
Borland/VCL, AlexMX, and MagicFM SDK dependencies, then use the editor-only
cache roots for target-local probes. If those packages remain unavailable,
prefer archive tools, format validators, or runtime restoration work.

## Checkpoint

The LevelOptions CMake shell is complete for this dependency-discovery phase:
the historical source/form ownership is represented, configuration succeeds,
and direct compilation reaches the real ElPack/VCL UI boundary. This does not
mean `xrLC_Options.dll` builds or that its GUI has been ported.

Do not fake `ElTree.hpp`, AlexMX controls, VCL form classes, or the missing
entry unit. Resume only after lawful, compatible editor packages are available.
