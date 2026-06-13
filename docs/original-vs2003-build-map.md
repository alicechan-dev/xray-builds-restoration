# Original VS2003 Build Map

This document records the original Visual Studio .NET 2003 build graph for the historical S.T.A.L.K.E.R. X-Ray Build 1935 snapshot. It is intended to be the source of truth for adding a modern CMake compatibility layer without rewriting the original `.sln` or `.vcproj` files.

Source solution: `xr_3da/XR_3DA.sln`

## Global Observations

- Solution format: `Microsoft Visual Studio Solution File, Format Version 8.00`.
- Project format: Visual C++ `.vcproj` files with `Version="7.10"`; this is Visual C++ 7.1 / Visual Studio .NET 2003.
- Platform: all solution configurations are `Win32`.
- Solution configurations: `Debug`, `Mixed`, `Release`.
- `ConfigurationType="1"` means application/executable.
- `ConfigurationType="2"` means dynamic library/DLL.
- No static-library target is referenced directly by `XR_3DA.sln`.
- Most targets use `OutputDirectory="x:\"`, with import libraries, maps, and type libraries also frequently written under `x:\`.
- CMake should treat `x:\` as a historical staging directory convention, not as a required source-tree location.
- Most projects have no `VCPostBuildEventTool` command. The only referenced project with post-build commands is `ode`, which runs `regsvr32 /s /c "$(TargetPath)"`.

## Suspicious Missing or Mismatched References

- `XR_3DA.sln` references `..\xrSound\xrSound.vcproj`, but this snapshot contains `xr_3da/xrSound/xrSound.vcproj`; there is no top-level `xrSound` directory.
- `XR_3DA.sln` references `..\xrCDB\xrCDB.vcproj`, but this snapshot contains `xr_3da/xrCDB/xrCDB.vcproj`; there is no top-level `xrCDB` directory.
- `xrSound` includes and links EAX/OpenAL/Ogg/Vorbis dependencies, but `eax.h` was not found in the snapshot.
- `xrXMLParser` links Chilkat import libraries that exist in `xrXMLParser`, but the matching runtime DLLs are not obvious in the snapshot.
- Several projects link or pragma-link against `x:\xr_3da.lib`; a CMake migration must preserve an import library for the `XR_3DA` executable or explicitly model this old link relationship.
- Runtime loading expects DLLs beside the executable: `xrGame.dll`, `xrRender_R1.dll`, `xrRender_R2.dll`, `xrCPU_Pipe.dll`, and `xrLauncher.dll`.

## Solution Dependency Graph

| Project | Explicit project dependencies from `XR_3DA.sln` |
|---|---|
| `XR_3DA` | `xrSound`, `xrParticles`, `xrLUA`, `xrCDB`, `xrCore` |
| `xrCPU_Pipe` | `XR_3DA`, `xrCore` |
| `xrGame` | `xrSound`, `xrParticles`, `xrXMLParser`, `xrNetServer`, `XR_3DA`, `xrCPU_Pipe`, `xrLUA`, `xrCDB`, `xrCore` |
| `xrSound` | `xrCDB`, `xrCore` |
| `xrCore` | none |
| `xrCDB` | `xrCore` |
| `xrRender_R1` | `xrParticles`, `XR_3DA`, `xrCPU_Pipe`, `xrCDB`, `xrCore` |
| `xrRender_R2` | `xrParticles`, `XR_3DA`, `xrCPU_Pipe`, `xrCDB`, `xrCore` |
| `xrXMLParser` | `xrCore` |
| `xrLUA` | `xrCore` |
| `ode` | none |
| `xrParticles` | `xrCore` |
| `xrNetServer` | `xrCore` |
| `xrSE_Factory` | `xrCore` |
| `xrCompress` | `xrCore` |

## Project Map

### XR_3DA

- Project path: `xr_3da/XR_3DA.vcproj`
- Output type: `exe`
- Output filename: `x:\XR_3DA.exe`
- Source files/directories: 252 files; `xr_3da`, `xr_3da/Blenders`
- Include directories:
  - Debug: none
  - Mixed/Release: `scripting\`, `..`
- Preprocessor definitions:
  - Debug: `_DEBUG`, `DEBUG`, `WIN32`, `_WINDOWS`, `ENGINE_BUILD`, `SECURE_GAMEDATA`
  - Mixed: `DEBUG`, `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `ENGINE_BUILD`
  - Release: `NDEBUG`, `WIN32`, `_WINDOWS`, `ENGINE_BUILD`
