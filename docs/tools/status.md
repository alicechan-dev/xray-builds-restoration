# Tools and SDK Status

This page inventories modding, SDK, archive, and asset-tooling components found in the build 1935 restoration tree. The repository focus remains engine/runtime restoration first; SDK and tool restoration is a separate planning track.

No proprietary assets, original archives, extracted `gamedata/`, repacks, cracks, leaked data, or gamedata dumps should be added to this repository. Tools are intended for lawful research, compatibility testing, and use with the user's own legally obtained game data outside version control.

## A. Existing / Restored Tools

| Component | Path | Type | Purpose | Current state | Dependencies | Build status | Runtime/modding value | Restoration priority | Notes |
|---|---|---|---|---|---|---|---|---|---|
| Archive listing helper | `tools/xrArchiveList/` | CLI diagnostic | List virtual paths from `.xp*` archives. | Restored read-only helper. | `xrCore`, local LZHUF path. | Local CMake target `xrArchiveList`. | Confirms archive mounting contents without extraction. | High | Keep read-only and diagnostic. |
| Archive unpacker | `tools/xr_unpack/` | CLI tool | Inspect, verify, plan, and extract local archives with explicit safety gates. | Scaffold plus proven `.xp*` directory parser, verify, dry-run planning, and explicit `--write` extraction support. | Local parser code, path-safety helpers, LZHUF-compatible directory decode. | Local CMake target `xr_unpack`, opt-in through `BUILD_XR_UNPACK`. | Primary safe archive workflow for local lawful data. | High | Never commit extracted output or archive samples. |
| Runtime compressor / packer | `xrCompress/` | CLI tool | Historical archive/data compression executable. | Source and legacy project present; restored behavior only partially documented. | `xrCore`, compression helpers, Win32 console runtime. | Root CMake target exists. | Format reference for archive research. | High | Treat as reference before changing writer behavior. |
| Runtime XML parser | `xrXMLParser/` | Library | XML parsing support used by runtime/UI tooling. | Runtime target modularized into local CMake. | Expat-style parser code, runtime include paths. | Local CMake target `xrXMLParser`. | Useful for XML/UI validators later. | Medium | Already part of runtime build. |
| Runtime Lua library | `xrLUA/` | Library | Lua runtime support. | Runtime target modularized into local CMake. | Lua sources and runtime include paths. | Local CMake target `xrLUA`. | Basis for future script validation. | Medium | Keep runtime ABI stable. |
| Runtime particles library | `xrParticles/` | Library | Runtime particle format and playback support. | Runtime target modularized into local CMake. | `xrCore`-style runtime headers. | Local CMake target `xrParticles`. | Reference for particle editor restoration. | Medium | Restore editor later, not now. |
| Runtime collision library | `xr_3da/xrCDB/` | Library | Runtime collision database and ray query support. | Runtime target modularized into local CMake. | Runtime include paths, math/collision code. | Local CMake target `xrCDB`. | Reference for level compiler and CFORM validation. | Medium | Keep runtime behavior unchanged. |
| Runtime network server library | `xrNetServer/` | Library | Runtime network/server support. | Runtime target modularized into local CMake. | WinSock/runtime headers. | Local CMake target `xrNetServer`. | Runtime dependency, not a modding tool by itself. | Low | Useful CMake modularization precedent. |

## B. Existing But Not Restored Tools

