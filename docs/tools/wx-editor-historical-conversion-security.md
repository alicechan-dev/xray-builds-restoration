# Historical Conversion Security

Conversion operates only on the already bounded, inert
`EditorHistoricalSceneDocument`. It never reopens the source path and never
requires that path after conversion.

Before replacement, `VerifyHistoricalSceneConversion` checks:

- unique canonical editable paths;
- finite editable transforms;
- unique retained stable origin IDs;
- origin-node count matching the report;
- explicit unsupported category for placeholders;
- bounded field, warning, and opaque summaries.

Report lists are deterministic, unique, and capped at 64 entries. Individual
report text is capped at 512 bytes; node field/warning summaries are capped at
1 KiB and opaque summaries at 512 bytes. Opaque data is represented only by
validated size/status text produced by the specialized decoders.

Candidate construction, verification, snapshot parsing, and destination
replacement are atomic. A failure leaves the source historical document and
the previous editable document unchanged. Conversion itself performs no file
write. Only a later explicit `.wx_tree_snapshot` Save As writes a file.

The security boundary prohibits `.level` saving/export, packet parsing,
attachment copying, unknown-body retention, asset loading, factory calls,
scripts, historical/runtime object construction, and source sidecars/caches.

Snapshot-v5 origin records are checked at save/load boundaries. Structural
corruption, duplicate IDs, invalid dispositions/classes, oversized fields, and
non-allowlisted opaque markers fail atomically. Optional summaries may be
empty; bounded `x_*` extensions are ignored. See [Converted Document
Validation](wx-editor-converted-document-validation.md).
