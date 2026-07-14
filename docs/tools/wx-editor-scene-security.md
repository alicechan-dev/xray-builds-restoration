# Historical Scene Probe Security Boundary

Historical `.level` files are treated as untrusted binary input. Inspection is
explicit, bounded, read-only, and independent of the editable wxSDKEditor
document.

The probe:

- opens only the file selected by the user and never scans a runtime tree;
- validates every chunk header and payload against its parent bounds;
- caps file size, chunk count, nesting, object count, retained body bytes,
  specialized body chunks, string length, and retained diagnostics;
- rejects unterminated strings and malformed confirmed records;
- refuses compressed known scene/object containers instead of guessing a
  decompressor path;
- interprets only source-proved container contexts;
- replaces the caller manifest only after complete success;
- performs no writes and stores no source path in snapshots.

It never constructs historical objects, follows decoded reference paths, invokes
scripts, loads plugins or libraries, opens archives, or decodes object-specific
payloads. Unknown chunks are metadata, not executable or recursively trusted
content. There is intentionally no raw-hex editor and no scene save path.

Inspection failure leaves the current inspector manifest and active document
unchanged. The read-only historical document conversion is a separate,
wx-free, tested model that copies only confirmed inert values. The manifest is
not permission to instantiate runtime/editor classes, and conversion still
performs no referenced-file loading or scene writes.
Historical LZHUF decoding adds strict output-size, expansion-ratio,
scene-budget, nesting, truncation, and trailing-data checks. See
[wx-editor-scene-decompression-security.md](wx-editor-scene-decompression-security.md).
The historical `_decompressLZ` implementation is not called directly because
it allocates from the stream header and substitutes zero bits at EOF.

The class-2 decoder rejects missing/duplicate required chunks, unsupported
versions, truncated values, child-bound violations, oversized strings, and
compressed specialized children. Unknown and motion chunks are bounded
inventory only. A malformed specialized body preserves its generic wrapper.

The class-1 glow decoder applies the same body/chunk/string retention limits.
It additionally validates version-specific params sizes and finite radius or
legacy-position floats. Shader and texture strings are inert and never opened.

The class-3 Light decoder additionally caps fuzzy point count at 100 and
validates the exact shape/count/payload relation plus all retained floats.
Unknown flag bits are preserved, and animation/falloff names are never opened.

The class-6 Spawn Point decoder validates subtype exclusivity, terminated
section strings, exact packet envelopes, finite environment floats, and a
1 MiB opaque payload cap. Packet and attachment bytes are never interpreted.
