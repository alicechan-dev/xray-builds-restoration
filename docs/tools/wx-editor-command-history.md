# wxSDKEditor Command History

`wxSDKEditor` now has a wx-free command-history seam for development model
mutations. `EditorCommandHistory` owns bounded undo and redo stacks, while
`EditorModelCommand` captures the existing in-memory tree snapshot before and
after a successful mutation. No new persistence format is introduced.

The presenter routes demo add, delete, move, rename, and editable property
changes through this boundary. Undo and redo rebuild the adapter projection and
restore selection by logical model path, avoiding pointers into a model that a
snapshot restore replaced. A successful new command clears redo history;
successful snapshot load and path-list import clear both stacks.

The wx shell exposes **Edit -> Undo** (`Ctrl+Z`) and **Edit -> Redo** (`Ctrl+Y`).
Menu availability follows presenter history state. This is still a prototype
history for the neutral demo model, not an implementation of historical
LevelEditor commands or asset transactions.

Headless presenter tests use fake `IEditorTree`, `IPropertyPanel`, and
`IDialogService` implementations. They cover model/view/property/dialog
coordination, delete confirmation, command undo/redo, logical selection
restoration, and the rule that failed path-list import preserves the existing
model. Tests-only configuration does not discover, include, or link wxWidgets.

The preserved `xrEProps` and LevelEditor sources remain untouched. Real SDK
data loading, on-disk command journals, command merging, and asset-aware
transactions remain future work.

Docking does not participate in command history. Pane positions and visibility
are user-interface state persisted separately through wxConfig; model commands
remain wx-free. See [wxSDKEditor Dockable Layout](wx-editor-layout.md).

`EditorDocument` now defines the save point by retaining the exact serialized
model baseline. Command history itself remains concerned only with execution,
undo, redo, and selection restoration; document dirtiness becomes clean when
undo returns to that baseline. See [wxSDKEditor Document Layer](wx-editor-document.md).

Move-gizmo motion is preview-only. Release submits one transform command;
Escape submits none. Undo/redo restores both position and logical selection.

Each placement click similarly creates one model command. Undo removes the
placed node; redo recreates its parent/name/category/kind/transform and selection.
Tool switches and canceled placement never enter history.

Catalog placement adds the stable asset ID and descriptor-derived metadata to
that same command snapshot. Delete/undo, subtree restoration, rename, move,
and property edits therefore preserve asset identity without a separate asset
command type.
