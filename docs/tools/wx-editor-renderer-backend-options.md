# wxSDKEditor Renderer Backend Options

The next backend should consume `EditorRenderAssetRegistry` and
`EditorRenderScene` without linking the experimental editor to runtime-heavy
renderer targets.

Ranked options:

1. **Standalone software/wireframe geometry backend.** Lowest integration risk,
   headless-testable, and sufficient to validate bounded static vertex/index
   decoding, transforms, clipping, and selection before GPU ownership exists.
2. **Standalone D3D11 backend.** Good native Win32 tooling and a clean modern
   device model, but introduces GPU lifecycle and shader work too early for the
   first geometry-decoding pass.
3. **Standalone OpenGL backend.** Portable in principle, but adds context and
   loader choices without helping historical format validation.
4. **Historical X-Ray renderer reuse.** Highest coupling and ownership risk;
   it would pull runtime assumptions into the editor and is not recommended for
   this restoration stage.

The recommended next pass is a bounded static position/index decoder feeding a
standalone software wireframe backend. Skeletal geometry, UV/material binding,
textures, shaders, thumbnails, and GPU resources remain deferred.

