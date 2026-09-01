# Player interactions and world collision

This campaign follows confirmed calls and shared data between player movement,
item use, interaction-image display, and the general world-collision query. It
does not claim that the investigated address band was one original translation
unit.

## Recovered layouts

The world-collision query optionally writes this complete `0x20`-byte object
when flag `0x800` is set:

| Offset | Size | Field | Type | Confidence |
| ---: | ---: | --- | --- | --- |
| `0x00` | `0x10` | `position` | `KfVec4i` | supported |
| `0x10` | `0x08` | `rotation` | `KfVec4s` | supported |
| `0x18` | `0x02` | `radius` | `u16` | supported |
| `0x1a` | `0x06` | `unknown_1a` | `u8[6]` | opaque |

The end at `+0x20` is supported by the next independently referenced BSS
state. The final six bytes remain part of the layout, but no reviewed access
supports semantic field names for them.

The exact function at `0x800150a8` provides a second boundary correction. It
copies exactly 176 words (`0x2c0` bytes), then visits sixteen records with a
`0x2c` stride and negates the halfword at `+0x26`. Therefore the destination
at `0x8009ff10` is `KfWeaponRecord weapon_records[16]`, ending exactly where
`collision_target` begins at `0x800a01d0`. The old `asset_block` identity was
too large and incorrectly absorbed that separate object.

`mirrored_angle` is only a working field name. Its offset and signed-negation
operation are directly observed and the refactored C remains a 100% match;
its gameplay meaning is still `candidate`.

## Function family

| Address | Identity | Main evidence |
| --- | --- | --- |
| `0x800150a8` | `player_weapon_load_records_and_mirror_angles` | exact 16-by-`0x2c` copy and field transform |
| `0x800171fc` | `player_move_horizontal` | collision query, grids, transform state, sole typed caller |
| `0x80017cf8` | `player_warp_to_floor_entry` | current-floor state and five-entry cell table |
| `0x80017e3c` | `player_update_transform_snapshot` | typed position/rotation consumers and halfword offsets |
| `0x80017edc` | `actor_show_info_image` | `KfActor.definition_id` and mutable ENE/EI path |
| `0x80017fa4` | `map_event_show_person_image` | `KfMapEvent.kind` and mutable PRSN/PER path |
| `0x80018054` | `player_use_item` | two decoded switch tables and `u8` dispatch range |
| `0x8001a5ac` | `collision_query_world` | 24 callers, six O32 arguments, object-pool accesses, optional output writes |

`player_warp_to_floor_entry` deliberately constructs an address two bytes
before `floor_entry_cells` and adds a one-based floor index. The relocation
inventory preserves that exact `-0x2` addend instead of moving the table
boundary to make the source model look simpler.

The two item switches are compiler data, not functions: 33 raw code pointers
for IDs `0x2a..0x4a` at `0x80012048`, followed by 42 pointers for nested
map-object types `0x51..0x7a` at `0x800120d0`. Their targets remain internal
case labels rather than invented function starts.

The per-function evidence and unresolved points are preserved in
`config/evidence/game_semantic_player_interactions.tsv`. Canonical sizes and
fields live in `config/retail/structures.tsv` and
`config/retail/structure_fields.tsv`; the checked C declarations and target
layout assertions live in `include/kf/semantic_types.h`.
