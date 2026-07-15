# wxSDKEditor Software Wireframe Renderer

The first real mesh backend is a wx-free CPU pipeline. It consumes shared
`EditorStaticAssetGeometry` and renderer-neutral `EditorRenderInstance`
records, applies instance scale/roll/pitch/yaw/translation, transforms into
camera space, clips depth and screen segments, and emits semantic 2D lines.
The wx layer only assigns colours and calls `wxDC::DrawLine`.

Each triangle emits three edges; shared edges are intentionally repeated.
Backface culling is optional and defaults off. Selected instances are processed
first and receive highlighted line metadata. Geometry is drawn before the
existing selection/gizmo/label overlays. `View -> Real Mesh Wireframe` defaults
on; `View -> Wireframe Backface Culling` is independent. Existing Object Bounds
remain the fallback for missing, unsupported, skeletal, failed, or budgeted
assets.

Picking remains the stable projected-bounds policy. Triangle and face picking,
filled rasterization, depth buffers, textures, materials, normals, shaders,
skeletal meshes, and GPU resources are deferred.

