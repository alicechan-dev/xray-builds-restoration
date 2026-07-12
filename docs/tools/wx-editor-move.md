# wx Editor Tree Move

## Historical Audit

The preserved move path is concentrated in `Editors/xrEProps/FolderLib.cpp`.
`StartDrag` stores the focused `TElTreeItem*`; `DragOver` identifies the drop
item and rejects the current parent, self moves, and folder-to-descendant
cycles. Dropping on an Object uses that object's parent folder. Dropping on
empty tree space moves an attached item to the root level.

Folders and Objects are the only proven historical hierarchy kinds. Folders
contain children; Objects do not. Moving a folder walks its complete subtree,
recreates destination nodes, invokes the rename callback for contained object
paths, and deletes the old folder. An Object collision at the destination is
reported and skipped. A same-name Folder can be reused as a merge destination.

Confirmation dialogs, mouse hit testing, drag images, expansion, widget
selection, drawing, callbacks, and form persistence are ElPack/VCL behavior.
They are deliberately not mapped into the neutral model.

## Neutral Model Contract

`EditorTreeModel::CanMoveNode` and `MoveNode` implement a stricter atomic
migration seam:

* the root cannot move;
* a node cannot move under itself or one of its descendants;
* only `Root` and `Folder` may contain children;
* `Object` and `Unknown` are not valid destinations;
* moving to the current parent is rejected as a no-op;
* every case-insensitive sibling-label collision is rejected, including a
  Folder collision, rather than reproducing the historical widget merge;
* validation failures leave ownership and paths unchanged.

After validation, the model reserves destination storage, extracts the existing
`std::unique_ptr` from the old parent's child vector, inserts it under the new
parent, updates the non-owning parent pointer, and refreshes paths recursively.
The moved node and descendants retain their addresses; labels, categories,
kinds, and owned children are preserved. Old paths stop resolving and new paths
resolve immediately.

## Selection And Presenter

`EditorSelectionModel::RemapPathPrefix` replaces a canonical path only when it
matches the moved node exactly or at a `/` component boundary. This remaps a
selected folder and any selected descendants without retaining stale paths.

`EditorTreePresenter::MoveSelectedTo` resolves the selected source and
destination paths, requests the model move, remaps selection, rebuilds the
disposable tree view, reselects the stable moved node, refreshes properties,
and reports status/output. Failures preserve model, view, selection, and
properties and use `IDialogService` for the reason.

The wx prototype is **Tools -> Move Selected To...**. It presents only valid
Root/Folder destination paths in a `wxSingleChoiceDialog`; it is not drag/drop.

Headless tests cover object and subtree moves, recursive path refresh, old/new
lookup, root/self/cycle/Object/duplicate rejection, atomic failure, metadata and
pointer preservation, selection remapping, snapshot round-trip, and presenter
coordination. Tests-only mode does not discover, include, or link wxWidgets.

Old `xrEProps` and LevelEditor sources remain untouched. This is a migration
seam, not a `FolderLib` port, and it loads no real levels or SDK assets.
