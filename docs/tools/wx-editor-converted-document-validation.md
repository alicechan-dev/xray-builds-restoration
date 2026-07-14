# wxSDKEditor Converted Document Validation

The stabilization suite exercises converted snapshots without wxWidgets or
real game assets.

Coverage includes immutable provenance through rename, move, transform,
category edit, delete, undo, and redo; provenance-free synthetic nodes;
origin-first preview identity; property section/read-only behavior; and
statistics/summary availability. Snapshot tests cover malformed required
fields, duplicate IDs, invalid class/disposition values, oversized summaries,
forbidden opaque markers, bounded future `x_*` fields, atomic failure, v1-v4
compatibility, and deterministic v5 round trips.

A synthetic 20,000-node converted model is verified, serialized, reloaded,
looked up by a tail canonical path, and rebuilt into a preview scene. Integrity
verification is deliberately kept out of routine per-frame selection and
rendering paths.

The snapshot file cap is 32 MiB. Individual origin summaries remain bounded to
1,024 bytes, opaque-size summaries to 512 bytes, source filenames to 255 bytes,
and optional extension values to 512 bytes.