| Component | Path | Type | Purpose | Current state | Dependencies | Build status | Runtime/modding value | Restoration priority | Notes |
|---|---|---|---|---|---|---|---|---|---|
| Filesystem/archive library | `xrFS/` | Library/tooling code | Historical filesystem, locator, archive, LZHUF, and LZO support. | Source and `.vcproj` present; not restored to modern CMake. | Win32, archive/compression helpers. | Unknown. | High-value archive format reference. | High | Inventory before moving into tools. |
| Level compiler | `xrLC/` | Compiler tool | Builds geometry, collision, lightmaps, sectors, portals, and visibility data. | Legacy projects/source present; not restored. | DirectX 9-era SDK, FreeImage, QSlim/OpenMesh/NVIDIA geometry code, `xrDXTC`, `xrCore`. | Unknown. | Required for full level modding workflow. | High | Large dependency surface; inventory first. |
| Level compiler helper tools | `xrLC/close/`, `xrLC/xrDO_Light/`, `xrLC/xrHemisphere/` | Helper CLIs | Auxiliary level/light/hemisphere compilation support. | Legacy project files present. | Same family as `xrLC`. | Unknown. | Supports compiler workflow. | Medium | Restore after main compiler map is understood. |
| AI / game graph compiler | `xrAI/` | Compiler tool | Builds AI maps, level graphs, game graphs, spawn/ALife graph data. | Legacy `.vcproj` present; not restored. | Runtime graph code, `xrSE_Factory` concepts, Lua/Luabind/Boost-era code, MagicFM-style library. | Unknown. | Required for spawn and AI-compatible mods. | High | Must match restored serialization formats. |
| Level editor | `Editors/LevelEditor/` plus historical generations | GUI editor | Historical level/world editing workflow. | Experimental opt-in `LevelEditor` CMake shell added behind `BUILD_XR_LEVEL_EDITOR`; sources are imported from the canonical Borland project without porting forms. | `xrECore`, `xrEProps`, `ETools`, runtime-style libraries, DirectX 9, Borland VCL/packages, ElPack/AlexMX, MagicFM, Boost 1.30.x. | Configure passes; target build stops in required `xrECore` at missing `ElTree.hpp`. Canonical project also references absent `Splash.cpp` and `resource.res`. | Dependency and source-ownership probe only. | Medium | See [Level Editor Inventory](level-editor-inventory.md) and [LevelEditor CMake Shell](level-editor-cmake.md). |
| Actor/model editor | `Editor/ActorEditor/`, `Editors/ActorEditor/`, `Editors/!old/ActorEditor/` | GUI editor | Edits models, skeletons, motions, materials, and export metadata. | Multiple source generations present. | Editor core, DirectX, image/mesh helpers. | Unknown. | Important for model modding. | Medium | Defer until formats and editor core are mapped. |
| Particle editor | `Editor/ParticleEditor/`, `Editors/ParticleEditor/`, `Editors/!old/ParticleEditor/` | GUI editor | Edits particle effects and related runtime data. | Source present. | Editor core, DirectX/editor rendering. | Unknown. | Useful once particle formats are documented. | Medium | Coordinate with `xrParticles`. |
| Shader editor | `Editor/ShaderEditor/`, `Editors/ShaderEditor/`, `Editors/!old/ShaderEditor/` | GUI editor | Edits shader/material descriptions. | Source present. | DirectX shader compiler era, editor rendering. | Unknown. | Useful for material workflows. | Medium | Sensitive to SDK versioning. |
| Editor core | `Editors/ECore/` | Shared editor library | Shared editor engine, thumbnails, object IO, properties, image helpers, and shader helpers. | Opt-in CMake target `xrECore` added as an experimental partial port. VCL form units are excluded for now. Tested non-GUI value shims are integrated for non-Borland builds only. | `xrCore`, `xrCDB`, `xrParticles`, `ETools`, DirectX 9/D3DX, `xrEProps`, Borland/VCL packages, ElPack/AlexMX, MagicFM. | Configure passes with `BUILD_XR_ECORE=ON`; compiler/value compatibility probes advance to the required `ElTree.hpp`/`elpackB6.lib` UI boundary. `XR_ELPACK_ROOT` provides an optional target-local path for lawful external dependency research, but ElPack remains a Borland UI boundary. | Foundation for restored editors. | High | See [ECore CMake Port Inventory](ecore-inventory.md), [VCL Compatibility Strategy](vcl-compat-strategy.md), and [ElPack Dependency](elpack-dependency.md). |
| Editor properties library | `Editors/xrEProps/` | Shared editor DLL | Property-grid, item-list, chooser, numeric, text, and shader-function UI support. | Experimental opt-in `xrEProps` CMake target added behind `BUILD_XR_EPROPS`; the full historical source/form set is retained. | `xrCore`, `ETools`, Borland VCL, ElPack, AlexMX controls, `.dfm` forms. | Configure passes; build stops honestly at `FolderLib.h` missing `ElTree.hpp` when `XR_ELPACK_ROOT` is unset. | Needed by editor apps. | High | See [xrEProps CMake Port Inventory](xreprops-inventory.md). |
| Level options tool | `Editors/LevelOptions/` | GUI DLL/plugin | Historical `xrLC_options.dll` build-options dialog for level compiler workflows. | Experimental opt-in `LevelOptions` CMake shell added behind `BUILD_XR_LEVEL_OPTIONS`; source/form ownership is represented without porting VCL forms. | `xrECore`, `xrEProps`, `ETools`, runtime-style libraries, DirectX 9, ElPack, AlexMX controls, Borland/VCL, ColorPicker/MagicFM-era editor packages. | Configure passes. A direct LevelOptions compile probe gets past the local Borland CRT compatibility issue and stops at `Editor/SceneProperties.h` missing `ElTree.hpp`; dependency-aware builds may stop earlier through required `xrECore`. The `.bpr` also references absent `xrLC_OptionsEntry.cpp`. | Optional helper for level compiler options once compiler/editor dependencies exist. | Low | See [LevelOptions CMake Shell](level-options-cmake.md). |
| Texture/DXT tools | `Editors/Tools/DXT/`, `Editor/Tools/DXT/`, `xrDXTC/` | Converter/tool library | Texture compression and DDS/DXT conversion support. | Legacy source/project files present. | NVIDIA DXT libraries, DirectX/image code. | Unknown. | Useful for asset conversion. | Medium | Document binary library provenance first. |
| Editor tools library | `Editors/Tools/ETools/` | Shared tool DLL | D3DX wrapper, editor helper exports, ray-triangle tests, and VIPM/progressive-mesh support. | Opt-in CMake target `ETools` added as a partial port. VIPM/xrQSlim sources are excluded until `xrQSlim` is restored. | `xrCore`, DirectX 9/D3DX, ColorPicker import library, future `xrQSlim`. | Configure passes with `BUILD_XR_ETOOLS=ON`; partial Release target builds after linking the matching historical `Editor/Tools/ETools/ColorPicker.lib` import library. | Early LevelEditor dependency and safer than full VCL GUI port. | High | See [ETools CMake Port Inventory](etools-inventory.md). Runtime use of the color picker export may still need the matching `ColorPicker.dll`. |
| LWO tool | `Editors/Tools/LWO/` | Converter/helper | LightWave object/envelope handling utility. | Legacy project present. | LightWave object parsing code. | Unknown. | Import/export pipeline support. | Medium | Host-format research needed. |
| Gauss tool | `Editors/Tools/gauss/` | Utility | Legacy utility with filesystem/math/log scaffolding. | Legacy project present. | Win32/editor-style helpers. | Unknown. | Unclear. | Low | Purpose needs source-level confirmation. |
| 3ds Max / LightWave plugins | `Plugins/`, `samples/xskinexp/` | Host plugins | Exporters/import helpers for models, skins, keys, shaders, and editor formats. | Source/project files present. | Host SDKs such as 3ds Max or LightWave. | Unknown. | Valuable but legally/toolchain-sensitive. | Low | Restore only after clean format docs. |
| Lua compiler utility | `Dima/xrLuaCompiler/` | CLI/helper | Lua/script compiler or validation utility. | Legacy project present. | Lua, Luabind-era code, Boost-compatible headers. | Unknown. | Candidate for script tooling. | Medium | Compare with runtime Lua version first. |
| Script debug IDE | `xr_3da/xrScriptDebugIde/` | GUI tool | Script debugging/editor support. | Legacy project present. | MFC, CJ60Lib, SciLexer/Scintilla. | Unknown. | Developer-only helper. | Low | Defer until script runtime stabilizes. |
| Launcher | `xrLauncher/` | GUI app | Historical launcher/configuration UI. | Managed C++ legacy project present. | .NET/Managed C++ era, WinForms/resources. | Unknown. | Optional runtime convenience. | Low | Runtime can be tested without it. |

