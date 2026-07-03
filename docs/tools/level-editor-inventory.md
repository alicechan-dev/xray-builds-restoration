# Level Editor Inventory

This document records the first SDK investigation pass for the historical X-Ray build 1935 LevelEditor sources. It is documentation only; no LevelEditor port or build-system migration is implemented here.

## Summary

The level editor source is present, but it is not a small isolated Visual Studio 2022 target. The most complete-looking tree is `Editors/LevelEditor/`, which is a Borland C++ Builder 6 VCL application that depends on shared editor DLL/library code, third-party Borland components, DirectX, MagicFM, old Boost, and runtime-style X-Ray libraries.

No modern CMake target for LevelEditor was found. No build probe was run because there is no existing CMake target and creating one would be a broad GUI/editor port rather than a safe probe.

## Source Locations Found

| Path | Project files | Likely output | Type | Notes |
|---|---|---|---|---|
| `Editors/LevelEditor/` | `Editors/LevelEditor/LevelEditor.bpr` | `x:\LevelEditor.exe` | Borland/VCL GUI editor | Best current candidate tree. Contains `Edit/` and `Engine/` subfolders, 130 `.cpp` files, 92 `.h` files, 23 `.dfm` forms, and one `.res` file. |
| `Editor/LevelEditor/` | `Editor/LevelEditor/LevelEditor.bpr` | `x:\LevelEditor.exe` | Older Borland/VCL GUI editor | Older/alternate generation with more monolithic source list. Useful for history comparison, not the first port target. |
| `Editors/!old/LevelEditor/` | `Editors/!old/LevelEditor/LevelEditor.bpr` | likely `LevelEditor.exe` | Old Borland/VCL GUI editor | Historical reference snapshot. |
| `Editors/ECore/` | `Editors/ECore/xrECoreB.bpr` | `x:\xrECoreB.dll` | Shared editor/runtime-style DLL | Required by the current `Editors/LevelEditor` tree. Contains editor rendering, thumbnails, image manager, object IO, particle support, render backend copies, and shared UI helpers. |
| `Editors/xrEProps/` | `Editors/xrEProps/xrEPropsB.bpr` | likely `xrEPropsB.dll` | Shared editor property UI DLL | Required by editor property helpers. Runtime docs already note this DLL is optional for game runtime but important for editor workflows. |
| `Editors/Tools/ETools/` | `Editors/Tools/ETools/tools.sln`, `Editors/Tools/ETools/ETools.vcproj` | likely `ETools.lib`/tool helper output | Visual Studio helper library/tool | Referenced by editor include paths as `P:\Tools\ETools` / `..\..\Tools\Etools`. |
| `Editor/LevelOptions/`, `Editors/LevelOptions/` | `xrLC_Options.bpr` | `xrLC_Options.dll` | Borland/VCL options dialog | Related to level/compiler options, not the editor itself. |
| `xrLC/` | `xrLC.sln`, `xrLC.vcproj`, helper projects | `xrLC.exe` and helper tools | Level compiler | Related compiler stage; should be inventoried separately before editor/compiler integration. |
| `AlexRR_Editor/` | `XRayMapEditor.dsp` | unknown map editor output | Legacy VS/VC6-style editor experiment | Separate map editor/reference source, not the main LevelEditor tree. |

## Current Project Structure

`Editors/LevelEditor/LevelEditor.cpp` uses Borland VCL application entry points and form registration macros:

* `Application->Initialize()` / `Application->Run()`;
* `USEFORM(...)` for `.dfm` UI forms;
* `TForm`, `TFrame`, `TComponent`, `AnsiString`, and Borland `__fastcall` conventions;
* `Core._initialize("level", ELogCallback)` before editor object creation;
* editor globals such as `Tools`, `UI`, and `IScene`.

`Editors/LevelEditor/stdafx.h` confirms Borland-specific and editor-specific assumptions:

* `#pragma warn -pck`;
* `#include <fastmath.h>`;
* Borland compatibility wrappers for `_eof`, `_access`, `_lseek`, `_dup`;
* DirectInput 7 era define: `DIRECTINPUT_VERSION 0x0700`;
* editor API macros for `ECORE_API` and `ENGINE_API`;
* includes `xrCore.h`, `xrCDB.h`, `Sound.h`, `PSystem.h`;
* includes DirectX headers `d3d9.h`, `d3dx9.h`, `dinput.h`, `dsound.h`;
* includes editor shared headers such as `D3DX_Wrapper.h`, `ETools.h`, `xrEProps.h`, `editor\engine.h`.

