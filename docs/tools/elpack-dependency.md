# ElPack Dependency

## Status

ElPack is a real Borland C++ Builder UI package used by the historical X-Ray
editors. It supplies `ElTree.hpp`, `TElTree`, `TElTreeItem`, and related
widgets. It is not vendored by this repository and must not be replaced with
placeholder classes.

Current users include:

- `Editors/ECore`, directly through `GameMtlLib.h` and indirectly through
  `xrEProps/ItemListHelper.h`;
- `Editors/xrEProps`, whose property and item-list APIs use ElPack tree types;
- LevelEditor, ActorEditor, ParticleEditor, ShaderEditor, and LevelOptions
  generations under `Editor/` and `Editors/`.

The experimental `xrECore`, `xrEProps`, and `LevelEditor` CMake targets consume
the option. Future editor targets should reuse the same cache path without
exposing it to runtime targets. `xrEProps` requires ElPack throughout its
folder, item-list, and property-list interfaces; LevelEditor also uses ElPack
widgets directly in its forms and frames.

## Local Configuration

Set `XR_ELPACK_ROOT` to a lawful local ElPack archive. It is empty by default.

The preferred historical package layout is:

```text
<root>/Code/Source/ElTree.hpp
<root>/Code/elpackB6.lib
```

A direct source root containing `<root>/ElTree.hpp` is also accepted.

```powershell
cmake -S . -B build-elpack-root-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_ECORE=ON `
  -DXR_ELPACK_ROOT=D:\path\to\lawful\ElPack
```

An invalid configured path produces an SDK-target warning. An unset path keeps
configuration working and reports that `xrECore` remains blocked at ElPack.

Finding the headers does not prove MSVC compatibility. The historical
`elpackB6.lib` is a Borland package library and may not be link-compatible with
Visual Studio. The option exists for dependency research and an honest
toolchain probe, not as a promise that the GUI layer builds.

Do not copy ElPack headers or binaries into this repository, add their paths
globally, or use this option for runtime targets.

## Local Search Result

The targeted local dependency inventory did not find `ElTree.hpp`,
`elpackB6.lib`, or an ElPack package directory under `D:\Projects`, the likely
installed Borland locations on `C:`, or `R:\Borland` (the `R:` drive was not
present). `XR_ELPACK_ROOT` therefore remains unset in local build probes.

See [Local Editor Dependency Inventory](editor-dependencies.md).
