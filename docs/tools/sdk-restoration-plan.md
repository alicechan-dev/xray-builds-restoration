# SDK Restoration Plan

This document describes the restoration roadmap for SDK, editor, converter, archive, compiler, and modding tools in the historical X-Ray / S.T.A.L.K.E.R. build 1935 tree. It is planning documentation only.

The current priority remains runtime correctness and binary compatibility. SDK restoration should proceed in small, auditable steps after each format or dependency assumption is proven from repository source, synthetic fixtures, or user-owned local data.

## Legal And Asset Policy

SDK/tool work must not add proprietary assets, original game archives, extracted `gamedata/`, repacks, cracks, leaked data, or gamedata dumps to the repository. Tools are intended for lawful research, compatibility testing, and use with the user's own legally obtained game data outside version control. Repository tests should use synthetic fixtures wherever possible.

## Phase 0: Inventory And Legal Boundaries

Goals:

* inventory every SDK/tool/editor/compiler/project file found in the tree;
* classify components as restored, present-but-unrestored, build-unknown, planned, or documentation-only;
* identify duplicate generations such as `Editor/`, `Editors/`, and `Editors/!old/`;
* document external dependencies, host SDK requirements, and legal limits;
* record which tools require user-supplied local data for manual testing.

Outputs:

* updated [Tools and SDK Status](status.md);
* documented legal/asset boundaries;
* initial priority order for safe tool restoration.

## Phase 1: Read-Only Tools

Goals:

* restore or create tools that inspect data without modifying it;
* prefer archive `info`, `list`, and `verify` workflows before extraction;
* add metadata inspectors for configs, XML, Lua, textures, models, particles, levels, and graph files only after layouts are proven;
* keep malformed input handling graceful and bounded.

Candidate targets:

* `xr_unpack info/list/verify` improvements;
* LTX/config validator scaffold;
* UI XML/string-reference validator;
* read-only model/texture/archive metadata inspectors.

## Phase 2: Safe Extraction And Validation

Goals:

* allow write-capable operations only behind explicit flags such as `--write`;
* require dry-run planning before recommended extraction workflows;
* prevent path traversal, absolute paths, drive-letter paths, duplicate output paths, out-of-bounds reads, and accidental overwrites;
* add synthetic tests for path safety and parser bounds;
* document that extracted proprietary data must stay outside the repository.

Candidate targets:

* `xr_unpack extract --dry-run` and `--write` hardening;
* optional `--filter` and `--limit` support;
* synthetic archive fixtures where the format can be generated without proprietary data.

## Phase 3: Low-Risk Converters And Validators

Goals:

* restore small command-line validators/converters before GUI editors;
* match runtime parser behavior instead of inventing modernized behavior;
* use synthetic fixtures and user-owned local data for manual checks;
* keep tools target-local in CMake.

Candidate targets:

* LTX/config validation;
* Lua syntax and binding inventory helpers;
* XML/UI validation;
* DDS/DXT metadata inspection;
* model/animation metadata inspection.

## Phase 4: SDK Build Restoration

Goals:

* restore larger historical compiler/editor targets after dependencies are mapped;
* move each tool into a local `CMakeLists.txt` without changing target names or output paths unnecessarily;
* keep external dependency discovery documented and opt-in;
* prioritize command-line compilers before GUI editors where practical.

Candidate targets:

* `xrFS` archive/filesystem tooling;
* `xrLC` and helper tools;
* `xrAI` graph/spawn tooling;
* `xrDXTC` and texture helpers;
* `Editors/Tools/ETools` and other low-level editor helper DLLs;
* editor core libraries needed by GUI applications.

## Phase 5: Modding Workflows

Goals:

* document practical local workflows for lawful user-owned data;
* prefer separate mod directories over editing extracted base data in place;
* explain runtime layout, VFS precedence, config loading, and generated data behavior;
* provide tutorials that do not require committed proprietary samples.

Candidate outputs:

* archive inspection/extraction tutorial;
* config/script validation tutorial;
* separate mod-directory workflow;
* troubleshooting notes for runtime resource lookup.

## Phase 6: Advanced SDK

Goals:

* restore complex GUI editors and host plugins only after lower-risk tooling is stable;
* document host SDK requirements and licensing constraints;
* avoid committing generated proprietary data;
* keep editor/compiler behavior compatible with build 1935 formats.

Candidate targets:

* Level editor, after `Editors/ECore` and `xrEProps` are inventoried;
* Actor/model editor;
* Particle editor;
* Shader editor;
* 3ds Max / LightWave exporters;
* launcher and script debug IDE.

