# D3D11 Geometry Cache

The GPU cache is keyed by normalized renderer-neutral asset ID. One asset is
flattened into one immutable position vertex buffer and one immutable 32-bit
index buffer; all scene instances share those buffers and supply different
world constants.

Limits are 512 MiB total, 128 MiB per asset, four uploads per frame, and 8192
draw calls per frame. Selected assets and instances are ordered first. The D3D
draw loop only reads already-decoded CPU geometry with `Find`; it performs no
filesystem access or mesh decoding.

Object Library generation changes clear GPU resources. Device recreation also
starts with an empty GPU cache. Cache state is session-only and is never stored
in snapshots.
