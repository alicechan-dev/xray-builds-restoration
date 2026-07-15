# D3D11 Flat Shaders

The first backend embeds and compiles a small audited HLSL pair. The vertex
format is only three `float` position values and indices remain `uint32`.

The per-frame constant buffer contains a row-major view-projection matrix. A
reused dynamic per-object buffer contains the row-major world matrix and flat
semantic color. HLSL uses row-vector `mul(position, matrix)` explicitly.

X-Ray editor coordinates are Y-up, with yaw zero looking along positive Z.
The world transform order matches the software oracle: scale, roll about Z,
pitch about X, yaw about Y, then translation. The view and projection are
left-handed and use near/far values 0.05/5000.
