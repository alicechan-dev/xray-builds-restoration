# wx Editor ElPack Migration Map

The Object Library browser is a native wx pane over a new GUI-independent
catalog. It does not instantiate `ELibrary`, `CEditableObject`, `TElTree`, or
other Borland/ElPack classes, and leaves the historical path untouched.

The historical conversion workflow remains on the wx-free adapter/model side
of this boundary. It does not compile or call `ItemListHelper`, `FolderLib`,
ElPack, VCL, or old LevelEditor code. Origin metadata and conversion reports
are modern inert records, not compatibility stand-ins for historical GUI types.

This document maps the historical ElPack tree dependency to the experimental
wxWidgets editor boundary. It is an analysis artifact, not a port: the old
Borland/VCL code remains unchanged and `wxSDKEditor` still uses demo-only data.

## Dependency Shape

The active Borland projects link `elpackB6.lib` and name an ElPack source root
containing `ElTree.hpp`. Direct use is concentrated in `xrEProps` and then
consumed by LevelEditor and the other GUI editors. The code relies on more than
tree rendering: `TElTreeItem` stores data/tag pointers, parent/child topology,
selection, checkbox state, sort state, custom height/drawing, inline editing,
and drag/drop state.

`IEditorTree` is therefore only the view/navigation seam. It must not grow into
a source-compatible imitation of `TElTree`. Hierarchy, identity, item type,
rename rules, and selection should move into a future GUI-independent
`IEditorTreeModel` (name provisional), while drawing and events stay in the wx
view adapter.

## Classification

Categories used below:

* **A** - tree display/navigation
* **B** - property/item-list helper
* **C** - material/object browser
* **D** - scene/object hierarchy
* **E** - dialog/form-only usage
* **F** - unclear, indirect, or not actually a tree seam

| Historical file(s) | Old dependency and apparent role | Class | Suggested future boundary |
|---|---|---:|---|
| `Editors/xrEProps/ItemListTypes.h`, `ItemListHelper.h/.cpp` | `ListItem` vector operations are mostly model-like; `NameAfterEdit()` reaches through `ListItem::Item()` to `TElTreeItem` for sibling uniqueness, text update, and path rename. | B | First extract rename/path/sibling rules into a GUI-independent tree model. Present results through `IEditorTree`. |
| `Editors/xrEProps/FolderLib.h/.cpp` | Central `TElTree` hierarchy helper: folder/object creation, path lookup, generated names, rename/remove, selection restore, expansion, drag/drop, popup menus, dialogs, and Win32 thumbnail drawing. | A/B/C | Split later. Path and hierarchy rules belong in `IEditorTreeModel`; selection/expansion in `IEditorTree`; confirmation in `IDialogService`. Drag/drop and drawing remain wx view work. Do not port this class wholesale. |
| `Editors/xrEProps/ItemList.h/.cpp` | Full VCL form over `TElTree`, advanced inline editor, menus, form storage, multi-select, checkboxes, thumbnails, sorting, and callbacks. | A/B/E | Model data should feed `IEditorTreeModel`; display/navigation uses `IEditorTree`. Form persistence, drawing, and editing need explicit wx implementations. Real ElPack/VCL is required for the historical path. |
| `Editors/xrEProps/PropertiesList.h/.cpp`, `PropertiesListTypes.h` | Property grid implemented with `TElTreeItem` tags, columns, owner drawing, inline number/text editors, chooser/color/vector actions, sorting, and focus callbacks. | B/E | Future `IPropertyPanel` model and editor contracts. It is not an `IEditorTree` port and currently requires ElPack/VCL. |
| `Editors/xrEProps/ChoseForm.*` | VCL chooser dialog backed by item/tree helpers. | C/E | Future browser model plus `IEditorTree`; modal behavior through a future dialog/controller boundary. Historical form still requires VCL/ElPack. |
| `Editors/ECore/Engine/GameMtlLib.h/.cpp` | Material data and serialization are mostly independent, but `_EDITOR` includes `ElTree.hpp` and adds property/editor operations. | C | Preserve the material model separately; expose materials through a future browser/tree model and properties through `IPropertyPanel`. Do not make the data library depend on a tree widget. |
| `Editors/ECore/Editor/SoundEditor.*`, `ImageEditor.*` | Asset browsers/forms with direct or helper-mediated tree selection. | C/E | Future asset-browser model plus `IEditorTree`; dialogs remain separate. Historical code requires real editor packages. |
| `Editors/LevelEditor/ObjectList.h/.cpp` | Builds folders from `EObjClass`, stores object pointers in tree items, supports multi-selection, search, visibility, locking, and scene selection. | A/D/E | Strong later consumer of a scene tree model. Use `IEditorTreeModel` for object identity/state and `IEditorTree` for presentation after scene data is decoupled. |
| `Editors/LevelEditor/EditLibrary.*`, `EditLightAnim.*` | Object/library and light-animation browsers using `TElTreeItem` selection plus item-list helpers. | A/C/E | Browser-specific models presented through `IEditorTree`; property editing through `IPropertyPanel`. |
| `Editors/LevelEditor/DOOneColor.*`, `DOShuffle.*`, `FrameAIMap.*` | Specialized forms with direct iteration, lookup, tags, selection, and drag/drop on `TElTree`. | D/E | Not a first seam. Define domain models before adding wx views; historical behavior still needs ElPack/VCL. |
| `Editors/LevelEditor/SceneProperties.*` and `Editors/LevelOptions/Editor/SceneProperties.*` | Build-options form represented as a tree with item callbacks. | B/E | Future options/property model and `IPropertyPanel`; not scene navigation. |
| `Editors/ActorEditor/BonePart.*`, `ClipEditor.*`, `LeftBar.*` | Multiple trees, multi-select, item data, and drag/drop for bones/clips plus navigation. | A/C/E | Domain models first, then `IEditorTree`; do not emulate `TElTreeDragObject`. |
| `Editors/ShaderEditor/LeftBar.*`, `SHToolsInterface.*`, `SHEngineToolsProperties.cpp` | Shader/material browser selection and property synchronization. | A/C | Material/shader browser model plus `IEditorTree` and `IPropertyPanel`. |
| `Editors/ParticleEditor/LeftBar.*`, `UI_ParticleTools.cpp` | Particle browser selection through item-list/tree helpers. | A/C | Particle browser model plus `IEditorTree`; no direct historical wiring yet. |
| `Editors/ECore/Editor/SceneClassList.h` | Object-class IDs and pick-query structures; no `TElTree` API despite being included near scene UI code. | F | Potential vocabulary/input for a later scene model, but not an ElPack adapter seam. |
| `Editors/ECore/Editor/UI_ToolsCustom.h/.cpp` | Includes `eltree.hpp` and item helpers, but owns tool actions, input, device callbacks, rendering, and property refresh rather than a tree. | F | Audit/remove transitive dependency only during a later old-code decomposition. Tool/controller interfaces are separate from `IEditorTree`. |

