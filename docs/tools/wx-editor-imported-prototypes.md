# wxSDKEditor Imported Spawn Prototypes

## Historical Evidence

The preserved `Editors/LevelEditor/FrameSpawn.cpp` enumerates configuration
sections containing `$spawn`, reads the value with `CInifile::r_string_wb`,
uses that value as a backslash-delimited browser label, and retains the section
name as the item payload. `r_string_wb` removes the surrounding quotes. This
proves an editor label hierarchy and section identity; it does not prove a
runtime class, visual, factory call, or construction policy.

A read-only audit of the local extracted build-1935 metadata found 188 `$spawn`
values: all were quoted, 186 used backslash hierarchy, and two were root labels.
The runtime tree was not modified and no audited data is stored in the repo.

## Conservative Classification

`EditorImportedPrototype` maps exactly one well-formed `$spawn` record to:

- stable ID `imported.section.<lowercase-section>`;
- display name `<spawn-leaf> [<section>]`;
- browser category `Imported/<first-spawn-segment>`;
- a deterministic node name derived from the section;
- inert `Spawn` preview and placement metadata.

Section identity is limited to ASCII letters, digits, `.`, `_`, and `-`.
`$spawn` must be quoted, nonempty, relative, backslash-delimited, and free of
empty, `.` or `..` segments, control characters, drive prefixes, colons, and
forward slashes. Ambiguous duplicate `$spawn` records and invalid values remain
browseable but are non-placeable and carry an explicit diagnostic.

This is a metadata prototype only. No object factory, script, mesh, texture,
archive, level, or runtime class is loaded.
