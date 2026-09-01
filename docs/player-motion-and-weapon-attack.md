# Player motion and weapon attack

This GAME campaign names eight functions connected by confirmed calls and
shared state. It does not claim one original translation unit. The evidence
ledger is `config/evidence/game_semantic_player_motion_attack.tsv`.

| Address | Identity | Reviewed role |
| --- | --- | --- |
| `0x80016b24` | `player_begin_weapon_attack` | Commits accumulated POWER and starts the swing phase. |
| `0x80016bc0` | `player_update_weapon_attack` | Charges POWER, advances the swing, builds its hit point, and applies actor damage. |
| `0x80016e24` | `game_initialize_session` | Establishes initial camera/options state and allocates the weapon asset buffer. |
| `0x80016eb8` | `player_clear_motion` | Clears one complete five-halfword motion object. |
| `0x80016ee8` | `player_sync_position_to_map` | Derives the current cell and floor height after loads and transitions. |
| `0x80017040` | `player_distance_to_point_in_cone` | Combines bounded distance with a wrapped 12-bit facing test. |
| `0x80017a24` | `player_update_view_bob` | Advances grounded view-bob phase from movement speed. |
| `0x80017a80` | `player_update_vertical_motion` | Follows floor height and handles the reviewed fall/hazard states. |

## Recovered layouts

`player_motion_state` at `0x800a0840` is a complete `0x0a`-byte
`KfPlayerMotionState`. Its extent is proved by `player_clear_motion`, while the
input loop and view-bob path establish each field:

| Offset | Size | Field | Type |
| ---: | ---: | --- | --- |
| `0x00` | `0x02` | `strafe_velocity` | `s16` |
| `0x02` | `0x02` | `forward_velocity` | `s16` |
| `0x04` | `0x02` | `movement_speed` | `u16` |
| `0x06` | `0x02` | `yaw_step` | `s16` |
| `0x08` | `0x02` | `pitch_step` | `s16` |

The packed two-byte `KfMapCell` stores `z` at offset zero and `x` at offset
one. This order is direct: `player_sync_position_to_map` divides camera z and
x by 2,000 and writes those bytes respectively. `player_map_cell` at
`0x800a084a` is the current cell; `player_previous_map_cell` at `0x800a084c`
is copied and compared by transition paths, although its exact policy name is
still provisional.

The runtime-loaded weapon table has a proven `0x2c` stride. The reviewed code
supports this partial `KfWeaponRecord` layout:

| Offset | Size | Field | Type | Status |
| ---: | ---: | --- | --- | --- |
| `0x00` | `0x01` | `unknown_00` | `u8` | opaque |
| `0x01` | `0x01` | `charge_rate` | `u8` | supported |
| `0x02` | `0x0a` | `attack_components` | `u16[5]` | supported |
| `0x0c` | `0x06` | `unknown_0c` | `u8[6]` | opaque |
| `0x12` | `0x02` | `attack_z_offset` | `u16` | supported |
| `0x14` | `0x18` | `unknown_14` | `u8[24]` | opaque |

The opaque ranges are intentional. They preserve the complete record without
inventing meanings for unreviewed bytes.

## Map grids

Three BSS objects have exact `0x2710` extents and `z * 100 + x` indexing:

| Address | Identity | Current evidence |
| --- | --- | --- |
| `0x80095900` | `map_floor_height_grid` | Bytes become world floor heights after multiplication by `-100`. |
| `0x80098018` | `map_collision_grid` | The full horizontal-motion body reads the grid when accepting or rejecting candidate movement. |
| `0x8009a748` | `map_cell_attribute_grid` | Vertical motion branches on cell policy bytes for fall and hazard handling. |

The floor and attribute bases were absent from the seed relocation census.
Their decoded `lui`/`addiu` pairs are now manually admitted in
`config/retail/relocs.tsv`. The collision-grid pairs were likewise admitted
only after reviewing the complete `0x800171fc..0x80017a23` function body.

The nearby weapon phase, charge-delay, asset-buffer, and full-charge fields
remain separate data identities. Address adjacency alone does not prove that
the original source declared one enclosing weapon-runtime structure.
