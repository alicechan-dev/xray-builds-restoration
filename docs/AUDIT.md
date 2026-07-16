# Initial Audit

This audit compares the previous build 1935 restoration in `D:/Projects/Github/xray-soc-history` with the 2571 candidate revision in this repository.

## Reusable Lessons from 1935

- Use CMake as an additive restoration layer, not a replacement for the historical Visual Studio files.
- Keep target names and output names aligned with the original DLLs, EXEs, and import libraries.
- Normalize outputs into `bin`, `lib`, and `pdb` directories instead of writing to `x:\`.
- Treat Win32/x86 as the first supported platform.
- Add dependency roots as explicit cache variables.
- Use target-local MSVC compatibility switches.
- Preserve old behavior and document any runtime fix separately from build fixes.
- Keep DirectX, OpenAL, EAX, Xiph, Lua, luabind, Boost, Loki, and ODE versions historically compatible.
- Audit hardcoded `#pragma comment(lib, ...)` and old library paths per target before suppressing or replacing them.

## Adaptable Directly

- Documentation structure.
- CMake minimum version and Visual Studio 2022 Win32 generator workflow.
- Output-directory conventions.
- Dependency variable pattern.
- Warning-only architecture guardrails.
- Target-by-target restoration order.

## 1935-Specific Work Not to Copy

- Concrete CMake source lists.
- Compatibility source files and overlays.
- Runtime fallback behavior.
- Linker workarounds for targets not yet audited in 2571.
- Generated `build-*` directories.
- Any assumptions that 1935 and 2571 have identical module contents.

## 2571 Candidate Findings

- The root initially had no README, docs, CMake root, presets, `.gitignore`, or `.gitattributes`.
- The primary runtime solution is `xr_3da/XR_3DA.sln`, Visual Studio Solution Format 8.00.
- The candidate tree contains 33 `.sln`, 118 `.vcproj`, and 44 `.dsp` files.
- Core `.vcproj` files are Visual Studio C++ `Version="7.10"`.
- Legacy `.dsp` files are Visual C++ 6 projects.
- Project configurations are Win32-oriented. Observed configuration names include `Debug|Win32`, `Release|Win32`, `Mixed|Win32`, `ReleaseDebug|Win32`, `DebugNoExceptions|Win32`, `Debug_Client|Win32`, and `Debug_Server|Win32`.
- Many projects write to `x:\`.
- DCC plugin projects reference local SDK paths such as `r:\sdk`, `r:\plugins`, `E:\3dsmax4`, and `c:\AW\Maya3.0`.
- The primary runtime solution includes stale or machine-specific references including `X:\gamedata\xrstalker.vcproj`, `..\xrCDB\xrCDB.vcproj`, and `..\ETools\ETools.vcproj`.
- `XR_3DA.vcproj` defines `ENGINE_BUILD` and `SECURE_GAMEDATA`.
- `XR_3DA.vcproj` links media/runtime libraries including `msacm32.lib`, `vfw32.lib`, Ogg, and Theora debug libraries.
- `xr_3da/xrSound` links OpenAL and Ogg/Vorbis debug libraries.
- `xrNetServer` references `Ws2_32.lib` and `dxerr9.lib`.
- `xr_3da/xrCPU_Pipe` and `xr_3da/xrGame` link against `x:\xr_3da.lib`.
- `xrAI` references `magicfm.lib`.
- Renderer modules remain DirectX-era `xrRender_R1` and `xrRender_R2`; no Vulkan backend exists.

## Likely New Problems in 2571

- The added `xrLUA_JIT` tree may change script dependency ordering compared with 1935.
- Additional old plugin trees increase hardcoded SDK path noise.
- Some solution paths do not match the checked-out directory layout and must be resolved before automated import.
- Build order may need adjustment once actual import libraries and `#pragma comment(lib, ...)` directives are audited.
- Tool/editor projects may depend on historical binaries or SDKs that were not needed for the initial 1935 runtime.

## Assumptions

- The repository is a 2571 candidate revision until source-level evidence proves an exact build match.
- Win32/x86 is the initial target architecture.
- The first successful modern build should use MSVC before Clang or Linux portability work.
