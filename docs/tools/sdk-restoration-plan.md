# SDK Restoration Plan

This document describes a future restoration roadmap for SDK, editor, converter, archive, and modding tools. It is planning documentation only; no tools are implemented here.

The current repository focus remains source restoration for the build 1935 engine/runtime. SDK restoration should proceed after the runtime behavior, data loading, and file format assumptions are documented well enough to avoid accidental modernization or proprietary data churn.

## Phase 0: Inventory

Goals:

* classify every tool/editor/project file found in the tree;
* identify duplicate generations such as `Editor/`, `Editors/`, and `Editors/!old/`;
* separate runtime-critical tools from optional SDK/editor tools;
* record external dependencies and host SDK requirements;
* document which tools require proprietary local data for manual testing.

Outputs:

* updated [Tools and SDK Status](status.md);
* dependency notes for each candidate tool;
* a list of planned components not found in this repository.

## Phase 1: Build System Discovery

Goals:

* read historical `.sln`, `.vcproj`, `.dsp`, `.dsw`, and Borland project files;
* identify output names, compiler definitions, include paths, and linked libraries;
* determine whether each target is command-line, DLL, editor UI, plugin, or host-specific extension;
* decide which targets should enter modern CMake first.

Command-line tools should be prioritized before large GUI editors because they are easier to test with synthetic data.

## Phase 2: Dependency Mapping

Goals:

* map DirectX 9, D3DX, DirectInput, DirectSound, OpenAL, EAX, Ogg/Vorbis, FreeImage, NVIDIA DXT, QSlim, MagicFM, MFC, SciLexer, CJ60Lib, and host SDK dependencies;
* decide which dependencies can be built from source, imported locally, or deferred;
* keep dependency configuration target-local and documented;
* avoid depending on proprietary game assets for compilation or tests.

## Phase 3: Minimal Tool Compilation

Goals:

* restore minimal command-line tools first;
* prefer read-only or diagnostic tools before write-capable tools;
* add synthetic fixtures for each supported format;
* keep behavior close to historical code while applying narrow modern compiler fixes.

Candidate early targets:

* archive listing and unpacker research tools;
* `xrCompress` compatibility documentation;
* script/config validators;
* minimal format inspection tools.

## Phase 4: Runtime-Compatible Workflows

Goals:

* confirm tools produce data compatible with the restored build 1935 runtime;
* document runtime layout and virtual filesystem precedence;
* define mod-directory override workflows;
* validate only with user-supplied lawful local data or synthetic fixtures.

This phase should avoid distributing extracted data. Manual compatibility notes can describe expected local inputs without committing them.

## Phase 5: Documentation And Tutorials

Goals:

* write tutorials for listing and extracting local archives;
* document config/script inspection;
* document separate mod-directory layout;
* document editor/compiler workflows as they become stable;
* maintain asset policy warnings near any workflow that touches proprietary data.

## Tool Category Plans

| Tool category | Purpose for modders | Expected inputs/outputs | Likely dependencies | Risks | Restoration priority |
|---|---|---|---|---|---|
| Archive unpacker | Inspect and extract local archive contents for research and mod setup. | Input: lawful local `.xp*` archive. Output: listed entries or extracted files under a chosen directory. | `xrFS`, `xrCompress`, LZHUF/LZO helpers, path-safety layer. | Path traversal, overwrite behavior, incomplete format knowledge, proprietary data handling. | High |
| Archive packer | Potentially create archives for local testing if legally and technically appropriate. | Input: local mod files. Output: archive package. | Full archive specification, deterministic writer, integrity metadata. | Could enable repack misuse; should not support cracks or proprietary redistribution. | Low |
| Level editor | Edit scenes and level data before compilation. | Input: local SDK scene data/assets. Output: editor scene files and compiler-ready data. | Editor core, DirectX, Borland/VCL or ported UI, image and geometry libraries. | Multiple editor generations, proprietary asset requirements, UI/toolchain age. | Medium |
| Actor/model editor | Edit meshes, skeletons, motions, and material assignments. | Input: local model source files and textures. Output: runtime/editor model data. | Editor core, DirectX, image libraries, object IO, skeleton/motion code. | Format drift, exporter dependency, high UI complexity. | Medium |
| Shader/material tools | Edit engine and compiler shader/material descriptions. | Input: local shader/material definitions. Output: runtime and compiler shader metadata. | DirectX shader compiler era, editor rendering code, `Shader_xrLC` libraries. | SDK version sensitivity and runtime/compiler mismatch. | Medium |
| Level compiler/light compiler | Build geometry, collision, lightmaps, sectors, and visibility data. | Input: editor-exported level data. Output: runtime level files, CFORM, lightmaps, AI/compiler artifacts. | `xrLC`, FreeImage, QSlim/OpenMesh/NVIDIA geometry code, `xrDXTC`, DirectX-era headers. | Large dependency surface, long-running jobs, binary format compatibility. | High |
| Spawn/game graph tools | Build AI maps, level graphs, game graphs, and spawn data. | Input: editor exports, graph points, game object definitions. Output: runtime graph/spawn files. | `xrAI`, `xrSE_Factory`, Lua/Luabind/Boost-era code, MagicFM-style geometry libraries. | Serialization compatibility and object factory drift. | High |
| Format converters | Convert source art and intermediate formats into X-Ray runtime formats. | Input: source models/textures/animations. Output: `.ogf`, textures, motions, or related runtime files. | Editor tools, plugins, QSlim, DXT libraries, host SDKs. | Host SDK licensing, binary library provenance, proprietary asset temptation. | Medium |
| Config/script validation tools | Validate `.ltx`, XML, Lua scripts, and references before runtime launch. | Input: local configs/scripts. Output: diagnostics. | `xrLUA`, `xrXMLParser`, config parser behavior. | False positives if runtime parser behavior is not matched. | Medium |
| Clean-room sample runtime tools | Generate tiny non-proprietary fixtures for tests and tutorials. | Input: simple generated metadata. Output: synthetic archives or minimal config/script samples. | Documented formats and small generators. | Accidentally depending on proprietary naming/data. | Medium |

## Policy Guardrails

SDK restoration must not add proprietary assets, original archives, extracted `gamedata/`, repacks, cracks, leaked data, or gamedata dumps. Repository tests should rely on synthetic fixtures. Manual tests may use the user's own legally obtained data outside version control.
