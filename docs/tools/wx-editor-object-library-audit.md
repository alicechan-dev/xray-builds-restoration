# Historical Object Library Audit

The canonical build-1935 editor library is the `_objects_` virtual root used
by `Editors/ECore/Editor/Library.cpp`. `GetObjects` recursively enumerates
`.object` files and `LoadEditObject` appends `.object` to an extensionless
reference. The older `Editor/LevelEditor/Edit/Library.cpp` and
`FileSystem.cpp` confirm relative-folder identity, extension removal,
lowercasing, and Windows backslash semantics. There is no central catalog.

The explicitly selected lawful local candidate
`D:/Projects/Github/stalker-dream/objects` contains 627 files in a nested
folder hierarchy (1,548,196,711 source bytes). The repository and
`D:/Projects/Others` contain no `.object` files. The largest candidate is
about 72.5 MB, which is why the bounded loader uses a 96 MiB single-file cap.

The final read-only production audit loaded 627 entries: 581 supported, 46
partial, zero malformed, and zero duplicate normalized IDs. It inventoried
2,418 meshes, 7,686,510 vertices, and 12,447,670 triangles without retaining
geometry payloads. All meshes use version `0x0011`; none were malformed. The
103 observed `0x1003` chunks are the source-declared retired slot. The reader
fetched 46,506,856 bytes from 1,548,196,711 source bytes.

All 15,416 CSceneObject references in the 17 lawful `.level` files resolved,
with zero missing, ambiguous, invalid, or fallback-bounds instances. The asset
registry classified 583 static objects as geometry decode candidates and 44
skeletal objects as deferred. Before/after SHA-256, size, and UTC timestamp
checks matched for all 627 `.object` and 17 `.level` files. No generated report
or local absolute root is committed.
