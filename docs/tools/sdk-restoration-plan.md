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

### Parallel wxWidgets UI Experiment

`Editors/wxSDKEditor` provides an opt-in `wxSDKEditor` shell behind
`BUILD_XR_WX_SDK_EDITOR`. This is a parallel portability experiment, not a
claim that the historical LevelEditor has been ported. The target uses an
external Win32/x86 wxWidgets package and keeps its initial tree, property, and
dialog contracts independent of wxWidgets.

The experiment should advance one audited interface at a time. It must not
emulate VCL forms, ElPack controls, AlexMX, MagicFM, `.dfm` loading, or old
editor behavior with placeholders. `IEditorTree` is now the first active
boundary: the shell populates an explicit demo hierarchy through the neutral
interface and mirrors selection into placeholder properties. It still loads
no real level, asset, or runtime data.

Next, clean up the tree contract from proven needs, map old `TElTree` usage,
and define a GUI-independent scene model. Real SDK/editor logic should connect
only after that model exists and is tested. See [Experimental wxWidgets SDK
Editor](wx-sdk-editor.md).

The first ElPack ownership audit is now recorded in the [wx Editor ElPack
Migration Map](wx-editor-elpack-migration.md). It selects
`xrEProps/ItemListHelper` as the first model seam: most list creation/search is
already widget-independent, while rename validation has one contained
`TElTreeItem` leak. `FolderLib` follows only after a tree model exists because
it mixes hierarchy rules with ElPack selection, drag/drop, drawing, menus, and
dialogs.

`EditorTreeModel` now supplies that first GUI-independent foundation inside
`wxSDKEditor` only. Demo nodes own their children and expose labels,
categories, paths, lookup, and reason-returning rename validation. The frame
projects this model through `IEditorTree`; the wx control does not own domain
state. Startup self-checks cover empty and duplicate rename rejection plus a
successful path-refreshing rename. Old `xrEProps` sources remain unchanged and
unlinked.

The wx tree now exposes F2 label editing through this model. The edit event is
vetoed unless `RenameNode()` accepts it; successful edits refresh the selected
label and path in the property panel, while failures retain old state and use
`IDialogService` for the returned reason. This proves the first
`NameAfterEdit()`-like flow without importing `TElTreeItem` or altering
`ItemListHelper`.

`EditorTreePresenter` is now the model-centered application seam. It owns the
model and coordinates tree projection, property refresh, dialogs, mutation,
snapshot handoff, and path-list import through neutral interfaces and callbacks.
`MainFrame` retains only wx layout, event translation, and file-dialog/file-read
work. This prepares later audited `ItemListHelper`/`FolderLib` migration without
modifying or linking the historical `xrEProps` implementation.

Model-backed demo create/delete commands now follow the same rule. Unique
child names and root-delete rejection live in `EditorTreeModel`; the wx tree
is rebuilt as a view after each mutation and selects the created node or
surviving parent. This is only the first small `FolderLib`-like hierarchy
prototype. Drag/drop, thumbnails, popup menus, VCL forms, and real SDK data
remain outside the wx path.

The model seam now includes dependency-free `.wx_tree_snapshot` v2
persistence. Save/load is explicit through File menu dialogs, stores the
development hierarchy, item kinds, labels, and categories, and atomically replaces the model
after strict validation. It is not a historical SDK format and creates no
files during startup. The reader remains compatible with v1 and conservatively
infers only known development categories. Real level and editor-library loading
remains deferred.

The first `ItemListTypes` audit maps historical folder/object structure into a
wx-free `EditorItemKind` with `Unknown`, structural `Root`, `Folder`, and
`Object`. The opaque caller-defined `ListItem::type` integer and all UI flags,
callbacks, pointers, icon/color fields, sorting, drawing, and form behavior are
intentionally unmapped. See [wx Editor Item Types](wx-editor-item-types.md).

