# Historical Object Body Decoders

## Conversion Consumer

The one-way historical converter consumes only typed inert decoder results.
Supported records become full conversions, Partial records keep safe fields
plus omission warnings, Unsupported records may become marked placeholders,
and Malformed records are skipped. Decoder bytes and retained unknown or
opaque chunks never enter editable snapshots.

`EditorHistoricalObjectBodyDecoder` is a wx-free, read-only dispatcher between
the confirmed generic wrapper and narrowly audited specialized records. It is
not a historical factory or plugin system.

The dispatcher maps class ID 1 to Glow, class ID 2 to Scene Object, class ID 3
to Light, and class ID 6 to Spawn Point. Every other ID returns `Unsupported`
and preserves the generic class/name/
transform manifest. Specialized failure returns `Malformed` on that object;
it does not erase the generic record or instantiate historical code.

Decode states are:

- `Supported`: all audited fields decoded and no unsupported child data;
- `Partial`: audited fields decoded while motion/legacy/unknown chunks remain
  bounded inventory;
- `Unsupported`: no class decoder exists;
- `Malformed`: the selected decoder found invalid required data or bounds.

The decoder accepts a retained bounded body, source provenance, and explicit
limits. Output is constructed locally and returned atomically. It never opens
references, calls old `Load`, follows paths, or writes bytes.

Current limits are 4 MiB per body, 64 child chunks, 4,096 bytes per string,
100 light fuzzy points, a 1 MiB spawn opaque-payload cap, and 32 retained
unknown or unsupported chunks. The production probe separately
caps retained bodies to 128 MiB per scene. Child sizes, duplicates, required
chunks, versions, and strings are validated before publication.

See [Historical Glow](wx-editor-historical-glow.md),
[Historical Scene Object](wx-editor-historical-scene-object.md),
[Historical Light](wx-editor-historical-light.md),
[Historical Spawn Point](wx-editor-historical-spawn-point.md), and
[Class Inventory](wx-editor-scene-class-inventory.md).