## C. Source Present But Build Unknown

| Component | Path | Type | Purpose | Current state | Dependencies | Build status | Runtime/modding value | Restoration priority | Notes |
|---|---|---|---|---|---|---|---|---|---|
| AlexRR map editor | `AlexRR_Editor/` | GUI/editor experiment | Map editor/export code, including builder/export references. | Source and `.dsp` present. | Legacy UI/toolchain unknown. | Unknown. | Possible historical reference. | Low | Compare with `Editors/LevelEditor` before investing. |
| QSlim / mesh simplification SDK | `SDK/QSlim/`, `xrQSlim/`, `xrProgressive/` | SDK/library/tools | Mesh simplification, progressive mesh, and viewers. | Many legacy projects present. | QSlim/MixKit/libgfx, OpenGL for viewers. | Unknown. | Useful for LOD/model/compiler pipeline. | Medium | Prefer library inventory before viewer restoration. |
| STRIPS / NvTriStrip tools | `xrLC/NvTriStrip/`, related geometry folders | Library/tooling code | Triangle strip/geometry optimization. | Source/project fragments present. | NVIDIA-era geometry code. | Unknown. | Compiler/model optimization support. | Medium | Dependency for `xrLC` likely. |
| Range coder samples | `xrRangeCoder/` | Compression research code | Compression/range coding experiments. | Source present. | Unknown. | Unknown. | Low | Reference only unless archive format needs it. |
| Occlusion/culling experiments | `AABB_Cull/`, `Dima/bge.root/` | Research/utility | Culling, geometry, and graph experiments. | Source/project files present. | Legacy toolchains. | Unknown. | Low direct value. | Low | Keep as historical reference. |
| Physics/test utilities | `TestBed/`, selected `Dima/` folders | Tests/experiments | Physics, memory, and utility experiments. | Source/project files present. | Mixed legacy deps. | Unknown. | Debug/reference only. | Low | Do not prioritize. |
| External library projects | `External Library/`, `SDK/`, embedded third-party dirs | Library projects | Ogg, OpenAL, geometry, and other external dependency projects. | Legacy project files present. | Third-party licensing/provenance review. | Unknown. | Supports tool/editor builds. | Medium | Document before vendoring or building. |
| Gamedata/script project wrappers | `xrStalker/` | Project wrappers | Historical wrappers for scripts/gamedata. | Legacy `.vcproj`/`.sln` present. | Local proprietary data not in repo. | Unknown. | Low by itself. | Low | Do not commit gamedata. |

