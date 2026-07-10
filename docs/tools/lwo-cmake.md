# LWO CMake Shell

## Scope

`BUILD_XR_LWO` adds an experimental `LWO` shared-library target from
`Editors/Tools/LWO`. This is a non-GUI SDK/tool probe for the historical
LightWave object helper DLL.

The target does not use or fake ElPack, VCL, AlexMX, or MagicFM dependencies.

## Historical Project

The source is represented by:

- `Editors/Tools/LWO/LWO.vcproj`
- historical output: `x:\LWO.dll`
- project type: Visual C++ 7.0 Win32 DLL

The project exports a tiny C interface from `LWO.cpp`:

- `LWO_ImportObject`
- `LWO_CloseFile`

Those exports wrap the LightWave object reader API (`lwGetObject` and
`lwFreeObject`).

## Source Snapshot Status

The active `Editors/Tools/LWO` snapshot contains:

- `LWO.cpp`
- `envelope.c`
- `LWO.vcproj`

The historical project references additional LightWave reader sources and
headers that are absent from that active folder, but present in the older
`Editor/Tools/LWO` generation:

- `lwo2.h`
- `envelope.h`
- `clip.c`
- `list.c`
- `lwio.c`
- `lwo2.c`
- `lwob.c`
- `pntspols.c`
- `surface.c`
- `vecmath.c`
- `vmap.c`

CMake imports those older-generation files narrowly for this target instead of
copying or reinventing them. If a project entry is absent from both snapshots,
CMake reports it.

## Build Probe

```powershell
cmake -S . -B build-lwo-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_LWO=ON

cmake --build build-lwo-check --config Release `
  --target LWO -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configure succeeds and the target is generated. The missing LightWave reader
files are resolved from `Editor/Tools/LWO`, and the Release build currently
passes.

```text
cmake --build build-lwo-check --config Release `
  --target LWO -- /m:1 /v:minimal /clp:ErrorsOnly
```

This target remains non-GUI and does not use the Borland/VCL editor package
stack.

## Output Verification

The Release build currently produces:

- `build-lwo-check/bin/LWO.dll`
- `build-lwo-check/lib/LWO.lib`
- `build-lwo-check/lib/LWO.exp`

Use Visual Studio `dumpbin` to inspect the generated DLL:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /exports build-lwo-check\bin\LWO.dll

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /headers build-lwo-check\bin\LWO.dll

& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x86\dumpbin.exe" `
  /dependents build-lwo-check\bin\LWO.dll
```

Current verification result:

- PE machine type: `14C machine (x86)` / 32-bit DLL.
- exports:
  - `_LWO_CloseFile@4`
  - `_LWO_ImportObject@8`
- dependents:
  - `VCRUNTIME140.dll`
  - Universal CRT API-set DLLs for stdio/heap/math/runtime
  - `KERNEL32.dll`

The decorated export names match the target's historical `__stdcall` calling
convention.

This verifies the binary shape only. Keep `build-lwo-check`, logs, import
libraries, and any test model/output data out of the repository.

## Next Honest Step

Next, validate the exported `LWO_ImportObject` and `LWO_CloseFile` entrypoints
against a lawful synthetic or user-owned LightWave object fixture. Do not
replace the reader with guessed parser code. A clean-room reader should be a
separate researched tool after the format is documented.
