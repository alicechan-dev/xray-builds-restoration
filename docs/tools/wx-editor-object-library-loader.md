# Object Library Loader

`EditorObjectLibraryLoader` is wx-free, read-only, streaming, and atomic. A
user must explicitly select a root. Files are deterministically sorted by
normalized reference and source-relative path. Malformed individual entries
remain visible with bounded diagnostics, while root traversal, containment,
and global-limit failures preserve the previously loaded session catalog.

The loader never writes, builds an index on disk, expands environment values,
executes plugins, or opens referenced resources. It seeks over mesh payloads
and reads only chunk headers and confirmed metadata fields.

