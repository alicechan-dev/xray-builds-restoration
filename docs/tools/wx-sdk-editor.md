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

## Current Shell

The first shell contains only infrastructure:

* a `wxApp` and top-level `wxFrame`;
* File, Edit, View, Tools, and Help menus;
* a status bar;
* a left scene/object tree populated with explicit demo-only categories;
* a central viewport placeholder;
* a right property-panel placeholder that follows tree selection;
* a bottom read-only output pane.

It does not load levels, assets, forms, or runtime data. `EditorTreeModel`
owns the demo Objects, Lights, Sounds, Sectors / Portals, and Spawn Elements
hierarchy. Selecting an entry shows its model label, category, path, and a
placeholder-only notice in the property panel. No historical editor logic has
been copied into the shell.

Pressing F2 on a selected visible item starts wx label editing. The completed
edit is sent to `EditorTreeModel::RenameNode()` before wx may commit it.
Accepted edits refresh the property panel and status bar from model state.
Rejected empty or duplicate sibling names are vetoed, keep the old label and
path, and report the model's reason through `IDialogService`.

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
initial selection. The frame traverses `EditorTreeModel`, populates
`IEditorTree`, and lets `wxEditorTree` translate that into `wxTreeCtrl` items.
The model remains independent of wxWidgets and owns every node referenced by
the view.

The first model seam provides owned children, non-owning parent links, stable
node addresses, labels, categories, paths, lookup, and rename validation.
Renames reject empty labels and case-insensitive sibling duplicates, return a
reason without showing UI, and refresh paths recursively. Quiet startup checks
cover duplicate rejection, successful unique rename, path refresh, and empty
name rejection.

This is the first active prototype of the historical
`ItemListHelper::NameAfterEdit()` responsibility without `TElTreeItem`. It
does not compile, call, or modify the old `xrEProps` implementation.

The historical ElPack usage map is documented in [wx Editor ElPack Migration
Map](wx-editor-elpack-migration.md). The map identifies `xrEProps` item-list
model/rename logic as the first extraction candidate and keeps the much larger
`FolderLib` widget behavior for a later, model-led split.

## Next Step

The next safe sequence is:

1. keep the current model contract small and add focused model tests;
2. map `ItemListHelper::NameAfterEdit()` inputs and path rules precisely;
3. add model commands only for behavior proven by that audit;
4. connect old SDK/editor logic only after those commands are tested.

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
