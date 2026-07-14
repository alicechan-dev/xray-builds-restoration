# Historical Scene Object Decoder

Class ID 2 is `OBJCLASS_SCENEOBJECT`, owned by `CSceneObject`. The canonical
evidence is `Editors/LevelEditor/Edit/SceneObjectIO.cpp`; common base chunks are
owned by `Editors/ECore/Editor/CustomObject.cpp`.

## Confirmed body layout

`CSceneObject::Save` first writes the `CCustomObject` base, then:

| Chunk | Required | Confirmed payload |
|---:|---|---|
| `0x0900` | yes | `u16` version (`0x0011`; loader also accepts `0x0010`) |
| `0x0902` | yes | signed 32-bit reference version, signed 32-bit reserved value, zero-terminated reference name |
| `0x0905` | no | `u32` scene-object flags |

The base contributes `0xF906` flags, `0xF907` name, `0xF903` nine-float
transform, and optional `0xF905` motion plus `0xF908` motion time. Version
`0x0010` also uses legacy placement chunk `0x0904`.

All 15,416 sampled records use version `0x0011`, reference, flags, name, and
transform chunks. Four also carry base motion chunks; those records are
`Partial`, with the motion bytes retained only as bounded chunk inventory.
The remaining 15,412 are `Supported`. No sampled scene-object body was
malformed.

## Exposed inert fields

Read-only document properties expose type, decode status, body version,
reference name, reference version, reserved value, flags, diagnostics, and
chunk counts. Provenance records retain each decoded field's body-relative
offset, source offset, size, and chunk path.

The reference name is never resolved. No object library, mesh, texture,
shader, renderer, factory, or plugin is loaded. Preview remains the existing
generic box; its label may include the inert reference name. No geometry is
invented.

Missing/duplicate required chunks, invalid sizes, unterminated/oversized
strings, unsupported versions, compressed specialized children, and child
bounds outside the body produce `Malformed`. Unknown chunks are retained up to
the decoder limit and produce `Partial`.
