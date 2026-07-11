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
| Level compiler helper tools | `xrLC/close/`, `xrLC/xrDO_Light/`, `xrLC/xrHemisphere/` | Helper CLIs | Auxiliary level/light/hemisphere compilation support. | Legacy project files present; the smallest helpers now have opt-in CMake shells. | Same family as `xrLC`; `xrDO_Light` additionally requires a local MSVC x86 FreeImage import library. | `close` and `xrHemisphere` build. `xrDO_Light` also builds when `XR_DO_LIGHT_FREEIMAGE_LIB` points at an external generated import library. | Supports compiler workflow. | Medium | Restore one helper at a time. |
| AI / game graph compiler | `xrAI/` | Compiler tool | Builds AI maps, level graphs, game graphs, spawn/ALife graph data. | Legacy `.vcproj` present; not restored. | Runtime graph code, `xrSE_Factory` concepts, Lua/Luabind/Boost-era code, MagicFM-style library. | Unknown. | Required for spawn and AI-compatible mods. | High | Must match restored serialization formats. |
| Level editor | `Editors/LevelEditor/` plus historical generations | GUI editor | Historical level/world editing workflow. | Experimental opt-in `LevelEditor` CMake shell added behind `BUILD_XR_LEVEL_EDITOR`; sources are imported from the canonical Borland project without porting forms. | `xrECore`, `xrEProps`, `ETools`, runtime-style libraries, DirectX 9, Borland VCL/packages, ElPack/AlexMX, MagicFM, Boost 1.30.x. | Configure passes; target build stops in required `xrECore` at missing `ElTree.hpp`. Canonical project also references absent `Splash.cpp` and `resource.res`. | Dependency and source-ownership probe only. | Medium | See [Level Editor Inventory](level-editor-inventory.md) and [LevelEditor CMake Shell](level-editor-cmake.md). |
| Actor/model editor | `Editors/ActorEditor/`, older `Editor/ActorEditor/`, `Editors/!old/ActorEditor/` | GUI editor | Edits models, skeletons, motions, materials, and export metadata. | Experimental opt-in `ActorEditor` CMake shell added behind `BUILD_XR_ACTOR_EDITOR`; source/form ownership is imported from the active Borland project without porting VCL forms. | `xrECore`, `xrEProps`, `ETools`, `xrParticles`, runtime-style libraries, DirectX 9, ElPack, AlexMX controls, Borland/VCL, MagicFM SDK headers/import library. | Configure passes. A direct ActorEditor compile probe gets past local Borland CRT aliases, DirectSound include ordering, and audited VCL value shims, then stops at real package/UI dependencies (`Classes.hpp`, `ElTree.hpp`). `MagicFMDLLB.lib` and matching MagicFM SDK headers remain missing. | Important for model modding. | Medium | See [ActorEditor CMake Shell](actor-editor-cmake.md). |
| Particle editor | `Editors/ParticleEditor/`, older `Editor/ParticleEditor/`, `Editors/!old/ParticleEditor/` | GUI editor | Edits particle effects and related runtime data. | Experimental opt-in `ParticleEditor` CMake shell added behind `BUILD_XR_PARTICLE_EDITOR`; source/form ownership is imported from the active Borland project without porting VCL forms. | `xrECore`, `xrEProps`, `ETools`, `xrParticles`, runtime-style libraries, DirectX 9, ElPack, AlexMX controls, Borland/VCL, MagicFM SDK headers/import library. | Configure passes. The dependency-aware probe did not finish before timeout in the current local check; a direct ParticleEditor compile probe gets past local Borland CRT aliases, DirectSound include ordering, and audited VCL value shims, then stops at real package/UI dependencies (`Classes.hpp`, `ElTree.hpp`, `RenderWindow.hpp`). `MagicFMDLLB.lib` and matching MagicFM SDK headers remain missing. | Useful once particle formats and editor dependencies are documented. | Medium | See [ParticleEditor CMake Shell](particle-editor-cmake.md). |
| Shader editor | `Editors/ShaderEditor/`, older `Editor/ShaderEditor/`, `Editors/!old/ShaderEditor/` | GUI editor | Edits shader/material, sound environment, and game material descriptions. | Experimental opt-in `ShaderEditor` CMake shell added behind `BUILD_XR_SHADER_EDITOR`; source/form ownership is imported from the active Borland project without porting VCL forms. | `xrECore`, `xrEProps`, `ETools`, runtime-style libraries, DirectX 9, ElPack, AlexMX controls, Borland/VCL, MagicFM SDK headers/import library. | Configure passes. The dependency-aware probe did not finish before timeout in the current local check; a direct ShaderEditor compile probe gets past local Borland CRT aliases, DirectSound include ordering, and audited VCL value shims, then stops at missing `splash.h` plus real package/UI dependencies (`ElTree.hpp`, `Classes.hpp`, `RenderWindow.hpp`). The older `Editor/ShaderEditor/Lib/MagicFM.lib` is present, but matching SDK headers remain missing. | Useful for material workflows after editor dependencies exist. | Medium | See [ShaderEditor CMake Shell](shader-editor-cmake.md). |
| Editor core | `Editors/ECore/` | Shared editor library | Shared editor engine, thumbnails, object IO, properties, image helpers, and shader helpers. | Opt-in CMake target `xrECore` added as an experimental partial port. VCL form units are excluded for now. Tested non-GUI value shims are integrated for non-Borland builds only. | `xrCore`, `xrCDB`, `xrParticles`, `ETools`, DirectX 9/D3DX, `xrEProps`, Borland/VCL packages, ElPack/AlexMX, MagicFM. | Configure passes with `BUILD_XR_ECORE=ON`; compiler/value compatibility probes advance to the required `ElTree.hpp`/`elpackB6.lib` UI boundary. `XR_ELPACK_ROOT` provides an optional target-local path for lawful external dependency research, but ElPack remains a Borland UI boundary. | Foundation for restored editors. | High | See [ECore CMake Port Inventory](ecore-inventory.md), [VCL Compatibility Strategy](vcl-compat-strategy.md), and [ElPack Dependency](elpack-dependency.md). |
| Editor properties library | `Editors/xrEProps/` | Shared editor DLL | Property-grid, item-list, chooser, numeric, text, and shader-function UI support. | Experimental opt-in `xrEProps` CMake target added behind `BUILD_XR_EPROPS`; the full historical source/form set is retained. | `xrCore`, `ETools`, Borland VCL, ElPack, AlexMX controls, `.dfm` forms. | Configure passes; build stops honestly at `FolderLib.h` missing `ElTree.hpp` when `XR_ELPACK_ROOT` is unset. | Needed by editor apps. | High | See [xrEProps CMake Port Inventory](xreprops-inventory.md). |
| Level options tool | `Editors/LevelOptions/` | GUI DLL/plugin | Historical `xrLC_options.dll` build-options dialog for level compiler workflows. | Experimental opt-in `LevelOptions` CMake shell added behind `BUILD_XR_LEVEL_OPTIONS`; source/form ownership is represented without porting VCL forms. | `xrECore`, `xrEProps`, `ETools`, runtime-style libraries, DirectX 9, ElPack, AlexMX controls, Borland/VCL, ColorPicker/MagicFM-era editor packages. | Configure passes. A direct LevelOptions compile probe gets past the local Borland CRT compatibility issue and stops at `Editor/SceneProperties.h` missing `ElTree.hpp`; dependency-aware builds may stop earlier through required `xrECore`. The `.bpr` also references absent `xrLC_OptionsEntry.cpp`. | Optional helper for level compiler options once compiler/editor dependencies exist. | Low | See [LevelOptions CMake Shell](level-options-cmake.md). |
| DXT tool | `Editor/Tools/DXT/` | Converter/helper DLL | Historical DDS/DXT texture compression helper around NVIDIA's DXT library. | Experimental opt-in `DXT` CMake shell added behind `BUILD_XR_DXT`; preserves the VC6 project shape by excluding `nvdxt.cpp` and linking the bundled `nvDXTlib.lib`. | Legacy DirectX header `d3dx.h`, bundled `Editor/Tools/DXT/nvDXTlib.lib`. | Configure passes with `BUILD_XR_DXT=ON`; Release `DXT` target builds and exports `DXTCompress` from a 32-bit `DXT.dll`. The target uses target-local legacy DirectX include paths through `XR_LEGACY_DX_ROOT` or the existing DirectX SDK selection. | Texture pipeline support. | Medium | See [DXT CMake Shell](dxt-cmake.md). |
| Texture/DXT tools | `Editors/Tools/DXT/`, `xrDXTC/` | Converter/tool library | Broader texture compression, thumbnail, DDS/DXT workflows. | Legacy source/project files present. | NVIDIA DXT libraries, DirectX/image code. | Unknown beyond the old `Editor/Tools/DXT` DLL. | Useful for asset conversion. | Medium | Keep active-folder `Editors/Tools/DXT` and `xrDXTC` separate from the first `DXT` helper probe. |
| Editor tools library | `Editors/Tools/ETools/` | Shared tool DLL | D3DX wrapper, editor helper exports, ray-triangle tests, and VIPM/progressive-mesh support. | Opt-in CMake target `ETools` added as a partial port. VIPM/xrQSlim sources are excluded until `xrQSlim` is restored. | `xrCore`, DirectX 9/D3DX, ColorPicker import library, future `xrQSlim`. | Configure passes with `BUILD_XR_ETOOLS=ON`; partial Release target builds after linking the matching historical `Editor/Tools/ETools/ColorPicker.lib` import library. | Early LevelEditor dependency and safer than full VCL GUI port. | High | See [ETools CMake Port Inventory](etools-inventory.md). Runtime use of the color picker export may still need the matching `ColorPicker.dll`. |
| wxWidgets SDK editor shell | `Editors/wxSDKEditor/` | Experimental GUI shell | Tests a parallel, portable SDK UI boundary without porting historical editor behavior. | Opt-in `wxSDKEditor` target behind `BUILD_XR_WX_SDK_EDITOR`; the wx-independent `EditorTreeModel` drives the demo hierarchy through `IEditorTree`. Rename/create/delete, strict atomic `.wx_tree_snapshot` v1 save/load, and a development-only `.wx_tree_paths` importer remain model-backed. | External wxWidgets built for Win32/x86, normally supplied through vcpkg `wxwidgets:x86-windows`. | Normal configuration is unaffected while OFF; the opt-in Win32 shell builds and passes a no-data launch smoke check. Separate `BUILD_XR_WX_SDK_EDITOR_MODEL_TESTS` / `wxSDKEditorModelTests` builds and passes without wxWidgets. | Foundation for incremental, testable UI migration after historical data/command contracts are understood. | Experimental | Headless tests cover model mutation, path-list import, and malformed snapshots without assets or runtime data. Neither development format is a real SDK format, and no `xrEProps` or historical editor logic is loaded. See [Experimental wxWidgets SDK Editor](wx-sdk-editor.md) and [ElPack Migration Map](wx-editor-elpack-migration.md). |
| LWO tool | `Editors/Tools/LWO/` plus older `Editor/Tools/LWO/` reader sources | Converter/helper DLL | LightWave object/envelope handling utility. | Experimental opt-in `LWO` CMake shell added behind `BUILD_XR_LWO`; active project source ownership is represented and missing reader entries are resolved from the older generation when present. | LightWave object parsing code. | Configure passes; Release `LWO` target builds and exports `_LWO_CloseFile@4` / `_LWO_ImportObject@8` from a 32-bit `LWO.dll`. | Import/export pipeline support. | Medium | See [LWO CMake Shell](lwo-cmake.md). |
| Gauss tool | `Editor/Tools/gauss/` | Image-filter helper DLL | Exposes a Gaussian kernel plus kernel normalization and RGBA filtering helpers. | Experimental opt-in `Gauss` CMake target added behind `BUILD_XR_GAUSS`; uses the complete VC6 project rather than the incomplete later snapshot. | Target-local DirectX 8-era headers/libraries; bounded Gauss-local debugger logging. | Configure and Release build pass. `dumpbin` verifies an x86 PE32 `gauss.dll` exporting `gauss`, `ip_BuildKernel`, and `ip_ProcessKernel`, with only kernel/MSVC/UCRT dependencies. | Small non-GUI image-processing helper. | Low | See [Gauss CMake Shell](gauss-cmake.md). |
| xrHemisphere | `xrLC/xrHemisphere/` | Helper DLL | Hemisphere sample generation for level compiler lighting workflows. | Experimental opt-in `xrHemisphere` CMake shell added behind `BUILD_XR_HEMISPHERE`; source ownership is limited to the historical helper DLL. | `xrCore`. | Configure passes; Release `xrHemisphere` target builds and exports `_xrHemisphereBuild@24` from a 32-bit `xrHemisphere.dll`. | Supports future level compiler restoration. | Medium | See [xrHemisphere CMake Shell](xrhemisphere-cmake.md). |
| close helper | `xrLC/close/` | Helper executable | Tiny Win32 mailslot helper from the level-compiler tree. | Experimental opt-in `close` CMake shell added behind `BUILD_XR_CLOSE`; source ownership is limited to the historical helper executable. | `xrCore`. | Configure passes; Release `close` target builds as a 32-bit Windows subsystem executable. | Low-level compiler workflow helper. | Low | See [close CMake Shell](close-cmake.md). |
| xrDO_Light | `xrLC/xrDO_Light/` | Helper executable | Historical detail-object lighting helper from the level-compiler tree. | Experimental opt-in `xrDO_Light` CMake shell added behind `BUILD_XR_DO_LIGHT`; source ownership mirrors `xrDO_Light.vcproj` and does not pull in full `xrLC`. | `xrCore`, `xrCDB`, `xrHemisphere`, DirectX 9/D3DX, MSVC-compatible FreeImage import library. | Configure passes; Release `xrDO_Light` builds as a 32-bit Windows GUI executable when `XR_DO_LIGHT_FREEIMAGE_LIB` points at an external MSVC x86 import library generated from `FreeImage.dll`. The in-repo `Editor/ShaderEditor/Lib/FreeImage.lib` remains unusable with MSVC. | Supports future level compiler restoration. | Medium | See [xrDO_Light CMake Shell](xrdolight-cmake.md) and [FreeImage Dependency](freeimage-dependency.md). |
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
* `Editors/ShaderEditor/CMakeLists.txt` for experimental `ShaderEditor`.
* `Editors/ParticleEditor/CMakeLists.txt` for experimental `ParticleEditor`.
* `Editors/ActorEditor/CMakeLists.txt` for experimental `ActorEditor`.
* `Editors/Tools/LWO/CMakeLists.txt` for experimental `LWO`.
* `Editor/Tools/DXT/CMakeLists.txt` for experimental `DXT`.
* `xrLC/xrHemisphere/CMakeLists.txt` for experimental `xrHemisphere`.
* `xrLC/close/CMakeLists.txt` for experimental `close`.
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

