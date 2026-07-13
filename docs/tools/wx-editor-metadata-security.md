# wxSDKEditor Metadata Import Security

Import is opt-in and read-only. The user chooses both a root directory and an
entry `.ltx`; wxSDKEditor never scans a runtime tree automatically.

The canonical root bounds the entry and every include. Relative includes are
resolved from the including file. Absolute/rooted includes and canonical paths
escaping through `..`, junctions, or symlinks are rejected. There is no
environment expansion, command execution, script evaluation, DLL loading,
archive access, network access, or write path.

Default limits are 16 include levels, 256 files, 2 MiB per file, 16 MiB total,
and 16 KiB per line. The active include stack detects cycles. Critical failures
are atomic and preserve the previous imported catalog. Successful loads report
compact totals and at most 20 file/line diagnostics in Output; source contents
are never dumped wholesale.

Headless tests create and remove private temporary roots. They cover nested
includes, cycles, traversal, absolute paths, depth/file/byte limits, malformed
records, duplicate behavior, deterministic ordering, and atomic failure. They
do not read `stalker-dream` or another runtime-data folder.

