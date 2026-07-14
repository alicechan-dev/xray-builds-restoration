# Historical Object Reference Resolution

Normalization follows the historical editor: `/` and `\\` become backslash,
ASCII letters become lowercase, and one terminal `.object` extension is
removed. Identity is relative to the selected object root. Original spelling
is retained separately. Absolute paths, drive prefixes, traversal, empty path
components, controls, and colons are invalid; collisions remain ambiguous.

Resolver states are Library Not Loaded, Resolved, Missing, Ambiguous, and
Invalid. Lookup is an in-memory operation. Converted CSceneObject properties
show this state plus matched ID, type, counts, source-relative file, and parser
status. Loading or clearing a library changes only application-session state:
it does not dirty the document, create command history, alter origin metadata,
or modify snapshot v5.