Opt-in ECore, EProps, LevelEditor, LevelOptions, ShaderEditor,
ParticleEditor, and ActorEditor configurations now print a consolidated
dependency summary.
Missing or invalid editor roots remain
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

1. Continue with one small isolated non-GUI helper, or return to a focused
   runtime investigation such as pickup/use interaction, the trader crash, or
   shader `CConstant` startup initialization.
2. Add synthetic archive fixtures and path-safety tests for `xr_unpack`.
3. Start a small read-only LTX/config validator scaffold.
4. Acquire and inventory lawful ElPack, Borland/VCL, AlexMX, and MagicFM SDK
   dependencies before continuing SDK GUI work.
5. Keep GUI editors and host plugins paused at the documented package boundary.

## SDK GUI Checkpoint

The modern CMake milestone now covers `ETools`, experimental `xrECore`,
experimental `xrEProps`, and the experimental LevelEditor, LevelOptions,
ShaderEditor, ParticleEditor, and ActorEditor dependency shells.
Standalone contracts cover the audited non-GUI VCL value types:
`AnsiString`, `TMsgDlgType`, `TMsgDlgButtons`, and `TShiftState`.

Current checkpoint:

The currently building non-GUI set is `ETools`, `LWO`, `DXT`,
`xrHemisphere`, `close`, `xrDO_Light` with its explicit external FreeImage
import library, and `Gauss`. Verified binary exports and dependency conditions
are recorded below and in each target's linked page. GUI/editor rows remain
configuration probes only.

