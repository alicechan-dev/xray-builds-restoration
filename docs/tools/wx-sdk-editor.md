# Experimental wxWidgets SDK Editor

`wxSDKEditor` is an experimental, parallel SDK user-interface shell. It is not
a port of the historical LevelEditor and does not claim compatibility with its
Borland/VCL forms, editor state, or data-loading behavior.

## Build

The target is opt-in and Win32-only:

```bash
/c/vcpkg/vcpkg.exe install wxwidgets:x86-windows
cmake -S . -B build-wx-sdk-editor-check -G "Visual Studio 17 2022" -A Win32 \
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_XR_WX_SDK_EDITOR=ON
cmake --build build-wx-sdk-editor-check --config Release --target wxSDKEditor -- //m:1 //v:minimal //clp:ErrorsOnly
```

The repository does not vendor wxWidgets or hardcode a package location.
`BUILD_XR_WX_SDK_EDITOR` defaults to `OFF`, so normal runtime and tool
configuration does not require wxWidgets. When enabled, CMake first attempts
wxWidgets config-package discovery and then the standard CMake
`FindwxWidgets` module. A missing dependency produces a focused diagnostic
with the required `x86-windows` triplet.

The GUI-independent model has a separate opt-in console test target:

```bash
cmake -S . -B build-wx-model-tests-check -G "Visual Studio 17 2022" -A Win32 \
  -DBUILD_XR_WX_SDK_EDITOR_MODEL_TESTS=ON
cmake --build build-wx-model-tests-check --config Release \
  --target wxSDKEditorModelTests -- //m:1 //v:minimal //clp:ErrorsOnly
./build-wx-model-tests-check/bin/wxSDKEditorModelTests.exe
```

`wxSDKEditorModelTests` compiles `EditorTreeModel`, the path-list importer,
`EditorTreeSnapshot`, and the wx-free `EditorTreePresenter`; it does not
discover, include, or link wxWidgets. Both
wx options default to OFF, so normal configuration remains unchanged.

## Foundation Checkpoint

The opt-in wxSDKEditor foundation is established, but remains an experimental
parallel path rather than a replacement for the historical Borland SDK. Its
current prototype behaviors are model-backed create, rename, delete, snapshot
save/load, and logical path-list import. No preserved `xrEProps` or
`LevelEditor` source is compiled, called, or modified.

Responsibilities are intentionally separated:

* `MainFrame` owns wx layout, menus, file dialogs, and wx event translation;
* `EditorTreePresenter` coordinates model, tree view, property panel, dialogs,
  snapshot/import handoff, and status/output callbacks;
* `EditorTreeModel` owns nodes and supplies paths plus create, rename, unique
  name, and delete rules;
* snapshot and path-list code implement development-only formats, not SDK
  assets, levels, or game data;
* `wxSDKEditorModelTests` verifies model, persistence, import, and presenter
  coordination headlessly without discovering, including, or linking wxWidgets.

## Current Shell

The first shell contains only infrastructure:

* a `wxApp` and top-level `wxFrame`;
* File, Edit, View, Tools, and Help menus;
* a status bar;
* a left scene/object tree populated with explicit demo-only categories;
* a central viewport placeholder;
* a right property-panel placeholder that follows tree selection;
* a bottom read-only output pane.

`EditorTreePresenter` is the GUI-facing coordination seam between
`EditorTreeModel`, `IEditorTree`, `IPropertyPanel`, and `IDialogService`. It owns
the model, rebuilds the disposable tree view, refreshes selection properties,
and coordinates create, delete, rename, snapshot, and path-list operations.
Status and output updates are supplied as callbacks, so the presenter includes
no wxWidgets headers.

`MainFrame` remains responsible for wx layout, menus and event binding, file
dialogs and path-list file reading, plus translation of wx tree events into
presenter calls. Rename rejection remains deferred through the frame so the wx
label-edit event can be vetoed before its dialog is shown.

