# CMake Output Layout

Historical snapshot: S.T.A.L.K.E.R. X-Ray Build 1935.

This document defines a path-agnostic CMake output layout for a preservation-oriented build layer. It does not require an `X:\` drive and does not modify the original VS2003 `.sln` or `.vcproj` files.

## Goals

- Build products must not depend on `X:\`.
- Runtime executables and DLLs must be staged into `build/bin`.
- Static libraries and DLL import libraries must be staged into `build/lib`.
- Object files, generated files, PDBs, and other intermediate files must remain under `build/`.
- Output file names must match original engine expectations.
- `XR_3DA.exe` must be launchable from `build/bin`.

## Directory Contract

Assuming an out-of-source configure such as:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The layout should be:

```text
build/
  bin/
    XR_3DA.exe
    xrCore.dll
    xrCDB.dll
    xrSound.dll
    xrLUA.dll
    xrXMLParser.dll
    xrParticles.dll
    xrNetServer.dll
    xrSE_Factory.dll
    xrGame.dll
    xrCPU_Pipe.dll
    xrRender_R1.dll
    xrRender_R2.dll
    wrap_oal.dll              # if OpenAL is built/copied dynamically
    ogg.dll                   # only if Ogg is dynamic
    vorbis.dll                # only if Vorbis is dynamic
    vorbisfile.dll            # only if Vorbisfile is dynamic
  lib/
    XR_3DA.lib
    xrCore.lib
    xrCDB.lib
    xrSound.lib
    xrLUA.lib
    xrXMLParser.lib
    xrParticles.lib
    xrNetServer.lib
    xrSE_Factory.lib
    xrGame.lib
    xrCPU_Pipe.lib
    xrRender_R1.lib
    xrRender_R2.lib
    *.a / *.lib static libraries
  obj/, CMakeFiles/, <target>.dir/, etc.
```

For multi-config generators such as Visual Studio, CMake normally appends the configuration name. To keep the launch path stable, set per-configuration output directories to the same `build/bin` and `build/lib` paths.

## Top-Level CMake Settings

Place these settings in the future top-level `CMakeLists.txt` before targets are created:

```cmake
cmake_minimum_required(VERSION 3.24)

project(XRayBuild1935 LANGUAGES C CXX)

