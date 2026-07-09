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

The repository snapshot contains:

- `LWO.cpp`
- `envelope.c`
- `LWO.vcproj`

The historical project references additional LightWave reader sources and
headers that are absent from this snapshot, including:

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

CMake reports the missing project entries instead of inventing replacements.

## Build Probe

```powershell
cmake -S . -B build-lwo-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_LWO=ON

cmake --build build-lwo-check --config Release `
  --target LWO -- /m:1 /v:minimal /clp:ErrorsOnly
```

Configure succeeds and the target is generated. The first compile blocker is
expected to be the missing LightWave reader header:

```text
Editors/Tools/LWO/LWO.cpp(...): fatal error C1083:
Cannot open include file: 'lwo2.h'
```

This is a source/dependency snapshot gap, not a GUI package blocker.

## Next Honest Step

Locate the matching historical LightWave reader source set or another lawful
copy of the missing files. Do not replace it with guessed parser code in this
target. A clean-room reader should be a separate researched tool after the
format is documented.
