# Restoration Plan

This project restores the 2571 candidate revision in stages. The first objective is not to port the engine, but to make the historical Windows source tree understandable, reproducible, and ready for narrow compile fixes.

## Scope Boundaries

Restoration means recovering the historical build and runtime behavior with modern tooling. Modernization means replacing machine-specific assumptions with reproducible configuration and narrow compatibility fixes. Porting means isolating platform dependencies for future Linux work. Renderer work means new graphics backends such as Vulkan.

These tracks must stay separate. A CMake target recovery commit should not also change gameplay behavior. A compiler compatibility commit should not also introduce Linux abstractions. Vulkan work should wait until the DirectX-era runtime is understood.

## Milestones

- M0 - Historical baseline identified and documented.
- M1 - Original project structure audited.
- M2 - Core libraries compile with modern MSVC.
- M3 - Full Win32 build reproduced.
- M4 - CMake build becomes reproducible from a fresh clone.
- M5 - Platform-specific code isolated.
- M6 - Clang compatibility on Windows.
- M7 - Linux non-rendering/core targets compile.
- M8 - Linux runtime starts.
- M9 - Minimal Vulkan renderer.
- M10 - Vulkan visual and behavioural parity.

## Proposed Build Order

1. `xrCore` - foundational runtime library and hardcoded path cleanup.
2. `xr_3da/xrCDB` - collision database library used broadly by runtime/editor code.
3. `xrLUA` and `xrLUA_JIT` audit - determine which scripting runtime the candidate revision expects.
4. `xrXMLParser` - small dependency with limited surface area.
5. `xrParticles` - shared runtime/editor dependency.
6. `xrNetServer` - networking layer, DirectX/DirectPlay header audit.
7. `xr_3da/xrSound` - OpenAL, EAX, Ogg/Vorbis dependency recovery.
8. `xrSE_Factory` - script binding, luabind, Boost, Loki, and Lua integration.
9. `xrODE` - physics dependency and `dSINGLE` assumptions.
10. `XR_3DA` - engine executable/import library.
11. `xr_3da/xrCPU_Pipe` - Win32/x86 CPU helper DLL.
12. `xr_3da/xrRender_R1` and `xr_3da/xrRender_R2` - DirectX 9 renderer targets.
13. `xr_3da/xrGame` - gameplay DLL after engine/import-library dependencies are stable.
14. Tools: `xrCompress`, `xrFS`, `xrLC`, `xrAI`, `xrQSlim`, DXT/ETools/editor targets.
15. DCC plugins and legacy experiments only after the engine/tool baseline is reproducible.

## Change Categories

- Build-system infrastructure.
- Dependency configuration.
- Compiler compatibility.
- Source correctness fixes.
- Restoration/behavioural fixes.
- Portability work.
- Renderer work.

Each category should be reviewable independently.
