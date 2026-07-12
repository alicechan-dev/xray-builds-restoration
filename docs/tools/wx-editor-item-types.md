# wx Editor Item Types

## Historical Audit

The preserved generic list boundary is smaller than its name suggests:

* `Editors/xrEProps/xrEProps.h` defines `EItemType` as `TYPE_INVALID = -1`,
  `TYPE_FOLDER = 0`, and `TYPE_OBJECT = 1`;
* `ItemListTypes.h` stores a caller-provided integer `type`, key, opaque object
  pointer, tag, icon index, color, callbacks, and UI flags;
* `ListItem::type` is not a closed global SDK enum. ParticleEditor and
  ActorEditor pass their own edit-mode integers;
* `flShowCB`, `flCBChecked`, `flDrawThumbnail`, `flDrawCanvas`, and `flSorted`
  describe list/widget presentation, not portable model identity;
* `ItemList` adds selection, drag, menu, icon, form-storage, drawing, and
  callback behavior through VCL, ElPack, and AlexMX.

Only the folder/object distinction is therefore mapped as historical model
semantics. Opaque caller mode integers, object pointers, callbacks, colors,
icons, checkboxes, thumbnails, canvas drawing, sorting, selection, drag/drop,
and form persistence remain intentionally unmapped.

No preserved `xrEProps` source or header is included or modified by the wx
model path.

## Neutral Kind

`EditorItemKind` is wx-free and contains:

| Kind | Meaning |
|---|---|
| `Unknown` | No proven mapping; preserves custom category data without guessing. |
| `Root` | Structural root introduced by the neutral model. |
| `Folder` | Audited counterpart of historical `TYPE_FOLDER`. |
| `Object` | Audited counterpart of historical `TYPE_OBJECT`. |

`ToString`, `ParseEditorItemKind`, `IsGroupKind`, and `IsLeafKind` provide the
small value contract. `Root` and `Folder` are group kinds; `Object` is the only
leaf kind. `Unknown` is deliberately neither.

`EditorTreeNode` stores the kind alongside its existing display category. Demo
lights, sounds, scene objects, and other leaves remain generic `Object` kinds;
their descriptive categories are not promoted into invented historical types.

## Persistence And Import

Snapshot writes now use `.wx_tree_snapshot` v2 with an explicit quoted `kind`
field. The reader remains backward-compatible with v1. A v1 root becomes
`Root`; known development categories infer `Folder` or `Object`; custom
categories remain `Unknown`.

The path-list importer assigns `Folder` to implicit groups and maps only known
development leaf categories to `Object`. Unknown/custom categories remain
valid and receive `Unknown`. Import remains atomic.

Both formats are development aids, not historical SDK assets or level data.

## Next Boundary

Further mapping requires a consumer-by-consumer audit of the opaque historical
`ListItem::type` integer. It must not be converted into a broad enum merely
because individual editors reused that field for local edit modes. Real SDK
metadata loading remains future work, and the Borland/VCL path stays separate.
