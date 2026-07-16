# Archaeology Notes

## Relationship to the 1935 Restoration

The previous `xray-soc-history` restoration is useful as a knowledge base, not as a file-copy source. It restored a build 1935-era tree with CMake, Visual Studio 2022, Win32 output conventions, dependency cache variables, compatibility shims, and extensive documentation.

Reusable lessons:

- Keep original Visual Studio project files as historical evidence.
- Add CMake target recovery incrementally.
- Preserve target names and output DLL/exe names.
- Centralize output directories into `bin`, `lib`, and `pdb`.
- Expose dependency roots through cache variables instead of hardcoded local paths.
- Treat old DirectX SDK, OpenAL, EAX, Xiph, Lua, luabind, Boost, Loki, and ODE as historically sensitive dependencies.
- Remove hardcoded default library paths through explicit target links only after the owning target is understood.
- Prefer target-local MSVC compatibility switches and source shims.
- Document runtime and data assumptions separately from build fixes.

Directly adaptable ideas:

- Win32/x86 guardrails.
- CMake output layout variables.
- Dependency-root cache variables.
- Documentation structure for dependency policy, path audit, known issues, and build order.
- Read-only helper tools can be introduced later when backed by source evidence and tests.

1935-specific work that must not be copied blindly:

- Exact source file lists.
- Compatibility source patches.
- Lua/luabind overlays.
- Runtime behavior fixes.
- Renderer linker workarounds.
- Archive/runtime data fallbacks.
- Generated build directories and local test outputs.

## 2571-Specific Observations

Compared with the restored 1935 tree, the 2571 candidate has additional top-level areas such as `IS`, `MTSetup`, `sql_config`, `xray_plugins`, `xrLUA_JIT`, `xrPlugins`, `xrStalkerDOCs`, `xrStalkerOld`, `xrTextureList`, and `xrUpdate`.

The 2571 candidate also contains a larger mix of old plugin and experimental projects. The initial restoration should focus on the core runtime and small foundational libraries before editors, SDK tools, or DCC plugins.

## Evidence Collected

- Current branch is `restoration-2571`.
- Root initially had no README, docs, CMake root, CMake presets, `.gitignore`, or `.gitattributes`.
- `xr_3da/XR_3DA.sln` is Visual Studio Solution Format Version 8.00.
- Core `.vcproj` files are Visual Studio C++ `Version="7.10"`.
- Legacy `.dsp` files are Visual C++ 6 project files.
- The main runtime project uses `ENGINE_BUILD` and `SECURE_GAMEDATA` preprocessor definitions.
- The checked-in projects strongly indicate Win32/x86 as the original platform.
