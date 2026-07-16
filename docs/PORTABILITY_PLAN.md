# Portability Plan

Portability is a later track. The first priority is a reproducible Win32/x86 restoration that preserves historical behavior.

## Near-Term Rules

- Do not switch the project to 64-bit.
- Do not replace DirectX with Vulkan during restoration.
- Do not rewrite platform APIs before the original runtime can be built and tested.
- Isolate compatibility fixes behind target-local build configuration where possible.

## Future Sequence

1. Restore Win32/x86 target builds.
2. Document Windows-specific assumptions in each target.
3. Isolate filesystem, threading, timing, input, networking, sound, and renderer boundaries.
4. Add Windows Clang compatibility after MSVC behavior is understood.
5. Compile non-rendering/core targets on Linux.
6. Start Linux runtime without renderer parity goals.
7. Add a minimal Vulkan renderer behind a separate renderer module.
8. Work toward visual and behavioural parity only after the historical DirectX renderer is reproducible.

## Vulkan Boundary

The Vulkan renderer should be a new backend track. It should not be introduced as a way to avoid restoring the DirectX-era build, because the DirectX renderer remains the behavioral reference for the 2571 candidate revision.
