# Tools and SDK Status

This page inventories modding, SDK, archive, and asset-tooling components found in the build 1935 restoration tree. The current repository focus remains source restoration for the engine/runtime; tool restoration is a future planning track.

No proprietary assets, original archives, extracted `gamedata/`, repacks, cracks, leaked data, or gamedata dumps should be added to this repository. Tools are intended for lawful research, compatibility testing, and use with the user's own legally obtained game data.

## Known Components Found In Tree

| Component | Path | Purpose | Current state | Dependencies | Restoration priority | Notes |
|---|---|---|---|---|---|---|
| Archive listing helper | `tools/xrArchiveList/` | Lists virtual entries from X-Ray `.xp*` archives. | Modern CMake target exists; read-only diagnostic. | `xrCore`, local `LzHuf.cpp`. | High | Current helper does not extract, modify, or repack archives. |
| Runtime compressor / packer | `xrCompress/` | Historical archive/data compression executable. | Root CMake target exists; historical `.vcproj` also present. | `xrCore`, bundled LZO code, Win32 console runtime. | High | Treat as source for archive format research before adding any extraction support. |
| Filesystem/archive library | `xrFS/` | Historical filesystem, locator, LZHUF, LZO, and archive support code. | Source and `.vcproj` present; not in root CMake. | Win32, old X-Ray filesystem code, compression helpers. | High | Likely reference point for unpacker behavior and path normalization. |
| Archive unpacker | `tools/xr_unpack/`, `docs/tools/unpacker-plan.md` | Future command-line unpacker design. | Initial opt-in CMake/CLI scaffold exists; archive parsing is not implemented. | Archive format research, path-safety layer, synthetic tests. | High | Must not distribute archives or extracted proprietary data. |
| Level compiler | `xrLC/` | Builds level geometry, collision, lightmaps, sectors, visibility, and game/export data. | Legacy `.sln`/`.vcproj` present; not restored in root CMake. | DirectX 9-era headers/libs, FreeImage, QSlim/OpenMesh/NV/RAPID-style geometry code, `xrDXTC`, `xrCore`-style helpers. | High | Important for runtime-compatible modding workflows, but high dependency and data-format risk. |
| Level compiler helper tools | `xrLC/close/`, `xrLC/xrDO_Light/`, `xrLC/xrHemisphere/` | Auxiliary level/light/hemisphere compilation helpers. | Legacy `.vcproj` projects present. | Same family as `xrLC`; likely DirectX/Win32 and geometry helpers. | Medium | Inventory source ownership and command-line contracts before migration. |
| AI / game graph compiler | `xrAI/` | AI-map, level graph, game graph, spawn, and ALife graph construction tooling. | Legacy `.vcproj` present; not restored in root CMake. | `xrCore`-style helpers, `xrSE_Factory` concepts, Lua/Luabind/Boost-era code, MagicFM binary lib appears present. | High | Important for spawn/game graph workflows; must be aligned with restored runtime serialization. |
| Level editor | `Editor/LevelEditor/`, `Editors/LevelEditor/`, `Editors/!old/LevelEditor/` | Historical world/level editing UI and scene workflow. | Source trees present; no modern CMake target. | Borland/VCL-era editor stack, DirectX, editor core, image/geometry libraries. | Medium | Multiple generations exist; decide canonical tree before porting. |
| Actor/model editor | `Editor/ActorEditor/`, `Editors/ActorEditor/`, `Editors/!old/ActorEditor/` | Edits models, skeletons, motions, materials, and export metadata. | Source trees present; no modern CMake target. | Borland/VCL-era UI, DirectX/editor core, image and mesh helpers. | Medium | Key modding workflow component, but likely harder than command-line tools. |
| Particle editor | `Editor/ParticleEditor/`, `Editors/ParticleEditor/`, `Editors/!old/ParticleEditor/` | Edits particle effects and related runtime data. | Source trees present; no modern CMake target. | Editor core, DirectX/editor rendering, Borland/VCL-era UI. | Medium | Coordinate with `xrParticles` runtime format support. |
| Shader editor | `Editor/ShaderEditor/`, `Editors/ShaderEditor/`, `Editors/!old/ShaderEditor/` | Edits engine/compiler shader and material descriptions. | Source trees present; no modern CMake target. | DirectX shader compiler era, editor rendering code, legacy DX libraries. | Medium | Important for materials but may require careful DirectX SDK versioning. |
| Editor core | `Editors/ECore/` | Shared editor engine, rendering, thumbnails, object IO, properties, and shader helpers. | Source present; no modern CMake target. | DirectX 9-era rendering, image libraries, shared editor components. | Medium | Likely dependency for restored editor applications. |
| Level options tool | `Editor/LevelOptions/`, `Editors/LevelOptions/` | Level option/configuration editor support. | Source present; no modern CMake target. | Editor components, DirectX/editor runtime assumptions. | Low | Inventory after core editor build strategy is known. |
| Texture/DXT tool | `Editors/Tools/DXT/`, `Editor/Tools/DXT/`, `xrDXTC/` | Texture compression and DXT/DDS-related conversion support. | Legacy `.vcproj`/`.dsp` and source present; not in root CMake. | NVIDIA DXT libraries, DirectX/image code, Win32 console or DLL runtime. | Medium | Useful for asset conversion; external binary library provenance must be documented. |
| Editor tools library | `Editors/Tools/ETools/` | Mesh optimization and shared asset-tool routines. | Legacy solution/project present. | DirectX wrapper, mesh/geometry code, Win32 toolchain. | Medium | Likely dependency or reference for editor/export pipeline. |
| Gauss tool | `Editors/Tools/gauss/` | Legacy utility with filesystem/math/log scaffolding. | Legacy `.vcproj` present. | Win32, editor-style filesystem and math helpers. | Low | Purpose needs source-level confirmation before prioritizing. |
| LWO tool | `Editors/Tools/LWO/` | LightWave object/envelope handling utility. | Legacy `.vcproj` present. | LightWave object parsing code, Win32 toolchain. | Medium | Relevant to import/export pipeline. |
| 3ds Max / LightWave plugin sources | `Plugins/`, `samples/xskinexp/` | Exporters/import helpers for models, skins, keys, shaders, and editor formats. | Legacy `.dsp`/`.vcproj` sources present. | Host SDKs such as 3ds Max or LightWave, Win32/MFC-era toolchains. | Low | Restore only after core formats and clean-room workflows are documented. |
| QSlim / mesh simplification SDK | `SDK/QSlim/`, `xrQSlim/`, `xrProgressive/` | Mesh simplification, progressive mesh, and viewer utilities. | Legacy projects/source present; not root CMake for SDK tools. | QSlim/MixKit/libgfx, OpenGL for viewers. | Medium | Useful for model/LOD pipelines and `xrLC` geometry optimization. |
| Launcher | `xrLauncher/` | Historical launcher/configuration UI. | Legacy managed C++ `.vcproj` present; not root CMake. | .NET/Managed C++ era, WinForms/resources. | Low | Runtime can be tested without this; treat as separate UI restoration. |
| Script debug IDE | `xr_3da/xrScriptDebugIde/` | Script debugging/editor IDE support. | Legacy `.vcproj` present; not root CMake. | MFC, CJ60Lib, SciLexer/Scintilla. | Low | Useful for tool developers after scripting runtime stabilizes. |
| Lua compiler utility | `Dima/xrLuaCompiler/` | Lua/script compiler or validation utility. | Legacy `.vcproj` present. | Lua, Luabind-era code, Boost-compatible headers. | Medium | Candidate for early script/config tooling after inventory. |
| Gamedata/script project files | `xrStalker/` | Historical project wrappers for gamedata/scripts. | Legacy `.vcproj`/`.sln` present. | Proprietary data not included; local lawful runtime data only. | Low | Do not commit generated or extracted gamedata. |

