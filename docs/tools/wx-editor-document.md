# wxSDKEditor Document Layer

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
