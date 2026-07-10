# Experimental wxWidgets SDK Editor

`wxSDKEditor` is an experimental, parallel SDK user-interface shell. It is not
a port of the historical LevelEditor and does not claim compatibility with its
Borland/VCL forms, editor state, or data-loading behavior.

## Build

The target is opt-in and Win32-only:

```powershell
C:\vcpkg\vcpkg.exe install wxwidgets:x86-windows
cmake -S . -B build-wx-sdk-editor-check -G "Visual Studio 17 2022" -A Win32 `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DBUILD_XR_WX_SDK_EDITOR=ON
cmake --build build-wx-sdk-editor-check --config Release --target wxSDKEditor -- /m:1 /v:minimal /clp:ErrorsOnly
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
* a left scene/object tree;
* a central viewport placeholder;
* a right property-panel placeholder;
* a bottom read-only output pane.

It does not load levels, assets, forms, or runtime data. The viewport and
properties are placeholders, and no historical editor logic has been copied
into the shell.

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

## Next Step

After wxWidgets is available locally and the shell builds, the next safe step
is a no-data launch smoke test. Functional editor work should then begin with
one read-only model, such as populating `IEditorTree` from synthetic data,
without linking runtime-heavy targets or loading proprietary assets.

Development is isolated on `experiment/sdk-wx-editor-ui`. If the experiment
is accepted, it can be squash-merged so the restoration branch receives one
reviewable SDK UI foundation commit.

## Current Validation

The Win32 configure with `BUILD_XR_WX_SDK_EDITOR=OFF` passes and does not
perform wxWidgets discovery. The opt-in configure was also tested with the
local `C:\vcpkg` toolchain; it reaches the focused dependency diagnostic
because `wxwidgets:x86-windows` is not currently installed there. The
`wxSDKEditor` compile and launch checks therefore remain pending installation
of that external package.
