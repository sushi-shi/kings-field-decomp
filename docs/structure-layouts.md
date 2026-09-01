# Structure layouts

Structure recovery is a first-class part of the semantic inventory:

- `config/retail/structures.tsv` records each target type and complete size;
- `config/retail/structure_fields.tsv` records every field's offset, extent,
  datatype, semantic confidence, evidence, and note;
- `include/kf/game_types.h` and `include/kf/semantic_types.h` are the C forms
  used by reconstructions.

`kf inventory check` calculates the PlayStation 32-bit C layout from both
headers and compares it with both TSVs. It rejects missing types or fields and
any disagreement in size, offset, extent, name, or datatype. A full `kf build`
also compiles the header's size/offset assertions with the pinned target
compiler.

The current inventory contains 37 structures and 234 fields. Of those fields,
177 have candidate-or-better meanings; 57 ranges are explicitly `opaque`.
Opaque fields still preserve exact layout and prevent known interior bytes from
being mislabeled as independent globals.

| Structure | Size | Structure | Size |
| --- | ---: | --- | ---: |
| `KfActor` | `0x48` | `KfActorActionProfile` | `0x0a` |
| `KfActorDefinition` | `0x98` | `KfActorPlacement` | `0x10` |
| `KfAudioVoiceSlots` | `0x64` | `KfCameraPathPoint` | `0x1c` |
| `KfCameraPathState` | `0x64` | `KfEulerAngles` | `0x06` |
| `KfMapCell` | `0x02` | `KfMapCopyRegion` | `0x06` |
| `KfMapEvent` | `0x44` | `KfMapEventDefinition` | `0x18` |
| `KfMapObject` | `0x2c` | `KfMapObjectDefinition` | `0x08` |
| `KfMapObjectPlacement` | `0x14` | `KfMatrix` | `0x20` |
| `KfPitchYaw` | `0x04` | `KfPlayerAttackChargeState` | `0x04` |
| `KfPlayerLevelGrowth` | `0x0c` | `KfPlayerMotionState` | `0x0a` |
| `KfPlayerProgressState` | `0x04` | `KfPlayerVitals` | `0x08` |
| `KfPoolRecord` | `0x14` | `KfPrimitiveBuffer` | `0x0c` |
| `KfSaveDirectory` | `0x80` | `KfTmdObject` | `0x1c` |
| `KfSaveHeader` | `0x280` | `KfSavePayload` | `0x2580` |
| `KfSaveSlotSummary` | `0x18` | `KfVec3i` | `0x0c` |
| `KfVec3s` | `0x06` | `KfVec4i` | `0x10` |
| `KfVec4s` | `0x08` | `KfVecXZs` | `0x04` |
| `KfWeaponRecord` | `0x2c` | `KfCollisionTarget` | `0x20` |
| `SoundRef` | `0x03` |  |  |

For example, `KfPlayerLevelGrowth` is represented exactly as:

| Offset | Size | Field | Type | Meaning |
| ---: | ---: | --- | --- | --- |
| `0x00` | `0x02` | `maximum_hp` | `u16` | supported |
| `0x02` | `0x02` | `maximum_mp` | `u16` | supported |
| `0x04` | `0x02` | `physical_power_step` | `u16` | supported |
| `0x06` | `0x02` | `magic_step` | `u16` | supported |
| `0x08` | `0x04` | `experience_threshold` | `u32` | supported |

These are reconstruction identities, not recovered debug types. `supported`
means the target layout and interpretation agree with reviewed MIPS accesses,
callers, xrefs, or format/SDK evidence. It does not prove the original name or
translation-unit ownership.

The collision-query output is represented exactly as:

| Offset | Size | Field | Type | Meaning |
| ---: | ---: | --- | --- | --- |
| `0x00` | `0x10` | `position` | `KfVec4i` | supported |
| `0x10` | `0x08` | `rotation` | `KfVec4s` | supported |
| `0x18` | `0x02` | `radius` | `u16` | supported |
| `0x1a` | `0x06` | `unknown_1a` | `u8[6]` | opaque |

The `0x20` extent ends at the next independently referenced state. The opaque
tail prevents those bytes from being advertised as known fields while keeping
the complete object boundary explicit.

The display/TMD campaign adds two more complete layouts:

| Structure | Offset | Size | Field | Type |
| --- | ---: | ---: | --- | --- |
| `KfPrimitiveBuffer` | `0x00` | `0x04` | `start` | `u8 *` |
| `KfPrimitiveBuffer` | `0x04` | `0x04` | `end` | `u8 *` |
| `KfPrimitiveBuffer` | `0x08` | `0x04` | `cursor` | `u8 *` |
| `KfTmdObject` | `0x00` | `0x04` | `vertex_offset` | `u32` |
| `KfTmdObject` | `0x04` | `0x04` | `vertex_count` | `u32` |
| `KfTmdObject` | `0x08` | `0x04` | `normal_offset` | `u32` |
| `KfTmdObject` | `0x0c` | `0x04` | `normal_count` | `u32` |
| `KfTmdObject` | `0x10` | `0x04` | `primitive_offset` | `u32` |
| `KfTmdObject` | `0x14` | `0x04` | `primitive_count` | `u32` |
| `KfTmdObject` | `0x18` | `0x04` | `scale` | `s32` |

The primitive-buffer extent is proved by complete initialization of two
0x0c-byte records followed by indexed frame selection. The TMD object extent
is supported by the standard format, the exact 0x1c lookup stride, and the
primitive stream fields read at `+0x10` and `+0x14`. See
[`display-and-tmd.md`](display-and-tmd.md).

The motion, map-cell, and partially decoded weapon-record fields are shown in
[`player-motion-and-weapon-attack.md`](player-motion-and-weapon-attack.md).
The corrected weapon-array boundary and collision output are documented in
[`player-interactions-and-collision.md`](player-interactions-and-collision.md).

The allocator campaign deliberately does **not** add a structure. Retail proves
seven adjacent state objects per overlay, including a `u32[16]` LIFO array,
but it addresses the array as `&memory_allocation_depth + 1`; adjacency and
derived-address arithmetic alone do not prove an enclosing source aggregate.
The separate typed extents and the evidence needed for a future promotion are
documented in [`memory-allocator.md`](memory-allocator.md).
