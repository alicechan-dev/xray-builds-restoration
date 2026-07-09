# ActorEditor CMake Shell

## Scope

`BUILD_XR_ACTOR_EDITOR` adds an experimental `ActorEditor` Win32 executable
target from `Editors/ActorEditor`. This is a dependency and source-ownership
shell, not a VCL GUI port.

The target does not fake ElPack, AlexMX, MagicFM SDK headers, Borland/VCL
classes, form behavior, or package libraries.

## Canonical Source

The canonical first source folder is:

- `Editors/ActorEditor`

This folder has the active Borland C++ Builder 6 project:

- `Editors/ActorEditor/ActorEditor.bpr`
- output: `x:\ActorEditor.exe`
- version tag: `BCB.06.00`

Older generations also exist:

- `Editor/ActorEditor`
- `Editors/!old/ActorEditor`

The active `.bpr` references `Splash.cpp`, `main.cpp`, `TopBar.cpp`,
`BottomBar.cpp`, `LeftBar.cpp`, `BonePart.cpp`, `ClipEditor.cpp`,
`KeyBar.cpp`, actor editor implementation units, and the `ActorEditor.res` /
`resource.res` resources. The active folder contains the referenced form and
resource metadata.

The CMake shell imports only `CCompiler` entries from `ActorEditor.bpr` and
keeps `.dfm`, `.ddp`, `.bpr`, `.todo`, and `.res` files visible as metadata.

## Dependency Chain

Enabling `BUILD_XR_ACTOR_EDITOR` also enables:

- `ETools`;
- `xrECore`;
- `xrEProps`.

The shell also declares target-local dependencies on:

- `xrCore`;
- `xrCDB`;
- `xrParticles`;
- `xrSound`;
- DirectX 9 / D3DX.

The active Borland project references:

- Borland/VCL headers and forms (`Classes.hpp`, `Application`, `TForm`,
  `TFrame`, `.dfm`);
- ElPack (`ElTree.hpp`, `elpackB6.lib`);
- AlexMX controls (`ExtBtn.hpp`, `mxPlacemnt.hpp`);
- MagicFM SDK include path (`P:\MagicFM_Borland\Sdk\Include`);
- `MagicFMDLLB.lib`, which is not present in this repository snapshot.

Existing MagicFM runtime DLLs or unrelated import libraries do not replace the
missing MagicFM SDK headers and do not prove ABI compatibility.

## Build Probe

```powershell
cmake -S . -B build-actoreditor-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_ACTOR_EDITOR=ON

cmake --build build-actoreditor-check --config Release `
  --target ActorEditor -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configure passes and prints the shared editor dependency summary. If
`XR_ELPACK_ROOT` is unset, configure warns that ElPack is required and builds
may stop at `ElTree.hpp`.

The dependency-aware build may stop through required editor dependencies such
as `xrECore` before compiling ActorEditor itself. A direct ActorEditor compile
probe gets past the local Borland CRT alias blocker, DirectSound
`WAVEFORMATEX` include-order issue, and the audited non-GUI VCL value shims.
It then stops at real GUI/package boundaries:

```text
Editors/ActorEditor/main.h(...): fatal error C1083:
Cannot open include file: 'Classes.hpp'

Editors/ECore/Engine/GameMtlLib.h(...): fatal error C1083:
Cannot open include file: 'ElTree.hpp'

Editors/ActorEditor/ClipEditor.h(...): fatal error C1083:
Cannot open include file: 'mxPlacemnt.hpp'

Editors/ActorEditor/KeyBar.h(...): fatal error C1083:
Cannot open include file: 'Gradient.hpp'
```

Do not fake these blockers. `Classes.hpp` and form/application behavior belong
to the Borland/VCL boundary, `ElTree.hpp` belongs to the ElPack boundary, and
`mxPlacemnt.hpp` / `Gradient.hpp` belong to missing editor control packages.
