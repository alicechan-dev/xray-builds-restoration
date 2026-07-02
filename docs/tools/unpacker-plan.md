# Archive Unpacker Plan

This document outlines the technical plan for the `xr_unpack` X-Ray archive unpacker scaffold in `tools/xr_unpack/`.

The current implementation is a safe inspection and extraction tool. It parses proven `.xp*` directory metadata, can dry-run planned output paths, and writes files only when the explicit `extract --write` mode is used.

## Goals

The unpacker should support:

* reporting help and clear unsupported-format errors;
* listing archive contents;
* extracting selected files;
* extracting all files;
* verifying archive integrity when the archive format provides enough metadata;
* reporting archive metadata useful for compatibility testing.

## Non-Goals

The unpacker must not:

* add proprietary assets to the repository;
* bypass DRM or access controls;
* support repacks, cracks, or modified proprietary runtime package distribution;
* become a general-purpose repacking workflow before the legal and technical boundaries are documented.

## Legal / Asset Policy

The unpacker is intended for lawful research, compatibility testing, and use with the user's own legally obtained game data. The repository must not distribute original game archives, extracted proprietary assets, repacks, cracks, leaked data, or gamedata dumps.

Synthetic test inputs may be committed when they are created specifically for this repository and contain no proprietary content.

No proprietary samples belong in `tools/xr_unpack/`, `docs/`, tests, or any other repository path.

## Supported Archive Families To Investigate

Initial research should focus on historical X-Ray archives referenced by the build 1935 runtime and tools:

* `.xp*` archive families used by historical runtime data;
* archives handled by `xrFS/LocatorAPI.cpp`;
* archives produced or consumed by `xrCompress/`;
* directory chunks compatible with the existing `tools/xrArchiveList/` diagnostic;
* any variant required by build 1935-era runtime compatibility testing.

The first supported phase is based on repository evidence from `xrCore/LocatorAPI.cpp`, `xrCompress/xrCompress.cpp`, and `tools/xrArchiveList/`: top-level chunks contain a directory chunk with entry records of `stringZ path`, `u32 offset`, `u32 unpacked size`, and `u32 packed size`. Extraction support should be added only after payload compression and overwrite behavior are documented well enough to extract safely.

## CLI Design

Proposed command examples:

```bat
xr_unpack help
xr_unpack info <archive>
xr_unpack list <archive> [--limit N]
xr_unpack extract <archive> <out_dir> --dry-run [--limit N]
xr_unpack extract <archive> <out_dir> --write
xr_unpack verify <archive>
```

Suggested command behavior:

* `help` prints usage and current limitations.
* `info`, `list`, and `verify` perform read-only directory inspection.
* `list` prints archive entry names, offsets, packed sizes, unpacked sizes, and compression status without writing files.
* `list --limit N` prints only the first `N` entries.
* `extract` writes files under `<out_dir>` after safety validation.
* `extract` refuses unless `--dry-run` or `--write` is passed.
* `extract --dry-run` validates all planned output paths, duplicate outputs, existing files, and entry bounds, then prints what would be written. It writes nothing.
* `extract --write` runs the same plan and writes files only if there are no unsafe entries, duplicate outputs, existing outputs, or out-of-bounds entries.
* `verify` checks structure, directory entries, bounds, duplicate names, and path-safety warnings when known.
* `info` reports archive family, directory chunk status, compression flags, entry count, and known limitations.

## Format Research Checklist

Before extraction implementation, document:

* archive header layout and magic/version fields, if any;
* any additional chunk types beyond the proven directory chunk;
* entry name encoding and path separator conventions;
* packed and unpacked size fields;
* file offset rules and alignment requirements;
* payload compression flags and algorithms, including LZHUF/LZO use where applicable;
* checksum, hash, or integrity behavior;
* behavior differences between historical archive versions;
* how the runtime resolves duplicate names or archive priority.

## Safety Requirements

The implementation must:

* reject absolute paths from archive entries;
* reject `..` path traversal;
* reject drive-letter paths such as `C:\...`;
* reject colons in archive entry path components;
* normalize `/` and `\` separators;
* ensure composed output paths remain under the selected output directory;
* avoid overwriting files unless a future explicit flag allows it;
* preserve directory structure for accepted entries;
* handle path encoding carefully;
* never write outside the output directory;
* use synthetic test archives only in the repository.

Extraction must normalize and validate output paths before writing. Archive entries using absolute paths, drive-qualified paths, parent-directory escapes, alternate separators, reserved device names, or ambiguous encodings should be rejected or safely remapped according to a documented policy.

Default overwrite behavior is conservative. Extraction refuses existing outputs unless a future explicit option such as `--overwrite` is added, and dry-run output shows exactly which files would be written.

## Testing Strategy

Testing should use synthetic test archives created specifically for this repository.

Required test categories:

* empty archive;
* archive with one file;
* nested directory structure;
* filtered extraction;
* compressed and uncompressed directory entries when format support exists;
* corrupted header or truncated chunk;
* duplicate entry names;
* path traversal attempts;
* overwrite policy cases;
* path encoding edge cases.

No real proprietary archives, extracted game assets, repacks, cracks, leaked runtime packages, or gamedata dumps should be committed. Compatibility checks against legally obtained local archives can be documented as manual tests without adding those inputs to version control.

## Integration With Modding Workflow

The unpacker should support local workflows where a user:

* obtains lawful compatible game/runtime data separately;
* lists archives to understand available files;
* extracts selected configs, scripts, or assets into a local workspace;
* keeps modified files in a separate mod directory;
* runs the restored engine with documented runtime layout rules;
* avoids committing proprietary assets or extracted gamedata.

The unpacker should produce predictable paths that match the runtime's virtual filesystem where possible, so tutorials can explain how local overrides relate to archive entries.

## Milestones

1. Document archive structures from `xrFS`, `xrCompress`, and the existing archive listing helper.
2. Keep the `tools/xr_unpack/` CLI skeleton buildable behind `BUILD_XR_UNPACK`.
3. Create synthetic archive fixtures with no proprietary content.
4. Extend the read-only listing behavior into `xr_unpack list`. Done for proven directory metadata.
5. Add `info` and `verify` behavior for structural validation. Done for proven directory metadata.
6. Add synthetic fixtures for the directory parser.
7. Add dry-run extraction planning. Done for full-archive plans with optional output limits.
8. Add explicit full extraction with `--write`, shared safety preflight, no overwrite by default, and proven runtime decompression. Done.
9. Add filters such as `--filter "*.ltx"` for listing and dry-run planning.
10. Add safe selected-file extraction with filters and dry-run support.
11. Add an explicit overwrite policy only if needed.
12. Document modding workflow examples using only lawful local data and synthetic repository fixtures.