set(XRAY_OUTPUT_ROOT "${CMAKE_BINARY_DIR}" CACHE PATH "Root for generated build outputs")
set(XRAY_RUNTIME_DIR "${XRAY_OUTPUT_ROOT}/bin" CACHE PATH "Directory for executables and runtime DLLs")
set(XRAY_LIBRARY_DIR "${XRAY_OUTPUT_ROOT}/lib" CACHE PATH "Directory for static and import libraries")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb")

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}")
  set(CMAKE_PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb")
endforeach()
```

Notes:

- `RUNTIME_OUTPUT_DIRECTORY` controls `.exe` and `.dll` outputs on Windows.
- `LIBRARY_OUTPUT_DIRECTORY` is included for portability, though Windows DLLs use the runtime directory.
- `ARCHIVE_OUTPUT_DIRECTORY` controls static libraries and MSVC DLL import libraries.
- `PDB_OUTPUT_DIRECTORY` keeps debug symbols under `build/pdb` instead of scattering them through source directories.

## Target Output Names

The engine loads several DLLs by exact filename. CMake targets may use clean internal target names, but their output names must match the historical runtime names.

Use target properties like this:

```cmake
set_target_properties(XR_3DA PROPERTIES
  OUTPUT_NAME "XR_3DA"
)

set_target_properties(xrRender_R1 PROPERTIES
  OUTPUT_NAME "xrRender_R1"
)

set_target_properties(xrRender_R2 PROPERTIES
  OUTPUT_NAME "xrRender_R2"
)

set_target_properties(xrGame PROPERTIES
  OUTPUT_NAME "xrGame"
)

set_target_properties(xrCPU_Pipe PROPERTIES
  OUTPUT_NAME "xrCPU_Pipe"
)
```

The minimum runtime-critical names are:

| CMake target | Output file expected by code |
|---|---|
| `XR_3DA` | `XR_3DA.exe` |
| `xrRender_R1` | `xrRender_R1.dll` |
| `xrRender_R2` | `xrRender_R2.dll` |
| `xrGame` | `xrGame.dll` |
| `xrCPU_Pipe` | `xrCPU_Pipe.dll` |
| `xrCore` | `xrCore.dll` |
| `xrCDB` | `xrCDB.dll` |
| `xrSound` | `xrSound.dll` |
| `xrLUA` | `xrLUA.dll` |
| `xrXMLParser` | `xrXMLParser.dll` |
| `xrParticles` | `xrParticles.dll` |
| `xrNetServer` | `xrNetServer.dll` |
| `xrSE_Factory` | `xrSE_Factory.dll` |

Optional/tool-phase names:

| CMake target | Output file expected by code/tool |
|---|---|
| `xrLauncher` | `xrLauncher.dll` |
| `SciLexer` | `SciLexer.dll` or `SciLexer.DLL` |
| `xrLC_Options` | `xrLC_Options.dll` |
| `xrEPropsB` | `xrEPropsB.dll` |
| `copt` | `tools/copt.dll` |

## Per-Target Placement

If a target needs explicit placement, use:

```cmake
set_target_properties(xrGame PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
  )
endforeach()
```

Prefer the top-level settings for consistency. Use per-target overrides only for special cases like tool DLLs that intentionally live in a subdirectory.

## Tool Subdirectories

Most engine DLLs should live directly beside `XR_3DA.exe`. A small number of old editor utilities expect subdirectories, for example `tools\copt.dll`.

For those, use explicit output directories:

```cmake
set(XRAY_TOOLS_RUNTIME_DIR "${XRAY_RUNTIME_DIR}/tools")

set_target_properties(copt PROPERTIES
  OUTPUT_NAME "copt"
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_TOOLS_RUNTIME_DIR}"
)
```

Do not use this pattern for `xrGame.dll`, render DLLs, `xrCPU_Pipe.dll`, or core engine DLLs; those must remain directly in `build/bin`.

## Runtime Working Directory

The engine should be launched with `build/bin` as the current working directory:

```powershell
cd build\bin
.\XR_3DA.exe
```

The CMake layer should also provide a helper target or script later:

```cmake
add_custom_target(run-xray
  COMMAND "$<TARGET_FILE:XR_3DA>"
  WORKING_DIRECTORY "${XRAY_RUNTIME_DIR}"
  DEPENDS XR_3DA xrGame xrCPU_Pipe xrRender_R1
)
```

This does not solve game data lookup by itself. Runtime data still needs an `fs.ltx` and the expected game-data aliases available from the working directory or a wrapper script.

## Third-Party Runtime DLLs

Do not copy Windows system DLLs into `build/bin`:

- `d3d9.dll`
- `dsound.dll`
- `dinput.dll`
- `DBGHELP.DLL`, unless a known redistributable copy is explicitly supplied
- `shell32.dll`
- `comctl32.dll`
- .NET Framework assemblies for `xrLauncher.dll`

Do stage non-system DLLs if the CMake configuration builds or imports them dynamically:

```cmake
add_custom_command(TARGET xrSound POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "$<TARGET_FILE:wrap_oal>"
          "${XRAY_RUNTIME_DIR}/$<TARGET_FILE_NAME:wrap_oal>"
)
```

For imported third-party DLLs:

```cmake
set(WRAP_OAL_DLL "" CACHE FILEPATH "Path to wrap_oal.dll")

if(WRAP_OAL_DLL)
  add_custom_command(TARGET xrSound POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${WRAP_OAL_DLL}"
            "${XRAY_RUNTIME_DIR}/wrap_oal.dll"
  )
endif()
```

## Preservation Rules

- Do not edit `xr_3da/XR_3DA.sln` or any original `.vcproj` files to achieve this layout.
- Do not recreate the old `X:\` output drive as the default build path.
- Do not rename runtime DLLs away from what `LoadLibrary` expects.
- Do not move runtime DLLs into per-target or per-config folders if `XR_3DA.exe` must launch directly from `build/bin`.
- Keep CMake-generated files under `build/`; never emit build artifacts into the source tree.

## Recommended First-Pass Target Set

For a launchable R1 runtime, the first CMake pass should aim to produce:

```text
build/bin/XR_3DA.exe
build/bin/xrCore.dll
build/bin/xrCDB.dll
build/bin/xrSound.dll
build/bin/xrLUA.dll
build/bin/xrXMLParser.dll
build/bin/xrParticles.dll
build/bin/xrNetServer.dll
build/bin/xrSE_Factory.dll
build/bin/xrGame.dll
build/bin/xrCPU_Pipe.dll
build/bin/xrRender_R1.dll
```

Add `xrRender_R2.dll`, `xrLauncher.dll`, editor DLLs, and tool-specific subdirectories only after the R1 runtime layout works.
