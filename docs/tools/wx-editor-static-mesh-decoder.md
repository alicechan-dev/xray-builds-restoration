# wxSDKEditor Static Mesh Decoder

The wx-free `EditorStaticMeshDecoder` reads only build-1935 static mesh version
`0x0011`. It follows `Editors/ECore/Editor/EditMeshIO.cpp`: object chunk
`0x0910` contains numbered meshes, `0x1005` stores a 32-bit vertex count,
12-byte `Fvector` positions and adjacency lists, and `0x1006` stores a 32-bit
triangle count followed by 24-byte `st_Face` records. Direct signed vertex
indices occur at byte offsets 0, 8, and 16. Their serialized order is retained.

Decoding is atomic and rejects truncated ranges, overflow, empty or excessive
counts, non-finite positions, unsupported versions, count mismatches, and
negative or out-of-range indices. Defaults cap files and decoded assets at 96
MiB, meshes at 4,096, and vertices or triangles at 4,000,000 per mesh.
Adjacency and VM-reference values are validated by the metadata stage but are
not retained by the geometry stage.

Degenerate and zero-area triangles are counted and preserved. Serialized and
computed bounds are compared with a relative tolerance; a mismatch is a
diagnostic rather than an invented geometry repair. No texture, UV, material,
normal, shader, motion, sound, thumbnail, or skeletal payload is loaded.

The lawful audit decoded all 583 static candidates: 7,685,336 vertices and
12,446,276 triangles, with zero unsupported/malformed assets, invalid indices,
non-finite positions, or bounds mismatches. It observed 959 repeated-index
degenerates and 984 zero-area triangles. Position/index storage totals
241,579,344 bytes; the largest decoded asset is 11,321,472 bytes.

The D3D11 backend consumes the same validated positions and uint32 indices.
Decode remains CPU-owned and outside the per-frame D3D draw loop.
