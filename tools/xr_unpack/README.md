# xr_unpack

`xr_unpack` is a restoration diagnostic and future unpacker scaffold for historical X-Ray archives.

The current tool performs read-only inspection of proven `.xp*` archive directory metadata. It intentionally does not extract file payloads yet.

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

Current command status:

* `info` prints archive size, directory chunk metadata, entry count, and parser status.
* `list` prints directory entries, offsets, packed sizes, unpacked sizes, and compression status.
* `verify` checks directory structure, entry bounds, duplicate names, and path-safety warnings without extracting payloads.
* `extract` is intentionally disabled and reports that extraction is not implemented yet.

The parser is based on existing repository evidence from `xrCore/LocatorAPI.cpp`, `xrCompress/xrCompress.cpp`, and `tools/xrArchiveList/`.

## Safety Rules

Extraction support must validate paths before writing files. The read-only `verify` command already applies the same archive-entry path checks to listed entries.

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

1. Add synthetic archive fixtures for the proven chunk/directory format.
2. Document payload compression and decompression behavior before reading file data.
3. Add extraction only after path safety and overwrite policy tests are in place.
