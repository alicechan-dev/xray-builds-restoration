# Historical Light Decoder

Class ID 3 is `OBJCLASS_LIGHT`, owned by `CLight`. The canonical enum is in
`Editors/ECore/Editor/SceneClassList.h`; serialization is in
`Editors/LevelEditor/Edit/ELight_IO.cpp`, with parameter structures in
`Editors/ECore/Editor/ELight_def.h` and
`Editors/ECore/Engine/_d3d_extensions.h`.

## Confirmed Body Layout

`CLight::Save` writes the shared `CCustomObject` name, transform, and flags,
then these specialized chunks:

| Chunk | Field | Wire type | Presence | Decoder decision |
|---:|---|---|---|---|
| `0xB411` | version | `u16` | required | versions `0x0010` and `0x0011` accepted |
| `0xB442` | current parameters | 48 bytes | current layout | decoded exactly |
| `0xB435` + `0xB425` | legacy `Flight` + brightness | 104 + 4 bytes | legacy fallback | decoded, embedded placement retained inert, `Partial` |
| `0xB436` | use in D3D | 32-bit `BOOL` | required | retained as raw `u32` |
| `0xB413` | flags | `u32` | optional | retained exactly, including unknown high bits |
| `0xB441` | light control | `u32` | optional | retained exactly |
| `0xB438` | animation reference | zero-terminated string | optional | inert metadata, `Partial`; never resolved |
| `0xB439` | falloff texture | zero-terminated string | optional | inert metadata; never resolved |
| `0xB440` | fuzzy placement | shape/radius/box/count/points | optional | structurally validated and summarized, `Partial` |
| `0xB437` | legacy rotation | three floats | optional legacy | retained unsupported, `Partial` |

The 48-byte current parameter block is `u32 type`, four color floats,
brightness, range, three attenuation floats, cone, and virtual size. Types 1
and 2 are Point and Spot. Type 3 is recognized but returns `Unsupported`
because the historical loader explicitly rejects directional lights. Other
values are malformed.

Fuzzy placement is one shape byte (0 sphere, 1 box), one radius float, three
box-dimension floats, a signed 16-bit point count, then that many three-float
positions. The decoder validates the complete payload and finite values but
does not retain or generate positions.

## Production Audit

The bounded read-only audit scanned 1,729 light records in all 17 lawful local
scenes. All had names, transforms, version `0x0011`, point type, the 48-byte
current parameter block, flags, use-in-D3D, and light-control chunks. Body
sizes were 171 to 327 bytes. No animation or falloff strings occurred.

- 1,221 records use the scalar layout and decode `Supported`.
- 508 add fuzzy placement and decode `Partial`: 495 sphere, 13 box, with 0 to
  10 points.
- 0 records are malformed or unsupported.
- All decoded scalar and fuzzy values are finite.
- Range is 1.5 to 70; brightness is -1 to 3; cone is 0 to 0.392699.
- `UseInD3D` is 0 for 152 and 1 for 1,577 records.
- Light control values are 0 (1,128), 1 (322), and 2 (279).

Flags are preserved as raw `u32`; observed files include both documented low
bits and high bits, so the decoder does not normalize or reject them.

## Read-Only Projection

Properties and Scene Inspector expose the confirmed parameters, inert optional
names, fuzzy summary, retained chunk counts, diagnostics, and source offsets.
All fields are read-only. The preview draws a bounded semantic range ring at
the shared transform and labels Point/Spot type. It does not create a real
light, draw a spot cone, use fuzzy positions, or load animations/textures.

Limits are 4 MiB per body, 64 child chunks, 4,096 bytes per string, 100 fuzzy
points, and 32 retained unknown/unsupported chunks. Required/duplicate chunks,
payload bounds, versions, enums, strings, and finite floats are validated.
Malformed specialized data never destroys the generic wrapper.

No historical object is constructed, no factory or renderer is called, no
referenced resource is opened, and no scene byte is written. The next useful
narrow decoder candidate is class 5, Sound Source, after its serializer and
audio-reference semantics are audited.