Duplicate generations under `Editor/` and `Editors/!old/` repeat much of this
surface. They are historical comparison sources, not additional migration
targets. The active `Editors/` generation should remain the primary map unless
a missing-file investigation proves otherwise.

## First Seam Candidate

The recommended first old-code seam is
`Editors/xrEProps/ItemListHelper.h/.cpp`, together with the data shape in
`ItemListTypes.h`.

Why this seam comes first:

* `FindItem()` and `CreateItem()` already operate on `ListItemsVec` without a
  widget;
* the ElPack leak is narrow and visible in `NameAfterEdit()`;
* the leaked behavior is domain logic worth preserving: normalized names,
  sibling uniqueness, and hierarchical path replacement;
* it is central to item, object, shader, particle, and asset browsers;
* it can be characterized with synthetic names and hierarchy data without
  compiling VCL, loading levels, or linking `xrECore`.

`EditorTreeModel` now establishes the first tiny owned-node model for labels,
categories, paths, and parent/child relationships. Its rename operation
rejects empty names and case-insensitive sibling duplicates, returns failure
reasons without invoking GUI code, and refreshes descendant paths. The current
implementation is inspired by the identified `NameAfterEdit()` needs but does
not modify, compile, or call `xrEProps`.

`wxEditorTree` renders model state through `IEditorTree` and carries only an
opaque reference to each stable model node. It does not own the model or
reproduce ElPack fields. Further migration still requires a precise audit of
historical path normalization and callback behavior before extending the
model.

F2 label editing now exercises this seam end to end. wx supplies the proposed
label, the model accepts or rejects it, and only accepted changes reach the
view and selection-driven property display. Empty and case-insensitive
duplicate sibling labels are vetoed with a reason from the model. This is a
prototype of `NameAfterEdit()`-style behavior, not a port of that function;
the preserved `xrEProps` files remain untouched.

Model-backed Add Demo Object, Add Demo Group, and Delete Selected actions now
exercise the next narrow hierarchy seam. The model generates unique sibling
names, rejects root deletion, and owns descendant destruction. The wx tree is
rebuilt after mutation, so it cannot retain item data for deleted nodes. This
prototypes a small subset of `ItemListHelper`/`FolderLib` responsibility
without copying their ElPack drag/drop, drawing, menu, or form behavior.

