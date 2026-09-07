# Player death/combat remainder literal ledger

Complete ledger for the eight functions and sound initializers below: **56
retained occurrences**. Claims and named definitions are excluded; signs are
operators. Together with the startup and progression ledgers, this covers
every inline numeric occurrence in `src/game/player_death.c`.
See the [damage-unit review](game-player-damage-units.md) for the scope audit.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `initializers` | 19 | `3` | `SoundRef player_sound_refs[3] = {` | Three packed program/tone/note selectors: weapon-attack start, death start and level-up, in the order consumed by their callers. |
| `initializers` | 20 | `7, 0, 80` | `{7, 0, 80},` | Program 7, tone 0, note 80 for the weapon-attack-start cue selected by player_begin_weapon_attack. These are authored VAB selectors, not HP or animation units. |
| `initializers` | 21 | `7, 1, 89` | `{7, 1, 89},` | Program 7, tone 1, note 89 for the death-start cue. Keep the distinct tone/note choice within the same program; original sound-design rationale is unknown. |
| `initializers` | 22 | `13, 0, 67` | `{13, 0, 67}` | Program 13, tone 0, note 67 for the level-up cue. Preserve the packed three-byte selector and its authored resource values. |
| `player_death_begin` | 39 | `0` | `player_state.death_camera_pitch_step = 0;` | Zero starts the named death pitch-step or visual-blend counter before saving the current render state; these counters have distinct consumers. |
| `player_death_begin` | 40 | `0` | `player_state.death_visual_blend = 0;` | Zero starts the named death pitch-step or visual-blend counter before saving the current render state; these counters have distinct consumers. |
| `player_death_begin` | 41 | `1` | `sound_ref_play(&player_sound_refs[1], KF_AUDIO_MAX_VOLUME);` | Sound-reference row one selects the death-start cue. Volume uses the separately named full-scale audio input. |
| `player_death_restart` | 147 | `0 × 2` | `if (map_floor1_script.revival_enabled == KF_MAP_SCRIPT_SET && item_stock[0][KF_ITEM_DRAGON_KING_GRASS_FRUIT] != 0) {` | Bank zero is player inventory. Revival requires a nonzero Dragon King Grass Fruit quantity and consumes one with the existing decrement before persisting world state. |
| `player_death_restart` | 148 | `0` | `item_stock[0][KF_ITEM_DRAGON_KING_GRASS_FRUIT]--;` | Bank zero is player inventory. Revival requires a nonzero Dragon King Grass Fruit quantity and consumes one with the existing decrement before persisting world state. |
| `player_death_restart` | 150 | `0xfa00` | `player_state.camera_position.vx = 0xfa00;` | Revival X coordinate is 64000 world units on floor one, after the enabled-revival and fruit-possession guards. This is an authored placement; the original coordinate choice is unproven. |
| `player_death_restart` | 153 | `0x4e20` | `player_state.camera_position.vz = 0x4e20;` | Revival Z coordinate is 20000 world units, paired with X=64000; the loaded floor geometry supplies height. This is an authored placement; the original coordinate choice is unproven. |
| `player_death_restart` | 154, 158 | `0 × 2` | `player_state.camera_rotation.vy = 0;` | Zero the named camera Euler angle or transient view-rotation component, preserving their independent stores and restart orientation. |
| `player_death_restart` | 156 | `0x7918` | `player_state.camera_position.vx = 0x7918;` | Fresh-start X coordinate is 31000 world units, chosen when revival is unavailable. This is an authored placement; the original coordinate choice is unproven. |
| `player_death_restart` | 157 | `0x1388` | `player_state.camera_position.vz = 0x1388;` | Fresh-start Z coordinate is 5000 world units, paired with X=31000 before game_state_initialize. This is an authored placement; the original coordinate choice is unproven. |
| `player_death_restart` | 162 | `0` | `player_state.status_effect_flags = 0;` | Clear all player status bits after either restart branch; this is the empty flag set. |
| `player_death_restart` | 163 | `0` | `player_state.camera_rotation.vz = 0;` | Zero the named camera Euler angle or transient view-rotation component, preserving their independent stores and restart orientation. |
| `player_death_restart` | 164 | `0` | `player_state.camera_rotation.vx = 0;` | Zero the named camera Euler angle or transient view-rotation component, preserving their independent stores and restart orientation. |
| `player_death_restart` | 165 | `1` | `if (floor != 1) {` | One-based floor identity one is the restart destination. The saved previous floor decides whether resources must be reloaded; the named fresh-start override forces that path. |
| `player_death_restart` | 166 | `1` | `player_state.progress_state.current_floor = 1;` | One-based floor identity one is the restart destination. The saved previous floor decides whether resources must be reloaded; the named fresh-start override forces that path. |
| `player_death_restart` | 167 | `0` | `player_state.map_variant = 0;` | Select the base floor-one resource variant when the restart path reloads the floor. |
| `player_death_restart` | 173 | `0` | `player_state.view_bob_offset = 0;` | Reset the named view-bob, death-pitch or death-blend value to zero before recovery fade; each field keeps its existing update behavior. |
| `player_death_restart` | 175 | `0` | `player_state.death_camera_pitch_step = 0;` | Reset the named view-bob, death-pitch or death-blend value to zero before recovery fade; each field keeps its existing update behavior. |
| `player_death_restart` | 176 | `0` | `player_state.death_visual_blend = 0;` | Reset the named view-bob, death-pitch or death-blend value to zero before recovery fade; each field keeps its existing update behavior. |
| `player_death_restart` | 177 | `0` | `hud_brightness = 0;` | Begin recovery fade with a dark HUD; the fade updater restores brightness. |
| `player_death_restart` | 178 | `0` | `player_state.view_rotation_offset.vz = 0;` | Zero the named camera Euler angle or transient view-rotation component, preserving their independent stores and restart orientation. |
| `player_death_restart` | 179 | `0` | `player_state.view_rotation_offset.vy = 0;` | Zero the named camera Euler angle or transient view-rotation component, preserving their independent stores and restart orientation. |
| `player_death_restart` | 180 | `0` | `player_state.view_rotation_offset.vx = 0;` | Zero the named camera Euler angle or transient view-rotation component, preserving their independent stores and restart orientation. |
| `player_death_restart` | 183 | `1500` | `player_state.camera_position.vy = player_state.floor_height - 1500;` | Place the restarted camera 1500 world units above the sampled floor in the downward-positive Y convention. The authored standing-camera offset is preserved. |
| `player_adjust_hp` | 194 | `0` | `if (value <= 0) {` | Zero is the lower resource bound after applying the signed delta. HP reaching it enters death; MP reaching it only stores zero and returns. Positive results are separately limited by the corresponding maximum. |
| `player_adjust_hp` | 195 | `0` | `player_state.vitals.current_hp = 0;` | Zero is the lower resource bound after applying the signed delta. HP reaching it enters death; MP reaching it only stores zero and returns. Positive results are separately limited by the corresponding maximum. |
| `player_adjust_mp` | 213 | `0` | `if (value <= 0) {` | Zero is the lower resource bound after applying the signed delta. HP reaching it enters death; MP reaching it only stores zero and returns. Positive results are separately limited by the corresponding maximum. |
| `player_adjust_mp` | 214 | `0` | `player_state.vitals.current_mp = 0;` | Zero is the lower resource bound after applying the signed delta. HP reaching it enters death; MP reaching it only stores zero and returns. Positive results are separately limited by the corresponding maximum. |
| `player_calculate_damage_component` | 460 | `0` | `if (attack == 0) {` | An absent attack channel contributes zero immediately, before division or defense arithmetic. |
| `player_calculate_damage_component` | 461 | `0` | `return 0;` | An absent attack channel contributes zero immediately, before division or defense arithmetic. |
| `player_calculate_damage_component` | 463 | `5` | `base_power = defense + base_power / 5;` | One-fifth of player physical power augments the defense threshold using signed integer division. This differs from the actor-target helper, which augments attack; original weighting rationale is unknown. |
| `player_calculate_damage_component` | 465 | `0` | `if (defense < 0) {` | After reusing defense for attack-minus-threshold, clamp its negative excess to zero before adding the squared-attack term. |
| `player_calculate_damage_component` | 466 | `0` | `defense = 0;` | After reusing defense for attack-minus-threshold, clamp its negative excess to zero before adding the squared-attack term. |
| `player_calculate_damage_component` | 468 | `0` | `if (base_power == 0) {` | Replace an exactly zero combined threshold with one so the following denominator is nonzero. Preserve the exact zero check rather than inventing a general negative clamp. |
| `player_calculate_damage_component` | 469 | `1` | `base_power = 1;` | Replace an exactly zero combined threshold with one so the following denominator is nonzero. Preserve the exact zero check rather than inventing a general negative clamp. |
| `player_calculate_damage_component` | 471 | `2` | `return defense + (attack * attack) / (base_power * 2);` | Twice the combined threshold divides the squared-attack term. The factor two is a mathematical weighting in this formula; its original balance rationale is unproven. |
| `player_apply_damage` | 542 | `2` | `damage += PLAYER_DAMAGE_SUBUNITS_PER_HP / 2;` | Half a damage unit is added before converting the summed tenths to HP. The literal two selects the midpoint; preserve this rounding before the independent Q12 and multiplier stages. |
| `player_apply_damage` | 545 | `0` | `if (loss != 0) {` | Only nonzero numeric loss updates HP and requests a damage frame. Status side effects were already processed, even if this branch is skipped. |
| `player_apply_damage` | 547 | `0` | `if (remaining <= 0) {` | Zero is the lower bound for remaining HP after subtraction. Store it without calling death_begin here; retain the existing later player-state handling. |
| `player_apply_damage` | 548 | `0` | `remaining = 0;` | Zero is the lower bound for remaining HP after subtraction. Store it without calling death_begin here; retain the existing later player-state handling. |
| `player_apply_radial_damage` | 576 | `1` | `if (distance == -1) {` | The signed distance-query rejection result -1 skips all radial arithmetic and the damage call. It is not an attenuation or radius value. |
| `player_apply_radial_damage` | 589 | `0` | `component0, component1, component2, 0, component3, component4,` | Status argument zero applies no new status through this radial wrapper; the other channel and multiplier arguments keep their ordered forwarding. |
| `player_select_magic` | 598 | `0` | `player_state.magic_charge = 0;` | Changing selected magic empties the independent magic charge before storing the selected ID. |
| `player_select_magic` | 601 | `0` | `player_state.selected_magic_record = 0;` | The named no-magic sentinel clears the selected record pointer to null; otherwise the stored byte indexes its record. |
