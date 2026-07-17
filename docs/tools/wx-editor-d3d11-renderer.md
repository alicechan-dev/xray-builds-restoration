# wxSDKEditor Direct3D 11 Renderer

The wx-only backend now consumes position-plus-generated-normal streams,
renders neutral directional shading, and optionally performs one directional
shadow-map pass before the main pass. It remains independent of every
historical renderer/runtime class and preserves Software Diagnostic fallback.

The opt-in Win32 `wxSDKEditor` now owns a standalone Direct3D 11 viewport
backend. It uses Windows SDK `d3d11`, `dxgi`, and `d3dcompiler` only; it does
not link or instantiate `xrRender_R1`, `xrRender_R2`, `XR_3DA`, or `xrGame`.

The CPU still builds renderer-neutral instances, performs coarse bounds
culling, and manages decoded geometry. D3D11 performs vertex transformation,
triangle rasterization, depth testing, and presentation. Camera changes update
a small constant buffer and reuse resident immutable geometry.

Current rendering is deliberately flat: opaque static triangles, semantic
selection color, optional wireframe overlay, and configurable backface
culling. Historical textures, materials, shaders, skeletal meshes, animation,
and triangle picking are not implemented.

The software diagnostic renderer remains selectable under **View > Renderer
Backend** and is the automatic fallback when D3D11 initialization or present
fails.