It does not load levels, assets, forms, or runtime data. `EditorTreeModel`
owns the demo Objects, Lights, Sounds, Sectors / Portals, and Spawn Elements
hierarchy. Each node carries a neutral `EditorItemKind` (`Unknown`, `Root`,
`Folder`, or `Object`) plus its display category. Selecting an entry shows its
model label, kind, category, path, and a
placeholder-only notice in the property panel. No historical editor logic has
been copied into the shell.

Pressing F2 on a selected visible item starts wx label editing. The completed
edit is sent to `EditorTreeModel::RenameNode()` before wx may commit it.
Accepted edits refresh the property panel and status bar from model state.
Rejected empty or duplicate sibling names are vetoed, keep the old label and
path, and report the model's reason through `IDialogService`.

The Tools menu also provides Add Demo Object, Add Demo Group, and Delete
Selected. Creation uses model-generated case-insensitive unique sibling names
(`new_object`, `new_object_1`, and so on). Deletion is confirmed through
`IDialogService`, rejects the model root, and destroys descendants through
owned node storage. After either operation, the wx tree is rebuilt entirely
from `EditorTreeModel` and selects the new node or surviving parent.

**Tools -> Move Selected To...** is a menu/dialog prototype, not drag/drop. It
lists only Root/Folder paths accepted by the wx-free model. A successful move
transfers the owned subtree, refreshes descendant paths, remaps path-based
selection, rebuilds the view, keeps the moved node selected, and updates the
property path. Root, self, descendant-cycle, Object destination, current-parent,
and case-insensitive duplicate moves are rejected without mutation. See
[wx Editor Tree Move](wx-editor-move.md).

**Tools → Find Item...** is a minimal query prototype. MainFrame collects text,
the presenter invokes the wx-free `EditorTreeQuery`, and the first
case-insensitive label-substring match is selected without filtering or
rebuilding the tree. Properties refresh from the matched model node and status
reports the result count; no match leaves selection unchanged. Path, exact,
case-sensitive, and kind-filter options exist at model level but are not yet a
large UI. See [wx Editor Tree Query](wx-editor-query.md).

The wx tree remains single-selection, but selection identity now also flows
through the wx-free `EditorSelectionModel` as canonical model paths. **Tools →
Show Selected Path** reports the current path/count, and **Tools → Clear
Selection** clears the view, selection model, and properties. Delete selects the
surviving parent; snapshot/import rebuilds select the new root, so stale pointers
are never retained. See [wx Editor Selection Model](wx-editor-selection.md).

The File menu provides Save Demo Snapshot and Load Demo Snapshot. These
commands use a dependency-free, development-only `.wx_tree_snapshot` format;
they do not read or write X-Ray level data. Loading parses into a temporary
model and replaces the current model only after every record validates, then
rebuilds the wx tree and property selection.

Snapshot v2 begins with:

```text
# wxSDKEditor tree snapshot v2
```

Each following line stores depth plus quoted kind, label, category, and
diagnostic path fields. Quotes, backslashes, tabs, and line breaks are escaped.
Paths are
regenerated from hierarchy during load and compared with the stored diagnostic
value. The parser rejects malformed records, invalid depth, empty labels,
case-insensitive duplicate siblings, path mismatches, multiple roots, empty
trees, and files larger than 8 MiB.

The reader still accepts v1 snapshots. It assigns the structural root kind,
infers only known development folder/object categories, and leaves custom
categories as `Unknown`. See [wx Editor Item Types](wx-editor-item-types.md).

The File menu also provides **Import Demo Path List** for dependency-free model
experiments. Its `.wx_tree_paths` text format accepts one absolute-style logical
path per line, with an optional category after `|`:

```text
# wxSDKEditor path list v1
/Scene/Objects/actor | demo scene object
/Scene/Objects/level_changer
/Scene/Lights/sun | demo light
```

Blank lines and comments are ignored. Missing categories become `imported item`,
and intermediate nodes are created as `imported group`. All entries must share
one root component; relative paths, empty components, empty explicit categories,
and case-insensitive duplicate paths are rejected with a line-numbered reason.
Import parses into a temporary model, so failure leaves the visible model
unchanged. This is a development format, not an X-Ray level or SDK format.

## Adapter Boundary

The first interfaces are deliberately independent of wxWidgets so later work
can move behavior across one audited boundary at a time.