| Target | Option | Current result | First blocker class |
|---|---|---|---|
| `ETools` | `BUILD_XR_ETOOLS` | Builds as the first modern editor helper target. | VIPM/xrQSlim source restoration remains deferred, but the partial target links. |
| `xrECore` | `BUILD_XR_ECORE` | Configures and advances through audited value/compiler compatibility. | ElPack `ElTree.hpp` / `elpackB6.lib`. |
| `xrEProps` | `BUILD_XR_EPROPS` | Configures with the historical source/form set retained. | ElPack `ElTree.hpp` from property-tree UI code. |
| `LevelEditor` | `BUILD_XR_LEVEL_EDITOR` | Configures as a dependency/source shell. | Required `xrECore` dependency stops at ElPack; project also has missing `Splash.cpp` / `resource.res`. |
| `LevelOptions` | `BUILD_XR_LEVEL_OPTIONS` | Configures as a dependency/source shell. | `SceneProperties.h` stops at ElPack `ElTree.hpp`; project also references missing `xrLC_OptionsEntry.cpp`. |
| `ShaderEditor` | `BUILD_XR_SHADER_EDITOR` | Configures as a dependency/source shell. | Missing `splash.h`/`Splash.cpp`, plus ElPack/VCL/editor GUI package headers. |
| `ParticleEditor` | `BUILD_XR_PARTICLE_EDITOR` | Configures as a dependency/source shell. | Borland/VCL `Classes.hpp`, ElPack `ElTree.hpp`, and editor GUI headers such as `RenderWindow.hpp`. |
| `ActorEditor` | `BUILD_XR_ACTOR_EDITOR` | Configures as a dependency/source shell. | Borland/VCL `Classes.hpp`, ElPack `ElTree.hpp`, AlexMX `mxPlacemnt.hpp`, and missing editor controls such as `Gradient.hpp`. |
| `LWO` | `BUILD_XR_LWO` | Builds as a non-GUI LightWave object helper DLL. | Needs asset-free export/load smoke tests. |
| `DXT` | `BUILD_XR_DXT` | Builds as a non-GUI DDS/DXT texture compression helper DLL. | Needs asset-free or synthetic compression smoke tests. |
| `xrHemisphere` | `BUILD_XR_HEMISPHERE` | Builds as a non-GUI hemisphere sampling helper DLL. | Needs asset-free callback contract smoke tests. |
| `close` | `BUILD_XR_CLOSE` | Builds as a non-GUI xrLC mailslot helper executable. | Needs integration context before runtime use. |
| `xrDO_Light` | `BUILD_XR_DO_LIGHT` | Builds as a non-GUI detail-object lighting helper executable with an external MSVC x86 FreeImage import library. | Requires `XR_DO_LIGHT_FREEIMAGE_LIB`; `FreeImage.dll` remains a runtime dependency. |
| `Gauss` | `BUILD_XR_GAUSS` | Release builds as a verified x86 image-filter DLL with three expected exports. | Win32 load-only and synthetic image-kernel contracts remain future work. |

