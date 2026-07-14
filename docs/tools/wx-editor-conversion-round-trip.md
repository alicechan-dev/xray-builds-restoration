# wxSDKEditor Conversion Round Trip

The lawful production audit remains read-only. For each historical scene it
opens and converts the manifest, verifies origins, serializes snapshot v5 to
memory, reloads it, verifies again, and requires byte-identical reserialization.
No temporary snapshot or `.level` output is written.

## July 2026 checkpoint

- Scenes processed/passed: 17/17
- Converted records: 22,767
- Fully converted: 17,151
- Partially converted: 4,839
- Placeholders: 777
- Skipped: 0
- Reload failures: 0
- Origin-integrity failures: 0
- Semantic/byte-equivalence failures: 0
- Serialized bytes: 16,343,204 total
- Per-scene serialized size: 127,342 minimum, 3,242,595 maximum
- Source file changes: 0 across SHA-256, size, and UTC timestamp checks

The audit reads lawful local files from outside the repository. Reports and
snapshots are not committed. Conversion remains one-way and no historical
scene writer or runtime entity construction is present.