| Historical concept | Neutral interface | Initial wxWidgets implementation |
|---|---|---|
| `TElTree` | `IEditorTree` | `wxTreeCtrl` |
| `TElTreeItem` | opaque `IEditorTree::ItemHandle` | internal `wxTreeItemId` map |
| `TForm` | application/window boundary | `wxFrame` or future `wxDialog` |
| `TFrame` / `TPanel` | panel boundary | `wxPanel` |
| `TButton` | future command/view adapter | `wxButton` |
| `TLabel` | view text | `wxStaticText` |
| `MessageDlg` | `IDialogService` | `wxMessageBox` |
| `xrEProps` | `IPropertyPanel` | placeholder `wxPanel`; future audited property grid |
| `.dfm` forms | no compatibility claim | future explicit wx layouts, migrated individually |

This layer must not grow fake VCL, ElPack, AlexMX, or MagicFM APIs. Historical
behavior should be moved only after its data and command contracts are
understood and covered by small tests.

`IEditorTree` is the first active adapter boundary. It currently owns only
opaque item handles, hierarchy insertion, selected item data, expansion, and
initial selection. The presenter traverses `EditorTreeModel`, populates
`IEditorTree`, and lets `wxEditorTree` translate that into `wxTreeCtrl` items.
The model remains independent of wxWidgets and owns every node referenced by
the view.

The presenter now performs that model-to-adapter projection instead of
`MainFrame`. This keeps widget construction and event objects at the wx edge
while model mutation and view synchronization share one explicit seam.

The first model seam provides owned children, non-owning parent links, stable
node addresses, labels, categories, paths, lookup, and rename validation.
Renames reject empty labels and case-insensitive sibling duplicates, return a
reason without showing UI, and refresh paths recursively. Quiet startup checks
cover duplicate rejection, successful unique rename, path refresh, and empty
name rejection.

This is the first active prototype of the historical
`ItemListHelper::NameAfterEdit()` responsibility without `TElTreeItem`. It
does not compile, call, or modify the old `xrEProps` implementation.

Together, model-backed create, delete, and rename are the first prototype of
the hierarchy responsibilities historically mixed across `ItemListHelper` and
`FolderLib`. The wx control remains a disposable view rather than the owner of
tree data.

The historical ElPack usage map is documented in [wx Editor ElPack Migration
Map](wx-editor-elpack-migration.md). The map identifies `xrEProps` item-list
model/rename logic as the first extraction candidate and keeps the much larger
`FolderLib` widget behavior for a later, model-led split.

## Next Step

The next safe sequence is:

1. optionally add an intentionally documented sample under `docs/examples`;
2. audit individual consumers of the remaining opaque `ListItem::type` integer
   without modifying `xrEProps`;
3. later add a read-only adapter for independently extracted metadata;
4. keep the historical Borland/VCL restoration path separate.

Development is isolated on `experiment/sdk-wx-editor-ui`. If the experiment
is accepted, it can be squash-merged so the restoration branch receives one
reviewable SDK UI foundation commit.

## Current Validation

The initial Win32 shell has been configured and built with the external
`wxwidgets:x86-windows` package. This adapter pass configures, builds, and
passes a model-driven no-data launch smoke check. Because the historical root removes
standard exception handling for runtime compatibility, the wx target restores
`/EHsc` and `/GR` locally to match the external wxWidgets binary without
changing runtime targets. Generated build output remains outside commits.

The rename-enabled Release target also passes a startup smoke check. Rename
acceptance and rejection remain covered by the quiet model self-check; full
interactive GUI automation is future test infrastructure.

Model self-checks now also cover unique child-name generation,
case-insensitive lookup, root-delete rejection, child deletion, and removal
from path lookup.

Snapshot self-checks serialize and deserialize the demo hierarchy entirely in
memory, verify a known path, and reject duplicate-sibling and malformed input.
No snapshot is written during startup.

