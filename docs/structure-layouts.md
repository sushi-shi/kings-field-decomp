# Structure layouts

Structure recovery is a first-class part of the semantic inventory:

- `config/retail/structures.tsv` records each target type and complete size;
- `config/retail/structure_fields.tsv` records every field's offset, extent,
  datatype, semantic confidence, evidence, and note;
- the ordered owner headers under `include/kf/` are the C forms used by
  reconstructions, which include their subsystem headers directly.

`kf inventory check` calculates the PlayStation 32-bit C layout from those
headers and compares it with both TSVs. It rejects missing or duplicate types
and fields and any disagreement in size, offset, extent, name, or datatype.
Production C and headers carry no compile-time size/offset assertions.
Independent target-compiler tests measure selected layouts and reject
deliberately incorrect fields and gaps. The inventory calculation is not a
compiler query and does not cover every local or anonymous aggregate.

The current inventory contains 129 structures and 871 fields. Of those fields,
783 have candidate-or-better meanings; 88 ranges are explicitly `opaque`.
Opaque fields still preserve exact layout and prevent known interior bytes from
being mislabeled as independent globals.

Selected checked layouts:

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
| `KfPlayerLevelGrowth` | `0x0c` | `KfPlayerMotionState` | `0x0c` |
| `KfPlayerProgressState` | `0x04` | `KfPlayerVitals` | `0x08` |
| `KfPlayerState` | `0xe0` |  |  |
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

`KfPlayerState` provides a checked complete view of the contiguous
`0x800a0780..0x800a0860` GAME player-state block. `game_main_loop` clears the
whole `0xe0`-byte extent and addresses later members from the same base; the
last typed member ends exactly at `+0xdf`. Its 83 fields preserve every byte:
known subobjects and scalars retain the existing data-identity names, while
unresolved gaps remain explicit opaque arrays/scalars. Individual data rows
are deliberately retained for relocation and source-linkage curation; the
structure is the common layout that prevents incompatible per-function views.
The player-update pass identifies its signed fields at `+0x50` and `+0x52` as
`player_state.fire_defense_timer` and `player_state.illusion_staff_timer`; their complete
set/use/countdown xrefs are documented in
[`player-update-and-lighting.md`](player-update-and-lighting.md).
The map-resource pass identifies byte `+0x0c` as `player_map_variant`: map
transitions set it, CHR resource loading uses it as the filename digit, and
floor five variant three selects the alternate sequence. See
[`map-resources.md`](map-resources.md).

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
