# Active Scene Asset Working Set

The Object Library catalog, active asset working set, and render instances are
separate concepts. The working set deduplicates asset IDs referenced by the
current `EditorRenderScene`; repeated instances share one asset and selected
assets sort first. Unused catalog assets are not decoded or uploaded for the
open scene.

The lawful production audit found 627 catalog assets, 562 assets used across
17 scenes, and 65 unused assets. `l01_escape.level` uses 114 unique static
assets. The largest scene uses 125 unique assets and an estimated 23,852,952
bytes of position/index data.

The set is session-only and rebuilds whenever render-scene submission or the
Object Library generation changes. Skeletal, missing, and unsupported assets
remain explicit fallback categories.