## D. Planned Tools Not Found In Repo

| Component | Path | Type | Purpose | Current state | Dependencies | Build status | Runtime/modding value | Restoration priority | Notes |
|---|---|---|---|---|---|---|---|---|---|
| LTX/config validator | Planned | CLI validator | Validate `.ltx` syntax, include/load order, and common runtime references. | Not found as standalone modern tool. | Runtime config parser behavior. | Not implemented. | Helps modders catch safe errors before launch. | High | Good first new read-only tool after archive work. |
| UI/XML validator | Planned | CLI validator | Validate UI XML files and string references. | Not found as standalone modern tool. | `xrXMLParser`, UI loader behavior. | Not implemented. | Useful after widescreen/UI restoration. | Medium | Keep read-only. |
| Lua/script syntax helper | Planned | CLI validator | Check Lua syntax and basic binding availability. | Not found as standalone modern tool. | `xrLUA`, restored binding inventory. | Not implemented. | Useful for script modding. | Medium | Avoid false positives. |
| OGF/OMF metadata inspector | Planned | CLI inspector | Read model/motion headers and metadata without conversion. | Not found as standalone modern tool. | Documented model formats. | Not implemented. | Useful for model pipeline research. | Medium | Read-only first. |
| Texture metadata inspector | Planned | CLI inspector | Inspect DDS/DXT metadata and expected texture variants. | Not found as standalone modern tool. | DDS parsing, texture loader behavior. | Not implemented. | Useful for asset compatibility. | Medium | Avoid proprietary samples. |
| Clean-room fixture generator | Planned | Test helper | Generate tiny non-proprietary fixtures for tool tests. | Not implemented. | Documented minimal formats. | Not implemented. | Enables CI-like tests without assets. | High | Synthetic data only. |

## E. Format / Documentation-Only Areas

| Component | Path | Type | Purpose | Current state | Dependencies | Build status | Runtime/modding value | Restoration priority | Notes |
|---|---|---|---|---|---|---|---|---|---|
| Archive formats | `docs/formats/archives.md` | Documentation | Record proven `.xp*` layout, unknowns, and safety constraints. | Active documentation. | `xrArchiveList`, `xr_unpack`, `xrFS`, `xrCompress`. | N/A. | Foundation for archive tools. | High | Do not document guesses as facts. |
| Runtime layout / VFS precedence | `docs/modding/runtime-layout.md` | Documentation | Explain runtime folders, archive mounting, and override behavior. | Existing documentation. | Runtime filesystem behavior. | N/A. | Helps safe local mod workflows. | High | Keep aligned with restored loader. |
| Asset policy | `docs/modding/assets-policy.md` | Documentation | Define legal boundaries for assets and archives. | Existing documentation. | None. | N/A. | Keeps repo clean. | High | Link from tool docs. |
| Level/model/particle/shader formats | Planned docs | Documentation | Record proven file formats for compilers/editors/converters. | Partial and scattered in source only. | Runtime/editor source review. | N/A. | Needed before converters. | Medium | Read-only research first. |
| Spawn/game graph serialization | Planned docs | Documentation | Record restored ALife/spawn/game graph binary layouts. | Runtime fixes exist but docs are incomplete. | Runtime serialization source. | N/A. | Needed before `xrAI` tooling. | High | Avoid data mutation until documented. |
| SDK/editor dependency map | Planned docs | Documentation | Map editor/compiler dependencies and tool generations. | Initial inventory only. | Legacy project files. | N/A. | Guides future CMake work. | High | Start before restoring GUI editors. |

