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

The current inventory contains 34 structures and 218 fields. Of those fields,
163 have candidate-or-better meanings; 55 ranges are explicitly `opaque`.
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
| `KfPoolRecord` | `0x14` | `KfSaveDirectory` | `0x80` |
| `KfSaveHeader` | `0x280` | `KfSavePayload` | `0x2580` |
| `KfSaveSlotSummary` | `0x18` | `KfVec3i` | `0x0c` |
| `KfVec3s` | `0x06` | `KfVec4i` | `0x10` |
| `KfVec4s` | `0x08` | `KfVecXZs` | `0x04` |
| `KfWeaponRecord` | `0x2c` | `SoundRef` | `0x03` |

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

The motion, map-cell, and partially decoded weapon-record fields are shown in
[`player-motion-and-weapon-attack.md`](player-motion-and-weapon-attack.md).
