# wx Editor Properties

## Historical Audit

The preserved `xrEProps` property layer has real value semantics beneath its
VCL/ElPack UI. `PropertiesListHelper` creates signed/unsigned integer, float,
BOOL/flag, token/choice, string, read-only text, vector, color, time, wave, and
chooser values. Numeric values carry ranges, increments, and precision.
Before/after callbacks transform or validate edits; accepted edits mark the
list modified. Text forms distinguish Apply/OK from Cancel and support
read-only mode.

ElPack rows and in-place editors, drawing, colors, icons, popup menus, chooser
forms, form persistence, object pointers, and control callbacks remain UI or
binding behavior and are not ported.

## Neutral Property Model

`EditorPropertyType` defines `String`, `Integer`, `Float`, `Boolean`, `Choice`,
and `ReadOnlyText`. `EditorProperty` stores a stable key, label, tagged type,
string representation, read-only state, optional choices, and description.
`EditorPropertySet` is a small ordered collection with case-insensitive key
lookup. It is intentionally not reflection or an SDK object-binding system.

`BuildEditorNodePropertySet` currently exposes:

| Key | State | Meaning |
|---|---|---|
| `label` | Editable | Tree label and generated path component. |
| `category` | Editable | Development-only display category. |
| `kind` | Read-only | Audited structural kind. |
| `path` | Read-only | Model-generated canonical path. |

`ApplyEditorNodeProperty` returns success, reason, tree-rebuild, and
property-refresh flags. Label edits reuse `EditorTreeModel::RenameNode`, so
empty and case-insensitive duplicate names fail atomically and successful
renames refresh descendant paths. Category accepts an empty display value and
does not change kind. Unknown/read-only keys reject edits.

## UI And Tests

`IPropertyPanel` accepts a property set and a wx-free apply callback. The
concrete `wxPropertyPanel` uses ordinary `wxTextCtrl` fields and one Apply
button; label/category are editable and kind/path are read-only. The presenter
applies edits, rebuilds the tree only for label changes, refreshes properties,
preserves selection, and reports validation through existing status/dialog
services.

Headless tests cover all type names, stable keys, editability, successful and
failed label edits, duplicate validation, recursive path refresh, category
editing, read-only rejection, result flags, and fake-panel presenter
coordination. Tests-only mode does not discover or link wxWidgets.

Category was already part of snapshot v2, so persistence is unchanged. No
demo-only fields were added. Old `xrEProps` and LevelEditor remain untouched;
real SDK object properties and assets remain future work.

The concrete property panel is now hosted in the right wxAUI pane and can be
docked, floated, hidden, and restored without changing `IPropertyPanel` or the
presenter. Pane state is UI configuration only; property values remain model
state. See [wxSDKEditor Dockable Layout](wx-editor-layout.md).

Successful label/category applies now change the owning `EditorDocument` model
and therefore its exact snapshot-based dirty state. Rejected or read-only
applies leave the saved-state comparison unchanged.

Object-like nodes expose editable `position.x/y/z`. Values must be finite
floats. Successful edits refresh preview and enter command history; rejected
values leave the model unchanged.

The property set also exposes `asset_id` as read-only. Empty IDs display as
`none`; unknown IDs remain visible unchanged. Asset reassignment is not an
editable property in this pass. Path and item kind remain read-only, while
label, category, and finite position fields retain their existing policies.
