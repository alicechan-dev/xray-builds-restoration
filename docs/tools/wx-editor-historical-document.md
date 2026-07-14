# wxSDKEditor Historical Scene Document

`EditorHistoricalSceneDocument` is the first direct, inert transplant of
confirmed build-1935 LevelEditor scene records into the modern wxSDKEditor
model boundary. It owns an accepted `EditorSceneManifest`, a separate
`EditorTreeModel`, separate selection state, and immutable provenance records.
It has no command history and always reports read-only and unmodified.

The converted tree is:

```text
Historical Scene
`-- Objects
    |-- source_name
    |-- source_name [#2]
    `-- unnamed_object_<record-index>
```

Only manifest records with confirmed object wrappers and class IDs become
objects. Unknown chunks never become nodes. Source names remain exact in
provenance; unsafe tree path separators/control bytes are replaced only in the
display label. Duplicate labels receive a display-only `[#N]` suffix.

Stable identity is `historical.object.<record-index>.<source-offset>`. It does
not depend on display or source name. Each provenance record retains class ID,
source name, chunk path, offset, scene version, confirmed transform state, and
specialized decode result. Class-2 records also retain inert reference,
version, flags, and field provenance. No raw manifest-buffer pointers are kept.
Class-1 records retain inert shader/texture names, radius, flags, version, and
field provenance.

Opening is atomic: probe and conversion failure leave the active editable or
historical document unchanged. The document never saves, rewrites, normalizes,
or exports `.level` data and never constructs historical classes or follows
asset references.
Confirmed records discovered inside successfully decoded LZHUF containers are
converted through the same inert read-only path. Their identity and properties
retain the physical compressed-container offset, decoded-stream offset, and
chunk path. Failed compressed containers create no objects.

The first specialized transplant supports `CSceneObject`: 15,412 sampled
records decode fully and four motion-bearing records decode partially. Generic
objects and malformed specialized bodies remain selectable and readable.
The second transplant supports all 511 sampled `CGlow` bodies. Referenced
shader and texture names are never opened; the source-confirmed radius drives
only a diagnostic preview ring.
