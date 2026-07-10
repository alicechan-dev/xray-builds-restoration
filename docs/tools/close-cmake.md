# close CMake Shell

## Scope

`BUILD_XR_CLOSE` adds an experimental `close` executable target from
`xrLC/close`. This is a small non-GUI Win32 helper from the level-compiler
tree.

The target does not use or fake ElPack, VCL, AlexMX, or MagicFM dependencies,
and it does not pull in the full `xrLC` compiler.

## Historical Project

The source is represented by:

- `xrLC/close/close.vcproj`
- historical output: `close.exe`
- project type: Visual C++ 7.1 Win32 executable

The helper sends a mailslot message to another process for selected historical
developer usernames. It links `xrCore` because the original source includes
`xrCore.h`.

The historical source carries a stale pragma for `x:\xrCore.lib`; the CMake
target links the local `xrCore` target and suppresses only that absolute
default-library request.

## Build Probe

```powershell
cmake -S . -B build-close-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_CLOSE=ON

cmake --build build-close-check --config Release `
  --target close -- /m:1 /v:minimal /clp:ErrorsOnly
```

## Verification

After a successful build, inspect the executable shape with Visual Studio
`dumpbin`:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /headers build-close-check\bin\close.exe

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /dependents build-close-check\bin\close.exe
```

## Current Status

The target is intentionally narrow. It restores the helper executable shape and
leaves the larger `xrLC`, `xrDO_Light`, and `xrAI` compiler/toolchain
restoration for separate dependency-focused passes.

The Release build currently produces:

- `build-close-check/bin/close.exe`

Current verification result:

- PE machine type: `14C machine (x86)` / 32-bit executable.
- subsystem: Windows GUI.
- dependents:
  - `KERNEL32.dll`
  - `ADVAPI32.dll`
  - `VCRUNTIME140.dll`
  - Universal CRT API-set DLLs for string/stdio/runtime/math/locale/heap

This checkpoint verifies the executable shape only. Do not commit the build
directory, logs, or generated tool output.
