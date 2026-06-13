# Hardcoded Path Audit

Historical snapshot: S.T.A.L.K.E.R. X-Ray Build 1935.

This audit is documentation-only. It records hardcoded path assumptions found in the original project files and source tree so the CMake compatibility layer can preserve behavior while avoiding edits to the historical VS2003/VCL projects.

## Risk Scale

- **High**: Blocks compilation, linking, or startup on a modern machine.
- **Medium**: Breaks optional/editor/tool flows, debug workflows, or non-default configurations.
- **Low**: Comment, sample, SourceSafe metadata, or inactive debug/test-only path.

## Recommended Fix Layers

- **CMake variable**: Model the path as a configurable build input or generated output path.
- **Wrapper script**: Set working directory, environment, or stage runtime files before launching.
- **mklink**: Last-resort compatibility shim for legacy tools that are not worth patching initially.
- **source patch**: Small guarded code change after the compatibility build proves the issue cannot be solved externally.
- **leave unchanged**: Preserve as historical metadata or inactive/debug-only reference.

## Engine And VS2003 Build Paths

| File | Line | Exact path | Context | Classification | Risk | Recommended fix layer |
|---|---:|---|---|---|---|---|
| `xr_3da/XR_3DA.vcproj` | 18 | `x:\xr_3da.exe` | Debug executable output | build output | High | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 64 | `x:\xr_3DA.tlb` | MIDL type library output | build output | Medium | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 70 | `x:\xr_3DA.map` | Linker map output | build output | Medium | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 79 | `x:\` | Build log/output directory | build output | High | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 103 | `x:\xr_3da.exe` | Release executable output | build output | High | CMake variable |
| `xr_3da/xrCPU_Pipe/xrCPU_Pipe.vcproj` | 18 | `x:\xrCPU_Pipe.dll` | DLL output | build output | High | CMake variable |
| `xr_3da/xrCPU_Pipe/xrCPU_Pipe.vcproj` | 62 | `x:\xrCPU_Pipe.lib` | Import library output | build output | High | CMake variable |
| `xr_3da/xrCPU_Pipe/xrCPU_Pipe.vcproj` | 69 | `x:\xrCPU_Pipe.map` | Linker map output | build output | Medium | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 19 | `x:\xrGame.dll` | Game DLL output | build output | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 61 | `x:\xrGame.lib` | Import library output | build output | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 70 | `x:\xrGame.map` | Linker map output | build output | Medium | CMake variable |
| `xr_3da/xrGame/xrGame.dsp` | 42 | `x:\game\` | Legacy VS6 output directory | build output | Medium | leave unchanged |
| `xr_3da/xrSound/xrSound.vcproj` | 19 | `x:\xrSound.dll` | Sound DLL output | build output | High | CMake variable |
| `xr_3da/xrSound/xrSound.vcproj` | 88 | `x:\xrSound.map` | Linker map output | build output | Medium | CMake variable |
| `xrCore/xrCore.vcproj` | 19 | `x:\xrCore.dll` | Core DLL output | build output | High | CMake variable |
| `xrCore/xrCore.vcproj` | 88 | `x:\xrCore.map` | Linker map output | build output | Medium | CMake variable |
| `xr_3da/xrCDB/xrCDB.vcproj` | 18 | `x:\xrCDB.dll` | Collision database DLL output | build output | High | CMake variable |
| `xr_3da/xrCDB/xrCDB.vcproj` | 62 | `x:\xrCDB.lib` | Import library output | build output | High | CMake variable |
| `xr_3da/xrCDB/xrCDB.dsp` | 41 | `x:\game\` | Legacy VS6 output directory | build output | Medium | leave unchanged |
| `xr_3da/xrRender_R1/xrRender_R1.vcproj` | 19 | `x:\xrRender_R1.dll` | Renderer R1 DLL output | build output | High | CMake variable |
| `xr_3da/xrRender_R2/xrRender_R2.vcproj` | 19 | `x:\xrRender_R2.dll` | Renderer R2 DLL output | build output | High | CMake variable |
| `xrXMLParser/xrXMLParser.vcproj` | 18 | `x:\xrXMLParser.dll` | XML parser DLL output | build output | High | CMake variable |
| `xrLUA/xrLUA.vcproj` | 19 | `x:\xrLUA.dll` | Lua wrapper DLL output | build output | High | CMake variable |
| `xrParticles/xrParticles.vcproj` | 18 | `x:\xrParticles.dll` | Particles DLL output | build output | High | CMake variable |
| `xrNetServer/xrNetServer.vcproj` | 18 | `x:\xrNetServer.dll` | Network server DLL output | build output | High | CMake variable |
| `xrSE_Factory/xrSE_Factory.vcproj` | 19 | `x:\xrSE_Factory.dll` | Spawn/entity factory DLL output | build output | High | CMake variable |
| `xrCompress/xrCompress.vcproj` | 18 | `x:\xrCompress.exe` | Compression tool executable output | build output | Medium | CMake variable |
| `xr_3da/xrScriptDebugIde/xrScriptDebudIde.vcproj` | 19 | `x:\xrScriptDbgIde.exe` | Script debug IDE executable output | editor tool | Medium | CMake variable |

## Link-Time Library Pragmas

These source-level `#pragma comment(lib, ...)` directives bypass the project-file linker settings and will fight a modern CMake target graph unless they are neutralized or the referenced import libraries are staged at equivalent paths.

