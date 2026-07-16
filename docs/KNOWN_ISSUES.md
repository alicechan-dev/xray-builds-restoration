# Known Issues

This list captures known restoration blockers and warnings.

- The repository is a 2571 candidate revision, not proven exact build 2571 source.
- Original projects write outputs to `x:\` and other local machine paths.
- The primary solution contains stale project references that do not match the current tree.
- The original workspace mixes Visual Studio 2003 `.vcproj` and Visual C++ 6 `.dsp` projects.
- All audited project configurations are Win32-oriented; x64 is out of scope for the initial restoration.
- Many projects use old debug/release variants such as `Mixed`, `ReleaseDebug`, `DebugNoExceptions`, `Debug_Client`, and `Debug_Server`.
- DirectX SDK-era headers and libraries are assumed, including DirectPlay and old D3DX support.
- Sound code depends on OpenAL, EAX, Ogg/Vorbis, and possibly Theora.
- Script code likely depends on historically compatible Lua, LuaJIT, luabind, Boost, and Loki versions.
- DCC plugins contain hardcoded SDK paths for 3ds Max, Maya, and LightWave.
- Some tool projects reference MagicFM and other non-repository libraries.
- Generated files, historical binaries, or matching SDK headers may be missing.
- `xrCore` builds in Debug and Release, but still emits historical warning classes: deprecated CRT/POSIX calls, C4595 for inline global new/delete overrides, local-name hiding warnings, and D9035 for `/Zc:forScope-`.
- Release `xrCore` emits D9025 because the target deliberately overrides CMake's default exception option with the historical no-exceptions Release behavior.
- Modern MSVC compatibility issues are still expected in later targets around removed STL adapters, old CRT link directives, calling conventions, precompiled headers, and legacy pragmas.

The current CMake layer intentionally does not suppress broad warning classes.
