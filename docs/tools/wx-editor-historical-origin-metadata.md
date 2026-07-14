# Historical Origin Metadata

Converted editable nodes may own one small `EditorHistoricalOriginMetadata`
record. It contains source scene version, class ID, object index, source name,
source offset, stable historical ID, decode status, conversion disposition,
inert reference and field summaries, warning summary, opaque size/status
summary, transform-confirmed state, placeholder state, and optional diagnostic
preview size.

Origin fields are read-only in Properties. Label, category, and editable
transform fields remain ordinary editable node properties. The metadata stores
no absolute `.level` path, pointer, callback, packet bytes, attachment bytes,
unknown chunks, object body, visual, mesh, texture, sound, shader, or script.

Snapshot v5 serializes the bounded metadata explicitly on each node. Readers
for snapshot v1 through v4 remain supported; older snapshots simply have no
origin metadata. Parsing v5 is atomic and rejects malformed numbers, enums,
booleans, non-finite preview sizes, trailing fields, or malformed quoting.
Future unknown origin syntax therefore fails safely instead of being guessed.
