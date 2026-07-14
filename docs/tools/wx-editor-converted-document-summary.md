# wxSDKEditor Converted Document Summary

`Tools -> Historical Conversion Summary...` is enabled only for an editable
snapshot containing historical-origin nodes. Statistics are derived on demand
from the wx-free model and verifier.

The dialog and Output pane show the source filename (never an absolute path),
current node count, historical-origin count, newly added object count,
Full/Partial/Placeholder counts, warning and missing-transform counts,
per-class totals, and origin-integrity status. Deleted historical objects are
reported as not tracked because the editor intentionally has no tombstone or
mutation audit log.

Every summary includes the warning that the document cannot be exported back
to historical `.level` format. It never displays packet bytes, unknown chunks,
or source paths.
