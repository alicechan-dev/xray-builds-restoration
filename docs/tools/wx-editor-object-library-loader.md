# Object Library Loader

`EditorObjectLibraryLoader` is wx-free, read-only, streaming, and atomic. A
user must explicitly select a root. Files are deterministically sorted by
normalized reference and source-relative path. Malformed individual entries
remain visible with bounded diagnostics, while root traversal, containment,
and global-limit failures preserve the previously loaded session catalog.

The loader never writes, builds an index on disk, expands environment values,
executes plugins, or opens referenced resources. It streams confirmed mesh
records with bounded seeks and reads to validate counts and aggregate bounds;
it never retains untrusted geometry payloads. Successful load is followed by
transactional construction of the session-only render asset registry.