The next neutral seam is `EditorTreeQuery`. Historical audit found exact key
and direct-child comparisons, component-wise path traversal, folder/object
restrictions, selected-key prefix checks, and folder-first widget sorting. The
wx-free query API maps only read-only model traversal: default case-insensitive
label substring, optional path/case/exact controls, optional item-kind filter,
and stable pre-order results. The wx prototype selects the first result and
updates properties/status without filtering the tree. UI sorting, selection
callbacks, drawing, and drag/drop remain excluded. See [wx Editor Tree
Query](wx-editor-query.md).

`EditorSelectionModel` adds the corresponding wx-free selection seam. The
historical widget returned full names or backing keys, applied a raw prefix
check, and used non-null payload as an object proxy. The neutral model stores
canonical paths, resolves selected nodes in model pre-order, exposes labels and
paths separately, and provides optional raw prefix and audited kind filters.
Presenter integration remains single-selection; report/clear actions prove the
seam without emulating ElPack multi-select, callbacks, expansion, or form
storage. See [wx Editor Selection Model](wx-editor-selection.md).

`BUILD_XR_WX_SDK_EDITOR_MODEL_TESTS` adds the headless
`wxSDKEditorModelTests` executable. It compiles only the independent model and
snapshot/import sources plus the wx-free presenter, requires no wxWidgets
package, and covers hierarchy
mutation, path propagation, rename/delete rules, path-list parsing, snapshot
round trips, escaping, malformed input, duplicates, and atomic failed loads.
The `.wx_tree_paths` importer creates a model from logical development paths
and implicit groups. It prepares an audited seam for later `ItemListHelper` and
`FolderLib` migration; it is not a level or game-data loader, and the preserved
`xrEProps` code remains untouched. No proprietary or runtime data is used.
Test-only fake implementations of `IEditorTree`, `IPropertyPanel`, and
`IDialogService` additionally verify model/view/property/dialog coordination,
including delete confirmation and cancellation, root rejection, and
preservation of the existing model after failed path-list import. Real SDK data
loading remains deferred.

This establishes the experimental wxSDKEditor foundation checkpoint. Both wx
options remain OFF by default; normal configuration has no wxWidgets
requirement, while tests-only configuration does not discover, include, or link
wxWidgets. The prototype does not replace the historical Borland editor and
does not compile, call, or modify `xrEProps` or LevelEditor code.

The next completed foundation slice is a bounded command history around the
neutral model. The presenter routes demo add/delete/move/rename/property
mutations through in-memory snapshot-backed commands, restores selection by
logical path, and clears history when a successful load/import establishes a
new model baseline. Fake UI/service tests exercise undo/redo coordination and
delete confirmation without wxWidgets; tests-only builds do not discover,
include, or link it. This does not port historical `xrEProps` commands or load
real SDK data. See [wx Editor Command History](wx-editor-command-history.md).

Reasonable next wx-path tasks are limited to an intentionally documented sample
path list, mapping `ItemListTypes` concepts to neutral model categories without
source changes, or a later read-only adapter for independently extracted
metadata. The real Borland/VCL restoration path remains separate.

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

`Editors/ActorEditor` now has an opt-in `ActorEditor` CMake shell behind
`BUILD_XR_ACTOR_EDITOR`. It imports the active Borland C++ Builder 6 project
source list, keeps the form metadata visible, and declares the existing
ECore/EProps/ETools plus `xrParticles` dependency chain. Configuration
succeeds; dependency-aware builds may stop through the required editor
dependencies at the known ElPack boundary while `XR_ELPACK_ROOT` is unset. A
direct ActorEditor compile probe gets past the local Borland CRT aliases,
DirectSound include ordering, and audited VCL value shims, then stops at real
GUI/package dependencies such as `Classes.hpp` and `ElTree.hpp`. The active
project asks for `MagicFMDLLB.lib`, which is not present in this repository
snapshot, and matching MagicFM SDK headers remain missing. See [ActorEditor
CMake Shell](actor-editor-cmake.md).