The headless tests provide broader, explicit coverage without launching a GUI.
Model checks cover demo paths, add/unique-name behavior, case-insensitive
lookup, accepted/rejected rename, descendant path refresh, root protection,
and subtree deletion. Import checks cover comments, implicit groups, default
categories, duplicate and malformed paths, atomic failure, and a snapshot
round trip. Snapshot checks cover in-memory round trips, escaped
fields, malformed headers and records, invalid depth, empty labels, duplicate
siblings, path mismatch, and atomic preservation after failed loads. Tests use
no real assets, runtime data, or generated snapshot files.

Presenter checks use deterministic test-only `FakeEditorTree`,
`FakePropertyPanel`, and `FakeDialogService` implementations. They cover demo
projection and selection properties, unique create behavior, accepted and
rejected rename, delete confirmation and cancellation, root-delete rejection,
successful path-list replacement, and preservation of the old model after a
failed import. These tests launch no wxWidgets code and add no production test
hooks. The preserved `xrEProps` code remains untouched, and real SDK data
loading remains future work.

The presenter also owns the first wx-free command history. Demo add, delete,
move, rename, and editable property changes use in-memory snapshot-backed
commands. Undo/redo restores logical selection paths after model replacement,
and the wx shell exposes `Ctrl+Z`/`Ctrl+Y`. Snapshot load and successful
path-list import establish new model baselines and clear history. See [wx
Editor Command History](wx-editor-command-history.md).

Headless coverage exercises command/model/view/property/dialog coordination,
including delete confirmation, undo/redo selection restoration, and atomic
failed path-list import. The fake `IEditorTree`, `IPropertyPanel`, and
`IDialogService` require no wxWidgets; tests-only configuration neither
discovers nor links it. Old `xrEProps` remains untouched, and real SDK data
loading remains future work.

Item-kind checks cover conversion/parsing, group/leaf classification, demo
assignments, v2 snapshot round trips, v1 compatibility, known path-list
category mapping, custom-category fallback, and presenter property text.

Query checks cover default case-insensitive label substrings, exact labels,
model paths, case-sensitive comparison, all four kind filters, empty and
no-result behavior, stable pre-order, and model immutability. Presenter checks
cover first-result selection, property refresh, match-count status, and safe
no-result behavior.

Selection checks cover select/deselect/toggle, duplicate prevention, model-order
paths, labels, raw prefix and kind filters, clearing, stale pruning, replacement
resolution, presenter single-selection synchronization, reporting, and clear.

The property placeholder is now a small editable prototype. Selection supplies
a wx-free `EditorPropertySet`; label and category use text fields, while kind
and generated path are read-only. Apply routes through the presenter and node
property adapter. Rename validation remains in `EditorTreeModel`, and category
remains development-only. See [wx Editor Properties](wx-editor-properties.md).

## Dockable Shell Layout

The shell layout now uses `wxAuiManager` instead of nested splitters. Scene,
properties, and output are named dockable panes around a non-dockable center
viewport placeholder. The View menu toggles panes and restores the default
layout. AUI perspective state is saved through `wxConfig` in the user profile,
restored defensively at startup, and never written into the repository.

Only the opt-in GUI target adds the wxWidgets `aui` component. Tests-only mode
still returns before wxWidgets discovery and remains wx-free. This modern shell
layout is not a port of VCL form persistence. See [wxSDKEditor Dockable
Layout](wx-editor-layout.md).

## Development Document

`MainFrame` now owns one wx-free `EditorDocument`; the presenter references its
model, selection, and command history. File actions provide New/Open/Save/Save
As for `.wx_tree_snapshot`, three-way Save/Discard/Cancel prompting protects
New/Open/Import/close, and the title shows filename plus a dirty marker. The
save point is an exact serialized-model baseline, so undo back to the saved
state becomes clean. Import creates a modified untitled document. See
[wxSDKEditor Document Layer](wx-editor-document.md).

## Viewport Host

The center pane now contains `wxEditorViewport`: a buffered dark grid and
diagnostic overlay backed by a wx-free state/controller and a no-op renderer
interface. Right drag, wheel, focused WASD/QE, grid toggle, camera reset, and
focus commands validate future input flow only. No graphics device, level,
asset, picking, or historical renderer is connected. See [wxSDKEditor Viewport
Host](wx-editor-viewport.md).

## Synthetic Preview Scene