## CMake Planning

The long-term direction is for the root `CMakeLists.txt` to provide project setup, global options, dependency discovery, shared helpers, and `add_subdirectory(...)` orchestration. Each restored component should own its target in a local `CMakeLists.txt`.

Planned option families:

* `BUILD_XR_TOOLS` for broad tool dispatch if needed.
* `BUILD_XR_UNPACK` for the archive unpacker.
* `BUILD_XR_SDK_TOOLS` for restored command-line SDK tools.
* `BUILD_XR_EDITORS` for large GUI editor targets.
* `BUILD_XR_COMPILERS` for level/AI/compiler targets.

Large options should default to `OFF` until the dependency and legal/test story is clear.

## Recommended First Practical Target

After the current inventory, the safest next target is still the archive/tooling path:

1. add synthetic path-safety and archive-directory tests for `xr_unpack`;
2. add optional `--filter` support for `xr_unpack list` and extraction planning;
3. start a read-only LTX/config validator scaffold.

The first large historical SDK candidates to inventory in depth are `Editors/ECore`, `xrLC`, and `xrAI`, but they should not be ported until dependencies, command-line contracts, and binary format compatibility are documented. `Editors/Tools/ETools` now has a partial opt-in CMake target that builds after linking the matching historical ColorPicker import library. `Editors/ECore` now has an experimental opt-in `xrECore` CMake target; its low-level compiler compatibility blockers are fixed, but retained core APIs still depend pervasively on Borland/VCL types and ElPack. The [VCL compatibility strategy](vcl-compat-strategy.md) provides opt-in contracts for `AnsiString`, the `TMsgDlgType` message category enum, and `TMsgDlgButtons` value-set metadata; these tested shims are integrated into `xrECore` for non-Borland builds only. True forms, application behavior, closures, modal results and dialog behavior, and ElPack remain outside that layer. `LevelEditor.exe` should still wait until `xrECore`, `xrEProps`, `xrQSlim`, the Borland/VCL boundary, runtime ColorPicker deployment, and shared editor DLL dependencies are mapped.

The locally inventoried DirectX 9.0 SDK Update (Summer 2004) matches the SDK
generation named by the historical editor projects. Restored SDK targets may
opt into it with `XR_LEGACY_DX_ROOT`; it is not a global runtime dependency.
See [Legacy DirectX SDK Support](legacy-directx.md).

ElPack is likewise an external editor-only dependency. `XR_ELPACK_ROOT`
supports target-local probing of a lawful package archive without vendoring
headers or exposing them to runtime targets. The option does not make Borland
VCL widgets or `elpackB6.lib` inherently compatible with MSVC. See
[ElPack Dependency](elpack-dependency.md).

`Editors/xrEProps` now has an opt-in `xrEProps` CMake build probe behind
`BUILD_XR_EPROPS`. It retains the complete historical DLL source and form set;
configuration succeeds and compilation stops at the expected ElPack boundary.
See [xrEProps CMake Port Inventory](xreprops-inventory.md).

`Editors/LevelEditor` now has an opt-in `LevelEditor` CMake shell behind
`BUILD_XR_LEVEL_EDITOR`. It imports its source list from the canonical Borland
project and enables the ECore/EProps/ETools dependency chain. Configuration
succeeds; compilation stops in `xrECore` at the expected ElPack boundary. See
[LevelEditor CMake Shell](level-editor-cmake.md).

`Editors/LevelOptions` now has an opt-in `LevelOptions` CMake shell behind
`BUILD_XR_LEVEL_OPTIONS`. It represents the historical `xrLC_options.dll`
Borland project as a dependency/source probe, keeps the `.dfm` metadata visible,
and does not port VCL or fake ElPack/AlexMX controls. Configuration succeeds;
dependency-aware builds may stop through the required `xrECore` dependency at
the expected ElPack boundary. A direct LevelOptions compile probe gets past the
local Borland CRT compatibility issue and stops at `SceneProperties.h` missing
`ElTree.hpp`. The historical project also references missing
`xrLC_OptionsEntry.cpp`. See [LevelOptions CMake Shell](level-options-cmake.md).

