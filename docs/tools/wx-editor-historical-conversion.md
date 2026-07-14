# Historical Scene to Editable Snapshot Conversion

wxSDKEditor now has an explicit one-way migration boundary:

```text
HistoricalSceneReadOnly
  -> bounded inert conversion candidate
  -> conversion verification
  -> new dirty EditableSnapshot document
  -> explicit Save As .wx_tree_snapshot
```

**File > Convert Historical Scene to Editable Copy...** is enabled only while
a historical scene is open read-only. The dialog reports the candidate counts
and lets the user include or exclude partial specialized records and generic
unsupported placeholders. Conversion builds and verifies a separate model
before atomically replacing the editable document. Failure preserves the old
editable document.

The resulting document is `Untitled *`, has no source path, starts with an
empty selection and command history, and supports the normal editable tools.
Conversion itself is not an undoable command. Saving is available only through
the existing `.wx_tree_snapshot` Save As path. There is no `.level` save or
export command.

The report is shown read-only and copied to the Output pane. It includes the
source filename (not an absolute path), version, full/partial/placeholder/skip
counts, transform omissions, per-class totals, bounded warnings, and the
explicit warning that the editable copy cannot be saved back to `.level`.

## Production Dry Run

The report-only converter processed all 17 lawful scenes and all 22,767
records without writing a snapshot:

| Result | Count |
|---|---:|
| Scenes convertible | 17 |
| Validation failures | 0 |
| Fully converted | 17,151 |
| Partially converted | 4,839 |
| Generic placeholders | 777 |
| Skipped/malformed | 0 |
| Records without confirmed transforms | 0 |
| Estimated editable object nodes | 22,767 |
| Aggregate in-memory snapshot bytes | 15,628,873 |
| Largest single-scene snapshot | 3,108,225 bytes |

The audit verified SHA-256, size, and timestamp stability for every source
scene. No sidecar, cache, report, or converted document was written.

This is a migration aid, not lossless historical round-tripping. Raw scene
chunks, packets, attachments, unknown bodies, and referenced assets are never
copied into editable behavior.

The stabilized conversion now verifies node-owned provenance before save and
after load, supports an on-demand read-only summary, and requires deterministic
v5 save/load/save output. The 17-scene in-memory audit passed with no source
changes. See [Conversion Round Trip](wx-editor-conversion-round-trip.md).
