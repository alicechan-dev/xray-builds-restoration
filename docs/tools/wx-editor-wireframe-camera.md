# wxSDKEditor Wireframe Camera

The wireframe backend has an explicit finite perspective camera contract:
position, yaw/pitch in degrees, vertical field of view, near/far planes, and
viewport dimensions. The wx viewport maps its existing camera state into a
60-degree vertical FOV with near `0.05` and far `5000.0` defaults. Yaw zero
looks along positive Z. Historical object rotation values remain radians and
are applied roll-Z, pitch-X, then yaw-Y after scale.

Segments are clipped against near and far depth planes before projection, then
against the viewport rectangle with a bounded Liang-Barsky pass. Zero-sized
viewports, invalid planes/FOV, and non-finite camera values produce an empty
invalid frame. Projection never sends unbounded coordinates to wx drawing APIs.

The old diagnostic preview and bounds picker keep their established projection
for this pass. Real wireframe is visual-only, so camera/picking unification is
a later independent task.

