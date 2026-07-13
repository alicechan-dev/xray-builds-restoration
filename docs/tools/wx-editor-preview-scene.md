# wxSDKEditor Preview Scene

The experimental editor now derives a tiny wx-free preview scene from
`EditorTreeModel`. This validates model-to-viewport flow without loading X-Ray
levels, meshes, textures, transforms, or renderer DLLs.

## Derived Scene

`EditorPreviewScene` stores a flat vector of visible preview objects identified
by canonical logical model path. Each object has a label, synthetic position
and size, selection flag, visibility flag, and one preview kind: box, marker,
light, spawn, or unknown. It is not an ECS or scene graph.

`EditorTreePreviewAdapter` rebuilds the scene from stable model pre-order:

* root and folder nodes are excluded;
* object nodes become boxes;
* categories containing `light` become elevated light markers;
* categories containing `spawn` become spawn markers;
* other non-structural nodes become markers;
* X positions advance across a five-column grid and Z advances by row;
* logical paths provide identity and selection matching.

Positions are deterministic placeholders because the neutral model has no real
SDK transforms. Preview state is never serialized and snapshot v2 is unchanged.
A complete rebuild follows selection refresh and all presenter model rebuilds,
including add, rename, move, delete, undo/redo, load, and import.

## Draw List And Software Preview

`EditorPreviewRenderer` implements `IEditorViewportRenderer` but remains
wx-free. It projects world X/Z around the placeholder camera into screen space
and produces `EditorViewportDrawList` line, rectangle, circle, and text
primitives with semantic styles. Selected objects receive a separate highlight
rectangle. Labels can be disabled.

`wxEditorViewport` translates those semantic primitives to `wxDC` colors and
keeps the existing grid and diagnostic overlay. No graphics API or historical
renderer is connected.

View actions rebuild the derived scene, toggle preview labels, and frame the
selected renderable object by centering the placeholder camera on its synthetic
X/Z position. A folder or empty selection reports a non-fatal status.

Headless tests cover scene add/remove/find/selection/visibility, folder
exclusion, category mapping, deterministic order and placement, path updates
after rename/move/delete, selected highlights, label toggling, empty/zero-size
draw lists, and frame-selected camera behavior. Tests-only builds remain
wxWidgets-free.

Future work may implement another `IEditorViewportRenderer` adapter, but real
assets, picking, gizmos, DirectX/OpenGL/Vulkan, LevelEditor, `xrEProps`, and
runtime renderer targets remain explicitly outside this pass.

The same projection now produces wx-free pick shapes. Left-click selection uses
canonical paths and reverse draw order, while empty space clears selection. See
[wxSDKEditor Preview Picking](wx-editor-picking.md).

Preview positions now come from node-owned transforms rather than traversal
indices. Demo positions are explicit, and selected objects emit semantic X/Z
gizmo lines. See [Transforms](wx-editor-transforms.md) and
[Move Gizmo](wx-editor-gizmo.md).
