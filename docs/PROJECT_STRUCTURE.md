# Project Structure

The 2571 candidate tree is a broad historical workspace, not just the game runtime.

## Legacy Project Inventory

- 33 Visual Studio solution files (`.sln`).
- 118 Visual Studio 2003 project files (`.vcproj`).
- 44 Visual C++ 6 project files (`.dsp`).

The primary runtime solution is `xr_3da/XR_3DA.sln`. It references:

- `XR_3DA`
- `xrCPU_Pipe`
- `xrGame`
- `xrSound`
- `xrCore`
- `xrCDB`
- `xrRender_R1`
- `xrRender_R2`
- `xrXMLParser`
- `ode`
- `xrParticles`
- `xrNetServer`
- `xrSE_Factory`
- `xrCompress`
- `DXT`
- `xrStalker`
- `xrLC`
- `xrQSlim`
- `xrDO_Light`
- `xrAI`
- `ETools`
- `xrD3D9-Null`
- `xrGameSpy`
- `xrLUA_JIT`
- `Stalker_net`
- `xrStatisticConvert`

Several solution references are stale or machine-specific, including `X:\gamedata\xrstalker.vcproj`, `..\xrCDB\xrCDB.vcproj`, `..\ETools\ETools.vcproj`, and `..\xrDXT\DXT.vcproj` paths that do not match the current directory layout directly.

## Major Source Areas

- `xrCore` - core runtime library, filesystem, memory, logging, math, debug support.
- `xr_3da` - engine executable, renderers, sound, gameplay, collision database, CPU helper DLL.
- `xrSE_Factory` - script/entity factory code and luabind integration.
- `xrLUA` and `xrLUA_JIT` - scripting runtimes.
- `xrNetServer` - networking layer.
- `xrParticles` - particle support.
- `xrODE` - physics dependency.
- `xrAI`, `xrLC`, `xrCompress`, `xrFS`, `xrQSlim` - tools and build-time utilities.
- `Editors`, `Editor`, `SDK` - editor/tool code and old SDK experiments.
- `xrPlugins`, `Plugins`, `xray_plugins` - DCC plugins for Max, Maya, and LightWave.
- `External Library` - historical OpenAL and Ogg/Vorbis project trees.
- `TestBed`, `samples`, `Dima`, `AlexMX`, `AlexRR_Editor` - experiments, samples, and developer work areas.

## Renderer Modules

- `xr_3da/xrRender_R1` - DirectX renderer path.
- `xr_3da/xrRender_R2` - DirectX renderer path.
- `xr_3da/xrGame/xrD3D9-Null` - null Direct3D test/stub project in the game subtree.

No Vulkan renderer exists in this baseline.
