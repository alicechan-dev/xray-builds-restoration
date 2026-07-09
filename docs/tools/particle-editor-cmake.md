# ParticleEditor CMake Shell

## Scope

`BUILD_XR_PARTICLE_EDITOR` adds an experimental `ParticleEditor` Win32
executable target from `Editors/ParticleEditor`. This is a dependency and
source-ownership shell, not a VCL GUI port.

The target does not fake ElPack, AlexMX, MagicFM SDK headers, Borland/VCL
classes, form behavior, or package libraries.

## Canonical Source

The canonical first source folder is:

- `Editors/ParticleEditor`

This folder has the active Borland C++ Builder 6 project:

- `Editors/ParticleEditor/ParticleEditor.bpr`
- output: `x:\ParticleEditor.exe`
- version tag: `BCB.06.00`

Older generations also exist:

- `Editor/ParticleEditor`
- `Editors/!old/ParticleEditor`

The active `.bpr` references `Splash.cpp`, `main.cpp`, `BottomBar.cpp`,
`TopBar.cpp`, `LeftBar.cpp`, particle editor implementation units, and the
`ParticleEditor.res` / `resource.res` resources. Unlike the ShaderEditor
snapshot, the active ParticleEditor folder contains the referenced splash form
and resources.

The CMake shell imports only `CCompiler` entries from `ParticleEditor.bpr` and
keeps `.dfm`, `.ddp`, `.bpr`, `.todo`, and `.res` files visible as metadata.

## Dependency Chain

Enabling `BUILD_XR_PARTICLE_EDITOR` also enables:

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
- ElPack (`ElTree.hpp`, `ElTreeAdvEdit.hpp`);
- AlexMX controls (`ExtBtn.hpp`, `mxPlacemnt.hpp`);
- MagicFM SDK include path (`P:\MagicFM_Borland\Sdk\Include`);
- `MagicFMDLLB.lib`, which is not present in this repository snapshot.

Existing MagicFM runtime DLLs or unrelated import libraries do not replace the
missing MagicFM SDK headers and do not prove ABI compatibility.

## Build Probe

```powershell
cmake -S . -B build-particleeditor-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_PARTICLE_EDITOR=ON

cmake --build build-particleeditor-check --config Release `
  --target ParticleEditor -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configure passes and prints the shared editor dependency summary. If
`XR_ELPACK_ROOT` is unset, configure warns that ElPack is required and builds
may stop at `ElTree.hpp`.

The dependency-aware build probe did not finish before the local timeout in the
current check. A direct ParticleEditor compile probe gets past the local
Borland CRT alias blocker, DirectSound `WAVEFORMATEX` include-order issue, and
the audited non-GUI VCL value shims. It then stops at real GUI/package
boundaries:

```text
Editors/ParticleEditor/splash.h(...): fatal error C1083:
Cannot open include file: 'Classes.hpp'

Editors/xrEProps/PropertiesList.h(...): fatal error C1083:
Cannot open include file: 'ElTree.hpp'
```

Other following blockers include `RenderWindow.hpp`, old iterator-to-pointer
assumptions in editor-local code, and the historical `ParticleSystem.h`
`time_t` mismatch when this target includes the runtime-style particle headers.

Do not fake these blockers. `Classes.hpp` and form/application behavior belong
to the Borland/VCL boundary, while `ElTree.hpp` belongs to the ElPack boundary.

## Checkpoint

The ParticleEditor CMake shell is complete for this dependency-discovery
phase: the active Borland source/form ownership is represented, configuration
succeeds, and direct compilation reaches the real VCL/ElPack/editor UI package
boundary. This does not mean ParticleEditor builds or that its GUI has been
ported.

Do not fake `Classes.hpp`, `ElTree.hpp`, `RenderWindow.hpp`, AlexMX controls,
or MagicFM SDK APIs. Resume only after lawful, compatible editor packages are
available.
