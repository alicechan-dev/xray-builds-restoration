# wxSDKEditor Document Layer

`ReplaceWithConvertedModel` is an atomic wx-free entry point used only after a
historical conversion candidate passes validation. It clears selection,
history, and file path, then marks the new editable copy dirty. Conversion is
not inserted into command history; future edits use normal commands.

Snapshot v5 persists bounded historical-origin metadata while keeping v1-v4
readable. No raw historical body or absolute source path is serialized.

`EditorDocument` is the first wx-free document boundary in the experimental
editor. `MainFrame` owns one document, and `EditorTreePresenter` references its
`EditorTreeModel`, `EditorSelectionModel`, and `EditorCommandHistory`. There is
no duplicate model ownership and no wxWidgets dependency in document code.

The document also owns the current path and saved-state baseline. Files use the
existing development-only `.wx_tree_snapshot` format; this is not an X-Ray
level or SDK asset format.

## Dirty State

The saved state is the exact serialized model snapshot captured after New,
Open, Save, or Save As. `IsModified()` compares the current model with that
baseline. Successful model commands become dirty automatically; failed
commands, selection, search, and pane layout changes do not. Undoing exactly
to the saved snapshot clears the marker, and redoing away from it restores it.
This remains correct after undo followed by a new command and does not require
command-stack revision bookkeeping.

## File Workflow

The File menu provides New (`Ctrl+N`), Open Snapshot (`Ctrl+O`), Save
(`Ctrl+S`), and Save As (`Ctrl+Shift+S`). Save without a path invokes Save As.
Before New, Open, Import, or application close, a modified document offers
Save, Discard, and Cancel. Cancel or failed saving aborts the destructive
operation.

The title is `wxSDKEditor - Untitled` or the snapshot filename, with ` *`
appended while modified. Presenter document-state callbacks update it after
commands, undo/redo, load, save, import, and New.

Path-list import replaces the current hierarchy after the same dirty prompt,
clears selection/history/path, and creates a modified untitled document. A
failed import preserves all existing document state.

Headless tests cover clean New state, command dirtiness, failed commands,
Save/Save As, exact saved-state undo/redo, successful and failed load, import
policy, path/display name, history reset, and selection reset. Temporary test
snapshots are created in the operating-system temporary directory and removed.
Tests-only configuration remains wxWidgets-free.

Real SDK documents, MDI, recent files, autosave, asset serialization, and level
loading remain future work. Preserved `xrEProps` and LevelEditor sources are
untouched.

Viewport focus, camera placeholders, frame ticks, and grid visibility are UI
state and never modify `EditorDocument` or its dirty baseline. See [wxSDKEditor
Viewport Host](wx-editor-viewport.md).

Preview objects are derived from the current tree and selection. Their source
node transforms are document state in snapshot v3, while preview draw data is
transient. See [wxSDKEditor Preview Scene](wx-editor-preview-scene.md).

Preview picking changes only logical selection. Hits and empty-space clears do
not affect the saved snapshot baseline or command history.

Node transforms are now document state persisted by snapshot v3. v1/v2 readers
assign defaults. Temporary gizmo motion is not document state until release.

Tool mode and placement preview are transient UI state. Accepted placements are
ordinary snapshot-v4 nodes and therefore follow existing dirty, save, load,
undo, and redo behavior. New, successful Open, and successful Import reset the
tool to Select; successful Save does the same after capturing the save point.

Snapshot v4 adds an optional escaped asset ID to each node record. Readers for
v1/v2/v3 remain supported and assign an empty ID. Unknown IDs are accepted and
round-trip atomically; catalog membership is not a precondition for opening a
document. The catalog itself is application state, not serialized document
content.

Imported metadata is application-session state, not document state. Loading,
searching, or clearing it does not change the model, history, save point, or
snapshot. New/Open/Save retain it; restart requires explicit reload.

Snapshot v4 needs no revision for imported placement: placed nodes store only
their `imported.section.*` asset ID and ordinary node fields. A missing session
catalog is an unresolved descriptor, not a load error; no provenance path or
metadata contents are serialized.

Historical scene inspection is also application-session state, not document
state. Inspecting or failing to inspect a `.level` file does not replace the
tree model, clear history, change the save point, or mark the snapshot dirty.

Opening a historical scene read-only uses a separate document and selection
model rather than changing this editable snapshot document. New and Open
Snapshot reactivate this document. See [Document
Modes](wx-editor-document-modes.md).

Converted documents remain ordinary pathless editable snapshots, but their
nodes may own immutable historical provenance. The summary is derived on
demand; no mutation/tombstone log is added, and Save remains snapshot-only.