The presenter now rebuilds a flat wx-free preview from current model paths and
selection. Stable pre-order produces deterministic synthetic grid positions;
the wx-free software renderer emits semantic draw-list primitives, and the wx
surface paints them over the existing grid. Labels can be toggled and Frame
Selected centers the placeholder camera. Preview data is derived only and does
not alter snapshots or document dirtiness. See [wxSDKEditor Preview
Scene](wx-editor-preview-scene.md).

The synthetic surface now supports wx-free click picking with shared projection
rules. Hits select the corresponding tree path and refresh properties/highlight;
empty space clears selection. Reverse draw order resolves overlaps. See
[wxSDKEditor Preview Picking](wx-editor-picking.md).

The development model now owns transforms and writes snapshot v3 while reading
v1/v2 with defaults. Finite position fields and a tested X/Z move gizmo use one
command per committed drag. See [Transforms](wx-editor-transforms.md) and
[Move Gizmo](wx-editor-gizmo.md).

The shell now exposes wx-free Select/Move/Place Object/Place Light modes through
a standard wx toolbar. Synthetic placement uses shared projection, transient
preview feedback, deterministic parenting/naming, and one command per click.
See [Tool Modes](wx-editor-tools.md) and [Placement](wx-editor-placement.md).

The shell now includes a wx-free synthetic descriptor catalog and selection
model plus a dockable wx Asset Browser. Catalog search/category queries are
deterministic, and generic placement derives metadata, default transform, and
semantic preview from the selected descriptor. Optional asset identity is
persisted by snapshot v4 while v1/v2/v3 and unknown IDs remain supported. See
[Synthetic Asset Catalog](wx-editor-assets.md) and
[Asset Browser](wx-editor-asset-browser.md). No real X-Ray assets are loaded.

An explicit read-only LTX bridge can inspect one user-selected include graph.
The preserved LevelEditor proves that `$spawn` is a quoted browser hierarchy
and that its section is the stable item identity. Safe, unambiguous records may
therefore place inert synthetic Spawn markers; malformed records remain
read-only. Parser, model, classifier, and adapter remain wx-free, bounded, and
root-contained; the frame supplies only dialogs and compact Output diagnostics. See
[Metadata Import](wx-editor-metadata-import.md) and
[Metadata Security](wx-editor-metadata-security.md). No runtime construction,
asset loading, or snapshot-format change is implied.

The shell also has an explicit read-only probe for canonical build-1935
`.level` scenes. A bounded wx-free reader produces a session-only chunk/object
manifest displayed by the dockable Scene Inspector. It does not construct
objects, load references, replace the development document, or write scenes.
See [Scene Format Audit](wx-editor-scene-format-audit.md), [Scene
Probe](wx-editor-scene-probe.md), and [Scene
Inspector](wx-editor-scene-inspector.md).

Confirmed manifest records can now be opened as a separate read-only
historical document. Stable record IDs support duplicate names, provenance is
shown read-only, and only confirmed finite transforms receive generic preview
boxes. Existing `.wx_tree_snapshot` editing is unchanged. See [Historical
Document](wx-editor-historical-document.md), [Historical
Preview](wx-editor-historical-preview.md), and [Document
Modes](wx-editor-document-modes.md).
The historical read-only path now understands source-confirmed X-Ray LZHUF
chunk compression using a dependency-light wx-free decoder. Scene Inspector
shows compression provenance and failures. No decompressed data is written.
The specialized dispatcher reads source-confirmed `CSceneObject`
version/reference/flags, `CGlow` version/shader/texture/radius/flags, and
`CLight` parameters/flags/control/inert optional metadata, plus `CSpawnPoint`
outer metadata and opaque payload summaries into inert records;
all other classes remain generic and references are never
resolved. Glow radius may produce a diagnostic preview ring, but no referenced
asset is loaded. See [Class Inventory](wx-editor-scene-class-inventory.md),
[Scene Object Decoder](wx-editor-historical-scene-object.md),
[Glow Decoder](wx-editor-historical-glow.md),
[Light Decoder](wx-editor-historical-light.md), and
[Spawn Point Decoder](wx-editor-historical-spawn-point.md).