| File | Line | Exact path | Context | Classification | Risk | Recommended fix layer |
|---|---:|---|---|---|---|---|
| `xr_3da/stdafx.h` | 54 | `x:\xrCore.lib` | Engine links core import library by absolute path | lib path | High | source patch |
| `xr_3da/stdafx.h` | 55 | `x:\xrCDB.lib` | Engine links collision import library by absolute path | lib path | High | source patch |
| `xr_3da/stdafx.h` | 56 | `x:\xrSound.lib` | Engine links sound import library by absolute path | lib path | High | source patch |
| `xr_3da/stdafx.h` | 57 | `x:\xrLUA.lib` | Engine links Lua wrapper import library by absolute path | lib path | High | source patch |
| `xr_3da/xrSound/stdafx.cpp` | 19 | `x:\eax.lib` | Sound DLL links EAX SDK library | lib path | High | CMake variable |
| `xr_3da/xrSound/stdafx.cpp` | 20 | `x:\vorbis.lib` | Sound DLL links Vorbis library | lib path | High | CMake variable |
| `xr_3da/xrSound/stdafx.cpp` | 21 | `x:\vorbisfile.lib` | Sound DLL links Vorbisfile library | lib path | High | CMake variable |
| `xr_3da/xrSound/stdafx.cpp` | 22 | `x:\xrCore.lib` | Sound DLL links core import library | lib path | High | source patch |
| `xr_3da/xrSound/stdafx.cpp` | 23 | `x:\xrCDB.lib` | Sound DLL links collision import library | lib path | Medium | source patch |
| `xr_3da/xrSound/stdafx.cpp` | 24 | `x:\wrap_oal.lib` | Sound DLL links OpenAL wrapper | lib path | High | CMake variable |
| `xr_3da/xrRender_R1/xrRender_R1.cpp` | 7 | `x:\xr_3DA.lib` | Renderer links engine import library | lib path | High | source patch |
| `xr_3da/xrRender_R2/xrRender_R2.cpp` | 6 | `x:\xr_3DA.lib` | Renderer links engine import library | lib path | High | source patch |
| `xrXMLParser/xrXMLParser.h` | 9 | `x:\xrXMLParser.lib` | Consumers link XML parser import library | lib path | High | source patch |
| `xrXMLParser/stdafx.h` | 17 | `x:\xrCoreB.lib` | Borland-specific core import library | lib path | Medium | source patch |
| `xrXMLParser/stdafx.h` | 19 | `x:\xrCore.lib` | MSVC core import library | lib path | High | source patch |
| `xrLUA/stdafx.h` | 28 | `x:\xrCoreB.lib` | Borland-specific core import library | lib path | Medium | source patch |
| `xrLUA/stdafx.h` | 30 | `x:\xrCore.lib` | MSVC core import library | lib path | High | source patch |
| `xrParticles/stdafx.h` | 17 | `x:\xrCoreB.lib` | Borland-specific core import library | lib path | Medium | source patch |
| `xrParticles/stdafx.h` | 19 | `x:\xrCore.lib` | MSVC core import library | lib path | High | source patch |
| `xrParticles/psystem.h` | 10 | `x:\xrParticlesB.lib` | Borland-specific particles import library | lib path | Medium | source patch |
| `xrParticles/psystem.h` | 12 | `x:\xrParticles.lib` | MSVC particles import library | lib path | High | source patch |
| `xr_3da/psystem.h` | 10 | `x:\xrParticlesB.lib` | Engine-side particles import library | lib path | Medium | source patch |
| `xr_3da/psystem.h` | 12 | `x:\xrParticles.lib` | Engine-side particles import library | lib path | High | source patch |
| `xr_3da/NET_Shared.h` | 11 | `x:\xrNetServer.lib` | Engine-side network server import library | lib path | High | source patch |
| `xrNetServer/stdafx.cpp` | 6 | `x:\xrCore.lib` | Network server links core import library | lib path | High | source patch |
| `xrNetServer/NET_Shared.h` | 11 | `x:\xrNetServer.lib` | Consumers link network server import library | lib path | High | source patch |
| `xrSE_Factory/stdafx.cpp` | 9 | `x:/xrCore.lib` | Entity factory links core import library | lib path | High | source patch |
| `xrSE_Factory/stdafx.cpp` | 10 | `x:/xrLUA.lib` | Entity factory links Lua wrapper import library | lib path | High | source patch |
| `xrCompress/xrCompress.cpp` | 7 | `x:\xrCore.lib` | Compression tool links core import library | lib path | Medium | source patch |

