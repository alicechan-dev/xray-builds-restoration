# wx Editor Selection Model

## Historical Selection Audit

The preserved `TItemList` keeps selection inside `TElTree`:

* `GetSelected(RStringVec&)` walks `GetNextSelected` and emits full hierarchical
  names produced by `MakeFullName`;
* the `ListItemsVec` overload emits backing `ListItem` keys and optionally
  accepts keys whose raw string begins with a supplied prefix (`Pos(pref) == 1`);
* its `bOnlyObject` option tests a non-null `m_Object` payload, not
  `TYPE_OBJECT`, so it is not copied as a portable kind rule;
* `SelectItem` resolves an exact full path, can clear previous widget selection,
  optionally expands ancestors, and invokes ElPack selection callbacks;
* selected order and duplicate prevention come from the widget iterator/state;
* selection persistence stores full names and restores the nearest valid item.

Widget selection, expansion, callbacks, form storage, and multi-select mechanics
are UI behavior. No preserved `xrEProps` file is included or modified.

## Neutral Design

`EditorSelectionModel` stores canonical model-generated paths rather than node
pointers. This avoids retaining invalid addresses after delete, snapshot load,
or import. Duplicate identity comparison ignores case, while returned paths
retain model spelling.

The API supports clear, select, deselect, toggle, membership, count, stale-path
pruning, and deterministic selected-node resolution. Selected results are
returned in model pre-order, not selection-click order.

`GetSelectedPaths` returns full model paths. `GetSelectedLabels` returns leaf
labels. Both accept an optional raw, case-sensitive path prefix and optional
`EditorItemKind` filter. This makes path versus key behavior explicit and uses
audited kinds rather than the historical non-null-payload proxy.

Read-only queries do not affect selection. Matching paths can resolve across a
replacement model; `Prune` removes paths absent from the current model.

Successful hierarchy moves use `RemapPathPrefix(oldPath, newPath)`. It remaps
only an exact canonical path or a descendant at a `/` component boundary, so a
selected moved folder and selected descendants follow the new hierarchy while
stale old paths disappear. This is separate from the raw prefix filter accepted
by collection methods. See [wx Editor Tree Move](wx-editor-move.md).

## Current UI Seam

The wx tree remains single-selection. Each view selection replaces the neutral
selection with one path. Create/find selects the resulting node; confirmed
delete selects the surviving parent; snapshot load and path-list import select
the new root. These rebuild paths therefore discard stale identity naturally.

**Tools → Show Selected Path** writes the current model path and count to the
output/status surfaces. **Tools → Clear Selection** clears the wx selection,
neutral selection, and property panel. No wx multi-select behavior is claimed.

Headless tests cover selection mutation, duplicate suppression, deterministic
order, label/path collection, prefix and kind filters, clearing, stale pruning,
replacement resolution, move-prefix remapping, presenter synchronization,
reporting, and clear action.
They require no wxWidgets package.

This is a model migration seam, not a port of ElPack selection state or real SDK
data loading.

Selection drives the editable [wx Editor Properties](wx-editor-properties.md)
set. Clearing selection clears the property panel; property edits never store
node pointers in the selection model.
