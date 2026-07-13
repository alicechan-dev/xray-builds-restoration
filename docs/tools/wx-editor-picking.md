# wxSDKEditor Preview Picking

The synthetic preview supports wx-free screen-space picking. This is not X-Ray
ray casting, mesh intersection, depth-buffer selection, or world editing.

`EditorPreviewPicking` owns the shared projection context used by both
`EditorPreviewRenderer` and hit testing. World X/Z is translated around the
placeholder camera, scaled by 40 pixels, and centered in the current viewport.
Zero-size and clipped objects produce no pick shapes.

Pick shapes mirror the visible preview primitives:

* boxes use their 28 by 20 pixel projected rectangle;
* lights use their 9 pixel circle;
* markers and spawn objects use an 18 pixel square plus 4 pixel tolerance;
* invisible and non-selectable objects are ignored.

Shapes preserve scene draw order. Hit testing visits them in reverse, so the
last-drawn visually topmost object wins deterministic overlaps. Results carry
the canonical logical path, preview kind, and click distance.

A left-button press asks the wx-free controller for a pick. Right-button camera
state and invalid viewport dimensions suppress picking. The wx control forwards
the resulting path to `EditorTreePresenter::SelectLogicalPath`, which resolves
the current model node, selects its tree item, refreshes properties, and rebuilds
the preview highlight. Empty space clears tree selection and properties.
Selection changes do not modify `EditorDocument` and do not enter command
history.

Pick shapes are rebuilt immediately whenever the derived preview scene changes
and during rendering, so resize, camera movement, add/rename/move/delete,
undo/redo, load, import, category changes, and label toggles cannot retain stale
logical paths.

Headless tests cover projection, resize/camera offsets, zero-size safety,
box/light/marker hits and misses, tolerance, ignored shapes, reverse-order
overlap, controller suppression, and presenter tree/property/dirty/history
coordination. Tests-only builds remain wxWidgets-free.

Real levels, assets, renderer targets, picking rays, depth, lasso selection,
gizmos, `xrEProps`, and LevelEditor remain outside this boundary.

The move gizmo now has priority over object picking. Outside an active drag or
axis hit, object and empty-space behavior is unchanged.
