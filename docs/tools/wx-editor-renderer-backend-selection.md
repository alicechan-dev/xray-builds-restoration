# Renderer Backend Selection

**View > Renderer Backend** selects Direct3D 11 or Software Diagnostic for the
current session. Direct3D 11 is the preferred default and software rendering
is retained as a correctness oracle and automatic failure fallback.

Backend selection does not mutate the document, command history, conversion
metadata, or snapshots. Additional session-only D3D controls provide filled
mesh preview, wireframe overlay, backface culling, and Isolate Selected.
