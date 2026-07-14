# wxSDKEditor Scene Inspector

**File > Inspect Historical Scene...** runs the bounded historical scene probe
for one explicitly selected `.level` file. On success, the dockable **Scene
Inspector** pane shows:

- format, version, file size, declared count, and confirmed object count;
- the source-proved chunk hierarchy with IDs, labels, offsets, sizes, and the
  compression marker;
- confirmed object class, name, source offset, chunk path, and transform;
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
