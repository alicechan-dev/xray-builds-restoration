# Object Library Security

Default limits are 250,000 files, 96 MiB per object, 2 GiB total source size,
100,000 chunks per file, 16 KiB strings, 100,000 mesh/surface metadata records,
256 retained references per kind, and 128 diagnostics per entry.

The loader rejects absolute, drive-qualified, traversal, control-character,
empty-component, and colon-bearing references. It does not follow symlinks;
canonical containment also rejects junction/reparse escapes. All chunk sizes,
container ranges, counts, and additions are bounds checked. Resolution performs
no filesystem access after loading.

No object files, indexes, reports, snapshots, dumps, logs, or external assets
belong in version control.