## Planned / Not Found Yet

| Component | Path | Purpose | Current state | Dependencies | Restoration priority | Notes |
|---|---|---|---|---|---|---|
| General archive extractor | Planned | Extract selected or all files from supported archives. | Not implemented. | Archive format research, `xrFS`/`xrCompress` behavior, safety layer. | High | See [Archive Unpacker Plan](unpacker-plan.md). |
| General archive verifier | Planned | Validate archive structure, directory table, sizes, and hashes/checksums when known. | Not implemented. | Format documentation and synthetic fixtures. | High | Should be read-only and safe for local lawful archives. |
| Archive packer | Planned / not found as standalone safe target | Create archives for mod workflows, if legally and technically appropriate. | Not planned for initial implementation. | Full format documentation, deterministic tests, clear legal review. | Low | No repack or crack support. |
| Config/script validator | Planned | Validate `.ltx`, XML, Lua scripts, and common runtime references. | Not found as standalone modern tool. | `xrLUA`, `xrXMLParser`, config parser behavior. | Medium | Prefer clean-room tests and user-supplied local data. |
| Clean-room sample runtime data tools | Planned | Generate tiny non-proprietary sample fixtures for tests and tutorials. | Not implemented. | Synthetic formats and minimal runtime compatibility fixtures. | Medium | Must avoid proprietary assets and gamedata dumps. |

## Near-Term Documentation Tasks

* Document runtime layout.
* Inventory SDK/tools.
* Document archive formats.
* Build minimal archive list/extract tool.
* Write modding tutorials.

## CMake Modularization Track

The root `CMakeLists.txt` still owns most restored runtime targets while compatibility work is active. New and restored tools should move toward local per-directory `CMakeLists.txt` files first, with the root file acting as orchestration over time.

Current phases:

* `tools/CMakeLists.txt` owns tool build options and delegates tool subdirectories.
* `tools/xrArchiveList/CMakeLists.txt` owns the `xrArchiveList` target.
* `tools/xr_unpack/CMakeLists.txt` owns the `xr_unpack` target.
* `xrXMLParser/CMakeLists.txt` owns the `xrXMLParser` runtime target as the first low-risk runtime extraction.
* `xr_3da/xrCDB/CMakeLists.txt` owns the `xrCDB` runtime target.

Future phases should move runtime components one target at a time, preserving target names, output paths, dependency discovery, and historical compatibility settings.

## Related Plans

* [Archive Unpacker Plan](unpacker-plan.md)
* [SDK Restoration Plan](sdk-restoration-plan.md)
* [Archive Formats](../formats/archives.md)
