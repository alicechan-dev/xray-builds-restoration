# xr_unpack

`xr_unpack` is a restoration diagnostic and future unpacker scaffold for historical X-Ray archives.

The current tool intentionally does not parse or extract archive contents yet. Its first purpose is to provide a clean CLI shape, CMake target, and path-safety helpers before archive format support is implemented.

## Legal and Asset Policy

The unpacker is intended for lawful research, compatibility testing, and use with the user's own legally obtained game data.

Do not commit original game archives, extracted proprietary assets, repacks, cracks, leaked data, or gamedata dumps to this repository. Tests should use synthetic archives created specifically for this project.

## Commands

```bat
xr_unpack help
xr_unpack info <archive>
xr_unpack list <archive>
xr_unpack extract <archive> <out_dir>
xr_unpack verify <archive>
```

Until format support is researched and implemented, archive commands fail with:

```text
archive parsing is not implemented yet; format research is required
```

## Safety Rules

Extraction support must validate paths before writing files.

The current safety helpers are designed to:

* reject absolute archive entry paths;
* reject `..` parent-directory traversal;
* reject drive-letter paths such as `C:\...`;
* normalize `/` and `\` separators;
* compose output paths under the requested output directory;
* refuse overwriting existing files unless a future explicit flag allows it.

## Build

Configure with the tool enabled:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 -DBUILD_XR_UNPACK=ON
```

Build only the tool:

```bat
cmake --build build --config Debug --target xr_unpack -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Next Steps

1. Document the `.xp*` archive layout from proven source code and synthetic fixtures.
2. Add read-only `info`, `list`, and `verify` implementations.
3. Add extraction only after path safety and overwrite policy tests are in place.
