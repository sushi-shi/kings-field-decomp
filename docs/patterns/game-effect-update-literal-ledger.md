# Effect-update remainder literal ledger

Complete per-occurrence account of **105 retained numeric literals** in
`src/game/effect_update.c`: 35 segment payload bytes and 70 function uses.
Named definitions and retail claims are excluded; comments, strings and identifier
digits are not numeric literals. Signs remain operators. See the
[effect-update review](game-effect-update-constants.md) for named units and verification.

`effect_magic_power`, `effect_spawn_trail_kind13` and `effect_spawn_ground_kind6`
have no remaining inline numeric literals.

| Function | Line | Token | Expression | Reason |
| --- | --- | --- | --- | --- |
| `initializers` | 29 | `65` | `{65, 80, 1, 0, 2, 0, 100},` | Segment 0, column: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 29 | `80` | `{65, 80, 1, 0, 2, 0, 100},` | Segment 0, row: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 29 | `1` | `{65, 80, 1, 0, 2, 0, 100},` | Segment 0, column_step: unit grid stride along this axis. |
| `initializers` | 29 | `0` | `{65, 80, 1, 0, 2, 0, 100},` | Segment 0, row_step: no movement along this axis. |
| `initializers` | 29 | `2` | `{65, 80, 1, 0, 2, 0, 100},` | Segment 0, cell_count: authored number of consecutive cells in this segment; table payload, not a shared loop capacity. |
| `initializers` | 29 | `0` | `{65, 80, 1, 0, 2, 0, 100},` | Segment 0, start_height: initial floor-height byte; zero is the interpolation origin. |
| `initializers` | 29 | `100` | `{65, 80, 1, 0, 2, 0, 100},` | Segment 0, end_height: authored final height byte, converted to world Y as -100 times the named map-height step. Designer choice of destination remains unresolved. |
| `initializers` | 30 | `61` | `{61, 73, 0, 255, 2, 0, 100},` | Segment 1, column: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 30 | `73` | `{61, 73, 0, 255, 2, 0, 100},` | Segment 1, row: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 30 | `0` | `{61, 73, 0, 255, 2, 0, 100},` | Segment 1, column_step: no movement along this axis. |
| `initializers` | 30 | `255` | `{61, 73, 0, 255, 2, 0, 100},` | Segment 1, row_step: byte 255 steps backward by one through byte-coordinate wrap, not an absent-value sentinel. |
| `initializers` | 30 | `2` | `{61, 73, 0, 255, 2, 0, 100},` | Segment 1, cell_count: authored number of consecutive cells in this segment; table payload, not a shared loop capacity. |
| `initializers` | 30 | `0` | `{61, 73, 0, 255, 2, 0, 100},` | Segment 1, start_height: initial floor-height byte; zero is the interpolation origin. |
| `initializers` | 30 | `100` | `{61, 73, 0, 255, 2, 0, 100},` | Segment 1, end_height: authored final height byte, converted to world Y as -100 times the named map-height step. Designer choice of destination remains unresolved. |
| `initializers` | 31 | `75` | `{75, 56, 1, 0, 3, 0, 100},` | Segment 2, column: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 31 | `56` | `{75, 56, 1, 0, 3, 0, 100},` | Segment 2, row: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 31 | `1` | `{75, 56, 1, 0, 3, 0, 100},` | Segment 2, column_step: unit grid stride along this axis. |
| `initializers` | 31 | `0` | `{75, 56, 1, 0, 3, 0, 100},` | Segment 2, row_step: no movement along this axis. |
| `initializers` | 31 | `3` | `{75, 56, 1, 0, 3, 0, 100},` | Segment 2, cell_count: authored number of consecutive cells in this segment; table payload, not a shared loop capacity. |
| `initializers` | 31 | `0` | `{75, 56, 1, 0, 3, 0, 100},` | Segment 2, start_height: initial floor-height byte; zero is the interpolation origin. |
| `initializers` | 31 | `100` | `{75, 56, 1, 0, 3, 0, 100},` | Segment 2, end_height: authored final height byte, converted to world Y as -100 times the named map-height step. Designer choice of destination remains unresolved. |
| `initializers` | 32 | `37` | `{37, 27, 0, 255, 3, 0, 100},` | Segment 3, column: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 32 | `27` | `{37, 27, 0, 255, 3, 0, 100},` | Segment 3, row: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 32 | `0` | `{37, 27, 0, 255, 3, 0, 100},` | Segment 3, column_step: no movement along this axis. |
| `initializers` | 32 | `255` | `{37, 27, 0, 255, 3, 0, 100},` | Segment 3, row_step: byte 255 steps backward by one through byte-coordinate wrap, not an absent-value sentinel. |
| `initializers` | 32 | `3` | `{37, 27, 0, 255, 3, 0, 100},` | Segment 3, cell_count: authored number of consecutive cells in this segment; table payload, not a shared loop capacity. |
| `initializers` | 32 | `0` | `{37, 27, 0, 255, 3, 0, 100},` | Segment 3, start_height: initial floor-height byte; zero is the interpolation origin. |
| `initializers` | 32 | `100` | `{37, 27, 0, 255, 3, 0, 100},` | Segment 3, end_height: authored final height byte, converted to world Y as -100 times the named map-height step. Designer choice of destination remains unresolved. |
| `initializers` | 33 | `32` | `{32, 82, 0, 1, 12, 0, 100}` | Segment 4, column: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 33 | `82` | `{32, 82, 0, 1, 12, 0, 100}` | Segment 4, row: authored floor-grid coordinate. Retain the map placement datum; no independently established location identity. |
| `initializers` | 33 | `0` | `{32, 82, 0, 1, 12, 0, 100}` | Segment 4, column_step: no movement along this axis. |
| `initializers` | 33 | `1` | `{32, 82, 0, 1, 12, 0, 100}` | Segment 4, row_step: unit grid stride along this axis. |
| `initializers` | 33 | `12` | `{32, 82, 0, 1, 12, 0, 100}` | Segment 4, cell_count: authored number of consecutive cells in this segment; table payload, not a shared loop capacity. |
| `initializers` | 33 | `0` | `{32, 82, 0, 1, 12, 0, 100}` | Segment 4, start_height: initial floor-height byte; zero is the interpolation origin. |
| `initializers` | 33 | `100` | `{32, 82, 0, 1, 12, 0, 100}` | Segment 4, end_height: authored final height byte, converted to world Y as -100 times the named map-height step. Designer choice of destination remains unresolved. |
| `effect_projectile_update_3d` | 61 | `1u` | `if (life < KF_EFFECT_HAZARD_RELEASE_REQUEST + 1u) {` | One expresses the exclusive upper bound that includes release-request phase; the orbit expression also extracts the stored phase byte with 0xff. |
| `effect_projectile_update_3d` | 72 | `16` | `if ((collision >> 16) == (KF_COLLISION_ACTOR >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_3d` | 72 | `16` | `if ((collision >> 16) == (KF_COLLISION_ACTOR >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_3d` | 73 | `0xffff` | `actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],` | Packed actor index uses low-halfword mask; zero base power and component slot zero begin the damage call. The remaining slots preserve the established 0,2,1 physical-component order. |
| `effect_projectile_update_3d` | 73 | `0` | `actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],` | Packed actor index uses low-halfword mask; zero base power and component slot zero begin the damage call. The remaining slots preserve the established 0,2,1 physical-component order. |
| `effect_projectile_update_3d` | 73 | `0` | `actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],` | Packed actor index uses low-halfword mask; zero base power and component slot zero begin the damage call. The remaining slots preserve the established 0,2,1 physical-component order. |
| `effect_projectile_update_3d` | 74 | `2` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_3d` | 74 | `1` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_3d` | 75 | `0` | `0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_3d` | 75 | `0` | `0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_3d` | 76 | `16` | `} else if ((collision >> 16) == (KF_COLLISION_PLAYER >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_3d` | 76 | `16` | `} else if ((collision >> 16) == (KF_COLLISION_PLAYER >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_3d` | 77 | `0` | `player_apply_damage(magic->damage_components[0],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_3d` | 78 | `2` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_3d` | 78 | `1` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_3d` | 79 | `0` | `0, 0, 0, KF_FIXED12_ONE, record->id);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_3d` | 79 | `0` | `0, 0, 0, KF_FIXED12_ONE, record->id);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_3d` | 79 | `0` | `0, 0, 0, KF_FIXED12_ONE, record->id);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_3d` | 83 | `0` | `if (record->sound_played == 0) {` | Zero is the sound-acceptance flag being tested or cleared so a later update may retry; no separate phase or sound identifier. |
| `effect_projectile_update_3d` | 86 | `0` | `&magic->sounds[0], &world, KF_AUDIO_MAX_VOLUME,` | First sound definition in the current magic record, used by this moving hazard; positional slot, distinct from the acceptance flag. |
| `effect_projectile_update_3d` | 92 | `0` | `record->direction.words.x = 0;` | Zero angular velocity when the pitch clamp fires. |
| `effect_projectile_update_3d` | 93 | `1` | `} else if (record->rotation.vy < -KF_ANGLE_EIGHTH_TURN + 1) {` | One expresses the inclusive negative bound as a strict comparison. Retail tests yaw here before clamping pitch; preserve the asymmetry. |
| `effect_projectile_update_3d` | 95 | `0` | `record->direction.words.x = 0;` | Zero angular velocity when the pitch clamp fires. |
| `effect_projectile_update_3d` | 97 | `0` | `if (record->rotation.vx > 0) {` | Pitch sign boundary selects acceleration toward the swing center. |
| `effect_projectile_update_3d` | 104 | `0` | `if ((next_pitch <= 0 && pitch >= 0) \|\| (next_pitch >= 0 && pitch <= 0)) {` | Four zero comparisons detect crossing or touching the pitch origin in either direction; both old and next pitch participate. |
| `effect_projectile_update_3d` | 104 | `0` | `if ((next_pitch <= 0 && pitch >= 0) \|\| (next_pitch >= 0 && pitch <= 0)) {` | Four zero comparisons detect crossing or touching the pitch origin in either direction; both old and next pitch participate. |
| `effect_projectile_update_3d` | 104 | `0` | `if ((next_pitch <= 0 && pitch >= 0) \|\| (next_pitch >= 0 && pitch <= 0)) {` | Four zero comparisons detect crossing or touching the pitch origin in either direction; both old and next pitch participate. |
| `effect_projectile_update_3d` | 104 | `0` | `if ((next_pitch <= 0 && pitch >= 0) \|\| (next_pitch >= 0 && pitch <= 0)) {` | Four zero comparisons detect crossing or touching the pitch origin in either direction; both old and next pitch participate. |
| `effect_projectile_update_3d` | 106 | `0` | `next_pitch = 0;` | Release snaps pitch to the swing origin before publishing the rise phase. |
| `effect_projectile_update_3d` | 109 | `0` | `record->sound_played = 0;` | Zero is the sound-acceptance flag being tested or cleared so a later update may retry; no separate phase or sound identifier. |
| `effect_projectile_update_2d` | 127 | `0xff` | `if ((life & 0xff) < KF_EFFECT_HAZARD_RELEASE_REQUEST + 1) {` | One expresses the exclusive upper bound that includes release-request phase; the orbit expression also extracts the stored phase byte with 0xff. |
| `effect_projectile_update_2d` | 127 | `1` | `if ((life & 0xff) < KF_EFFECT_HAZARD_RELEASE_REQUEST + 1) {` | One expresses the exclusive upper bound that includes release-request phase; the orbit expression also extracts the stored phase byte with 0xff. |
| `effect_projectile_update_2d` | 133 | `1` | `+ (rsin((s16)record->control.orbit_angle << 1) >> 2);` | Double the angle for two vertical cycles per horizontal orbit, then divide Q12 sine by four for 1024-world-unit amplitude. These arithmetic factors directly state the waveform; no separate state identity. |
| `effect_projectile_update_2d` | 133 | `2` | `+ (rsin((s16)record->control.orbit_angle << 1) >> 2);` | Double the angle for two vertical cycles per horizontal orbit, then divide Q12 sine by four for 1024-world-unit amplitude. These arithmetic factors directly state the waveform; no separate state identity. |
| `effect_projectile_update_2d` | 138 | `16` | `if ((collision >> 16) == (KF_COLLISION_ACTOR >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_2d` | 138 | `16` | `if ((collision >> 16) == (KF_COLLISION_ACTOR >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_2d` | 139 | `0xffff` | `actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],` | Packed actor index uses low-halfword mask; zero base power and component slot zero begin the damage call. The remaining slots preserve the established 0,2,1 physical-component order. |
| `effect_projectile_update_2d` | 139 | `0` | `actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],` | Packed actor index uses low-halfword mask; zero base power and component slot zero begin the damage call. The remaining slots preserve the established 0,2,1 physical-component order. |
| `effect_projectile_update_2d` | 139 | `0` | `actor_apply_damage(collision & 0xffff, 0, magic->damage_components[0],` | Packed actor index uses low-halfword mask; zero base power and component slot zero begin the damage call. The remaining slots preserve the established 0,2,1 physical-component order. |
| `effect_projectile_update_2d` | 140 | `2` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_2d` | 140 | `1` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_2d` | 141 | `0` | `0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_2d` | 141 | `0` | `0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, record->type);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_2d` | 142 | `16` | `} else if ((collision >> 16) == (KF_COLLISION_PLAYER >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_2d` | 142 | `16` | `} else if ((collision >> 16) == (KF_COLLISION_PLAYER >> 16)) {` | Upper-halfword extraction of the packed collision class on both sides of the comparison; retain the representation width. |
| `effect_projectile_update_2d` | 143 | `0` | `player_apply_damage(magic->damage_components[0],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_2d` | 144 | `2` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_2d` | 144 | `1` | `magic->damage_components[2], magic->damage_components[1],` | Authored magic damage array uses slots 0,2,1 at the established physical-component boundary; retain the positional array mapping without inventing new channel identities. |
| `effect_projectile_update_2d` | 145 | `0` | `0, 0, 0, KF_FIXED12_ONE, record->id);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_2d` | 145 | `0` | `0, 0, 0, KF_FIXED12_ONE, record->id);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_2d` | 145 | `0` | `0, 0, 0, KF_FIXED12_ONE, record->id);` | Zero arguments omit the unused damage channels/status on this call; actor and player signatures preserve their distinct argument positions. |
| `effect_projectile_update_2d` | 148 | `0` | `if (record->sound_played == 0) {` | Zero is the sound-acceptance flag being tested or cleared so a later update may retry; no separate phase or sound identifier. |
| `effect_projectile_update_2d` | 151 | `0` | `&magic->sounds[0], &record->position,` | First sound definition in the current magic record, used by this moving hazard; positional slot, distinct from the acceptance flag. |
| `effect_projectile_update_2d` | 160 | `0` | `record->sound_played = 0;` | Zero is the sound-acceptance flag being tested or cleared so a later update may retry; no separate phase or sound identifier. |
| `effect_projectile_update_2d` | 163 | `0xff` | `} else if ((life & 0xff) != KF_EFFECT_HAZARD_RUNNING && (s16)phase_limit >= (int)(life & 0xff)) {` | Extract the stored unsigned phase byte before the phase/limit comparison; retain the byte representation mask. |
| `effect_projectile_update_2d` | 163 | `0xff` | `} else if ((life & 0xff) != KF_EFFECT_HAZARD_RUNNING && (s16)phase_limit >= (int)(life & 0xff)) {` | Extract the stored unsigned phase byte before the phase/limit comparison; retain the byte representation mask. |
| `effect_floor_deform_line` | 180 | `0` | `if (range < 0) {` | Sign test implements abs(progress_step), retaining a zero-width silent interval when the step is zero. |
| `effect_floor_deform_line` | 188 | `1` | `while (--count != -1) {` | Predecrement traversal terminates at minus one after exactly cell_count visits; retain the original loop convention. |
| `effect_floor_deform_line` | 191 | `0` | `if (progress < 0) {` | Clamp negative progress to the interpolation origin, zero, before considering sound playback. |
| `effect_floor_deform_line` | 192 | `0` | `progress = 0;` | Clamp negative progress to the interpolation origin, zero, before considering sound playback. |
| `effect_floor_deform_line` | 193 | `1` | `} else if (progress >= KF_FIXED12_ONE + 1) {` | One makes the strict overflow threshold above Q12 unity; unity itself can still enter the sound interval. |
| `effect_scatter_triple` | 223 | `0` | `centered = values[0] - SCATTER_VELOCITY_BIAS;` | Halfword lane 0 (X) is read and written in sequence; array index describes coordinates, not an effect ID. |
| `effect_scatter_triple` | 225 | `0` | `values[0] = centered;` | Halfword lane 0 (X) is read and written in sequence; array index describes coordinates, not an effect ID. |
| `effect_scatter_triple` | 227 | `1` | `centered = values[1] - SCATTER_VELOCITY_BIAS;` | Halfword lane 1 (Y) is read and written in sequence; array index describes coordinates, not an effect ID. |
| `effect_scatter_triple` | 229 | `1` | `values[1] = centered;` | Halfword lane 1 (Y) is read and written in sequence; array index describes coordinates, not an effect ID. |
| `effect_scatter_triple` | 231 | `2` | `centered = values[2] - SCATTER_VELOCITY_BIAS;` | Halfword lane 2 (Z) is read and written in sequence; array index describes coordinates, not an effect ID. |
| `effect_scatter_triple` | 233 | `2` | `values[2] = centered;` | Halfword lane 2 (Z) is read and written in sequence; array index describes coordinates, not an effect ID. |
| `effect_rotate_scale_offset_y` | 244 | `0` | `scaled.vy = 0;` | Zero removes the vertical offset before rotation. |
| `effect_rotate_scale_offset_y` | 246 | `0` | `rotation.vx = 0;` | Zero pitch/roll leaves only the supplied yaw in the SDK rotation vector. |
| `effect_rotate_scale_offset_y` | 248 | `0` | `rotation.vz = 0;` | Zero pitch/roll leaves only the supplied yaw in the SDK rotation vector. |