`Editors/Tools/LWO` now has an opt-in non-GUI `LWO` CMake shell behind
`BUILD_XR_LWO`. It represents the historical LightWave object helper DLL and
does not depend on ElPack, VCL, AlexMX, or MagicFM. Configuration succeeds;
the missing `lwo2.h` and related reader sources in the active folder are
resolved narrowly from the older `Editor/Tools/LWO` generation, and the
Release target builds. See [LWO CMake Shell](lwo-cmake.md).

`Editor/Tools/DXT` now has an opt-in non-GUI `DXT` CMake shell behind
`BUILD_XR_DXT`. It represents the historical DDS/DXT texture compression helper
DLL and preserves the VC6 project split by linking the bundled
`nvDXTlib.lib` rather than compiling the excluded `nvdxt.cpp`. The target uses
target-local legacy DirectX include paths for `d3dx.h`, contains the VC6 CRT
compatibility needed by the bundled NVIDIA library, and does not touch the GUI
editor dependency stack. The Release target builds as a 32-bit DLL exporting
`DXTCompress`. See [DXT CMake Shell](dxt-cmake.md).

`xrLC/xrHemisphere` now has an opt-in non-GUI `xrHemisphere` CMake shell behind
`BUILD_XR_HEMISPHERE`. It represents the small level-compiler hemisphere sample
helper DLL without pulling in the full `xrLC` toolchain. The Release target
builds as a 32-bit DLL exporting `_xrHemisphereBuild@24`. See [xrHemisphere
CMake Shell](xrhemisphere-cmake.md).

`xrLC/close` now has an opt-in non-GUI `close` CMake shell behind
`BUILD_XR_CLOSE`. It represents the tiny historical Win32 mailslot helper
executable from the level-compiler tree without pulling in the full `xrLC`
toolchain. The Release target builds as a 32-bit Windows subsystem executable.
See [close CMake Shell](close-cmake.md).

`xrLC/xrDO_Light` now has an opt-in non-GUI `xrDO_Light` CMake shell behind
`BUILD_XR_DO_LIGHT`. It represents the historical detail-object lighting helper
and keeps the source list scoped to `xrDO_Light.vcproj`. The target links the
real restored `xrCore`, `xrCDB`, and `xrHemisphere` targets plus a target-local
FreeImage import library instead of honoring stale `X:\...` pragma library
paths. The in-repo `Editor/ShaderEditor/Lib/FreeImage.lib` is still not
accepted by MSVC, but a helper script can generate a local MSVC x86 import
library outside the repo from the local runtime `FreeImage.dll`. With
`XR_DO_LIGHT_FREEIMAGE_LIB` pointed at that generated library, Release
`xrDO_Light` builds as a 32-bit Windows GUI executable and depends on
`FreeImage.dll` at runtime. See [xrDO_Light CMake Shell](xrdolight-cmake.md)
and [FreeImage Dependency](freeimage-dependency.md).

`Editor/Tools/gauss` now has an opt-in non-GUI `Gauss` CMake shell behind
`BUILD_XR_GAUSS`. The complete VC6 project is used because the later
`Editors/Tools/gauss` snapshot lacks its filter entry source and several listed
headers while containing unrelated editor scaffolding. The target keeps its
DirectX 8-era dependency paths local. See [Gauss CMake Shell](gauss-cmake.md).

The [Local Editor Dependency Inventory](editor-dependencies.md) records the
current machine-level search. DirectX Summer 2004, ColorPicker, and Boost
1.33.1 candidates exist. ElPack, Borland/VCL, AlexMX controls, and a complete
MagicFM SDK remain unavailable. `XR_ALEXMX_ROOT`, `XR_MAGICFM_ROOT`, and
`XR_BORLAND_ROOT` now exist as editor-only diagnostics and future target-local
probes; they do not vendor packages, fake headers, or prove compiler/linker
compatibility.

The editor CMake dispatcher reports a consolidated dependency summary whenever
ECore, EProps, LevelEditor, LevelOptions, ShaderEditor, ParticleEditor, or
ActorEditor probes are enabled.
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
- `ActorEditor` configures as an opt-in dependency/source shell;
- non-GUI compatibility contracts cover `AnsiString`, `TMsgDlgType`,
  `TMsgDlgButtons`, and `TShiftState`;
