# D3D11 Viewport Performance

The old interactive path transformed, clipped, projected, and emitted up to
450,000 individual wx line primitives every frame. That CPU work and the large
number of GDI calls caused camera-rotation latency.

The D3D11 path reuses immutable GPU geometry. A camera-only frame updates one
matrix buffer, performs coarse per-instance bounds culling, submits resident
draws, and presents. Diagnostics report CPU preparation milliseconds, working
set and resident assets, uploads, draws, instances, triangles, and fallbacks.

On the local production `l01_escape.level` validation, the active set was 114
assets and approximately 660,000 visible triangles rendered with CPU frame
preparation around 2-3 ms. Isolate Selected reduced preparation to about 0.2
ms. These are local observations, not a portable benchmark.
