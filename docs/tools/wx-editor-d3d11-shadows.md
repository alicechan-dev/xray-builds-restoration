# wxSDKEditor D3D11 shadows

The wx D3D11 renderer owns one 1024x1024 typeless depth texture with D32 depth
and R32 shader views. It draws current visible static meshes to an orthographic
directional-light pass, then samples it with 3x3 PCF. Slope-scaled rasterizer
bias and receiver bias reduce acne.

Bounds come only from finite current-scene world bounds, reject unreasonably
large objects, clamp extent to 4096 units, and snap the light target to shadow
texels. This single volume has no cascades or historical sun/weather rules.
Missing, malformed, and skeletal-deferred assets keep fallbacks and do not cast
mesh shadows; runtime markers never cast shadows.
