# wxSDKEditor Document Modes

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

Compressed known containers remain rejected by the probe. Unknown compressed
chunks and specialized object bodies are reported as unsupported; the reader
does not parse through them, so confirmed counts may be partial.
Historical compressed chunks are decoded only into bounded session memory.
This does not make `HistoricalSceneReadOnly` editable and does not add a scene
save/export path.

