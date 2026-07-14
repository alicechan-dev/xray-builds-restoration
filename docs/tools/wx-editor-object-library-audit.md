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

The read-only production audit loaded 627 entries: 581 supported, 46 partial,
zero malformed, and zero duplicate normalized IDs. It resolved all 15,416
CSceneObject references in the 17 lawful `.level` files, with zero missing,
ambiguous, or invalid references. The streaming reader fetched 793,385 bytes
of metadata and skipped mesh payloads. This report is not committed as a
generated inventory and the local absolute root is not serialized.

