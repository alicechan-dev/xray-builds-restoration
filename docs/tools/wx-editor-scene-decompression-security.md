# Historical Scene Decompression Security

Historical scene decompression is read-only and runs before any semantic
record is accepted. The decoder replaces its caller's output only after a
complete successful decode.

Default limits are:

- compressed chunk: 64 MiB;
- decompressed chunk: 256 MiB;
- expansion ratio: 64x;
- total decompressed bytes per scene: 512 MiB;
- nested compressed depth: 8;
- existing scene limit: 256 MiB, 250,000 chunks, 100,000 objects, depth 32.

The decoder rejects a missing size header, an output size over policy,
excessive expansion, allocation failure, truncated Huffman/back-reference
data, a match crossing the declared output boundary, non-zero padding, and
trailing bytes. The probe also rejects a scene-wide budget overflow. Failed
non-critical containers are retained as inspector metadata with a diagnostic;
they are never treated as empty confirmed content. A failed critical field
causes the probe to fail atomically, preserving the previous manifest/document.

Physical source offsets remain attached to the compressed container. Records
inside decoded bytes additionally carry a decoded-stream offset and chunk
path; these values are not presented as physical file positions.

Tests use only tiny synthetic format fixtures generated from the audited
historical coding tables. No `.level` file, extracted chunk, decompressed
payload, cache, binary dump, or full binary diagnostic is committed or written.

