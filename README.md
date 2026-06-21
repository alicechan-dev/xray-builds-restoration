# X-Ray Historical Restoration

Unofficial restoration workspace for historical **X-Ray / S.T.A.L.K.E.R.: Shadow of Chernobyl** source snapshots.

This branch currently focuses on the **build 1935-era X-Ray source tree**, restored for **Visual Studio 2022**, **CMake**, and Win32 runtime testing.

> This is not an official release and does not include original game assets.

## Current restoration branch

* `restoration-1935` — VS2022/CMake restoration branch for the historical X-Ray build 1935-era source state.

## Current status

The project currently reaches a working build state for the restored Win32 Debug configuration.

Several major engine components have been restored to compile and link under Visual Studio 2022:

* `xrCore`
* `xrCDB`
* `xrLUA`
* `xrXMLParser`
* `xrParticles`
* `xrCompress`
* `xrSE_Factory`
* `xrNetServer`
* `xrSound`
* `XR_3DA`
* `xrCPU_Pipe`
* `xrRender_R1`
* `xrRender_R2`
* `xrGame`

The runtime can start the engine, load scripts, initialize server/client startup, load ALife data, and enter level loading.

This is still an experimental restoration. The build may compile successfully, but runtime execution can still hit missing data, old serialization assumptions, renderer issues, or debug assertions.

## Known current runtime issue

The current runtime testing state reaches texture loading and may fail on missing generated sky cubemap data:

```text
Can't find texture 'sky\sky_11_cube#small'
```

Investigation notes:

* `#small` is not a generic texture loader suffix.
* It is an expected generated DDS texture name.
* `Environment.cpp` appends `#small` for the environment sky cubemap.
* Editor-side code appears to generate these smaller sky cubemaps from base sky cubemap textures.
* Some runtime data packages contain the base sky cubemap, for example `sky_11_cube.dds`, but not the generated `sky_11_cube#small.dds`.

Possible restoration paths:

1. recover or regenerate the missing `#small` cubemap data using the original/editor pipeline;
2. add a narrow runtime generator for missing environment sky cubemaps;
3. use a temporary env-sky-only fallback from `sky_11_cube#small` to `sky_11_cube` for further runtime debugging.

The preferred long-term approach is to preserve the original behavior and document missing generated data instead of hiding broad texture loading errors.

## What has been restored so far

This branch includes compatibility work for:

* CMake target recovery for old Visual Studio 2003-era projects;
* Visual Studio 2022 Win32 build compatibility;
* legacy calling convention and linker compatibility;
* DirectX 9 renderer targets;
* OpenAL/EAX sound target recovery;
* Lua / Luabind 0.7 compatibility;
* Boost 1.33.1 compatibility;
* old STL / iterator / loop-scope issues;
* old X-Ray server object and ALife serialization/runtime startup issues;
* debug runtime startup through server/client level loading;
* a read-only archive listing helper tool for X-Ray `.xp*` archives.

## Repository layout

The repository is source-focused.

Original game assets are not included.

Do not commit local runtime data, build outputs, logs, or proprietary game packages.

Recommended ignored local artifacts:

```text
build/
*.log
log*.txt
xp0_entries.txt
*.pdb
*.ilk
*.obj
*.exe
*.dll
*.lib
*.exp
*.map
gamedata/
gamedata.xp*
savedgames/
```

## Required external dependencies

The restored build expects old SDK/library dependencies to be available locally.

Example local dependency layout used during restoration:

```text
D:/Projects/Toolchains/
  boost_1_33_1/
  luabind-0.7/
    luabind/
  loki-legacy/
  OpenAL/
  EAX/
  Xiph-msvc/

D:/Projects/Others/
  DXGarbage/
```

Known dependency set:

* Visual Studio 2022 with C++ desktop workload
* CMake
* DirectX 9 SDK-era headers/libs, preferably the 2004 SDK used by this source snapshot.
* Boost 1.33.1
* Luabind 0.7
* Lua compatible with the historical source tree
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

The exact dependency roots can be adjusted in your local CMake configuration.

## Configure

Example configure command:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 ^
  -DDIRECTX_ROOT="D:/Projects/Toolchains/DirectX" ^
  -DBOOST_ROOT="D:/Projects/Toolchains/boost_1_33_1" ^
  -DLUABIND_ROOT="D:/Projects/Toolchains/luabind-0.7/luabind" ^
  -DLOKI_ROOT="D:/Projects/Toolchains/loki-legacy" ^
  -DOPENAL_ROOT="D:/Projects/Toolchains/OpenAL" ^
  -DEAX_ROOT="D:/Projects/Toolchains/EAX" ^
  -DXIPH_ROOT="D:/Projects/Toolchains/Xiph-msvc"
```

If your local CMake option names differ, inspect the root `CMakeLists.txt` and toolchain discovery logic.

## Build

Recommended Debug build command:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

For more detailed diagnostics:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Individual targets can be built separately, for example:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

The built binaries should be placed into a compatible historical X-Ray build 1935 runtime folder.

Expected runtime folder contents include:

```text
XR_3DA.exe
xrCore.dll
xrGame.dll
xrRender_R1.dll
xrRender_R2.dll
xrSound.dll
xrLUA.dll
xrXMLParser.dll
xrCDB.dll
xrParticles.dll
xrNetServer.dll
xrCPU_Pipe.dll
fsgame.ltx
user_koan.ltx
gamedata.xp0 or unpacked gamedata/
```

Original runtime assets are not provided by this repository.

## Example runtime launch

Example command used during restoration testing:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

Example Visual Studio debugger launch:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

## Archive inspection helper

This branch includes a read-only helper tool for listing X-Ray `.xp*` archive entries:

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Example usage:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Then search the listing:

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

This tool is intended for diagnostics only. It does not extract, modify, or repack archives.

## Development notes

This restoration tries to preserve original behavior where possible.

General rules used during restoration:

* avoid editing gameplay/config/script/data unless the root cause proves it is required;
* avoid editing external SDK/toolchain files;
* avoid fake stubs for missing engine behavior;
* prefer target-local CMake/source/link fixes;
* keep compatibility fixes narrow and documented;
* do not silently hide missing data problems.

Recommended whitespace check for this legacy CRLF-heavy source tree:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Plain `git diff --check` may report noisy CRLF-related warnings on old files.

## Releases

Recommended release tag format:

```text
v1935-vs2022-alpha-1
```

Recommended release title:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Release archives should not include original game assets.

Suggested binary package contents:

```text
XR_3DA.exe
xrCore.dll
xrGame.dll
xrRender_R1.dll
xrRender_R2.dll
xrSound.dll
xrLUA.dll
xrXMLParser.dll
xrCDB.dll
xrParticles.dll
xrNetServer.dll
xrCPU_Pipe.dll
README_RUNTIME.txt
```

Optional separate debug symbols package:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Development assistance

This restoration work was developed with assistance from **OpenAI Codex**.

Codex was used as an AI coding assistant for:

* CMake target reconstruction;
* legacy Visual Studio project migration;
* compatibility patch drafting;
* build error investigation;
* runtime crash analysis;
* source navigation and refactoring suggestions;
* diagnostic tooling drafts.

All changes are reviewed, built, tested, and curated manually as part of the restoration process.

The goal is not to modernize the engine aggressively, but to preserve the historical source behavior while making the build and runtime usable on a modern Windows toolchain.

## Legal / assets notice

This repository is for source restoration, build scripts, documentation, and compatibility research.

Original game assets, proprietary data packages, and runtime content are not included.

Use this only with a legally obtained compatible runtime data package.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.