- editor-only DirectX and ElPack roots have validation diagnostics.

The shared first blocker is ElPack (`ElTree.hpp`, `TElTree`, `TElTreeItem`,
and historically `elpackB6.lib`). The value shim strategy ends here because
forms, widgets, application services, and component packages have behavior and
ABI contracts that cannot be represented honestly by placeholder types.

Current editor-shell checkpoint:

| Target | Option | Checkpoint result |
|---|---|---|
| `ETools` | `BUILD_XR_ETOOLS` | Builds as the first modern editor helper dependency. |
| `xrECore` | `BUILD_XR_ECORE` | Configures and reaches the ElPack `ElTree.hpp` boundary. |
| `xrEProps` | `BUILD_XR_EPROPS` | Configures and reaches `FolderLib.h` / ElPack `ElTree.hpp`. |
| `LevelEditor` | `BUILD_XR_LEVEL_EDITOR` | Configures; dependency-aware builds stop through `xrECore` at ElPack. |
| `LevelOptions` | `BUILD_XR_LEVEL_OPTIONS` | Configures; direct probe stops at `SceneProperties.h` / `ElTree.hpp`. |
| `ShaderEditor` | `BUILD_XR_SHADER_EDITOR` | Configures; direct probe stops at missing `splash.h` and GUI package headers. |
| `ParticleEditor` | `BUILD_XR_PARTICLE_EDITOR` | Configures; direct probe stops at `Classes.hpp`, `ElTree.hpp`, and editor GUI headers. |
| `ActorEditor` | `BUILD_XR_ACTOR_EDITOR` | Configures; direct probe stops at `Classes.hpp`, `ElTree.hpp`, AlexMX, and missing editor controls. |
| `LWO` | `BUILD_XR_LWO` | Builds as a non-GUI LightWave object helper DLL. |
| `DXT` | `BUILD_XR_DXT` | Builds as a non-GUI DDS/DXT texture compression helper DLL. |
| `xrHemisphere` | `BUILD_XR_HEMISPHERE` | Builds as a non-GUI hemisphere sampling helper DLL. |
| `close` | `BUILD_XR_CLOSE` | Builds as a non-GUI xrLC mailslot helper executable. |
| `xrDO_Light` | `BUILD_XR_DO_LIGHT` | Builds as a non-GUI detail-object lighting helper executable when pointed at an external MSVC x86 FreeImage import library generated outside the repo. |
| `Gauss` | `BUILD_XR_GAUSS` | Release builds as an x86 image-filter DLL exporting `gauss`, `ip_BuildKernel`, and `ip_ProcessKernel`; diagnostics use a bounded Gauss-local debugger logger. |

Verified non-GUI binary shapes at this checkpoint:

- `LWO.dll` is x86 and exports `_LWO_CloseFile@4` and
  `_LWO_ImportObject@8`;
- `DXT.dll` is x86 and exports `DXTCompress`;
- `xrHemisphere.dll` is x86 and exports `_xrHemisphereBuild@24`;
- `close.exe` is an x86 Windows GUI-subsystem helper;
- `xrDO_Light.exe` is an x86 Windows GUI-subsystem helper and links when
  `XR_DO_LIGHT_FREEIMAGE_LIB` selects an external generated MSVC import
  library; the executable depends on `FreeImage.dll` at runtime.
- `gauss.dll` is x86 PE32, exports `gauss`, `ip_BuildKernel`, and
  `ip_ProcessKernel`, and depends only on `KERNEL32`, the MSVC runtime, and the
  UCRT runtime API set. A separate Win32 load-only smoke target remains future
  work.

Build trees, logs, extracted data, generated FreeImage `.lib`/`.def` files,
and external dependency binaries are local verification artifacts and must not
be committed. The next safe restoration pass should select another small
non-GUI helper or return to a focused runtime investigation such as pickup/use
interaction, the trader crash, or shader `CConstant` startup initialization.
GUI SDK work remains paused until lawful Borland/VCL, ElPack, AlexMX, and
MagicFM SDK packages are found.

