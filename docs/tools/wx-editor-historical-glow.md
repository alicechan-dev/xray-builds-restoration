# Historical Glow Decoder

## Editable Conversion Boundary

Confirmed transform, radius, shader name, texture name, and flags are retained
as inert values. Radius drives only the existing diagnostic preview marker.
Shader and texture strings are never resolved or loaded.

Class ID 1 is `OBJCLASS_GLOW`, owned by `CGlow`. The canonical build-1935
evidence is `CGlow::Load/Save` in `Editors/LevelEditor/Edit/Glow.cpp`; common
base chunks remain owned by `CCustomObject`.

## Confirmed body layout

| Chunk | Load requirement | Payload |
|---:|---|---|
| `0xC411` | required | `u16` version, `0x0012`; loader also accepts `0x0011` |
| `0xC413` | required | version `0x0012`: one `float` radius; version `0x0011`: radius then three position floats |
| `0xC414` | optional | zero-terminated shader name |
| `0xC415` | required | zero-terminated texture name |
| `0xC416` | optional | `u16` flags |

The base contributes `0xF906` flags, `0xF907` name, `0xF903` transform, and
optional motion chunks. Although current `Save` writes shader and flags,
`Load` treats both as optional; the decoder preserves that historical rule.

The read-only audit found 511 glows across all 17 lawful scenes. Every body
uses version `0x0012` and the same eight-child layout, including all five glow
chunks and three common chunks. All shader and texture strings are terminated
and nonempty. Radius values are finite from 0.5 through 5.0; all sampled flags
are zero. All 511 decode as `Supported`.

## Inert representation

`EditorHistoricalGlowRecord` exposes version, optional shader name, required
texture name, radius, optional flags, version-`0x0011` legacy position, and
per-field chunk provenance. Names are displayed only; they are never resolved
or opened. No historical class, shader, texture, sprite, renderer, factory, or
plugin is constructed.

Version `0x0011` is `Partial`: its embedded position is retained but is not
applied over the shared wrapper transform. Unknown and motion chunks are
bounded inventory and also make the result `Partial`.

The viewport uses radius only for a bounded diagnostic ring. This is grounded
in historical `CGlow::GetBox` and `CGlow::Render`, both of which use
`m_fRadius`; it does not render the referenced sprite or access assets.

The decoder enforces the shared limits of 4 MiB per body, 64 child chunks,
4,096 bytes per string, and 32 retained unknown/unsupported chunks. It rejects
missing or duplicate specialized chunks, invalid version-specific parameter
sizes, unsupported versions, compressed specialized children, unterminated
strings, non-finite radius/legacy position, and child bounds outside the body.

The later class-6 Spawn Point branch is independent; all 511 sampled Glow
records remain Supported with unchanged read-only preview policy.
