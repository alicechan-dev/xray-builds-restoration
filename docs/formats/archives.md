# Archive Formats

This page is a research-oriented note for X-Ray archive formats relevant to the build 1935 restoration. It is documentation only and does not include proprietary archive samples.

The project currently focuses on source restoration. Archive research should support lawful compatibility testing, local modding workflows, and future safe tooling.

`tools/xr_unpack/` now contains an opt-in read-only CLI for proven `.xp*` directory metadata. It must not extract or write archive contents until payload handling is backed by reliable source-code evidence and synthetic tests.

## Known / Expected Archive Extensions

Known or expected archive families to investigate:

* `.xp*` files referenced by the current runtime setup notes, for example `gamedata.xp0`;
* archive variants handled by `xrFS/LocatorAPI.cpp`;
* archive data produced or consumed by `xrCompress/`;
* archive directory chunks listed by `tools/xrArchiveList/`.

The exact extension set, version range, and compatibility boundaries still need to be documented.

## Proven Current Layout

The current read-only parser is based on matching behavior in `xrCore/LocatorAPI.cpp`, `xrCompress/xrCompress.cpp`, and `tools/xrArchiveList/`.

Known structure for the supported `.xp*` directory metadata phase:

* top-level archive data is scanned as chunks;
* each chunk starts with little-endian `u32 type` and `u32 size`;
* bit `1 << 31` in `type` marks compressed chunk payload;
* chunk type `1` is the directory table used by `LocatorAPI::ProcessArchive`;
* compressed directory chunks are decompressed through the existing LZHUF helper;
* each directory entry is `stringZ path`, `u32 offset`, `u32 unpacked_size`, `u32 packed_size`;
* entries are registered under the archive base name plus the stored relative path at runtime.

`xr_unpack info`, `xr_unpack list`, and `xr_unpack verify` use only this metadata and do not read or extract file payloads.

## Unknowns To Investigate

Research tasks:

* identify archive header layout, magic values, and version fields;
* identify additional chunk framing variants beyond the proven directory chunk;
* identify filename encoding, separator behavior, and case rules;
* identify packed size, unpacked size, offsets, and alignment rules;
* identify file payload compression flags and algorithms;
* identify checksum, hash, or integrity behavior;
* identify duplicate entry behavior and archive priority rules;
* compare historical archive versions used by nearby X-Ray builds.

## Header / Directory / Compression Questions

### Header Layout

The existing archive listing helper treats archive content as typed chunks and looks for a directory chunk. Research should confirm:

* whether all supported archive variants use the same chunk framing;
* whether there are magic values, version fields, or archive-level flags;
* how chunk type and size fields are encoded and aligned;
* how malformed or truncated top-level chunks should be rejected.

### Directory Table

Proven for the current `.xp*` directory path: the directory chunk can be compressed with the top-level compression marker, and each entry stores a NUL-terminated path plus offset, unpacked size, and packed size fields.

Research should still confirm:

* whether entry records contain flags beyond name, offset, size, and compressed-size style fields;
* how file offsets relate to chunk boundaries;
* how duplicate virtual paths are resolved;
* how the runtime handles malformed or truncated records.

### Compression

Research should confirm:

* whether file payloads are individually compressed or stored through chunk-level compression;
* which entries use LZHUF, LZO, raw storage, or other algorithms;
* how compressed and uncompressed sizes are encoded;
* how decompression failures should be reported.

### Checksums / Hashes

Unknowns:

* whether archive-level or per-entry checksums exist;
* whether hashes are used for lookup, validation, or priority;
* whether verification can be implemented without extracting payloads.

### File Path Encoding

Unknowns:

* whether paths are ASCII, current ANSI codepage, UTF-8, or another historical encoding;
* whether archive paths are case-sensitive;
* whether `/` and `\` are equivalent in all historical archive families;
* how reserved device names and invalid Windows path characters should be handled.

## Historical Version Comparison

Compare build 1935-era behavior against nearby historical X-Ray builds only as research material. Any comparison should document structural differences without importing proprietary data.

Useful comparison points include:

* archive extension naming;
* directory table structure;
* compression marker behavior;
* checksum or hash availability;
* virtual path normalization rules;
* runtime archive priority.

## Synthetic Test Archive Requirement

Repository tests must use synthetic archives created specifically for this project. Synthetic fixtures should be small, documented, and free of proprietary content.

Do not commit real proprietary archives, extracted game assets, repacks, cracks, leaked runtime packages, leaked data, or gamedata dumps. Manual compatibility checks may use the user's legally obtained local data outside version control.

Synthetic tests should cover:

* empty archives;
* one-file archives;
* nested virtual paths;
* path traversal attempts;
* absolute and drive-qualified entry names;
* duplicate entries;
* truncated chunks;
* compressed and uncompressed directory data when the format is proven.

## Related Plans

* [Archive Unpacker Plan](../tools/unpacker-plan.md)
* [Tools and SDK Status](../tools/status.md)
