# wxSDKEditor Document Modes

**Convert Historical Scene to Editable Copy...** is the only transition from
the read-only mode into a new `EditableSnapshot`. It creates a pathless dirty
document with empty selection/history and does not alter the historical
document. This is not Save As or a mode toggle on the `.level` file.

wxSDKEditor exposes two explicit modes through `EditorDocumentMode`:

| Mode | Ownership | Mutation policy |
|---|---|---|
| `EditableSnapshot` | Existing `EditorDocument` | Existing snapshot, history, property, placement, and save behavior. |
| `HistoricalSceneReadOnly` | Separate `EditorHistoricalSceneDocument` | Selection/query/preview only; never dirty and never saved. |

**File > Open Historical Scene Read-Only...** first runs the existing bounded
probe and then converts its inert manifest. It is separate from **Inspect
Historical Scene...**, whose manifest remains application-session inspector
state. A dirty editable document still receives Save/Discard/Cancel before a
historical open. Failure preserves the prior workspace.

Historical mode sets the title to `<scene>.level [Read-Only]`. Save, Save As,
Undo, Redo, Add, Delete, Rename, Reparent, Move, placement, property Apply, and
move snapping are disabled in the UI and rejected again in the presenter with
`Historical scene is read-only.` Select, Find, selection reporting, viewport
picking, camera, grid, labels, Frame Selected, Scene Inspector, and metadata
browsing remain available.

New and successful Open Snapshot return to editable mode and restore property
editing and tools. Closing a historical scene needs no save prompt. Imported
metadata remains session state, and no historical source path or content is
written into an editable snapshot.

Historical compressed chunks are decoded only into bounded session memory.
Class-2 specialized bodies may expose audited inert metadata; unknown classes
and unsupported child chunks remain explicit inventory.
This does not make `HistoricalSceneReadOnly` editable and does not add a scene
save/export path.
