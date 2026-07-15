# Object Bounds Preview

The existing software preview remains a diagnostic placeholder, but resolved
historical SceneObjects now derive their conservative display size from real
Object Library mesh bounds and the instance scale. The implementation computes
a rotation-safe enclosing diameter and keeps the existing box-based projection;
it does not pretend to display the source mesh.

`View -> Object Bounds` controls the bounds overlay. `View -> Render Asset
Diagnostics` displays readiness and mesh/vertex/triangle counts. Selection,
picking, and framing use the same projected extent, avoiding a separate hidden
shape. Clearing the Object Library safely restores the prior fallback preview;
reloading restores real bounds.

No textures, materials, shaders, thumbnails, vertex buffers, index buffers, or
GPU resources are loaded. Real mesh rendering is a separate next pass.

