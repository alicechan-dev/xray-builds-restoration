# wxSDKEditor Dockable Layout

`wxSDKEditor` uses wxWidgets AUI for its opt-in editor shell. This replaces the
initial fixed hierarchy of three splitter windows; it does not change the
wx-free model, presenter, command history, or adapter contracts.

## Panes

| Pane | Default position | Behavior |
|---|---|---|
| Scene / Objects | Left | Dockable, resizable, closable; owns `wxEditorTree`. |
| Properties | Right | Dockable, resizable, closable; owns `wxPropertyPanel`. |
| Output | Bottom | Dockable, resizable, closable; owns the read-only output control. |
| Viewport | Center | Always-present, non-dockable placeholder. It renders no scene. |

The **View** menu can show or hide each non-central pane and reflects pane
close-button changes. **Reset Layout** restores the default left/right/bottom
arrangement and keeps the center viewport available.

## Persistence

On clean shutdown, `wxAuiManager::SavePerspective()` is stored through
`wxConfig` under the user's platform configuration location. Startup attempts
to restore that perspective and falls back to the captured default if it is
invalid. Resetting the layout also replaces the stored perspective. No layout
file is written into the repository and no model or SDK data is stored in it.

The frame saves the perspective and calls `wxAuiManager::UnInit()` in its
destructor before wx destroys the managed child windows. Pane ownership remains
with the frame; AUI only manages their layout.

Tree input, label-edit, and selection bindings are explicitly removed before
the presenter is destroyed. This prevents `wxTreeCtrl` item deletion during
child teardown from dispatching a selection callback to an expired presenter.

The wx target links the target-local wxWidgets `aui` component. The tests-only
option returns before wx discovery and remains wxWidgets-free. This is a modern
shell layout, not a port or emulation of VCL form persistence, ElPack, or old
LevelEditor docking behavior. Preserved `xrEProps` and LevelEditor sources are
untouched.

The frame also owns the single `EditorDocument`, but AUI perspective state is
kept separate from document state. Moving or hiding panes never marks the
document modified. See [wxSDKEditor Document Layer](wx-editor-document.md).

The AUI center pane now owns `wxEditorViewport` rather than a static label. It
remains non-closable and non-dockable; reset layout always restores it. See
[wxSDKEditor Viewport Host](wx-editor-viewport.md).

The center surface now overlays derived preview boxes, markers, lights, labels,
and selection highlighting while retaining the same fixed AUI pane policy.

A normal frame toolbar now sits above the AUI workspace. It uses built-in wx
art and is not part of the persisted AUI perspective; no icon assets are added.

The synthetic Asset Browser adds a left-docked, closable, resizable AUI pane.
Its `asset_browser` identity participates in the existing perspective, its View
menu check follows actual visibility, and Reset Layout restores it. Browser
filters and selection are UI state and never dirty the document.

The read-only Scene Inspector adds a hidden-by-default right-docked pane. Its
visibility participates in the AUI perspective, while its inspected manifest
does not. Reset Layout hides the pane and inspection never dirties a document.
