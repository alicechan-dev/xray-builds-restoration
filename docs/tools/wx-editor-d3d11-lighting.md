# wxSDKEditor D3D11 lighting

The standalone wx D3D11 backend renders neutral editor colors with one fixed
directional light, 0.32 ambient intensity, and 0.82 Lambert diffuse intensity.
Selected objects retain an orange semantic tint. `Shaded Mesh Preview` can
switch to unlit neutral color without changing the scene.

Build-1935 meshes expose positions, indexed faces, adjacency, and a smoothing
group chunk, but no confirmed explicit normal stream. Normals are generated
once during bounded static-geometry decode by deterministic area-weighted
accumulation over preserved winding. Zero-area triangles are ignored and
invalid/non-finite input fails safely. This is a conservative preview policy,
not historical material recovery. UVs, textures, tangents, specular maps,
transparency, and historical shaders are not used.
