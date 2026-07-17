# wxSDKEditor Object Library configuration

`Configure...` selects an SDK data root once. It may contain `_objects_` or the
lawful build-1935 `objects` layout, or may be that directory itself. Its
canonical path is stored only in the user's `wxConfig` profile and is never
serialized into snapshots or committed files.

Startup restores and validates the path and builds the read-only metadata index
without decoding geometry or uploading GPU buffers. If no root exists when a
level opens, discovery checks only five ancestors of that explicitly opened
scene. A unique candidate requires confirmation before persistence; absence or
ambiguity leaves the level open with fallbacks. No drive-wide/network scan or
script, DLL, or plugin loading occurs.