`Editors/ShaderEditor` now has an opt-in `ShaderEditor` CMake shell behind
`BUILD_XR_SHADER_EDITOR`. It imports the active Borland C++ Builder 6 project
source list, keeps the form metadata visible, and declares the existing
ECore/EProps/ETools dependency chain. Configuration succeeds; builds are
expected to stop through `xrECore` at the known ElPack boundary while
`XR_ELPACK_ROOT` is unset. A direct ShaderEditor compile probe gets past the
local Borland CRT aliases, DirectSound include ordering, and audited VCL value
shims, then stops at missing `splash.h` from an absent `Splash.cpp` form plus
real GUI/package dependencies such as `ElTree.hpp`, `Classes.hpp`, and
`RenderWindow.hpp`. The older `Editor/ShaderEditor/Lib/MagicFM.lib` exists and
is linked as a historical import-library candidate, but matching MagicFM SDK
headers remain missing. See [ShaderEditor CMake Shell](shader-editor-cmake.md).

`Editors/ParticleEditor` now has an opt-in `ParticleEditor` CMake shell behind
`BUILD_XR_PARTICLE_EDITOR`. It imports the active Borland C++ Builder 6 project
source list, keeps the form metadata visible, and declares the existing
ECore/EProps/ETools plus `xrParticles` dependency chain. Configuration
succeeds; dependency-aware builds may stop through the required editor
dependencies at the known ElPack boundary while `XR_ELPACK_ROOT` is unset. A
direct ParticleEditor compile probe gets past the local Borland CRT aliases,
DirectSound include ordering, and audited VCL value shims, then stops at real
GUI/package dependencies such as `Classes.hpp`, `ElTree.hpp`, and
`RenderWindow.hpp`. The active project asks for `MagicFMDLLB.lib`, which is not
present in this repository snapshot, and matching MagicFM SDK headers remain
missing. See [ParticleEditor CMake Shell](particle-editor-cmake.md).

The [Local Editor Dependency Inventory](editor-dependencies.md) records the
current machine-level search. DirectX Summer 2004, ColorPicker, and Boost
1.33.1 candidates exist. ElPack, Borland/VCL, AlexMX controls, and a complete
MagicFM SDK remain unavailable. `XR_ALEXMX_ROOT`, `XR_MAGICFM_ROOT`, and
`XR_BORLAND_ROOT` now exist as editor-only diagnostics and future target-local
probes; they do not vendor packages, fake headers, or prove compiler/linker
compatibility.

The editor CMake dispatcher reports a consolidated dependency summary whenever
ECore, EProps, LevelEditor, LevelOptions, ShaderEditor, or ParticleEditor probes
are enabled.
External roots remain optional at configure time: invalid paths warn with the
accepted layout or candidate-file expectations, while builds remain free to
stop honestly at the unresolved UI boundary.

## SDK GUI Checkpoint

Completed CMake milestones:

- `ETools` builds as a modern target;
- `xrECore` and `xrEProps` configure as opt-in build probes;
- `LevelEditor` configures as an opt-in dependency/source shell;
- `LevelOptions` configures as an opt-in dependency/source shell;
- `ShaderEditor` configures as an opt-in dependency/source shell;
- `ParticleEditor` configures as an opt-in dependency/source shell;
- non-GUI compatibility contracts cover `AnsiString`, `TMsgDlgType`,
  `TMsgDlgButtons`, and `TShiftState`;
- editor-only DirectX and ElPack roots have validation diagnostics.

The shared first blocker is ElPack (`ElTree.hpp`, `TElTree`, `TElTreeItem`,
and historically `elpackB6.lib`). The value shim strategy ends here because
forms, widgets, application services, and component packages have behavior and
ABI contracts that cannot be represented honestly by placeholder types.

Before resuming SDK GUI work, locate lawful copies of ElPack, a compatible
Borland/VCL installation, the AlexMX controls, and matching MagicFM SDK
headers. Never fake `ElTree.hpp` or VCL classes, and never vendor those
third-party packages. If those dependencies remain unavailable, pause the GUI
branch and continue archive tools, format validation, compiler inventories, or
runtime bug restoration.

## Related Documents

* [Tools and SDK Status](status.md)
* [ECore CMake Port Inventory](ecore-inventory.md)
* [Legacy DirectX SDK Support](legacy-directx.md)
* [ElPack Dependency](elpack-dependency.md)
* [xrEProps CMake Port Inventory](xreprops-inventory.md)
* [LevelEditor CMake Shell](level-editor-cmake.md)
* [ShaderEditor CMake Shell](shader-editor-cmake.md)
* [ParticleEditor CMake Shell](particle-editor-cmake.md)
* [Local Editor Dependency Inventory](editor-dependencies.md)
* [Archive Unpacker Plan](unpacker-plan.md)
* [Archive Formats](../formats/archives.md)
* [Modding Overview](../modding/overview.md)
* [Assets Policy](../modding/assets-policy.md)
