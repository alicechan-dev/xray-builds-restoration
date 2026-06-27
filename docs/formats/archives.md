# Archive Formats

This page is a research-oriented note for X-Ray archive formats relevant to the build 1935 restoration. It is documentation only and does not include proprietary archive samples.

The project currently focuses on source restoration. Archive research should support lawful compatibility testing, local modding workflows, and future safe tooling.

## Known / Expected Archive Extensions

Known or expected archive families to investigate:

* `.xp*` files referenced by the current runtime setup notes, for example `gamedata.xp0`;
* archive variants handled by `xrFS/LocatorAPI.cpp`;
* archive data produced or consumed by `xrCompress/`;
* archive directory chunks listed by `tools/xrArchiveList/`.

The exact extension set, version range, and compatibility boundaries still need to be documented.

## Unknowns To Investigate

Research tasks:

* identify archive header layout, magic values, and version fields;
* identify chunk framing and directory table layout;
* identify filename encoding, separator behavior, and case rules;
* identify packed size, unpacked size, offsets, and alignment rules;
* identify compression flags and algorithms;
* identify checksum, hash, or integrity behavior;
* identify duplicate entry behavior and archive priority rules;
* compare historical archive versions used by nearby X-Ray builds.

## Header / Directory / Compression Questions

The existing archive listing helper treats archive content as typed chunks and looks for a directory chunk. Research should confirm:

* whether all supported archive variants use the same chunk framing;
* whether the directory chunk can be compressed;
* whether file payloads are individually compressed or stored through chunk-level compression;
* whether entry records contain flags beyond name, offset, size, and compressed-size style fields;
* how the runtime handles malformed or truncated records.

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

## Related Plans

* [Archive Unpacker Plan](../tools/unpacker-plan.md)
* [Tools and SDK Status](../tools/status.md)
