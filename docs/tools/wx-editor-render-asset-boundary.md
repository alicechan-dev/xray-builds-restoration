# wxSDKEditor Render Asset Boundary

`EditorRenderObjectAsset` is the renderer-neutral boundary between the
read-only Object Library and a future preview backend. It carries normalized
asset identity, static/skeletal kind, readiness, aggregate bounds, mesh and
surface counts, total vertex/triangle counts, source-relative provenance, mesh
metadata, and bounded diagnostics.

The boundary deliberately owns no graphics API objects. It contains no device,
buffer, texture, shader, material, thumbnail, renderer-module, or runtime-game
handle. The current decoder also does not retain full geometry. This makes the
asset inventory testable without wxWidgets and prevents an experimental editor
from coupling to `xrRender_R1`, `xrRender_R2`, or historical runtime ownership.

Readiness is explicit:

- `BoundsOnly`: usable bounds, no decoded mesh metadata.
- `StaticGeometryMetadataReady`: mesh metadata exists but is not a proven
  static decode candidate.
- `StaticGeometryDecodeCandidate`: static object with validated supported mesh
  metadata; full geometry is still not decoded.
- `SkeletalDeferred`: skeletal object is inventoried but decoding is deferred.
- `Unsupported`: no safe supported representation is available.
- `Malformed`: structural validation failed.

The production registry starts with 583 static decode candidates and 44
skeletal deferred assets. A successful lazy request transitions a candidate to
`StaticGeometryDecoded`; geometry remains shared in the session cache and is
rendered only as CPU wireframe. Skeletal assets never make that transition.