## CMake Modularization Track

The root `CMakeLists.txt` still owns complex runtime targets while compatibility work is active. New and restored tools should use local per-directory `CMakeLists.txt` files, with the root file acting as orchestration over time.

Current local target ownership includes:

* `tools/CMakeLists.txt` for tool dispatch and tool options.
* `tools/xrArchiveList/CMakeLists.txt` for `xrArchiveList`.
* `tools/xr_unpack/CMakeLists.txt` for `xr_unpack`.
* `Editors/ECore/CMakeLists.txt` for experimental `xrECore`.
* `Editors/xrEProps/CMakeLists.txt` for experimental `xrEProps`.
* `Editors/LevelOptions/CMakeLists.txt` for experimental `LevelOptions`.
* `xrXMLParser/CMakeLists.txt` for `xrXMLParser`.
* `xr_3da/xrCDB/CMakeLists.txt` for `xrCDB`.

Restored editor targets may use the optional `XR_LEGACY_DX_ROOT` cache path.
The inventoried Summer 2004 DirectX 9.0 SDK is a strong historical match, but
the option is deliberately target-local and does not affect runtime targets.
See [Legacy DirectX SDK Support](legacy-directx.md).

The local editor dependency search found ColorPicker and Boost 1.33.1
candidates, but no ElPack, Borland/VCL installation, AlexMX control package, or
complete MagicFM SDK. Editor-only cache roots now exist for diagnostics and
future target-local probes: `XR_ELPACK_ROOT`, `XR_ALEXMX_ROOT`,
`XR_MAGICFM_ROOT`, and `XR_BORLAND_ROOT`. See [Local Editor Dependency
Inventory](editor-dependencies.md).

Opt-in ECore, EProps, LevelEditor, and LevelOptions configurations now print a
consolidated dependency summary. Missing or invalid editor roots remain
non-fatal configure warnings/status messages and predict the expected build
stops, especially `ElTree.hpp`.

`xr_unpack verify-extracted` performs read-only post-extraction checks for
missing files and unpacked-size mismatches, with optional path filtering.
* `xrParticles/CMakeLists.txt` for `xrParticles`.
* `xrLUA/CMakeLists.txt` for `xrLUA`.
* `xrNetServer/CMakeLists.txt` for `xrNetServer`.

Future options should remain explicit and opt-in where appropriate:

* `BUILD_XR_TOOLS` for broad tool dispatch if/when the tree grows enough to need it.
* `BUILD_XR_UNPACK` for the archive unpacker.
* `BUILD_XR_SDK_TOOLS` for restored command-line SDK tools.
* `BUILD_XR_EDITORS` for large GUI editor targets.
* `BUILD_XR_COMPILERS` for level/AI/compiler targets.

## Recommended Next Actions

1. Add synthetic archive fixtures and path-safety tests for `xr_unpack`.
2. Start a small read-only LTX/config validator scaffold.
3. Acquire and inventory lawful ElPack/Borland dependencies before continuing
   ECore, EProps, or LevelEditor GUI work.
4. Inventory `xrLC` and `xrAI` dependencies in more detail before attempting CMake restoration.
5. Keep GUI editors and host plugins paused at the documented package boundary.

## SDK GUI Checkpoint

The modern CMake milestone now covers `ETools`, experimental `xrECore`,
experimental `xrEProps`, and the experimental `LevelEditor` dependency shell.
Standalone contracts cover the audited non-GUI VCL value types:
`AnsiString`, `TMsgDlgType`, `TMsgDlgButtons`, and `TShiftState`.

This compatibility work stops deliberately before GUI behavior. ElPack tree
widgets, VCL forms/application services, AlexMX controls, and MagicFM APIs are
real external dependencies, not value types suitable for small shims.

The active blocker is `ElTree.hpp`/`elpackB6.lib`. Continuing the GUI path
requires a lawful ElPack and Borland/VCL installation, the AlexMX controls,
and matching MagicFM SDK headers. Do not fake these APIs or vendor third-party
packages. Until those dependencies are available, the recommended branch is
archive/tooling work or focused runtime restoration.

## Related Plans

* [Archive Unpacker Plan](unpacker-plan.md)
* [SDK Restoration Plan](sdk-restoration-plan.md)
* [LevelOptions CMake Shell](level-options-cmake.md)
* [Archive Formats](../formats/archives.md)
* [Modding Overview](../modding/overview.md)
