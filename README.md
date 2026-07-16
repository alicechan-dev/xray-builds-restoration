# X-Ray Build 2571 Candidate Restoration

Unofficial restoration workspace for a historical **X-Ray / S.T.A.L.K.E.R.: Shadow of Chernobyl** source revision that is being treated as the **2571 candidate revision**.

This repository currently preserves the original Visual Studio 2003/Visual C++ 6-era project layout while adding documentation and a minimal modern build scaffold for future restoration work.

This is not an official release and does not include original game assets.

## Current Status

- Branch: `restoration-2571`
- Source baseline: historical candidate revision for build 2571, not yet proven to be an exact build match.
- Original primary platform: Win32/x86, based on the checked-in Visual Studio project files.
- Modern build status: initial CMake configure scaffold only. No engine target is claimed to compile yet.
- Historical projects remain authoritative until each target is audited and restored.

## Restoration Principles

- Preserve original behavior unless a source-level fix is proven necessary.
- Keep restoration, modernization, portability, and renderer work separated.
- Do not remove the original `.sln`, `.vcproj`, `.dsp`, or `.dsw` files.
- Do not add proprietary SDKs, binary dependencies, or game assets to the repository.
- Prefer small compatibility shims and target-local build fixes over broad rewrites.
- Keep the first restored target set on Windows Win32/x86.
- Treat Linux and Vulkan work as later portability tracks, not part of the initial historical restoration.

## Documentation

- [Restoration plan](docs/RESTORATION_PLAN.md)
- [Initial audit](docs/AUDIT.md)
- [Building](docs/BUILDING.md)
- [Dependencies](docs/DEPENDENCIES.md)
- [Project structure](docs/PROJECT_STRUCTURE.md)
- [Known issues](docs/KNOWN_ISSUES.md)
- [Archaeology notes](docs/ARCHAEOLOGY_NOTES.md)
- [Portability plan](docs/PORTABILITY_PLAN.md)

## Initial Configure Check

From a Visual Studio developer prompt:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug --target xray2571_build_order
```

The current CMake layer is documentation-first. It validates the toolchain shape, creates stable output directories, exposes dependency cache variables, and records the proposed build order. It does not yet compile engine code.

## Legal / Assets Notice

This repository is for source restoration, build scripts, documentation, and compatibility research. Original game assets, proprietary data packages, and runtime content are not included.
