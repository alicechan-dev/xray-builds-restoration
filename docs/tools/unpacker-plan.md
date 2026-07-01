# Archive Unpacker Plan

This document outlines the technical plan for the `xr_unpack` X-Ray archive unpacker scaffold in `tools/xr_unpack/`.

The current implementation is intentionally a safe CLI skeleton only. Archive parsing and extraction are not implemented yet; format research is required before the tool may read payloads or write extracted files.

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

Support should be added only after the format is documented well enough to extract safely.

## CLI Design

Proposed command examples:

```bat
xr_unpack help
xr_unpack info <archive>
xr_unpack list <archive>
xr_unpack extract <archive> <out_dir>
xr_unpack verify <archive>
```

Suggested command behavior:

* `help` prints usage and current limitations.
* `info`, `list`, `extract`, and `verify` currently fail with `archive parsing is not implemented yet; format research is required`.
* `list` prints archive entry names and optional metadata without writing files.
* `extract` writes files under `<out_dir>` after safety validation.
* `verify` checks structure, directory entries, sizes, compression metadata, and hashes/checksums when known.
* `info` reports archive family, header fields, directory chunk status, compression flags, and known limitations.

## Format Research Checklist

Before implementation, document:

* archive header layout and magic/version fields, if any;
* chunk structure and directory table layout;
* entry name encoding and path separator conventions;
* packed and unpacked size fields;
* file offset rules and alignment requirements;
* compression flags and algorithms, including LZHUF/LZO use where applicable;
* checksum, hash, or integrity behavior;
* behavior differences between historical archive versions;
* how the runtime resolves duplicate names or archive priority.

## Safety Requirements

The implementation must:

* reject absolute paths from archive entries;
* reject `..` path traversal;
* reject drive-letter paths such as `C:\...`;
* normalize `/` and `\` separators;
* ensure composed output paths remain under the selected output directory;
* avoid overwriting files unless a future explicit flag allows it;
* preserve directory structure for accepted entries;
* handle path encoding carefully;
* never write outside the output directory;
* use synthetic test archives only in the repository.

Extraction must normalize and validate output paths before writing. Archive entries using absolute paths, drive-qualified paths, parent-directory escapes, alternate separators, reserved device names, or ambiguous encodings should be rejected or safely remapped according to a documented policy.

Default overwrite behavior should be conservative. A future implementation should prefer no overwrite unless the user passes an explicit option such as `--overwrite`, and dry-run output should show exactly which files would be written.

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
4. Extend the read-only listing behavior into `xr_unpack list`.
5. Add `info` and `verify` behavior for structural validation.
6. Add safe selected-file extraction with filters and dry-run support.
7. Add full extraction only after path safety, overwrite policy, and integrity tests are stable.
8. Document modding workflow examples using only lawful local data and synthetic repository fixtures.
