# Historical Conversion Policy

`EditorHistoricalConversionPolicy` is wx-free and deterministic. Its default
options include supported records, partial specialized records, generic
placeholders, and bounded source provenance.

| Source state | Default disposition | Behavior |
|---|---|---|
| Supported specialized record | Fully Converted | Confirmed inert fields and transform are copied. |
| Partial specialized record | Partially Converted | Safe fields are copied and omitted behavior is stated explicitly. |
| Unsupported generic record | Placeholder Converted | Name, class ID, confirmed transform, and warning are retained. |
| Malformed record | Skipped | Diagnostic/count only; no editable node. |

Partial and placeholder inclusion can be disabled independently. Malformed
records are never promoted by an option. Records without a confirmed transform
remain in the hierarchy but are omitted from preview; no origin placement is
invented.

Class mappings are deliberately narrow:

- Scene Object: inert reference, flags, transform, and motion/retained warning.
- Glow: radius, shader/texture names, flags, and diagnostic radius; no lookup.
- Light: confirmed scalar values and range, with fuzzy/animation warnings.
- Spawn Point: section, subtype, flags, respawn/environment scalars, and only
  packet/attachment size summaries.
- Minor generic classes: clearly marked unsupported placeholders only.

No converter path invokes historical classes, factories, scripts, runtime
configuration, renderers, resource managers, or filesystem asset resolution.