This compatibility work stops deliberately before GUI behavior. ElPack tree
widgets, VCL forms/application services, AlexMX controls, and MagicFM APIs are
real external dependencies, not value types suitable for small shims.

The active blocker is `ElTree.hpp`/`elpackB6.lib`. Continuing the GUI path
requires a lawful ElPack and Borland/VCL installation, the AlexMX controls,
and matching MagicFM SDK headers. Do not fake these APIs or vendor third-party
packages. Until those dependencies are available, the recommended branch is
archive/tooling work or focused runtime restoration.

Repository hygiene at this checkpoint is strict: do not commit build folders,
logs, extracted data, generated FreeImage `.lib`/`.def` files, or external
DLLs/libraries. Keep generated import libraries and all third-party binaries in
local toolchain/runtime folders outside the repository.

Explicitly prohibited for the SDK GUI branch:

* fake `ElTree.hpp` or placeholder ElPack tree widgets;
* fake `TForm`, `TFrame`, `Application`, `Classes.hpp`, or other VCL GUI
  runtime behavior;
* vendor third-party package files into this repository;
* touch runtime targets to work around editor-only dependency gaps.

## Related Plans

* [Archive Unpacker Plan](unpacker-plan.md)
* [SDK Restoration Plan](sdk-restoration-plan.md)
* [ShaderEditor CMake Shell](shader-editor-cmake.md)
* [ParticleEditor CMake Shell](particle-editor-cmake.md)
* [ActorEditor CMake Shell](actor-editor-cmake.md)
* [LevelOptions CMake Shell](level-options-cmake.md)
* [LWO CMake Shell](lwo-cmake.md)
* [DXT CMake Shell](dxt-cmake.md)
* [xrHemisphere CMake Shell](xrhemisphere-cmake.md)
* [close CMake Shell](close-cmake.md)
* [xrDO_Light CMake Shell](xrdolight-cmake.md)
* [FreeImage Dependency](freeimage-dependency.md)
* [Gauss CMake Shell](gauss-cmake.md)
* [Experimental wxWidgets SDK Editor](wx-sdk-editor.md)
* [wx Editor ElPack Migration Map](wx-editor-elpack-migration.md)
* [Archive Formats](../formats/archives.md)
* [Modding Overview](../modding/overview.md)
