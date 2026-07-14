# wxSDKEditor Historical Origin Integrity

Converted snapshot nodes own their historical provenance directly. There is no
separate path-indexed origin map: rename and reparent operations may change the
editable canonical path without moving provenance elsewhere. Historical fields
are immutable after conversion.

## Mutation policy

- Rename changes only the editable label. The source name is unchanged.
- Move changes only the editable hierarchy. Stable record ID, class, index,
  offset, and source scene stay unchanged.
- Transform and category edits change only editable state.
- Delete destroys the node and its provenance. Snapshot-backed undo restores
  both; redo removes both.
- New objects have no historical provenance.
- Duplication is not implemented. A future duplicate command must use a
  Derived Copy policy and must not reuse an editable identity.
- Detaching provenance is deferred until a single explicit undoable policy is
  designed.

`VerifyEditorHistoricalOrigins` is wx-free and runs after conversion, before
snapshot serialization, and after snapshot parsing but before document
replacement. It verifies unique paths and nonempty origin ownership, unique
nonempty stable IDs, bounded 16-bit class IDs and 32-bit indexes/offsets,
finite transforms and preview sizes, known decode/disposition values, bounded
strings, consistent placeholder state, and an allowlist of opaque-size summary
keys. Diagnostics are deterministic and capped at 32 messages.

Opaque bytes are never stored. The only accepted opaque summaries are decimal
sizes named `runtime_packet_bytes` and `attached_object_bytes`.

## Structural consequence

Stale metadata entries and missing-node references cannot be represented by
the current node-owned model. Deleting a subtree deletes its metadata by
ownership. This invariant is tested through delete/undo/redo rather than by
creating a second metadata registry.