The same model now has a versioned `.wx_tree_snapshot` development format.
Persistence operates on model hierarchy, labels, and categories without
serializing wx or ElPack item state. Atomic load and strict duplicate/path
validation make it useful for testing the independent seam, but it is not an
X-Ray level, library, or SDK asset format and must not be treated as one.

`wxSDKEditorModelTests` now exercises this model/snapshot seam headlessly,
without wxWidgets, ElPack, VCL, `xrEProps`, or runtime data. This gives the
create/rename/delete/path and malformed-snapshot behavior a repeatable contract
before any historical editor data importer is considered.

The model also has a dependency-free `.wx_tree_paths` development importer.
It creates hierarchy and leaf categories from simple logical paths, with
implicit groups and atomic validation, without compiling or calling
`ItemListHelper`, `FolderLib`, ElPack, VCL, or runtime code. This gives future
path-rule extraction a small test surface; it is not a historical SDK importer
and does not read levels or game data.

`EditorTreePresenter` now coordinates this model with `IEditorTree`,
`IPropertyPanel`, and `IDialogService`. This removes hierarchy traversal and
mutation policy from `MainFrame` without importing historical `TElTree` or
`xrEProps` code. The separation gives future `ItemListHelper`/`FolderLib`
behavior one model-centered integration point while preserving wx as a view
adapter only.

That coordination seam is covered headlessly with fake tree, property-panel,
and dialog adapters. Tests exercise model/view/property/dialog coordination,
including delete confirmation and cancellation, root rejection, rebuild and
selection behavior, and atomic failed path-list import. This makes future
`ItemListHelper`/`FolderLib` behavior testable without wxWidgets and before any
ElPack-dependent implementation or preserved `xrEProps` source is touched.

The first type audit is also complete. Historical `EItemType` contributes only
invalid/folder/object semantics; `ListItem::type` itself is an opaque integer
whose meaning belongs to each caller. The neutral model therefore adds only
`Unknown`, structural `Root`, `Folder`, and `Object`. UI flags, callbacks,
icons, colors, object pointers, and caller-specific edit modes remain unmapped.
See [wx Editor Item Types](wx-editor-item-types.md).

The first search audit found exact vector-key lookup, exact direct-child lookup,
component-wise path navigation, folder/object restrictions, a selected-key
prefix check, and folder-first ElPack sorting. `EditorTreeQuery` extracts only a
read-only traversal/query seam; widget sorting, selection callbacks, expansion,
and drawing remain outside it. Its default case-insensitive substring search is
a documented neutral convenience rather than a historical compatibility claim.
See [wx Editor Tree Query](wx-editor-query.md).

The selection audit found full paths from `MakeFullName`, backing `ListItem`
keys, raw prefix checks, and widget-owned ordering/state. Its `bOnlyObject`
branch tests a non-null payload pointer rather than `TYPE_OBJECT`. The neutral
`EditorSelectionModel` therefore stores paths, resolves in model order, exposes
labels and paths separately, and uses optional audited kind filters. ElPack
multi-select, expansion, callbacks, and form storage remain excluded. See [wx
Editor Selection Model](wx-editor-selection.md).

This marks the wx model/presenter foundation checkpoint. Future work can map
`ItemListTypes` concepts onto neutral categories or add a read-only metadata
adapter, but should not modify `xrEProps` merely to feed the prototype. A small
path-list example belongs under documentation only if it is deliberately added
and explained. The historical Borland/VCL and real ElPack path remains separate.

`FolderLib` is the second seam, after the model exists. Its pure path and
hierarchy operations can migrate incrementally; its drag/drop, popup menu,
thumbnail drawing, and dialog behavior must be redesigned against explicit wx
view and service interfaces.

The first hierarchy-move slice is now mapped without compiling `FolderLib`.
The audit preserves its proven folder/object containment, full-subtree move,
path-change, root-level drop, current-parent rejection, and cycle-prevention
semantics. ElPack mouse hit testing, drag state, confirmation, selection,
expansion, drawing, and callbacks remain excluded. The neutral model uses a
stricter atomic collision policy: every case-insensitive destination duplicate
rejects the move instead of merging folders or skipping duplicate objects.
Selection paths remap after success, and the wx shell exposes only a
menu/choice-dialog prototype. See [wx Editor Tree Move](wx-editor-move.md).

## Explicit Non-Goals

The first property seam maps only audited value descriptors and apply
validation. It excludes ElPack rows/editors, drawing, chooser forms, callbacks,
and object bindings. See [wx Editor Properties](wx-editor-properties.md).

This map does not add ElPack headers, package libraries, VCL shims, real level
loading, or old editor calls. It does not alter the preserved Borland sources.
The model-driven demo tree in `wxSDKEditor` remains placeholder-only.

## Command Coordination Checkpoint

