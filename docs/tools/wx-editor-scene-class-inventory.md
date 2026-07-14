# Historical Scene Class Inventory

The bounded `wxSDKEditorModelTests --audit-scenes <root>` mode scanned the 17
lawful build-1935 `.level` files available for restoration research. It is
read-only, sorts input paths deterministically, and prints aggregate metadata;
it does not export object bodies or assets.

The scan confirmed 22,767 object wrappers. Every record had a name and the
common nine-float transform, and the probe retained no structural diagnostic.

| ID | Historical class | Records | Scenes | Body bytes | Current decode |
|---:|---|---:|---:|---:|---|
| 0 | `OBJCLASS_GROUP` / `CGroupObject` | 6 | 5 | 595-13,835 | Unsupported |
| 1 | `OBJCLASS_GLOW` / `CGlow` | 511 | 17 | 145-166 | 511 Supported |
| 2 | `OBJCLASS_SCENEOBJECT` / `CSceneObject` | 15,416 | 17 | 122-19,789 | 15,412 Supported; 4 Partial |
| 3 | `OBJCLASS_LIGHT` / `CLight` | 1,729 | 17 | 171-327 | Unsupported |
| 5 | `OBJCLASS_SOUND_SRC` / `ESoundSource` | 187 | 5 | 166-206 | Unsupported |
| 6 | `OBJCLASS_SPAWNPOINT` / `CSpawnPoint` | 4,334 | 17 | 131-951 | Unsupported |
| 7 | `OBJCLASS_WAY` / `CWayObject` | 151 | 3 | 137-4,131 | Unsupported |
| 8 | `OBJCLASS_SECTOR` / `CSector` | 134 | 6 | 164-106,242 | Unsupported |
| 9 | `OBJCLASS_PORTAL` / `CPortal` | 233 | 6 | 169-521 | Unsupported |
| 10 | `OBJCLASS_SOUND_ENV` / `ESoundEnvironment` | 17 | 1 | 108-121 | Unsupported |
| 11 | `OBJCLASS_PS` / `EParticlesObject` | 49 | 5 | 105-139 | Unsupported |

No class 4, 12, 13, or 14 wrapper was observed. Absence from this sample is not
proof that those historical classes never occur.

## Source mapping

The class IDs are defined by `Editors/ECore/Editor/SceneClassList.h`. Exact
serialization owners are:

| ID | Load/Save evidence | Dependency character |
|---:|---|---|
| 0 | `Editors/LevelEditor/Edit/GroupObject.cpp`, `CGroupObject::Load/Save` | Nested object wrappers and factory-owned children |
| 1 | `Editors/LevelEditor/Edit/Glow.cpp`, `CGlow::Load/Save` | Scalar radius/flags plus inert shader and texture names |
| 2 | `Editors/LevelEditor/Edit/SceneObjectIO.cpp`, `CSceneObject::Load/Save` | Version, flags, and inert object-library reference name |
| 3 | `Editors/LevelEditor/Edit/ELight_IO.cpp`, `CLight::Load/Save` | Scalar light parameters plus optional animation, falloff, and fuzzy data |
| 5 | `Editors/LevelEditor/Edit/ESound_Source.cpp`, `ESoundSource::Load/Save` | Sound name and fixed parameters; historical loader creates sound state |
| 6 | `Editors/LevelEditor/Edit/SpawnPoint.cpp`, `CSpawnPoint::Load/Save` | Multiple generations, spawn packets, optional attached shape |
| 7 | `Editors/LevelEditor/Edit/WayPoint.cpp`, `CWayObject::Load/Save` | Bounded point and link arrays |
| 8 | `Editors/LevelEditor/Edit/sector.cpp`, `CSector::Load/Save` | Nested object/mesh reference list |
| 9 | `Editors/LevelEditor/Edit/portal.cpp`, `CPortal::Load/Save` | Sector-name references and vertex array |
| 10 | `Editors/LevelEditor/Edit/ESound_Environment.cpp`, `ESoundEnvironment::Load/Save` | Environment reference string |
| 11 | `Editors/LevelEditor/Edit/EParticlesObject.cpp`, `EParticlesObject::Load/Save` | Particle-library reference string |

All listed classes invoke `CCustomObject::Load/Save` from
`Editors/ECore/Editor/CustomObject.cpp`. That base owns flags `0xF906`, name
`0xF907`, transform `0xF903`, and optional motion `0xF905`/`0xF908`.

## Candidate ranking

1. **Scene object (2):** dominant frequency, exact small chunk layout, useful
   inert reference metadata, and no need to resolve the reference.
2. **Glow (1):** compact and clear in every sampled scene, but much less common.
3. **Light (3):** completed as the third specialized decoder.
4. **Particle (11):** simple inert reference, but low coverage.
5. **Sound source (5):** clear fixed record, but audio-state semantics need care.
6. **Spawn point (6):** common, but versioned spawn packets and attachments are
   substantially more complex.

Scene object therefore won the first decoder pass. Glow was the second pass:
all 511 records use version `0x0012`, have finite radii from 0.5 to 5.0, and
decode as `Supported`. The Light audit found 1,729 version-`0x0011` point
lights: 1,221 decode `Supported`, while 508 fuzzy-placement records are
`Partial` because those positions remain inert. Class 5 Sound Source is the
next recommended candidate. Historical sources and scene files remain
untouched.
