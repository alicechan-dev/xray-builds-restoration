# wxSDKEditor Historical Scene Probe

The experimental wxSDKEditor has a wx-free, read-only probe for canonical
build-1935 `.level` scenes. It produces an `EditorSceneManifest`; it does not
produce or replace an `EditorDocument`.

## Components

- `EditorBinaryReader` provides bounded little-endian primitive reads,
  bounded slices, offset-aware errors, and capped zero-terminated strings.
- `EditorSceneManifest` stores source/format summary, chunk inventory,
  confirmed object records, and diagnostics.
- `EditorHistoricalSceneProbe` applies the audited scene structure and only
  publishes a result after the whole probe succeeds.

Default limits are 256 MiB per file, 250,000 chunks, 32 nesting levels,
100,000 objects, 4 MiB per retained object body, 128 MiB retained body bytes
per scene, 4,096 bytes per string, and 256 retained diagnostics. Parent
chunk bounds are enforced before every slice. Unsupported versions,
truncation, malformed sizes, duplicate critical fields, and compressed known
containers fail with an offset-bearing reason.

The probe confirms names and transforms only inside source-proved object
containers. Unknown chunks are counted and displayed without speculative
recursion. No X-Ray factory, script, plugin, DLL, renderer, archive, object
library, mesh, texture, or referenced path is loaded.

Headless synthetic tests cover valid empty and object scenes, deterministic
chunk paths, unknown chunks, malformed and truncated input, parent-bound
violations, unsupported versions, compressed critical chunks, resource
limits, file reads, and atomic preservation of the previous manifest after a
failed probe. Tests-only configuration does not discover or link wxWidgets.

See [Scene Format Audit](wx-editor-scene-format-audit.md), [Security
Boundary](wx-editor-scene-security.md), and [Scene
Inspector](wx-editor-scene-inspector.md).

The same atomic manifest can now feed the separate read-only historical
document mode. Probe failure still leaves both editable and historical
workspace state unchanged; conversion adds no parsing or object construction.
## Compressed chunks

The probe now decodes source-confirmed X-Ray LZHUF chunks through the wx-free
bounded decoder. It parses the decoded child buffer with the same chunk logic,
retains compression provenance, and applies per-chunk, total-byte, ratio, and
nested-depth limits. Failed non-critical payloads remain explicit diagnostics;
they are not parsed through or silently converted to empty data.

The headless executable also accepts `--audit-scenes <root>`. It recursively
selects only `.level` files, sorts them, and reports aggregate class/body
coverage without writing a report or exporting bytes. Retained bodies are
offered to the explicit class dispatcher; currently only class 2 has a decoder.
