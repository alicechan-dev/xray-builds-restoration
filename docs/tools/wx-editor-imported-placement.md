# wxSDKEditor Imported Prototype Placement

Audited imported `$spawn` descriptors may use the existing generic
`PlaceAsset` command path. Placement creates an inert synthetic Spawn marker
with the descriptor's stable ID, sanitized section-derived name, `imported
spawn` category, object item kind, and clicked transform. Repeated names use
the normal deterministic suffix policy.

Each accepted placement is one normal `EditorModelCommand`. Undo, redo,
selection, dirty-state tracking, and snapshot v4 therefore require no special
imported-object command or persistence format. Cancellation creates nothing.
Snapshot v4 continues to store only the asset ID and node data, never imported
metadata contents or absolute metadata roots.

Imported catalogs are application-session state. Clearing the catalog cancels
active imported placement and removes current provenance resolution, but it
does not remove placed markers, alter history, or change document dirtiness.
Placed markers continue to load and render as Spawn previews by their
`imported.section.*` identity. Properties report the prototype section and
`unresolved` until the matching catalog is loaded again.

This bridge does not create a real X-Ray object. Runtime class resolution,
visuals, scripts, object-library integration, and level serialization remain
future work requiring separate historical evidence.

The historical Scene Inspector does not feed this placement path. It exposes
only a session-local manifest and cannot place, import, or serialize objects.