- Linked libraries from project file: `msacm32.lib`, `vfw32.lib`
- Additional pragma-linked libraries observed: `x:\xrCore.lib`, `x:\xrCDB.lib`, `x:\xrSound.lib`, `x:\xrLUA.lib`, `winmm.lib`, `d3d9.lib`, `dinput.lib`, `dxguid.lib`, `d3dx9.lib`
- Project dependencies: `xrSound`, `xrParticles`, `xrLUA`, `xrCDB`, `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `.\Mixed`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\XR_3DA.exe`, `x:\XR_3DA.map`, `x:\XR_3DA.tlb`, pragma links to `x:\*.lib`

### xrCPU_Pipe

- Project path: `xr_3da/xrCPU_Pipe/xrCPU_Pipe.vcproj`
- Output type: `dll`
- Output filename: `x:\xrCPU_Pipe.dll`
- Source files/directories: 19 files; `xr_3da/xrCPU_Pipe`
- Include directories: none specified
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `DEBUG`, `_WINDOWS`, `_USRDLL`, `XRCPU_PIPE_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRCPU_PIPE_EXPORTS`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRCPU_PIPE_EXPORTS`
- Linked libraries: `x:\xr_3da.lib`, `odbc32.lib`, `odbccp32.lib`
- Project dependencies: `XR_3DA`, `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `Mixed`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\xrCPU_Pipe.dll`, `x:\xrCPU_Pipe.map`, `x:\xrCPU_Pipe.lib`, `x:\xrCPU_Pipe.tlb`, `x:\xr_3da.lib`

### xrGame

- Project path: `xr_3da/xrGame/xrGame.vcproj`
- Output type: `dll`
- Output filename: `x:\xrGame.dll`
- Source files/directories: 2048 files; `xr_3da/xrGame`, `AI`, `AI/Crow`, `AI/Idol`, `AI/Monsters/*`, `AI/Rat`, `AI/script`, `AI/Stalker`, `AI/Trader`, `dCylinder`, `GameSpy`, `GameSpy/CDKey`, `GameSpy/Qr2`, `luabind`, `tri-colliderknoopc`, `ui`
- Include directories: none specified in project file
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `DEBUG`, `_WINDOWS`, `_USRDLL`, `XRGAME_EXPORTS`, `dSINGLE`, `MSVC`
  - Mixed: `DEBUG`, `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRGAME_EXPORTS`, `dSINGLE`, `MSVC`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRGAME_EXPORTS`, `dSINGLE`, `MSVC`
- Linked libraries from project file:
  - Debug: `x:\xr_3da.lib`
  - Mixed/Release: `odbc32.lib`, `odbccp32.lib`, `x:\xr_3da.lib`
- Additional pragma-linked libraries observed: `ode.lib`, `x:\xrNetServer`, `x:\xrXMLParser.lib`, GameSpy pragmas for `ws2_32`, `wsock32`, `advapi32`, `platutil`, `winsock`
- Project dependencies: `xrSound`, `xrParticles`, `xrXMLParser`, `xrNetServer`, `XR_3DA`, `xrCPU_Pipe`, `xrLUA`, `xrCDB`, `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `.\Mixed`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\xrGame.dll`, `x:\xrGame.map`, `x:\xrGame.lib`, `x:\xrGame.tlb`, `x:\xr_3da.lib`

### xrSound

- Project path in solution: `..\xrSound\xrSound.vcproj`
- Project path in snapshot: `xr_3da/xrSound/xrSound.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)/xrSound.dll`, with `OutDir=x:\`
- Source files/directories: 42 files; `xr_3da/xrSound`
- Include directories: none specified in `.vcproj`; Borland sidecar mentions historical SDK paths for DirectX 9, Ogg/Vorbis, EAX 2.0, and OpenAL
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `_WINDOWS`, `_USRDLL`, `XRSOUND_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRSOUND_EXPORTS`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRSOUND_EXPORTS`
- Linked libraries from project file: none
- Additional pragma-linked libraries observed: `dxguid`, `eaxB.lib`, `oggB.lib`, `vorbisfileB.lib`, `dsoundb.lib`, `x:\eax.lib`, `x:\vorbis.lib`, `x:\vorbisfile.lib`, `dsound.lib`, `msacm32.lib`
- Project dependencies: `xrCDB`, `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `Mixed`, `Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\` output dir, `x:\xrSound.map`, pragma links to `x:\eax.lib`, `x:\vorbis.lib`, `x:\vorbisfile.lib`
- Suspicious references: missing solution path; missing EAX header/import library in repository

### xrCore

- Project path: `xrCore/xrCore.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)/xrCore.dll`, with `OutDir=x:\`
- Source files/directories: 94 files; `xrCore`
- Include directories: none specified
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `_WINDOWS`, `_USRDLL`, `XRCORE_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRCORE_EXPORTS`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRCORE_EXPORTS`
- Linked libraries from project file: none
- Additional pragma-linked libraries observed: `winmm.lib`; `DBGHELP.DLL` is loaded dynamically by crash/minidump code
- Project dependencies: none
- Output directories: `x:\`; intermediates are `.\Debug`, `Mixed`, `Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\` output dir, `x:\xrCore.map`

### xrCDB

- Project path in solution: `..\xrCDB\xrCDB.vcproj`
- Project path in snapshot: `xr_3da/xrCDB/xrCDB.vcproj`
- Output type: `dll`
- Output filename: `x:\xrCDB.dll`
- Source files/directories: 74 files; `xr_3da/xrCDB`
- Include directories: none specified
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `_WINDOWS`, `_USRDLL`, `XRCDB_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRCDB_EXPORTS`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRCDB_EXPORTS`
- Linked libraries from project file:
  - Debug: `odbc32.lib`, `odbccp32.lib`
  - Mixed/Release: none
- Additional pragma-linked libraries observed: `xrCoreB.lib`, `x:\xrCore.lib`, `winmm.lib`
- Project dependencies: `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `.\Mixed`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\xrCDB.dll`, `x:\xrCDB.map`, `x:\xrCDB.lib`, `x:\xrCDB.tlb`, `x:\xrCore.lib`
- Suspicious references: missing solution path

### xrRender_R1

- Project path: `xr_3da/xrRender_R1/xrRender_R1.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)/xrRender_R1.dll`, with `OutDir=x:\`
- Source files/directories: 117 files; `xr_3da/xrRender_R1`
- Include directories: none specified
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `_WINDOWS`, `_USRDLL`, `XRRENDER_R1_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRRENDER_R1_EXPORTS`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRRENDER_R1_EXPORTS`
- Linked libraries from project file: none
- Additional pragma-linked libraries observed: `x:\xr_3DA`, `d3dx9.lib`
- Project dependencies: `xrParticles`, `XR_3DA`, `xrCPU_Pipe`, `xrCDB`, `xrCore`
- Output directories: `x:\`; intermediates are `Debug`, `Mixed`, `Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\` output dir, pragma link to `x:\xr_3DA`

### xrRender_R2

- Project path: `xr_3da/xrRender_R2/xrRender_R2.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)/xrRender_R2.dll`, with `OutDir=x:\`
- Source files/directories: 142 files; `xr_3da/xrRender_R2`
- Include directories: none specified
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `_WINDOWS`, `_USRDLL`, `XRRENDER_R2_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRRENDER_R2_EXPORTS`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRRENDER_R2_EXPORTS`
- Linked libraries from project file: none
- Additional pragma-linked libraries observed: `x:\xr_3DA`, `d3dx9.lib`
- Project dependencies: `xrParticles`, `XR_3DA`, `xrCPU_Pipe`, `xrCDB`, `xrCore`
- Output directories: `x:\`; intermediates are `Debug`, `Mixed`, `Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\` output dir, pragma link to `x:\xr_3DA`

### xrXMLParser

- Project path: `xrXMLParser/xrXMLParser.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)\xrXMLParser.dll`, with `OutDir=x:\`
- Source files/directories: 10 files; `xrXMLParser`
- Include directories: `..`
- Preprocessor definitions:
  - Debug: `_DEBUG`, `WIN32`, `_WINDOWS`, `XRXMLPARSER_EXPORTS`
  - Mixed: `MIXED`, `NDEBUG`, `WIN32`, `_WINDOWS`, `XRXMLPARSER_EXPORTS`
  - Release: `NDEBUG`, `WIN32`, `_WINDOWS`, `XRXMLPARSER_EXPORTS`
- Linked libraries:
  - Debug: `odbc32.lib`, `odbccp32.lib`, `ChilKatDbgDll.lib`, `CKBaseDbgDll.lib`, `wininet.lib`
  - Mixed/Release: `odbc32.lib`, `odbccp32.lib`, `ChilKatRelDll.lib`, `CKBaseRelDll.lib`, `wininet.lib`
- Additional pragma-linked libraries observed: `x:\xrCoreB.lib`, `x:\xrCore.lib`
- Project dependencies: `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `.\Mixed`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\` output dir, pragma links to `x:\xrCore*.lib`
- Suspicious references: Chilkat import libraries are present, runtime DLLs are not obvious

### xrLUA

- Project path: `xrLUA/xrLUA.vcproj`
- Output type: `dll`
- Output filename:
  - Debug: `x:\xrLUA.dll`
  - Mixed/Release: `$(OutDir)/xrLUA.dll`, with `OutDir=x:\`
- Source files/directories: 54 files; `xrLUA`, `xrLUA/etc`, `xrLUA/src`, `xrLUA/src/lib`
- Include directories: `include\`
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `_WINDOWS`, `_USRDLL`, `XRLUA_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRLUA_EXPORTS`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRLUA_EXPORTS`
- Linked libraries from project file: none
- Additional pragma-linked libraries observed: `x:\xrCoreB.lib`, `x:\xrCore.lib`
- Project dependencies: `xrCore`
- Output directories: `x:\`; intermediates are `Debug`, `.\Mixed`, `Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\xrLUA.dll`, pragma links to `x:\xrCore*.lib`

### ode

- Project path: `xrODE/contrib/msvc7/ode_default/default.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)/ode.dll`, with `OutDir=x:\`
- Source files/directories: 77 files; `xrODE/config`, `xrODE/include/ode`, `xrODE/ode/src`
- Include directories: `../../../include`
- Preprocessor definitions:
  - Debug: `WIN32`, `_WINDOWS`, `_DEBUG`, `_USRDLL`, `MSVC`, `dSINGLE`
  - Mixed/Release: `WIN32`, `_WINDOWS`, `NDEBUG`, `_USRDLL`, `dSINGLE`, `MSVC`, `dNODEBUG`
- Linked libraries from project file: none
- Project dependencies: none
- Output directories: `x:\`; intermediates are `Debug`, `Release`, `$(ConfigurationName)`
- Post-build commands: `regsvr32 /s /c "$(TargetPath)"` for Debug, Mixed, and Release
- Hardcoded absolute paths: `x:\` output dir
- Migration note: registering ODE with `regsvr32` should be opt-in or omitted until proven necessary

### xrParticles

- Project path: `xrParticles/xrParticles.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)\xrParticles.dll`, with `OutDir=x:\`
- Source files/directories: 16 files; `xrParticles`
- Include directories: `..`
- Preprocessor definitions:
  - Debug: `_DEBUG`, `WIN32`, `_WINDOWS`, `XR_PARTICLES_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `XR_PARTICLES_EXPORTS`
  - Release: `NDEBUG`, `WIN32`, `_WINDOWS`, `XR_PARTICLES_EXPORTS`
- Linked libraries: `odbc32.lib`, `odbccp32.lib`
- Additional pragma-linked libraries observed: `x:\xrCoreB.lib`, `x:\xrCore.lib`
- Project dependencies: `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `.\Mixed`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\` output dir, pragma links to `x:\xrCore*.lib`

### xrNetServer

- Project path: `xrNetServer/xrNetServer.vcproj`
- Output type: `dll`
- Output filename: `$(OutDir)\xrNetServer.dll`, with `OutDir=x:\`
- Source files/directories: 11 files; `xrNetServer`
- Include directories: `..`
- Preprocessor definitions:
  - Debug: `_DEBUG`, `WIN32`, `_WINDOWS`, `XR_NETSERVER_EXPORTS`
  - Mixed: `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `XR_NETSERVER_EXPORTS`
  - Release: `NDEBUG`, `WIN32`, `_WINDOWS`, `XR_NETSERVER_EXPORTS`
- Linked libraries: `odbc32.lib`, `odbccp32.lib`
- Additional pragma-linked libraries observed: `x:\xrCore`, `dxguid.lib`, `x:\xrNetServer`
- Project dependencies: `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `.\Mixed`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\` output dir, pragma links to `x:\xrCore`, `x:\xrNetServer`

### xrSE_Factory

- Project path: `xrSE_Factory/xrSE_Factory.vcproj`
- Output type: `dll`
- Output filename:
  - Debug/Mixed: `x:\xrSE_Factory.dll`
  - Release: `$(OutDir)/xrSE_Factory.dll`, with `OutDir=x:\`
- Source files/directories: 182 files; `xrSE_Factory`, `xrSE_Factory/luabind`
- Include directories: none specified
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `DEBUG`, `_WINDOWS`, `_USRDLL`, `XRSE_FACTORY_EXPORTS`, `MSVC`
  - Mixed: `DEBUG`, `MIXED`, `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRSE_FACTORY_EXPORTS`, `dSINGLE`, `MSVC`
  - Release: `WIN32`, `NDEBUG`, `_WINDOWS`, `_USRDLL`, `XRSE_FACTORY_EXPORTS`
- Linked libraries:
  - Mixed: `odbc32.lib`, `odbccp32.lib`
  - Debug/Release: none in project file
- Additional pragma-linked libraries observed: `x:\xrCore.lib`, `x:\xrLUA.lib`, `x:\xrXMLParser.lib`
- Project dependencies: `xrCore`
- Output directories: `x:\`; intermediates are `.\Debug`, `.\Mixed`, `Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:\xrSE_Factory.dll`, `x:\xrSE_Factory.map`, `x:\xrSE_Factory.lib`, `x:\xrSE_Factory.tlb`, pragma links to `x:\*.lib`

### xrCompress

- Project path: `xrCompress/xrCompress.vcproj`
- Output type: `exe`
- Output filename: `$(OutDir)\xrCompress.exe`, with `OutDir=x:\` or `x:`
- Source files/directories: 126 files; `xrCompress`, `xrCompress/lzo`
- Include directories: none specified
- Preprocessor definitions:
  - Debug: `WIN32`, `_DEBUG`, `_CONSOLE`
  - Release: `WIN32`, `NDEBUG`, `_CONSOLE`
- Linked libraries: `odbc32.lib`, `odbccp32.lib`
- Project dependencies: `xrCore`
- Output directories:
  - Debug: `x:`
  - Release: `x:\`
  - Intermediates: `.\Debug`, `.\Release`
- Post-build commands: none
- Hardcoded absolute paths: `x:` / `x:\` output dirs

## Hardcoded Absolute Path Inventory

The referenced projects repeatedly hardcode:

- `x:\` as output directory.
- `x:\*.dll`, `x:\*.exe`, `x:\*.lib`, `x:\*.map`, `x:\*.tlb` as generated artifacts.
- `x:\xr_3da.lib` as an import library consumed by `xrCPU_Pipe`, `xrGame`, and renderers.
- `x:\xrCore.lib` / `x:\xrCoreB.lib` in pragma comments.
- `x:\xrXMLParser.lib`, `x:\xrLUA.lib`, `x:\xrNetServer`, `x:\xrParticles.lib`, and related debug variants in pragma comments.
- `x:\eax.lib`, `x:\vorbis.lib`, `x:\vorbisfile.lib` in `xrSound`.

Historical Borland sidecar files also mention SDK roots such as `R:\SDK\DirectX9.0_sum_2004`, `R:\SDK\ogg`, `R:\SDK\eax20`, and `R:\SDK\OpenAL`; these are not part of `XR_3DA.sln` but are useful dependency clues.

## CMake-Relevant Linked Library Notes

- `#pragma comment(lib, ...)` is heavily used and must be accounted for, either by preserving MSVC pragma behavior or replacing it with explicit CMake `target_link_libraries`.
- DirectX 9 dependencies include `d3d9.lib`, `d3dx9.lib`, `dinput.lib`, `dxguid.lib`, and `dsound.lib`.
- Windows system libraries include `winmm.lib`, `msacm32.lib`, `vfw32.lib`, `wininet.lib`, `odbc32.lib`, `odbccp32.lib`, `ws2_32.lib`, `wsock32.lib`, and `advapi32.lib`.
- Audio dependencies include OpenAL, EAX 2.0, Ogg, Vorbis, and VorbisFile.
- XML dependency is Chilkat via `ChilKatRelDll.lib`, `CKBaseRelDll.lib`, `ChilKatDbgDll.lib`, and `CKBaseDbgDll.lib`.
- Physics dependency is `ode.dll` built from the bundled ODE project.

