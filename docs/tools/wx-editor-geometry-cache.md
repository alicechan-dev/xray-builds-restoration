# wxSDKEditor Geometry Cache

`EditorRenderGeometryCache` is presenter-owned, session-only derived state. It
is bound to the explicitly selected Object Library root and asset registry,
keys entries by normalized asset ID, and lazily decodes a static asset once on
its first visible request. Repeated requests are memory-only cache hits.

The default cache permits 512 entries, 96 MiB per asset, and 256 MiB total
position/index storage. It does not evict in this first pass; requests beyond a
limit retain bounds fallback and a diagnostic result. Success transitions asset
readiness from `StaticGeometryDecodeCandidate` to `StaticGeometryDecoded`.
Unsupported or malformed results are cached as failures. Clear restores each
entry's original readiness and releases all geometry; Object Library reload
clears the old cache before rebuilding the registry.

The cache is not serialized, does not alter command history or dirty state,
and performs no writes. Rendering may cause a first lazy read, but no further
filesystem access occurs for a cached asset.

For D3D11, active-scene assets are prepared on scene submission and uploaded
under a per-frame budget. Camera-only frames use `Find` and never decode.
