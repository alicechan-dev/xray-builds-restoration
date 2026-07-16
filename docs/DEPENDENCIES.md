# Dependencies

The dependency list is inferred from the 2571 candidate project files and the previous 1935 restoration. Exact versions still need validation against compiler and link errors.

## Expected Runtime Dependencies

- Legacy DirectX 9 SDK-era headers and libraries, including Direct3D, DirectInput, DirectPlay, `dxguid`, and `dxerr9` references.
- OpenAL SDK.
- EAX headers/libraries for old sound code.
- Ogg/Vorbis and Theora libraries.
- Lua and LuaJIT-era sources present in `xrLUA` and `xrLUA_JIT`.
- Luabind-compatible headers/sources for script bindings.
- Boost version compatible with old luabind code.
- Loki or equivalent legacy typelist headers if required by script factory code.
- ODE under `xrODE`.
- GameSpy code under `xr_3da/xrGame/GameSpy`.

## Tool and Plugin Dependencies

- 3ds Max SDKs for Max exporter/material plugins.
- Maya SDKs for Maya exporter/material plugins.
- LightWave SDKs for LW exporter/shader plugins.
- MagicFM libraries used by some AI/tool projects.
- FreeImage or other image libraries may be required by later light/build tools, pending source audit.

## Dependency Policy

Do not commit proprietary SDKs, redistributable binaries, or game data. All dependency locations should be configured through CMake cache variables, environment variables, or local user presets that are not committed.

The 1935 restoration showed that old dependency versions matter. Do not silently replace historical Boost, luabind, DirectX, OpenAL, or EAX dependencies with modern incompatible packages.
