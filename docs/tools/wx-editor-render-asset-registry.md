# wxSDKEditor Render Asset Registry

`EditorRenderAssetRegistry` is session-only derived state. The presenter builds
it immediately after a successful Object Library load. Construction uses a
temporary candidate and replaces live state only after all entries and count
aggregates are valid. If registry construction fails, the just-loaded Object
Library is cleared rather than leaving mismatched state.

Normalized historical reference IDs index immutable render-asset records.
Duplicate normalized IDs retain the first deterministic Object Library entry,
matching the resolver's need to treat ambiguous references separately. Clearing
the Object Library also clears the registry, refreshes selection properties,
and rebuilds the preview with its safe fallback representation. Reloading
rebuilds the registry and restores real bounds.

The registry is not serialized into `.wx_tree_snapshot` files and does not
modify `.object` or `.level` files. Synthetic tests cover build, lookup,
readiness aggregation, clear, and preview fallback after clear.

