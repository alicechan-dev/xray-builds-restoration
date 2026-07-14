# wxSDKEditor Historical Scene Preview

`EditorHistoricalScenePreviewAdapter` builds an inert `EditorPreviewScene`
from a loaded historical document. Only records with a confirmed, finite
common transform are emitted. Records without a transform are retained in the
tree and properties but omitted from the viewport.

Preview objects use a generic box. No class-specific visual, mesh, texture,
shader, sound, object library, or runtime asset is resolved. Position maps
directly to X/Y/Z; the three historical rotation components and scale are
preserved in document provenance and read-only properties.

The preview logical identity is the stable historical record ID, not its
possibly duplicated name or tree path. Picking therefore maps duplicate-named
objects back to distinct records, selects the matching tree item, refreshes
properties, and highlights the correct preview object. Camera movement, grid,
labels, picking, and Frame Selected remain available. Move gizmo and placement
operations remain disabled.
Compression does not change preview policy: only confirmed finite transforms
produce generic markers. Decoded records do not trigger asset loading or
class-specific historical rendering.

