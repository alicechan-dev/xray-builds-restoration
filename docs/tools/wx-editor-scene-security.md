# Historical Scene Probe Security Boundary

Historical `.level` files are treated as untrusted binary input. Inspection is
explicit, bounded, read-only, and independent of the editable wxSDKEditor
document.

The probe:

- opens only the file selected by the user and never scans a runtime tree;
- validates every chunk header and payload against its parent bounds;
- caps file size, chunk count, nesting, object count, string length, and
  retained diagnostics;
- rejects unterminated strings and malformed confirmed records;
- refuses compressed known scene/object containers instead of guessing a
  decompressor path;
- interprets only source-proved container contexts;
- replaces the caller manifest only after complete success;
- performs no writes and stores no source path in snapshots.

It never constructs historical objects, follows referenced paths, invokes
scripts, loads plugins or libraries, opens archives, or decodes object-specific
payloads. Unknown chunks are metadata, not executable or recursively trusted
content. There is intentionally no raw-hex editor and no scene save path.

Inspection failure leaves the current inspector manifest and the editable
development document unchanged. A future import or conversion feature must be
a separate audited operation with its own model and tests; the manifest is not
permission to instantiate runtime/editor classes.