The neutral presenter now routes demo hierarchy and property mutations through
a bounded, snapshot-backed command history. Headless fake tree, property-panel,
and dialog adapters verify undo/redo coordination and logical path selection
restoration without wxWidgets. Delete confirmation is part of this coverage,
and a failed path-list import leaves both the old model and its history intact.

This command seam does not port `ItemListHelper`, `FolderLib`, or historical
ElPack command behavior. Preserved `xrEProps` remains untouched; real SDK data
loading and asset-aware transactions remain future work. See [wx Editor Command
History](wx-editor-command-history.md).

## Modern Layout Boundary

The wx shell now hosts its adapter views in built-in wxAUI panes. This improves
the experimental shell without mapping ElPack widget state or VCL form
persistence: scene tree, properties, and output can dock independently, while
the viewport remains a non-functional center placeholder. The AUI perspective
is user UI configuration only. Historical `xrEProps` and LevelEditor layout
code remains untouched. See [wxSDKEditor Dockable Layout](wx-editor-layout.md).

The document layer is likewise a neutral ownership boundary, not a port of VCL
document/form behavior. It coordinates only the development snapshot, neutral
model services, path, and dirty state. Real historical document formats remain
unmapped. See [wxSDKEditor Document Layer](wx-editor-document.md).

The viewport host is another modern boundary, not an ElPack or VCL mapping.
No historical renderer or editor viewport code is compiled or called. See
[wxSDKEditor Viewport Host](wx-editor-viewport.md).

The synthetic preview adapter consumes only the neutral tree model and logical
selection paths. It does not port historical viewport widgets, object classes,
or rendering behavior. See [wxSDKEditor Preview Scene](wx-editor-preview-scene.md).

Picking likewise uses neutral logical paths and screen-space preview shapes; it
does not emulate ElPack widget hit testing or historical scene ray casting.

Transforms and gizmos remain neutral development-model boundaries. Old
`xrEProps`, LevelEditor, VCL, ElPack, and runtime renderer sources are untouched.

The tool controller and placement command are new neutral seams, not emulations
of ElPack/VCL editor actions or real SDK object factories.

The synthetic catalog and Asset Browser are likewise a modern adapter seam,
not a reconstruction of `TElTree`, `ItemListHelper`, `FolderLib`, or historical
object libraries. Stable IDs and descriptor metadata let future audited model
adapters feed the same placement path without fake ElPack headers. Old
`xrEProps` and LevelEditor sources remain preserved and untouched.

The metadata bridge reads only audited build-1935 LTX text. It does not call
old object factories, `ItemListHelper`, `FolderLib`, ElPack, or VCL. The
preserved spawn frame proves enough label/identity semantics to create an inert
synthetic Spawn marker through the neutral placement seam, but not enough to
construct a historical runtime object. Real object-library and factory
semantics remain separately blocked.

The historical Scene Inspector is another neutral read-only seam. Its chunk
and object manifest is rendered by a dedicated wx tree without calling
`TElTree`, `ItemListHelper`, `FolderLib`, xrEProps, or old LevelEditor code.
This does not port historical scene editing or grant object-construction
semantics to the manifest.

The read-only historical document now transplants confirmed manifest values
through `EditorTreeModel`, properties, selection, and preview without old
`TElTree` or object classes. It remains a new adapter path, not an ElPack or
LevelEditor compatibility implementation.
Scene compression support is independent of ElPack/VCL migration. It decodes
only inert chunk bytes and does not construct old editor objects or controls.

The class-2 scene-object decoder is likewise a wx-free data transplant, not an
ElPack bridge. It does not call `ItemListHelper`, `FolderLib`, old factories,
or object-library UI. The class-1 glow decoder follows the same inert wx-free
boundary and does not use old rendering/property controls. Historical
Borland/VCL sources remain untouched.
The class-3 Light decoder follows the same boundary: only inert serialized
values reach the modern tree/properties/preview adapters. It does not invoke
`CLight`, `GameMtlLib`, ElPack controls, or old property helpers.
The class-6 Spawn Point decoder also stays below this boundary: it does not
construct `CSpawnPoint`/`CSE_*`, call factories, or expose packets to VCL
property controls.

Conversion integrity/statistics and all new lifecycle tests remain below the
same wx-free adapter boundary. They do not revive `TElTree`, `xrEProps`, VCL,
or ElPack dependencies.

The new mesh metadata, asset registry, render-scene submission, and real-bounds
preview also remain below this boundary. They are wx-free value/model code and
do not revive `TElTree`, `xrEProps`, VCL, ElPack, or the historical renderer.

Static position/index decoding, cache ownership, projection, clipping, budgets,
and semantic line generation remain in the same wx-free boundary. The concrete
wx layer only draws already-clipped lines and provides menu toggles.
