# wx Editor Tree Query

## Historical Search Audit

The preserved `xrEProps` code exposes several narrow lookup behaviors rather
than one generic search engine:

* `CListHelper::FindItem` performs exact key equality over a flat vector;
* `CFolderHelper::FindItemInFolder` compares exact direct-child text, optionally
  restricted to `TYPE_FOLDER` or `TYPE_OBJECT`;
* `FindItem`, `FindFolder`, and `FindObject` walk backslash-separated path
  components one direct child at a time;
* `TItemList::FindItem` and `SelectItem` delegate to those exact path walkers;
* selected-item collection can apply a prefix check to stored keys;
* the ElPack comparator sorts folders before objects and then compares visible
  text.

Case folding occurs in rename flows, but the lookup functions themselves use
the stored string equality supplied by the historical types. ElPack selection,
expansion, sorting, callbacks, checkbox state, icons, drawing, drag/drop, and
form behavior are UI concerns and are not part of the neutral query layer.

No preserved `xrEProps` file is included or modified.

## Neutral Query Contract

`QueryEditorTree(const EditorTreeModel&, const EditorTreeQueryOptions&)` returns
stable node pointers in model pre-order and never mutates the model.

Options provide:

* label matching, enabled by default;
* optional model-generated path matching;
* case-insensitive comparison by default;
* substring matching by default or exact-field matching;
* an optional `EditorItemKind` filter for `Unknown`, `Root`, `Folder`, or
  `Object`.

An empty query returns every node admitted by the field-independent kind
filter. If both label and path matching are disabled, non-empty text matches
nothing. Substring search is an explicit development convenience, not a claim
about historical ItemList behavior.

## UI Prototype

**Tools → Find Item...** collects one label query through wx and passes it to
the wx-free presenter. The presenter uses default query behavior, selects the
first result through `IEditorTree`, refreshes properties, and reports the match
count in status text. No result is non-fatal and preserves the current
selection and model. The tree is not filtered or rebuilt.

Headless tests cover matching modes, path and kind filters, empty/no-result
behavior, traversal order, model immutability, first-result selection,
properties, and status text without discovering, including, or linking
wxWidgets.

This remains a migration seam, not a port of ItemListHelper or the historical
GUI, and it does not load SDK assets or game data.
