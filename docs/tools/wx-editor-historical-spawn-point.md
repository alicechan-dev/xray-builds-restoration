# Historical Spawn Point Decoder

## Editable Conversion Boundary

The one-way converter preserves the confirmed transform, section, subtype,
flags, and respawn/environment scalars. Runtime packet and attachment content
is not copied; only validated byte counts and an explicit omission warning are
stored in read-only origin metadata. No entity, factory, config, script, or
visual is resolved. Runtime-entity records remain partial conversions.

Class ID 6 is `OBJCLASS_SPAWNPOINT`, owned by `CSpawnPoint`. The class enum is
in `Editors/ECore/Editor/SceneClassList.h`; the canonical body contract is in
`Editors/LevelEditor/Edit/SpawnPoint.cpp` and `SpawnPoint.h`. Editor subtype
values are defined in `Editors/LevelEditor/Edit/LevelGameDef.h`.

## Source Boundary

`CSpawnPoint::Save` writes the shared `CCustomObject` chunks and version
`0x0014`. A runtime-entity record writes an entity section string and a
length-prefixed `NET_Packet`. Historical load calls `create_entity(section)`
and then `ISE_Abstract::Spawn_Read(packet)`. Rendering may load a visual,
motion, animation, icon, or attached shape. None of those operations are part
of this decoder.

Dependency-free legacy subtypes are `ptRPoint` (0) and `ptEnvMod` (1). A valid
runtime entity is represented as `ptSpawnPoint` (2), implied by the presence
of entity-reference and packet chunks rather than a serialized type chunk.

## Confirmed Layout And Safety Tiers

| Chunk | Historical field | Layout | Presence | Safety tier | Decision |
|---:|---|---|---|---|---|
| `0xE411` | version | `u16` | required | Safe Inert Scalar | require `0x0014` |
| `0xE417` | editor subtype | `u32` | legacy only | Safe Inert Scalar | decode 0/1; type 2 requires entity layout |
| `0xE413` | respawn metadata | `u8 team`, `u8 type`, `u16 reserved` | subtype 0 | Safe Inert Scalar | decode exactly |
| `0xE422` | environment modifier | 3 floats, color, float, 2 colors | subtype 1 | Safe Inert Scalar | decode finite floats and raw colors |
| `0xE419` | entity section | stringZ | runtime entity | Safe Inert String | expose read-only; never resolve |
| `0xE420` | server spawn packet | `u32 size` plus bytes | runtime entity | Runtime-Coupled / Retained Opaque Data | validate envelope and report size only |
| `0xE421` | attached object | nested serialized object bytes | optional | Retained Opaque Data | report size only |
| `0xE418` | flags | `u32` | optional legacy constant | Safe Inert Scalar | retain raw when present |

Shared motion chunks remain bounded opaque metadata. No independent game-type
mask or custom-data field is serialized by `CSpawnPoint::Save`; such data may
exist inside the runtime packet and is deliberately not inspected.

## Production Audit

The bounded read-only audit scanned all 4,334 class-6 records in 17 lawful
local scenes. Every record is named, transformed, and version `0x0014`; body
sizes are 131 to 951 bytes.

- 4,327 runtime-entity records contain terminated section strings and exact
  packet envelopes. There are 101 distinct sections, string lengths 5 to 31,
  and opaque packet sizes 64 to 820 bytes.
- 244 runtime entities also contain an opaque attachment of 185 to 189 bytes.
- 7 records are environment modifiers with type 1 and 28-byte data. Radius is
  4 to 50; power is 1; view distance is 300; fog density is 1. All floats are
  finite.
- No respawn-point subtype, specialized flags chunk, unknown child, malformed
  field, or unsupported version occurred.

The decoder classifies the 7 dependency-free environment modifiers as
`Supported`. The 4,327 runtime entities are `Partial` because their packet is
factory-dependent opaque data. There are zero `Unsupported` and zero
`Malformed` real records.

## Read-Only Projection

Properties and Scene Inspector expose subtype, inert section, packet and
attachment sizes, safe subtype values, raw colors/flags, provenance, retained
chunk counts, and diagnostics. They never display packet bytes or resolve a
section. Preview uses the existing semantic Spawn cross marker and may append
the inert section to its label. It does not instantiate an entity, load a
visual, draw an attached shape, run a script, or infer faction/team data.

Limits remain stricter than the observed data: 4 MiB body, 64 child chunks,
4,096-byte strings, 1 MiB per opaque packet/attachment, and 32 retained
unknown/unsupported chunks. Chunk bounds, duplicates, required layouts,
versions, subtype conflicts, packet lengths, strings, and finite floats are
validated. Malformed specialized data preserves the generic record.

This historical decoder is separate from the editable imported-LTX `$spawn`
prototype workflow. It cannot place, edit, save, or export historical spawn
records. With classes 1, 2, 3, and 6 decoded, 21,990 of 22,767 observed records
(96.6%) have specialized read-only coverage. The next migration stage should
stabilize this four-class document/model boundary before adding minor classes.
