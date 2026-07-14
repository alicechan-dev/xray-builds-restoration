# wxSDKEditor Scene Inspector

**File > Inspect Historical Scene...** runs the bounded historical scene probe
for one explicitly selected `.level` file. On success, the dockable **Scene
Inspector** pane shows:

- format, version, file size, declared count, and confirmed object count;
- the source-proved chunk hierarchy with IDs, labels, offsets, sizes, and the
  compression marker;
- confirmed object class, historical type/decode status, name, source offset,
  chunk path, transform, and supported specialized fields;
- retained warnings and format diagnostics.

The pane starts hidden and can be toggled from **View > Scene Inspector**. Its
data is application-session state. It is not serialized, does not participate
in undo/redo, and never marks the current document modified. A failed
inspection reports the precise reason and preserves the previously displayed
manifest.

This is an inspector, not a historical level importer. It does not populate
the development scene tree or viewport, construct objects, load assets, or
write `.level` files. The existing snapshot and imported-metadata workflows
remain separate.

The inspector command remains distinct from **Open Historical Scene
Read-Only...**. The latter builds a separate inert tree/preview document from
confirmed records, while this pane continues to display its own session
manifest and never edits either document.
## Compression details

The read-only inspector summary reports compressed chunks, successful decodes,
failures, compressed/decompressed byte totals, and algorithm. Chunk details
show sizes, success, diagnostics, and decoded-source provenance. It never
exports or writes decompressed payloads.

For decoded class-2 records, object details show body version, inert reference
metadata, flags, unknown/unsupported child counts, and decoder diagnostics.
All values remain read-only; the inspector never resolves the reference.

For decoded class-1 glows, details show version, shader and texture names,
radius, optional flags, field source offsets, retained chunk counts, and
diagnostics. The names are not resolved and no rendering resource is created.

For decoded class-3 lights, details show type, color, brightness, range,
attenuation, cone, virtual size, use flag, raw flags/control, fuzzy summary,
parameter source offset, retained chunks, and diagnostics. Animation and
falloff names are inert; no light or resource is created.

For decoded class-6 Spawn Points, details show subtype, inert entity section,
opaque packet/attachment sizes, safe respawn/environment values, provenance,
retained chunks, and diagnostics. Packet bytes are never displayed or parsed.
