# wxSDKEditor Read-Only Metadata Import

## Audited Source Format

Build 1935 reads configuration through `xrCore/Xr_ini.cpp`. The repository and
the locally inspected extracted data use `.ltx` files with relative
`#include "file.ltx"` directives, `[section]` headers, `key = value` records,
bare records with empty values, and semicolon comments. The historical reader
lowercases section names, treats duplicate sections as fatal, and retains
duplicate keys. Bare records are required by sections such as `[music]`.

No `[section]:parent` use was found in the build-1935 repository or inspected
metadata, and the historical reader does not resolve inheritance. This loader
warns about and ignores text after `]`; it does not invent inheritance
semantics.

Runtime `CInifile` was not linked because it depends on `IReader`, global `FS`,
shared strings, xrCore allocation, and runtime assertions. wxSDKEditor instead
uses a small wx-free read-only loader that follows the audited text structure
without typed reads, inheritance resolution, or write support.

## Model And Loader

`EditorImportedMetadata` stores raw sections and entries, source file/line
provenance, load order, include and byte counts, and diagnostics. It remains
separate from `EditorAssetDescriptor`.

`EditorMetadataLoader` accepts an explicit root and entry file and reads only
that include graph. Results replace the caller model only after complete
success. Defaults are 16 include levels, 256 files, 2 MiB per file, 16 MiB
total, and 16 KiB per line. Cycles, duplicate sections, malformed headers,
unreadable files, and exceeded limits fail. Empty keys and outside-section
records warn and are ignored; duplicate keys are retained with warnings.

## Imported Catalog Policy

Only sections with a `$spawn` record become descriptors. The preserved
LevelEditor uses the quote-stripped value as an editor label hierarchy and the
section as item identity. Exactly one safe quoted label plus a safe section
identity may therefore be placed as an inert synthetic Spawn marker. This does
not imply runtime construction semantics.

IDs use `imported.section.<lowercase-section-name>` and cannot replace synthetic
`demo.*` entries. Collisions are diagnosed and skipped. Malformed or ambiguous
`$spawn` sections remain browseable and non-placeable with a reason. Other
sections are counted as unsupported raw metadata. Descriptors retain source
file, section, line, and raw `$spawn` provenance.

Imported metadata belongs to the application session. New/Open/Save retain it,
Clear Imported Metadata is explicit, and restart does not reload it. Snapshots
still store only optional asset IDs, so unavailable imported IDs remain valid
unknown IDs. No level, mesh, texture, object library, archive, DLL, or script is
loaded, and no runtime file is written or normalized. See [Imported Spawn
Prototypes](wx-editor-imported-prototypes.md) and [Imported Prototype
Placement](wx-editor-imported-placement.md).
