# Build 1935 LevelEditor Scene Format Audit

This audit records only structures proved by the active historical source
under `Editors/LevelEditor` and `Editors/ECore`. It does not describe a full
scene loader and does not infer layouts from chunk IDs alone.

## Canonical File And Chunk Framing

The active `Editors/LevelEditor/Edit/SceneIO.cpp` identifies scene version 5
and writes `.level` files as X-Ray chunks. `xrCore/FS.cpp` and `FS.h` define a
chunk as little-endian `u32 id`, little-endian `u32 payload size`, then exactly
that many payload bytes. Bit 31 of the ID is the compression marker.

The confirmed top-level scene chunks are:

| ID | Meaning | Evidence |
|---|---|---|
| `0x00009DF3` | Scene version (`u32`, expected 5) | `SceneIO.cpp` |
| `0x00007712` | Declared object count (`u32`) | `SceneIO.cpp` |
| `0x00007708` | Legacy object-list container | compatibility load path in `SceneIO.cpp` |
| `0x00008000 + class` | Per-tool data | canonical save path in `SceneIO.cpp` |

`SceneClassList.h` proves object tool classes 0 through 11. Classes 12 through
14 are specialized tool data and are inventoried but not decoded by the probe.

## Confirmed Object Records

`ESceneCustomOToolsIO.cpp` proves that an object-tool payload contains chunk
`0x00000002` for its object count and chunk `0x00000003` for numbered object
records. `SceneIO.cpp::SaveObject` proves that each record contains:

- `0x00007703`: object class as one `u32`;
- `0x00007777`: object body container.

`CustomObject.cpp` proves two common body fields used by the first manifest
reader:

- `0x0000F907`: zero-terminated object name;
- `0x0000F903`: position, rotation, and scale as nine 32-bit floats.

All other common chunks remain inventory records. A separate audited
dispatcher now decodes class ID 1 (`CGlow`) and class ID 2 (`CSceneObject`)
specialized fields;
unknown classes remain generic. It never resolves a class factory or loads a
referenced asset.

## Evidence Ladder

1. Top-level chunk inventory, version, and declared count.
2. Confirmed object wrappers and class IDs in the legacy list or object-tool
   classes 0 through 11.
3. Common object names.
4. Common transforms.

5. Source-audited class-2 body version, inert reference, and flags.

Every further class still requires a separate source audit. The current probe
does not support historical save/write behavior or speculative body layouts.

The read-only historical document consumes levels 2 through 5. Classes 1 and 2
currently reaches level 5; every other specialized body remains generic. See
[Historical Scene Document](wx-editor-historical-document.md).
## Confirmed compression

Bit 31 is `CFS_CompressMark`. Historical `IWriter::w_chunk` and
`IReader::open_chunk` route marked payloads through `_compressLZ` and
`_decompressLZ` in `xrCore/LzHuf.cpp`. The payload is a little-endian `u32`
output size followed by X-Ray LZHUF (LZSS plus adaptive Huffman), not LZO.
See [wx-editor-scene-compression.md](wx-editor-scene-compression.md).

The real-scene class frequencies, source mapping, and candidate ranking are in
[Historical Scene Class Inventory](wx-editor-scene-class-inventory.md).
The third specialized audit confirms class 3 `CLight` versions `0x0010` and
`0x0011`, current and legacy parameter layouts, and bounded fuzzy placement.
See [Historical Light](wx-editor-historical-light.md).
Class 6 `CSpawnPoint` adds version `0x0014`, dependency-free editor subtype
records, inert section strings, and length-validated opaque packet/attachment
metadata. See [Historical Spawn Point](wx-editor-historical-spawn-point.md).
