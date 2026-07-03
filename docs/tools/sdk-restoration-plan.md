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

The first large historical SDK candidates to inventory in depth are `Editors/ECore`, `xrLC`, and `xrAI`, but they should not be ported until dependencies, command-line contracts, and binary format compatibility are documented. `Editors/Tools/ETools` now has a partial opt-in CMake target; `LevelEditor.exe` should wait until ETools, its ColorPicker dependency, `xrQSlim`, the Borland/VCL boundary, and shared editor DLL dependencies are mapped.

## Related Documents

* [Tools and SDK Status](status.md)
* [Archive Unpacker Plan](unpacker-plan.md)
* [Archive Formats](../formats/archives.md)
* [Modding Overview](../modding/overview.md)
* [Assets Policy](../modding/assets-policy.md)
