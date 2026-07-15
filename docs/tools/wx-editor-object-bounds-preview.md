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

Object Bounds remain independently toggleable and remain the picking/fallback
shape after real static wireframe was added. No textures, materials, shaders,
thumbnails, GPU vertex/index buffers, or other GPU resources are loaded.
