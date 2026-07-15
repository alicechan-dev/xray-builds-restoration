# Object Library Browser

The dockable **Object Library** pane is separate from the spawn-metadata Asset
Browser. It offers explicit Load/Clear actions, filtering, a read-only entry
list, source-relative provenance, confirmed counts/flags/references, parser
status, and diagnostics. Its wxAUI pane state participates in the normal
perspective lifecycle.

Activation reports that placement is deferred. There are no thumbnails,
previews, resource loads, or generic-box placement claims. **Tools > Object
Library Summary...** reports bounded aggregate counts.

The summary now also reports renderer-neutral readiness and aggregate mesh,
vertex, and triangle counts. These are metadata diagnostics, not thumbnails or
real mesh rendering. Selection properties expose the resolved asset readiness
and real bounds without loading referenced resources.

The renderer builds a separate active-scene working set from catalog IDs; it
does not upload all 627 catalog entries. Used/missing/deferred browser filters
remain a follow-up UI refinement.
