# Building

The 2571 candidate revision is not yet expected to compile with the new CMake scaffold. The current build layer is a safe foundation for later target-by-target restoration.

## Required Shape

- Windows host.
- Visual Studio 2022 or newer with C++ desktop workload.
- CMake 3.24 or newer.
- Win32/x86 generator architecture.

The original checked-in projects are Visual Studio 2003 `.vcproj` files and Visual C++ 6 `.dsp` files. They target `Win32`; no checked-in evidence currently supports switching the restoration baseline to x64.

## Configure

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
```

Optional dependency roots can be supplied as cache variables:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 ^
  -DXRAY_DIRECTX_SDK_ROOT="D:/Projects/Toolchains/DirectX" ^
  -DXRAY_BOOST_ROOT="D:/Projects/Toolchains/boost_1_33_1" ^
  -DXRAY_LUABIND_ROOT="D:/Projects/Toolchains/luabind-0.7" ^
  -DXRAY_LOKI_ROOT="D:/Projects/Toolchains/loki-legacy" ^
  -DXRAY_OPENAL_ROOT="D:/Projects/Toolchains/OpenAL" ^
  -DXRAY_EAX_ROOT="D:/Projects/Toolchains/EAX" ^
  -DXRAY_XIPH_ROOT="D:/Projects/Toolchains/Xiph-msvc"
```

## Current Build Check

```bat
cmake --build build --config Debug --target xray2571_build_order
```

This target prints the documented restoration order. It does not compile engine source yet.

## Future Target Rules

- Preserve original target names where practical.
- Keep runtime DLLs and executables in `build/bin`.
- Keep import/static libraries in `build/lib`.
- Keep debug symbols in `build/pdb`.
- Do not copy SDK binaries or game assets into the repository.
- Keep the old Visual Studio files available for archaeology and comparison.