Before resuming SDK GUI work, locate lawful copies of ElPack, a compatible
Borland/VCL installation, the AlexMX controls, and matching MagicFM SDK
headers. Never fake `ElTree.hpp` or VCL classes, and never vendor those
third-party packages. If those dependencies remain unavailable, pause the GUI
branch and continue archive tools, format validation, compiler inventories, or
runtime bug restoration.

Do not use runtime target changes to bypass SDK GUI blockers. The editor
dependency roots are target-local diagnostics/probes, and the runtime-heavy
targets should remain on their own restoration path.

The wx experiment has now extracted one additional neutral hierarchy seam from
the historical ElPack boundary: model-level move/reparent. Root and Folder are
the only destinations; ownership transfer preserves node addresses and complete
subtrees; generated paths and selected path prefixes refresh after success;
logical failures are non-mutating. The wx shell uses a menu/choice dialog only.
Historical drag images, mouse callbacks, widget selection/expansion, popup
menus, and FolderLib's merge behavior are not ported. Tests-only mode remains
wxWidgets-free. See [wx Editor Tree Move](wx-editor-move.md).

The next completed neutral slice is an editable node property model. It maps
only audited string/integer/float/boolean/choice/read-only type categories and
currently binds label/category plus read-only kind/path. Presenter apply uses
existing rename rules and a basic wx form; ElPack editors, chooser dialogs,
drawing, callbacks, reflection, and real SDK binding remain excluded. Snapshot
format is unchanged because category was already persisted. See
[wx Editor Properties](wx-editor-properties.md).

## Related Documents

* [Tools and SDK Status](status.md)
* [ECore CMake Port Inventory](ecore-inventory.md)
* [Legacy DirectX SDK Support](legacy-directx.md)
* [ElPack Dependency](elpack-dependency.md)
* [xrEProps CMake Port Inventory](xreprops-inventory.md)
* [LevelEditor CMake Shell](level-editor-cmake.md)
* [ShaderEditor CMake Shell](shader-editor-cmake.md)
* [ParticleEditor CMake Shell](particle-editor-cmake.md)
* [ActorEditor CMake Shell](actor-editor-cmake.md)
* [LWO CMake Shell](lwo-cmake.md)
* [DXT CMake Shell](dxt-cmake.md)
* [xrHemisphere CMake Shell](xrhemisphere-cmake.md)
* [close CMake Shell](close-cmake.md)
* [xrDO_Light CMake Shell](xrdolight-cmake.md)
* [FreeImage Dependency](freeimage-dependency.md)
* [Gauss CMake Shell](gauss-cmake.md)
* [wx Editor Tree Move](wx-editor-move.md)
* [wx Editor Properties](wx-editor-properties.md)
* [Local Editor Dependency Inventory](editor-dependencies.md)
* [Archive Unpacker Plan](unpacker-plan.md)
* [Archive Formats](../formats/archives.md)
* [Modding Overview](../modding/overview.md)
* [Assets Policy](../modding/assets-policy.md)
* [wxSDKEditor Dockable Layout](wx-editor-layout.md)
* [wxSDKEditor Document Layer](wx-editor-document.md)

## wxSDKEditor Dockable Layout Checkpoint

The shell now has a modern docking boundary using the built-in wxWidgets AUI
component. Named scene, properties, and output panes surround a fixed center
viewport placeholder; View actions toggle/reset them, and wxConfig persists a
defensively restored perspective outside the repository. This UI-only step
does not alter presenter/model ownership, does not enter headless builds, and
does not port VCL form persistence.

## wxSDKEditor Document Checkpoint

One frame-owned, wx-free `EditorDocument` now coordinates the neutral model,
selection, command history, development snapshot path, and exact serialized
save baseline. The frame translates New/Open/Save/Save As and
Save/Discard/Cancel prompts, while imports establish a modified untitled
document. Undo/redo dirtiness follows exact saved content, and tests remain
independent of wxWidgets. This is not a real SDK document format. See
[wxSDKEditor Document Layer](wx-editor-document.md).
