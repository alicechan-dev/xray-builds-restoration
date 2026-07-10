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

The stale `Log.h` and `Engine.h` PCH includes are removed. Neither header is in
the canonical VC6 project, neither exists in its source folder, and no compiled
Gauss source references their symbols.

The build now stops at the next VC6 library-internal dependency:
`_vector3d.h` calls the removed implementation detail `std::_cpp_max`.

Those following issues are intentionally left for separate narrow passes. No
`gauss.dll` output has been verified yet.
