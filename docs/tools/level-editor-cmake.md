# LevelEditor CMake Shell

## Scope

`BUILD_XR_LEVEL_EDITOR` adds an experimental `LevelEditor` Win32 executable
target from `Editors/LevelEditor`. This is a dependency and source-ownership
shell, not a VCL GUI port.

The source list is imported from the `CCompiler` entries in
`Editors/LevelEditor/LevelEditor.bpr`. The `.dfm` forms and Borland project
resources remain visible as project metadata; no form conversion or fake VCL
types are supplied.

## Canonical Source

`Editors/LevelEditor` is the canonical first target:

- it is in the active `Editors` tree;
- its `.bpr` is byte-identical to `Editors/!old/LevelEditor/LevelEditor.bpr`;
- it uses the DirectX 9.0 Summer 2004 SDK generation;
- `Editor/LevelEditor` is an older DirectX 8.1-era generation.

The canonical `.bpr` references `Splash.cpp` and `resource.res`, which are
absent from this snapshot. CMake reports both as warnings and does not invent
replacement files.

## Dependency Chain

Enabling `BUILD_XR_LEVEL_EDITOR` also enables:

- `ETools`;
- `xrECore`;
- `xrEProps`.

The shell also declares the historical engine-side libraries used by the
editor: `xrCore`, `xrCDB`, `xrParticles`, and `xrSound`. DirectX remains
target-local through `XR_LEGACY_DX_ROOT`, and ElPack remains target-local
through `XR_ELPACK_ROOT`.

Further historical dependencies remain unresolved: Borland VCL runtime and
packages, AlexMX controls, MagicFM, old Boost, FreeImage/DXT helpers, LWO,
hemisphere, Ogg tools, and progressive-mesh libraries.

The local dependency inventory found the matching DirectX SDK, ColorPicker
runtime/import pair, and Boost 1.33.1. It did not find ElPack, Borland/VCL,
AlexMX control headers, or MagicFM SDK headers. See
[Local Editor Dependency Inventory](editor-dependencies.md).

## Build Probe

```powershell
cmake -S . -B build-leveleditor-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_LEVEL_EDITOR=ON

cmake --build build-leveleditor-check --config Release `
  --target LevelEditor -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configuration succeeds. The target build first stops in the required `xrECore`
dependency:

```text
Editors/ECore/Engine/GameMtlLib.h(50): error C1083:
Cannot open include file: 'ElTree.hpp'
```

This is expected and honest. The application itself remains a Borland/VCL GUI
using `Application`, `TForm`, `TFrame`, `USEFORM`, and `.dfm` resources.

## Next Honest Step

Do not patch LevelEditor around its prerequisites. Obtain and assess lawful
ElPack/VCL dependencies, or define a deliberate editor UI migration boundary,
before attempting application-level compilation.

## Checkpoint

The LevelEditor CMake shell is complete for this dependency-discovery phase:
the canonical source ownership and ECore/EProps/ETools chain are represented,
configuration succeeds, and the build stops at the known ElPack prerequisite.
This does not mean LevelEditor builds or that its GUI has been ported.

Further work requires ElPack (`ElTree.hpp` and `elpackB6.lib`), a compatible
Borland/VCL installation, AlexMX controls, and MagicFM SDK headers. Do not
replace those packages with fake widgets or commit third-party files. Without
them, pause this target and return to archive tools or runtime restoration.
