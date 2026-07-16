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

## xrCore Audit

Original project files inspected:

- `xrCore/xrCore.vcproj`
- `xrCore/xrCore.sln`
- `xrPlugins/xrCore/xrCoreStatic.vcproj`

`xrCore/xrCore.vcproj` was selected as the historical source of truth because it is the runtime DLL project referenced by the engine solution and emits `xrCore.dll`/`xrCore.lib`. `xrPlugins/xrCore/xrCoreStatic.vcproj` shares the project GUID but is a plugin-side static-library variant with `XRCORE_STATIC`; it was not used for the runtime target.

Key original settings reproduced:

- Visual Studio C++ 7.10 project.
- `Win32` platform only.
- Configuration type `2` shared library.
- `CharacterSet="2"` Multi-Byte.
- Debug definitions: `WIN32`, `_DEBUG`, `_WINDOWS`, `_USRDLL`, `XRCORE_EXPORTS`.
- Release definitions: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRCORE_EXPORTS`.
- Debug runtime: multi-threaded debug DLL.
- Release runtime: multi-threaded DLL.
- Warning level 4.
- RTTI enabled.
- Default calling convention stdcall (`/Gz`).
- Debug exceptions enabled; Release exceptions disabled.
- PCH through `stdafx.h`, created by `stdafx.cpp` in the original project and represented through CMake target precompiled headers.
- Historical output path `x:\` replaced by the repository output convention `build/bin`, `build/lib`, and `build/pdb`.

Compatibility fixes applied for modern MSVC:

- Local `typeinfo.h` shim includes modern `<typeinfo>`.
- Local `dxerr9.h` shim lets `xrDebug.cpp` use its existing `FormatMessage` fallback without requiring legacy `dxerr9.lib`.
- Local `afxres.h` shim includes Windows SDK `winres.h` for the resource script.
- `xray_msvc_compat.hpp` is included through the target PCH to make old STL adapter declarations available.
- `xrMemory.h` global new/delete overrides now explicitly use `__cdecl`, matching modern CRT declarations while preserving the intended allocator path.
- `_stl_extensions.h` no longer depends on removed private `std::_Construct`/`std::_Destroy` helpers.
- `_USE_32BIT_TIME_T` keeps `_finddata_t`, `time_t`, and `FS_File::size` aligned with the Win32-era project assumptions.
- `LocatorAPI_defs.h` includes `<io.h>` before selecting `_FINDDATA_T`, matching the UCRT's mapped `_finddata_t`.
- `xrMemory_subst_msvc.h` const `xr_delete` overload now actually calls the selected `xr_special_free` functor with a const-cast pointer, matching the 1935 restoration fix.
- `xrDebug.cpp` uses `<new.h>` for modern `_set_new_mode` and `_set_new_handler` declarations instead of hand-written old CRT prototypes.
