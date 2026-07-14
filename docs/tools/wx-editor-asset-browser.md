# wxSDKEditor Asset Browser

The existing Asset Browser continues to own synthetic descriptors and imported
LTX spawn metadata. Historical `.object` entries live in the separate Object
Library pane so the two provenance and placement models are not conflated.

The Asset Browser is a modest concrete wxWidgets view over the wx-free
synthetic catalog. It contains a search field, category selector, asset list,
description area, and **Place Selected** command. Double-clicking an entry has
the same activation behavior. Search is case-insensitive and category filters
compose with the search result.

The pane is hosted by wxAUI as `asset_browser`, docks on the left by default,
and is closable and resizable. **View > Asset Browser** controls visibility and
tracks the pane state. The existing saved perspective includes the pane, and
Reset Layout restores the default placement.

Activation selects the stable asset ID in `EditorTreePresenter`, switches to
the generic `PlaceAsset` tool, and supplies the descriptor to the viewport.
Mouse motion previews the descriptor's semantic marker and default Y value;
the document is changed only when a valid left click commits one placement
command. Escape returns to Select without creating a node.

This first browser has no thumbnails, filesystem scanning, real SDK classes,
or X-Ray asset loading. The catalog remains available across document
operations; New/Open/Import reset the active tool to Select while the browser
selection can remain visible.

The browser also provides **Load Metadata...** and **Clear Imported**. Loading
requires explicit root and entry-file choices. Categories are distinguished as
`Synthetic/` and evidence-derived `Imported/<spawn-group>` paths. Imported
details show source file, section, line, raw `$spawn`, placeability, and policy
reason. Safe descriptors enter generic inert Spawn placement; invalid or
ambiguous descriptors report their read-only reason instead.