## Dependency Map

| Dependency | Evidence | Role | First restoration concern |
|---|---|---|---|
| Borland C++ Builder 6 / VCL | `LevelEditor.bpr` has `VERSION value="BCB.06.00"`, packages `vcl.bpi`, `rtl.bpi`, `vclx.bpi`, source uses `Application`, `TForm`, `USEFORM`. | GUI framework and project system. | VS2022 cannot build this directly without a VCL replacement/compatibility strategy. |
| Borland packages/components | `elpackB6.lib`, `bcbsmp.lib`, `editor.lib`, AlexMX and ElPack include/lib paths. | UI components/property controls. | Missing/proprietary/old component packages need inventory before porting. |
| DirectX 9 SDK | `R:\SDK\DirectX9.0_sum_2004\Include`, headers `d3d9.h`, `d3dx9.h`, `dinput.h`, `dsound.h`. | Editor viewport rendering/input/sound preview. | Need target-local SDK discovery when porting editor dependencies. |
| MagicFM | `P:\MagicFM_Borland\Sdk\Include`. | Geometry/math/collision support in editor/compiler code. | Dependency availability/provenance must be documented. |
| Boost 1.30.x | `P:\boost_1_30_0`. | Editor/helper templates or tool code. | Must compare with existing runtime Boost compatibility. |
| X-Ray runtime libraries | Includes `xrCore`, `xrCDB`, `xrSound`, `xrParticles`. | Shared runtime data structures, collision, sound preview, particle support. | Avoid pulling runtime-heavy targets into editor port until boundary is mapped. |
| `Editors/ECore` | `..\ECore` include paths and `xrECoreB.dll` project. | Shared editor engine/render/object/image code. | Best first dependency inventory target before LevelEditor itself. |
| `Editors/xrEProps` | `..\xrEProps` include path and runtime DLL notes. | Editor property helper UI. | Needed for property panels and entity/spawn helper workflows. |
| `Editors/Tools/ETools` | Include path to `P:\Tools\ETools` / `..\..\Tools\Etools`. | Mesh/tool helper routines. | Has VS project files; may be a safer isolated build probe later. |
| `xrSE_Factory` | Runtime docs show `Editors/LevelEditor/Edit/ESceneSpawnTools.cpp` loads entity factory exports. | Spawn/entity bridge for editor spawn tools. | Existing runtime target may be reused, but only after editor side is isolated. |

## Existing CMake Status

No `LevelEditor`, `xrLevelEditor`, `xrECoreB`, or `xrEPropsB` CMake target was found in the current tree. The editor project is not currently part of the modern CMake build.

A build probe was not run in this pass because the available project file is a Borland `.bpr`, not a VS2022/CMake target. Creating a new target would require a broad port of VCL forms, Borland-specific headers, packages, and shared editor libraries.

## First Blocker

The first real blocker is not a missing include or one library; it is the Borland C++ Builder 6 / VCL project boundary. The current LevelEditor entry point and forms depend on Borland VCL concepts (`Application`, `TForm`, `.dfm`, `USEFORM`, `__fastcall`, Borland packages), while the restoration build uses Visual Studio 2022 Win32.

Before trying to build `LevelEditor.exe`, the project needs a smaller dependency plan:

1. inventory and classify `Editors/ECore/xrECoreB.bpr` sources;
2. inventory `Editors/xrEProps/xrEPropsB.bpr` and property helper boundaries;
3. inspect `Editors/Tools/ETools` as a possible isolated VS project build probe;
4. decide whether GUI restoration will preserve Borland/VCL externally, build a compatibility layer, or port editor UI forms incrementally.

## Recommended Next Safe Step

Do not start with `LevelEditor.exe`. The safer next pass is to inventory `Editors/ECore` in detail, because LevelEditor depends on it and it contains many reusable editor/runtime-style systems: render backend copies, thumbnails, image manager, object IO, particle support, and shared editor helpers.

A second low-risk option is a build probe for `Editors/Tools/ETools` because it already has a Visual Studio solution/project file and appears more isolated than the VCL GUI editor.

## Validation Performed

Commands used for this investigation were read-only searches and file inspection only. No build was run, and no runtime/gameplay/render targets were touched.
