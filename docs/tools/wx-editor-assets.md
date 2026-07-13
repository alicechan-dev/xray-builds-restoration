# wxSDKEditor Synthetic Asset Catalog

The experimental editor now has a wx-free asset seam under
`Editors/wxSDKEditor/include/editor_assets`. `EditorAssetDescriptor` carries a
stable ID, display text, category path, node metadata, default transform,
preview kind, placement type, and a placeable flag. `EditorAssetCatalog` owns
the descriptors and provides deterministic enumeration, unique-ID lookup,
category filtering, and case-insensitive search over ID, name, and category.

The built-in catalog is deliberately synthetic:

| ID | Display name | Category | Preview |
| --- | --- | --- | --- |
| `demo.actor` | Actor Marker | Objects | Box |
| `demo.physic_object` | Physics Object | Objects | Box |
| `demo.level_changer` | Level Changer | Logic | Marker |
| `demo.point_light` | Point Light | Lights | Light, default Y 1 |
| `demo.ambient_sound` | Ambient Sound | Sounds | Marker |
| `demo.spawn` | Spawn Element | Spawn | Spawn |

These entries are editor-development prototypes, not historical X-Ray SDK
class definitions. No gamedata, archives, meshes, textures, or object-library
files are scanned or loaded.

`EditorAssetSelectionModel` stores only the selected stable ID and resolves it
through the catalog. Unknown or non-placeable IDs are rejected and clearing the
selection is explicit. The implementation has no wxWidgets dependency and is
compiled into both `wxSDKEditor` and `wxSDKEditorModelTests`.

Placed nodes persist the optional ID in snapshot v4. Snapshot v1/v2/v3 files
remain readable with empty IDs. Unknown IDs are retained during load/save and
use normal category fallback in the preview, so opening a document never
requires the current catalog to contain every prototype.

A separate session catalog can now be populated from explicitly selected,
read-only LTX metadata. Only `$spawn` sections are conservatively adapted under
the `imported.section.*` namespace, and all remain non-placeable. See
[Read-Only Metadata Import](wx-editor-metadata-import.md).
