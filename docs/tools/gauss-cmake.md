# Gauss CMake Shell

## Scope

`BUILD_XR_GAUSS` adds an experimental `Gauss` shared-library target from
`Editor/Tools/gauss`. The DLL provides small image-kernel helpers and does not
depend on the Borland/VCL, ElPack, AlexMX, or MagicFM GUI package stack.

## Historical Project

The canonical first target is the complete Visual C++ 6 project:

- `Editor/Tools/gauss/gauss.dsp`
- project type: Win32 x86 DLL
- historical output: `x:\gauss.dll`

The later `Editors/Tools/gauss/Gauss.vcproj` snapshot is not used. It lacks
`Gauss.cpp` and several listed shared headers while adding unrelated editor
engine/log/filesystem scaffolding. The older project is complete and directly
owns the filter exports.

The DLL exports:

- `gauss`, a 7 by 7 kernel data symbol;
- `ip_BuildKernel`, which normalizes a caller-provided kernel;
- `ip_ProcessKernel`, which applies a kernel to packed RGBA pixels.

## Dependencies

The historical PCH includes DirectX 8-era headers. The target uses
`XR_LEGACY_DX_ROOT` first and otherwise falls back to the existing DirectX SDK
selection. These include/library paths remain target-local.

The VC6 snapshot also retained an include of an absent `clsid.h`, although no
project source uses `CLASS_ID` or its conversion helpers. The modern target
removes that orphan include instead of importing an incompatible header or
adding an unnecessary xrCore dependency.

## Build Probe

```powershell
cmake -S . -B build-gauss-check -G "Visual Studio 17 2022" -A Win32 `
  -DBUILD_XR_GAUSS=ON `
  -DXR_LEGACY_DX_ROOT=D:\Projects\Others\DXGarbage

cmake --build build-gauss-check --config Release `
  --target Gauss -- /m:1 /v:minimal /clp:ErrorsOnly
```

## Verification

After a successful build, inspect the DLL without using proprietary assets:

```powershell
dumpbin /headers build-gauss-check\bin\gauss.dll
dumpbin /exports build-gauss-check\bin\gauss.dll
dumpbin /dependents build-gauss-check\bin\gauss.dll
```

Do not commit the build directory, DLL/import library, logs, or generated image
data.

## Current Status

Configure succeeds with `BUILD_XR_GAUSS=ON`. The first build probe found an
orphan PCH include of an absent `clsid.h`; no project source uses its types or
conversion functions, so that stale include was removed without importing an
incompatible header or adding xrCore.

The `_sphere.h` dependent-type blocker is fixed with source-local `typename`
qualification on the `VectorList` iterator aliases and out-of-class nested-type
return declarations. No algorithm or container behavior changed.

The Gauss-local `_stl_extensions.h` no longer depends on removed
`std::binary_function`. A stateless local compatibility base preserves its
`first_argument_type`, `second_argument_type`, and `result_type` aliases while
leaving both string predicate call operators unchanged. The shared xrCore copy
is not modified.

The absent `Log.h` and `Engine.h` PCH includes were removed because neither
header is in the canonical VC6 project or source folder. A later compile stage
shows that logging itself is not entirely unused: `_math.cpp` and `FS.cpp`
still call the free `Msg(...)` function.

The `_vector3d.h` component-wise minimum and maximum operations now use
macro-safe `(std::min)(...)` and `(std::max)(...)` calls instead of removed VC6
STL implementation details. `NOMINMAX` and `<algorithm>` were already present,
and comparison behavior is unchanged.

The newer sibling's `Msg(...)` implementation is not imported because it routes
through global `CLog` state and carries editor splash, dialog, plugin, and file
logging dependencies. The modern target instead owns a bounded variadic
`GaussLog` helper with the same call contract and debugger-only output.

Release now builds and produces:

- `build-gauss-check/bin/gauss.dll`;
- `build-gauss-check/lib/gauss.lib`;
- `build-gauss-check/lib/gauss.exp`.

`dumpbin` verification reports:

- PE machine type `14C` (x86), PE32 DLL;
- exports `gauss`, `ip_BuildKernel`, and `ip_ProcessKernel`;
- dependencies `KERNEL32.dll`, `VCRUNTIME140.dll`, and the Universal CRT
  runtime API set.
