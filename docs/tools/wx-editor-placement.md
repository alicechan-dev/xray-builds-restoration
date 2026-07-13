# wxSDKEditor Synthetic Placement

Object and light placement reuse the wx-free preview projection. For an
orthographic preview, screen center maps to camera X/Z and pixel offsets divide
by 40. Objects use Y=0; lights use Y=1. Optional 1.0-unit snapping rounds X/Z.
Invalid or zero-size viewports reject placement. The cursor marker and numeric
overlay are transient and never mutate the document.

Parent selection is deterministic: selected root/folder, otherwise the selected
object's parent, otherwise a group named `Objects`, finally the root. Object
nodes cannot become parents. Names use `new_object`, `new_object_1`, and so on,
or the equivalent `new_light` sequence. Both use `EditorItemKind::Object`; their
categories are `demo scene object` and `demo light` so preview mapping remains
explicit.

Each accepted click executes one `EditorModelCommand`, selects the new node,
refreshes tree/properties/preview, and participates in normal dirty/save-point,
undo, redo, and snapshot-v4 behavior. Cancellation and mode changes create no
command. No real SDK class, mesh, light, level, or asset is created.

Generic asset placement derives label base, category, item kind, default
transform, preview kind, and stable asset ID from one descriptor. Click X/Z and
optional snapping are combined with descriptor defaults, including Point
Light's Y=1. The old Object/Light tools route through matching built-ins while
keeping `new_object` and `new_light` naming. Undo/redo restores the complete
metadata and selection.

Audited imported `$spawn` prototypes reuse this exact command path. They place
only inert Spawn markers and persist the stable `imported.section.*` ID; no
runtime object factory or second placement system is involved. Clearing the
session catalog leaves existing markers and history intact. See [Imported
Prototype Placement](wx-editor-imported-placement.md).