## Borland And Editor Tool Paths

The editor and several tool projects preserve Borland C++ Builder-era assumptions. These should be treated as a separate migration phase from the runtime engine.

| File | Line | Exact path | Context | Classification | Risk | Recommended fix layer |
|---|---:|---|---|---|---|---|
| `Editor/ActorEditor/ActorEditor.bpr` | 6 | `x:\ActorEditor.exe` | Actor editor executable output | editor tool | Medium | CMake variable |
| `Editor/ParticleEditor/ParticleEditor.bpr` | 6 | `x:\ParticleEditor.exe` | Particle editor executable output | editor tool | Medium | CMake variable |
| `Editor/ShaderEditor/ShaderEditor.bpr` | 6 | `x:\ShaderEditor.exe` | Shader editor executable output | editor tool | Medium | CMake variable |
| `Editor/LevelEditor/LevelEditor.bpr` | 6 | `x:\LevelEditor.exe` | Level editor executable output | editor tool | Medium | CMake variable |
| `Editors/LevelEditor.bpr` | 46 | `x:\LevelEditor.exe` | HostApplication for debugging | working directory | Medium | wrapper script |
| `Editors/ParticleEditor.bpr` | 46 | `x:\ParticleEditor.exe` | HostApplication for debugging | working directory | Medium | wrapper script |
| `Editors/ShaderEditor.bpr` | 46 | `x:\ShaderEditor.exe` | HostApplication for debugging | working directory | Medium | wrapper script |
| `Editor/ActorEditor/Editor/FileSystem.cpp` | 76 | `x:\` | Editor local filesystem root | runtime resource | High | source patch |
| `Editor/LevelEditor/Edit/FileSystem.cpp` | 76 | `x:\` | Editor local filesystem root | runtime resource | High | source patch |
| `Editor/ParticleEditor/Editor/FileSystem.cpp` | 76 | `x:\` | Editor local filesystem root | runtime resource | High | source patch |
| `Editor/LevelOptions/Editor/FileSystem.cpp` | 76 | `x:\` | Editor local filesystem root | runtime resource | High | source patch |
| `Editor/ActorEditor/Engine/xrLoadSurface.cpp` | 77 | `x:\textures\` | Editor texture loading fallback/root | runtime resource | High | wrapper script |
| `Editor/ParticleEditor/Engine/xrLoadSurface.cpp` | 77 | `x:\textures\` | Editor texture loading fallback/root | runtime resource | High | wrapper script |
| `Editors/ECore/Editor/ExportSkeleton.cpp` | 175 | `x:\import\test_skl.smf` | Export test target | editor tool | Low | leave unchanged |
| `Editors/ECore/Editor/ExportSkeleton.cpp` | 382 | `x:\import\test_skm.smf` | Export test target | editor tool | Low | leave unchanged |
| `Editors/ECore/Editor/ExportObjectOGF.cpp` | 181 | `x:\import\test_object.smf` | Export test target | editor tool | Low | leave unchanged |
| `Editors/Tools/ETools/PropSlimTools.cpp` | 154 | `x:\import\original.smf` | Tool reads sample/reference mesh | editor tool | Medium | wrapper script |
| `Editors/Tools/DXT/DXT.vcproj` | 18 | `x:\DXT.dll` | DXT tool output | editor tool | Medium | CMake variable |
| `Editors/Tools/ETools/ETools.vcproj` | 18 | `x:\ETools.dll` | Editor tools DLL output | editor tool | Medium | CMake variable |
| `Editors/Tools/LWO/LWO.vcproj` | 18 | `x:\LWO.dll` | LightWave object plugin DLL output | editor tool | Medium | CMake variable |
| `Editors/Tools/gauss/Gauss.vcproj` | 18 | `x:\Gauss.exe` | Gaussian utility output | editor tool | Low | CMake variable |
| `xr_3da/xrScriptDebugIde/ide2.cpp` | 240 | `x:\wincmp.exe` | Script IDE launches external compare tool | editor tool | Medium | wrapper script |

## Absolute SDK, Include, And Library Roots

These absolute roots appear primarily in `.bpr` files and encode the original GSC workstation layout. They should become cache variables or imported targets in any modern build layer.

| File | Line | Exact path | Context | Classification | Risk | Recommended fix layer |
|---|---:|---|---|---|---|---|
| `xr_3da/xrSound/xrSoundB.bpr` | 42 | `R:\SDK\DirectX9.0_sum_2004\Include` | DirectX include path | include path | High | CMake variable |
| `xr_3da/xrSound/xrSoundB.bpr` | 52 | `R:\SDK\eax20\Include` | EAX SDK include path | include path | High | CMake variable |
| `xr_3da/xrSound/xrSoundB.bpr` | 53 | `P:\xrCore` | Cross-project include path | include path | High | CMake variable |
| `Editors/ShaderEditor/ShaderEditor.bpr` | 37 | `P:\xrCore` | Cross-project include path | include path | High | CMake variable |
| `Editors/ShaderEditor/ShaderEditor.bpr` | 44 | `P:\xrCDB` | Cross-project include path | include path | High | CMake variable |
| `Editors/ShaderEditor/ShaderEditor.bpr` | 46 | `P:\xrSound` | Cross-project include path | include path | High | CMake variable |
| `Editors/ShaderEditor/ShaderEditor.bpr` | 47 | `R:\SDK\eax20\Include` | EAX SDK include path | include path | High | CMake variable |
| `Editors/ShaderEditor/ShaderEditor.bpr` | 50 | `P:\Tools\ETools` | Editor tool include path | include path | Medium | CMake variable |
| `Editors/ShaderEditor/ShaderEditor.bpr` | 126 | `P:\Editor\!run` | Editor runtime/run directory | working directory | Medium | wrapper script |
| `Editor/ActorEditor/ActorEditor.bpr` | 37 | `R:\DirectX8.1\include` | Older DirectX include path | include path | High | CMake variable |
| `Editor/ActorEditor/ActorEditor.bpr` | 38 | `R:\STLport-4.5\stlport` | STLport include path | include path | High | CMake variable |
| `xrCore/xrCoreB.bpr` | 43 | `R:\Borland\CBuilder6\Projects\Lib` | Borland library path | lib path | Medium | CMake variable |
| `xrLUA/xrLuaB.bpr` | 44 | `R:\SDK\boost-03-12-12-0400` | Boost include path | include path | High | CMake variable |
| `xrLUA/xrLuaB.bpr` | 45 | `P:\luabind` | Luabind include path | include path | High | CMake variable |
| `xrParticles/xrParticlesB.bpr` | 43 | `P:\xrCore` | Cross-project include path | include path | High | CMake variable |
| `Editors/LevelEditor/LevelEditor.bpr` | 37 | `R:\SDK\DirectX9.0\include` | DirectX include path | include path | High | CMake variable |
| `Editors/LevelEditor/LevelEditor.bpr` | 45 | `P:\boost_1_30_2` | Boost include path | include path | High | CMake variable |
| `Editors/LevelEditor/LevelEditor.bpr` | 48 | `P:\MagicFM_Borland\Sdk\Include` | Magic Software/FMagic include path | include path | High | CMake variable |
| `Editors/LevelEditor/LevelEditor.bpr` | 50 | `P:\Editor\QHull` | QHull include path | include path | Medium | CMake variable |
| `xrStalker/Scripts/Jon_scripts/Jon_scripts.vcproj.vspscc` | 7 | `E:\1New\selo\gamedata\scripts\Jon_scripts\Jon_scripts.vcproj` | SourceSafe original project path | unknown | Low | leave unchanged |
| `xrStalker/Scripts/xr_sos.script` | 4 | `d:\selo\gamedata\scripts\xr_sos.script` | Script header/comment path | unknown | Low | leave unchanged |
| `xrStalker/Scripts/xr_sos.script` | 5 | `d:\selo\gamedata\scripts` | Script header/comment directory | unknown | Low | leave unchanged |
| `xrStalker/w_mac10.ltx` | 1 | `S:\GameData\w_mac10.ltx` | Config comment/source path | unknown | Low | leave unchanged |

## Relative Layout Assumptions In Build Files

These are not absolute paths, but they assume the original source checkout shape. They are safer than `X:\`/`P:\` paths, but should still be captured in CMake as target include directories and target dependencies instead of copied literally into global include paths.

| File | Line | Exact path | Context | Classification | Risk | Recommended fix layer |
|---|---:|---|---|---|---|---|
| `xr_3da/XR_3DA.vcproj` | 22 | `../xrCore` | Additional include directory | include path | High | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 22 | `xrCPU_Pipe` | Additional include directory | include path | Medium | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 24 | `$(XRAY_EXTERNAL)\dxsdk\include` | External DirectX include root | include path | High | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 24 | `$(XRAY_EXTERNAL)\eax` | External EAX include root | include path | High | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 30 | `$(XRAY_EXTERNAL)\dxsdk\lib` | External DirectX library root | lib path | High | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 31 | `$(XRAY_EXTERNAL)\dxsdk\lib\d3dx9d.lib` | Debug Direct3D utility library | lib path | High | CMake variable |
| `xr_3da/XR_3DA.vcproj` | 114 | `$(XRAY_EXTERNAL)\dxsdk\lib\d3dx9.lib` | Release Direct3D utility library | lib path | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 23 | `../xrSE_Factory` | Cross-project include directory | include path | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 23 | `../xrLUA` | Cross-project include directory | include path | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 23 | `xrGameSpy` | GameSpy wrapper include directory | include path | Medium | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 24 | `../xrODE/include` | ODE include directory | include path | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 32 | `../lib/oggvorbis/win32/Vorbis_Dynamic_Debug` | Vorbis debug library directory | lib path | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 33 | `../lib/oggvorbis/win32/Ogg_Dynamic_Debug` | Ogg debug library directory | lib path | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 114 | `../lib/oggvorbis/win32/Vorbis_Dynamic_Release` | Vorbis release library directory | lib path | High | CMake variable |
| `xr_3da/xrGame/xrGame.vcproj` | 115 | `../lib/oggvorbis/win32/Ogg_Dynamic_Release` | Ogg release library directory | lib path | High | CMake variable |
| `xr_3da/xrSound/xrSound.vcproj` | 23 | `$(XRAY_EXTERNAL)\dxsdk\include` | DirectX include root | include path | High | CMake variable |
| `xr_3da/xrSound/xrSound.vcproj` | 23 | `$(XRAY_EXTERNAL)\eax` | EAX include root | include path | High | CMake variable |
| `xr_3da/xrSound/xrSound.vcproj` | 27 | `$(XRAY_EXTERNAL)\dxsdk\lib` | DirectX library root | lib path | High | CMake variable |
| `xr_3da/xrCDB/xrCDB.vcproj` | 22 | `../xrCore` | Cross-project include directory | include path | High | CMake variable |
| `xrLUA/xrLUA.vcproj` | 23 | `../xrCore` | Cross-project include directory | include path | High | CMake variable |
| `xrLUA/xrLUA.vcproj` | 23 | `../luabind` | Luabind include directory | include path | High | CMake variable |
| `xrSE_Factory/xrSE_Factory.vcproj` | 23 | `../xrLUA` | Cross-project include directory | include path | High | CMake variable |
| `xrSE_Factory/xrSE_Factory.vcproj` | 23 | `../xr_3da/xrGame` | Game include directory from outside target | include path | High | CMake variable |
| `xrSE_Factory/xrSE_Factory.vcproj` | 25 | `../lib/DX` | DirectX helper library directory | lib path | High | CMake variable |
| `Editors/Tools/ETools/ETools.vcproj` | 22 | `../../../xrCore` | Cross-project include directory | include path | Medium | CMake variable |
| `Editors/Tools/DXT/DXT.vcproj` | 22 | `../../../xrCore` | Cross-project include directory | include path | Medium | CMake variable |

## Runtime Resource Paths

The engine is designed around `fs.ltx` path aliases. A modern run wrapper should stage binaries and game data into the expected working directory, then launch from that directory so these aliases resolve consistently.

| File | Line | Exact path | Context | Classification | Risk | Recommended fix layer |
|---|---:|---|---|---|---|---|
| `xrCore/LocatorAPI.cpp` | 20 | `fs.ltx` | Filesystem alias table loaded at startup | runtime resource | High | wrapper script |
| `xrCore/LocatorAPI.cpp` | 748 | `$game_textures$` | Texture resource alias | runtime resource | High | wrapper script |
| `xrCore/LocatorAPI.cpp` | 759 | `$game_sounds$` | Sound resource alias | runtime resource | High | wrapper script |
| `xrCore/log.cpp` | 143 | `$logs$` | Log directory alias | runtime resource | Medium | wrapper script |
| `xrCore/xrDebug.cpp` | 222 | `logs\` | Debug crash/log fallback directory | debug-only | Medium | wrapper script |
| `xr_3da/x_ray.cpp` | 57 | `$game_data$` | Opens `system.ltx` | runtime resource | High | wrapper script |
| `xr_3da/x_ray.cpp` | 103 | `user.ltx` | User config path through filesystem API | runtime resource | Medium | wrapper script |
| `xr_3da/x_ray.cpp` | 517 | `$game_levels$` | Level root alias | runtime resource | High | wrapper script |
| `xr_3da/x_ray.cpp` | 523 | `level.ltx` | Level metadata file | runtime resource | High | wrapper script |
| `xr_3da/x_ray.cpp` | 525 | `level.game` | Level game graph/data file | runtime resource | High | wrapper script |
| `xr_3da/x_ray.cpp` | 530 | `level.cform` | Level collision form file | runtime resource | High | wrapper script |
| `xr_3da/Device_create.cpp` | 112 | `$game_data$` | Opens `shaders.xr` | runtime resource | High | wrapper script |
| `xr_3da/IGame_Level.cpp` | 68 | `$level$` | Opens `level.ltx` for current level | runtime resource | High | wrapper script |
| `xr_3da/xr_creator.cpp` | 70 | `$level$` | Opens `level.ltx` | runtime resource | High | wrapper script |
| `xr_3da/xr_area.cpp` | 108 | `$level$` | Opens `level.cform` | runtime resource | High | wrapper script |
| `xr_3da/ResourceManager_Loader.cpp` | 131 | `$game_textures$` | Opens `textures.ltx` | runtime resource | High | wrapper script |
| `xr_3da/ResourceManager_Scripting.cpp` | 161 | `$game_shaders$` | Shader script alias | runtime resource | High | wrapper script |
| `xr_3da/GameFont.cpp` | 34 | `$game_textures$` | Font texture/config lookup | runtime resource | Medium | wrapper script |
| `xr_3da/SH_Texture.cpp` | 137 | `.avi` | Texture animation/video sidecar | runtime resource | Medium | wrapper script |
| `xr_3da/SH_Texture.cpp` | 163 | `.seq` | Texture sequence sidecar | runtime resource | Medium | wrapper script |
| `xr_3da/Texture.cpp` | 366 | `.dds` | Texture file extension lookup | runtime resource | High | wrapper script |
| `xr_3da/xrSound/xr_streamsnd.cpp` | 272 | `$game_sounds$` | Streaming sound lookup | runtime resource | High | wrapper script |
| `xr_3da/xrSound/SoundRender_Core.cpp` | 82 | `$game_data$` | Sound environment/config lookup | runtime resource | Medium | wrapper script |
| `xr_3da/xrSound/SoundRender_Source_loader.cpp` | 98 | `$level$` | Level-local sound lookup | runtime resource | Medium | wrapper script |
| `xr_3da/xrSound/SoundRender_Source_loader.cpp` | 102 | `$game_sounds$` | Game sound lookup | runtime resource | High | wrapper script |
| `xr_3da/xrSound/SoundRender_Source_loader.cpp` | 102 | `$no_sound.ogg` | Missing-sound fallback asset | runtime resource | Medium | wrapper script |
| `xr_3da/xrRender_R1/FStaticRender_Screenshot.cpp` | 76 | `$screenshots$` | Screenshot output directory | runtime resource | Medium | wrapper script |
| `xr_3da/xrRender_R2/r2_screenshot.cpp` | 81 | `$screenshots$` | Screenshot output directory | runtime resource | Medium | wrapper script |
| `xr_3da/xrRender_R2/r2.cpp` | 356 | `$logs$` | Renderer log/debug output | debug-only | Low | wrapper script |
| `xr_3da/ai_script_lua_extension.cpp` | 102 | `$game_data$` | Opens `script.ltx` | runtime resource | High | wrapper script |
| `xr_3da/ai_script_lua_extension.cpp` | 110 | `$game_scripts$` | Lua script loading | runtime resource | High | wrapper script |
| `xr_3da/xrGame/script_engine.cpp` | 424 | `$game_data$` | Opens `script.ltx` | runtime resource | High | wrapper script |
| `xr_3da/xrGame/script_engine.cpp` | 457 | `$game_scripts$` | Script load root | runtime resource | High | wrapper script |
| `xr_3da/xrGame/game_base.cpp` | 114 | `$game_data$` | Script/config lookup | runtime resource | High | wrapper script |
| `xr_3da/xrGame/game_sv_base.cpp` | 242 | `$level$` | Opens `level.game` | runtime resource | High | wrapper script |
| `xr_3da/xrGame/Level_load.cpp` | 21 | `$level$` | Level game loading | runtime resource | High | wrapper script |
| `xr_3da/xrGame/character_info.cpp` | 86 | `$game_data$` | Character XML/config lookup | runtime resource | Medium | wrapper script |
| `xr_3da/xrGame/UIZoneMap.cpp` | 122 | `$game_data$` | Opens `game.ltx` | runtime resource | Medium | wrapper script |
| `xr_3da/xrGame/ui/UIMapWnd.cpp` | 772 | `$game_data$` | Opens `game.ltx` | runtime resource | Medium | wrapper script |
| `xr_3da/xrGame/xrServer_Objects_ALife.cpp` | 49 | `$game_data$` | Opens `game.ltx` | runtime resource | High | wrapper script |
| `xrAI/xrAI.cpp` | 40 | `$game_data$` | AI compiler data root | runtime resource | Medium | wrapper script |
| `xrAI/xrAI.cpp` | 42 | `$game_levels$` | AI compiler level root | runtime resource | Medium | wrapper script |
| `xrAI/xrAI.cpp` | 93 | `level.ai` | AI level file | runtime resource | Medium | wrapper script |
| `xrLC/xrLC.cpp` | 29 | `GameData\Levels\<NAME>\` | Light compiler expected level layout in help text | runtime resource | Medium | wrapper script |
| `xrLC/xrLC.cpp` | 76 | `$game_levels$` | Light compiler level root | runtime resource | Medium | wrapper script |
| `xrLC/xrLC.cpp` | 122 | `gamedata\levels\` | Light compiler fallback/assumed data layout | runtime resource | Medium | wrapper script |
| `xrLC/xrDO_Light/xrDO_Light.cpp` | 25 | `gamedata\levels\` | Detail-object lighting expected data layout | runtime resource | Medium | wrapper script |

## Debug, Test, And Comment-Only Paths

These paths should not drive the initial compatibility build. Keep them recorded so future editor/tool cleanup can distinguish real dependencies from historical notes.

| File | Line | Exact path | Context | Classification | Risk | Recommended fix layer |
|---|---:|---|---|---|---|---|
| `xrLUA/LUA_TEST/debug_test.cpp` | 26 | `x:\bug6.script` | Lua debugger test script | debug-only | Low | leave unchanged |
| `xrLUA/LUA_TEST/debug_test.cpp` | 32 | `x:\test6.script` | Lua debugger test script | debug-only | Low | leave unchanged |
| `xrLUA/LUA_TEST/luabind_test.cpp` | 204 | `x:\luabind.script` | Luabind test script | debug-only | Low | leave unchanged |
| `xrLUA/LUA_TEST/luabind_test.cpp` | 267 | `x:\object.script` | Luabind test script | debug-only | Low | leave unchanged |
| `xrLUA/LUA_TEST/lua_test.cpp` | 41 | `x:\sort.script` | Lua test script | debug-only | Low | leave unchanged |
| `xrLUA/LUA_TEST/script_debugger.cpp` | 167 | `x:\test6.script` | Lua debugger test script | debug-only | Low | leave unchanged |
| `xrXMLParser/xrXMLParser.cpp` | 28 | `x:\Memory.xml` | Commented XML parser test input | debug-only | Low | leave unchanged |
| `xr_3da/xrRender_R2/r2_rendertarget.cpp` | 365 | `x:\r2_material.dds` | Commented render-target dump path | debug-only | Low | leave unchanged |
| `Editors/Tools/DXT/NormalMapGen.cpp` | 720 | `x:\*.tga` | Commented/test texture processing path | debug-only | Low | leave unchanged |

## Practical Migration Guidance

1. **Do not recreate `X:\` as the primary strategy.** It hides the problem and makes CMake less portable. Use a single binary output directory instead, with target runtime outputs matching the legacy names.
2. **Convert build outputs first.** Model all `x:\*.exe`, `x:\*.dll`, `x:\*.lib`, `.map`, `.pdb`, and `.tlb` outputs as CMake runtime/archive/library output directories.
3. **Replace source-level absolute library pragmas cautiously.** They are the highest-risk compile blockers after target creation. Prefer conditional guards for MSVC/CMake rather than deleting historical intent.
4. **Centralize third-party SDK roots.** DirectX, EAX, OpenAL, Ogg/Vorbis, Boost, Luabind, ODE, STLport, MagicFM, and QHull paths should become cache variables or imported targets.
5. **Treat editor and Borland paths as a later phase.** The runtime engine can be made buildable before attempting VCL editor tools.
6. **Use a launcher wrapper for runtime data.** The wrapper should set the working directory beside `fs.ltx`, stage `xr_3da.exe`, `xrGame.dll`, render DLLs, `xrCPU_Pipe.dll`, and make the expected `gamedata`/level/log/screenshot aliases available.
7. **Reserve `mklink` for preservation testing.** If a legacy tool cannot run without `X:\`, a temporary junction can validate behavior, but it should not be the default migration interface.
