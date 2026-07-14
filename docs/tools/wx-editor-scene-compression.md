# Historical Scene Compression

wxSDKEditor recognizes the high bit (`0x80000000`, `CFS_CompressMark`) on an
XR chunk ID as the historical compressed-payload marker. The semantic chunk ID
is the stored ID with that bit cleared.

## Source evidence

The build-1935 path is unambiguous:

- `xrCore/FS.h` defines `CFS_CompressMark`.
- `xrCore/FS.cpp`, `IWriter::w_chunk` sends marked payloads to
  `IWriter::w_compressed`, which calls `_compressLZ`.
- `xrCore/FS.cpp`, `IReader::open_chunk` calls `_decompressLZ` for marked
  payloads before returning a child reader.
- `xrCore/LzHuf.cpp`, `Encode` and `Decode` define the exact stream.
- `xrCore/LocatorAPI.cpp`, `open_chunk` confirms the same bit-mask and decoder
  on another historical filesystem path.

This is X-Ray LZHUF: a 4096-byte LZSS ring, 60-byte lookahead,
`THRESHOLD=2`, and adaptive Huffman character coding. It is distinct from the
LZO payload path used by archive entries.

## Payload framing

The marked XR chunk still has the normal little-endian ID and compressed-size
header. Its payload is:

1. little-endian `u32` decompressed byte count;
2. adaptive-Huffman/LZSS bitstream, most-significant bit first;
3. zero padding in the final partial byte.

There is no signature, checksum, algorithm version, or external dictionary.
The LZSS dictionary starts as spaces, matching `xrCore/LzHuf.cpp::Decode`.

## Implementation

`EditorSceneCompression` is a wx-free, state-local decoder. It does not link
`xrCore`, use historical allocators, or call runtime logging. The historical
implementation remains unchanged. The scene probe decodes a marked payload
into a bounded temporary vector and applies the existing chunk parser to that
buffer. Nested marked chunks are supported because historical `IReader`
recursion permits them, but wxSDKEditor adds an explicit nesting limit.

Inspector records retain compressed and decompressed sizes, algorithm,
success/failure, physical compressed-container offset, decoded-stream offset,
and deterministic diagnostics. No decoded bytes are written to disk.

The wx-free test executable also accepts
`wxSDKEditorModelTests --probe-scene <path>` for aggregate validation. It
prints counts and byte totals only; it does not export payloads or assets.

## Current scope

Successful decoding can reveal confirmed wrappers, names, classes, and
transforms that were inside compressed containers. It does not decode
specialized object bodies, instantiate historical objects, or load referenced
assets. Since the format has no checksum, a corrupt stream that remains
structurally valid cannot be cryptographically authenticated.

The current lawful local `stalker-dream/maps/koan/lvl01-16` set contains 17
scenes, but the production probe found no marked chunks along the currently
confirmed scene/container paths. Real-file compressed-scene UI validation
therefore remains pending a lawful matching sample; decoder coverage uses only
the synthetic fixtures described above.
