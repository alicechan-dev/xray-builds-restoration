# xr_unpack

`xr_unpack` is a restoration diagnostic and future unpacker scaffold for historical X-Ray archives.

The current tool performs inspection of proven `.xp*` archive directory metadata and extracts payloads only when explicitly requested with `--write`.

## Legal and Asset Policy

The unpacker is intended for lawful research, compatibility testing, and use with the user's own legally obtained game data.

Do not commit original game archives, extracted proprietary assets, repacks, cracks, leaked data, or gamedata dumps to this repository. Tests should use synthetic archives created specifically for this project.

## Commands

```bat
xr_unpack help
xr_unpack info <archive>
xr_unpack list <archive> [--limit N] [--filter PATTERN]
xr_unpack extract <archive> <out_dir> --dry-run [--limit N] [--filter PATTERN]
xr_unpack extract <archive> <out_dir> --write [--filter PATTERN]
xr_unpack verify <archive>
xr_unpack verify-extracted <archive> <out_dir> [--filter PATTERN]
```

Current command status:

* `info` prints archive size, directory chunk metadata, entry count, and parser status.
* `list` prints directory entries, offsets, packed sizes, unpacked sizes, and compression status. Use `--limit N` to print only the first `N` entries and `--filter PATTERN` to match archive paths with simple `*` and `?` wildcards.
* `verify` checks directory structure, entry bounds, duplicate names, and path-safety warnings without extracting payloads.
* `extract` refuses unless `--dry-run` or `--write` is passed.
* `extract --dry-run` validates planned output paths, duplicate outputs, existing target files, and entry bounds, then prints the paths it would write. It writes nothing. Use `--filter PATTERN` to plan only matching entries.
* `extract --write` runs the same safety plan, refuses unsafe paths, duplicate outputs, out-of-bounds entries, and existing output files, then writes files under `<out_dir>`. Use `--filter PATTERN` to extract only matching entries after the filtered plan passes.
* `verify-extracted` performs a read-only comparison of archive entries against an extracted output tree. It reports missing files and size mismatches and supports `--filter PATTERN`. Directory placeholders are counted and skipped; extra output files are not checked.

The parser is based on existing repository evidence from `xrCore/LocatorAPI.cpp`, `xrCompress/xrCompress.cpp`, and `tools/xrArchiveList/`.

## Safety Rules

Extraction support validates paths before writing files. The `verify` command, dry-run extraction planner, and `--write` mode apply the same archive-entry path checks to listed entries.

The current safety helpers are designed to:

* reject absolute archive entry paths;
* reject `..` parent-directory traversal;
* reject drive-letter paths such as `C:\...`;
* reject colons in archive entry path components;
* normalize `/` and `\` separators;
* compose output paths under the requested output directory;
* refuse overwriting existing files unless a future explicit flag allows it.
* remove a newly created partial output if payload reading or decompression fails.

Run `extract --dry-run` before `extract --write`, especially when inspecting a new archive. Do not run extraction into the repository root or commit extracted proprietary files.

After extraction, run `verify-extracted` against the output directory. The
command writes nothing and reports `status: ok` only when every matched safe
file exists with the expected unpacked size.

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
2. Add synthetic archive fixtures for stored and compressed payload extraction.
3. Add synthetic coverage for selected-file extraction.
4. Add synthetic coverage for extracted-tree verification.
5. Add an explicit `--overwrite` policy only if it becomes necessary.
