# GAME map-object motion constants

## Function Match Plan

Baseline `f203a2d`; image GAME, `probe-gcc257-o2-g0`. Revisit both complete
map-object modules after the [original audit](game-map-object-constants.md).
The fresh evidence snapshot covers all sixteen claims, six semantic views
per claim, source history, and neighboring effect, collision, interaction and
damage consumers. Retail was initialized and hash-verified before capture.
The custom map definitions, placement records, linked doors and floor scripts
establish game ownership; SDK math, RNG, copying and audio callees remain
separate library bodies. No inventory evidence tier is promoted.

Name interaction distances, drop action-band bounds, RNG extraction, door
timers and motion, falling/bouncing units, emitter velocity numerators and
shifts, launch offsets, switch interpolation and reveal displacement. Reuse
the existing collision-cell and Fire Ball/Wind Cutter identities. Keep
unsigned emitter multiplication, signed halfword velocities, action-specific
timer meanings, exact predicates, call arguments and ordering. Content IDs
without supported asset identities remain explicit. In particular, retain
the uninitialized forward-probe result and boss-emitter coordinates outside
their supported cases, and the acquire helper's possible null result.

Each function's initial address, extent and strict score is listed below.
Shared signatures and field widths stay unchanged. The earlier dossier
records the action-table destinations, caller constraints and data owners;
the refreshed disassembly confirms those bindings, delay slots and ordered
references. Changes are constant substitutions, with no CFG reconstruction.
The first hypothesis is unchanged emitted runtime bytes and relocations.

Force both units to compile, compare all 484 scores, independently compile
all 112 units with frozen before/after sources and headers, and resolve each
reviewed function's words and references. Recheck exact functions against
retail and delinked targets. Run modern type checking, inventory, Ruff,
existing tests, whitespace validation and full `kf build`. Document existing
partial results and full-build failures without claiming closure.

| GAME VA / bytes | Function | Initial strict % |
| --- | --- | ---: |
| 80030a98 / 484 | map_apply_copy_region | 100 |
| 80030c7c / 572 | map_object_mark_collision_edge | 100 |
| 80030eb8 / 196 | map_object_probe_forward | 93.755104 |
| 80030f7c / 96 | map_object_pool_clear | 100 |
| 80030fdc / 44 | map_object_definitions_load | 100 |
| 80031008 / 1096 | map_object_pool_load | 100 |
| 80031450 / 168 | map_object_distance_to_point | 100 |
| 800314f8 / 204 | map_object_pool_find_near_point | 100 |
| 800315c4 / 448 | map_object_pool_find_interaction_from | 100 |
| 80031784 / 32 | map_object_start_action_if_idle | 100 |
| 800317a4 / 144 | map_object_effect_pool_acquire | 100 |
| 80031834 / 404 | map_object_spawn_effect | 94.455444 |
| 800319c8 / 396 | map_object_spawn_actor_debris | 100 |
| 80031b54 / 240 | map_object_pool_trigger_link | 100 |
| 80031c44 / 132 | map_object_pool_clear_link | 100 |
| 80031cc8 / 3096 | map_object_pool_update | 98.966410 |

## Units and relationships

The names describe decoded behavior. They do not establish the original
designer's tuning rationale. The earlier dossier supplies the full action
trace and content bindings; this pass gives the motion values source names.

| Named family | Value and consumer evidence |
| --- | --- |
| Door interaction local Z / closing probe radius | 550 is the local Z component rotated and added to the query point for both hinged leaves; 3000 is the actor/player clearance radius before closing. Neither is the map object's own collision radius. |
| Drop ID band ends | Exclusive 43/48/65 bounds route objects to fall-and-tip, fall-and-spin and bounce. Individual asset identities remain unresolved. |
| Drop random yaw | Shift 3 extracts 12 angle bits from the SDK's 15-bit random result. This is distinct from the equal-valued length-square downshift. |
| Gold scatter / initial velocity | Radius 600 in X/Z, initial Y velocity -120 world units/update. Bounce gravity then increases Y velocity by 30 each update. |
| Door hold / close first | Timer 250 starts the stationary hold; 300 starts closing, giving 50 stationary updates. A nonempty probe holds timer 300. |
| Swing door | 32 opening updates, quarter-turn/32 = 32 yaw units per update, close end 300+32 = 332. The partner moves oppositely; sound and collision-edge changes keep their original timing. |
| Lift door | 41 opening updates at 60 world units/update give 2460 units of lift. Close end 300+41 = 341 restores the same displacement. |
| Fall-and-tip | Gravity 20 world units/update squared. At floor contact the velocity halfword changes meaning: initial angular velocity 16, angular acceleration 16 angle units/update squared, clamped at a quarter turn. Separate names express those units. |
| Fall-and-spin | Y step 20 world units/update, yaw step 256 angle units/update (22.5 degrees). |
| Bounce | Pitch step 160 (14.0625 degrees); gravity 30; stop threshold 120 applied to the updated velocity. Reversal keeps signed arithmetic halving, and the timer remains a direction toggle. |
| Emitter range / countdown | Range 30000. Random shift 12 produces 0..7, added to base 10 or boss base 20. Stored countdowns are 10..17 or 20..27; old-zero testing makes emission intervals 11..18 or 21..28 updates. |
| Emitter unsigned velocity | Q12 trig times 175u shifted 10 gives nominal speed 700; Fire Ball uses 25u shifted 7 for 800; boss uses 225u shifted 10 for 900. Macros retain unsigned multiplication and logical shifting before signed halfword storage; no signed rewrite or reassociation is made. |
| Launch offsets | Fire Ball Y -1400, ordinary Wind Cutter Y +600; boss Y -1000 and X/Z offsets ±1100/±1000 for yaw zero/half-turn. Other boss yaws still leave X/Z uninitialized. |
| Boss sound random limit | (RAND_MAX+1)/8 = 4096 accepted outcomes of 32768, independent of Q12 unity. The sound decision and countdown consume separate RNG draws. |
| Effect switch phase step | 128 is 1/32 of Q12 unity. Forward clamps to 4095; reverse retains unsigned halfword underflow and the strict greater-than-unity test. |
| Reveal depth / settle step / end / lift | Loader adds 10000 Y; initial action subtracts 10000+(6-1)*40 = 10200. Timers 1..5 add 40 each, exactly undoing the 200-unit overshoot. Timer 6 performs floor completion once. The algebra expresses the observed relationship, not recovered original syntax. |
| Restore-point yaw step | 8 angle units/update, one full rotation per 512 updates. |

Existing `KF_MAP_CELL_FLOOR` and `KF_MAP_CELL_BLOCKED` now reach all five
collision-edge call sites. The setter writes those values to the collision
shape grid; they are not Boolean action flags. Existing Fire Ball and Wind
Cutter effect-kind names replace their three emitter arguments. The effect
constructor's variadic argument counts and sound behavior stay unchanged.

## Remaining occurrence ledger

This census supersedes the original dossier's 259-occurrence ledger for these
two source modules. After the [map-effect follow-up](game-map-effect-identities.md),
**170 remain**, comprising 77 in `map_object.c` and 93 in `map_object_pool.c`.
The motion pass removed **88 inline numeric uses**; the follow-up names one
more effect-kind argument. Thirty of
the remaining occurrences are authored map-copy bytes. The lexer excludes
comments, strings, digits in identifiers, retail claims, enums and named
macro definitions. Signs are operators; a listed token `1` can belong to
the no-result expression `-1`. Repeated tokens each receive their own row.

The 0x20 effect class and effect 14 remain explicit protocol identities;
broader effect naming is still open. The follow-up establishes kind 48 as
`KF_EFFECT_KIND_MAP_SWITCH` and removes that occurrence from this ledger. Object IDs and floor/copy-region bindings remain authored
data, without invented asset names. All other retained occurrences have
their specific arithmetic, initialization, geometry or state role below.

### `src/game/map_object.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| map_object_pool_find_interaction_from | 77 | `0` | `offset.vy = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_find_interaction_from | 85 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| map_object_pool_find_interaction_from | 90 | `0` | `offset.vy = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_find_interaction_from | 98 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| map_object_pool_find_interaction_from | 103 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| map_object_pool_find_interaction_from | 107 | `1` | `return -1;` | Established negative-one distance-query miss result. |
| map_object_start_action_if_idle | 115 | `0` | `object->action_timer = 0;` | Start this selected action at its initial timer. |
| map_object_effect_pool_acquire | 123 | `0` | `KfMapObject *oldest = 0;` | Null object pointer initialization or presence check. |
| map_object_effect_pool_acquire | 124 | `0` | `s32 oldest_age = 0;` | Initial greatest age; strict improvement preserves the first winner and can leave no winner. |
| map_object_effect_pool_acquire | 132 | `0` | `if (age < 0) {` | Negative difference detects halfword sequence wrap. |
| map_object_effect_pool_acquire | 133 | `0x10000` | `age += 0x10000;` | Halfword modulus 65536 corrects a negative sequence-age difference. |
| map_object_effect_pool_acquire | 140 | `0` | `} while (--count != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_spawn_effect | 170 | `0` | `object->rotation.z = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_spawn_effect | 171 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_spawn_effect | 176 | `0` | `object->link.vertical_velocity = 0;` | Initial vertical rest before the selected drop action accelerates. |
| map_object_spawn_effect | 179 | `0` | `object->link.vertical_velocity = 0;` | Initial vertical rest before the selected drop action accelerates. |
| map_object_spawn_effect | 182 | `0` | `object->link.vertical_velocity = 0;` | Initial vertical rest before the selected drop action accelerates. |
| map_object_spawn_actor_debris | 197 | `39` | `object->object_id = 39;` | Authored object resource selector: gold drop 39, door sound variant 119, or enabled restore point 123; asset identity not inferred. |
| map_object_spawn_actor_debris | 206 | `0` | `object->rotation.z = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_spawn_actor_debris | 207 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_trigger_link | 222 | `1` | `u16 count = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| map_object_pool_trigger_link | 243 | `0` | `} while (count-- != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_pool_clear_link | 251 | `1` | `u16 count = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| map_object_pool_clear_link | 261 | `0` | `} while (count-- != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_pool_update | 286 | `0` | `for (count = KF_MAP_OBJECT_CAPACITY; count != 0; object++, count--) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_pool_update | 296 | `0` | `pair = 0;` | Null object pointer initialization or presence check. |
| map_object_pool_update | 301 | `0` | `if (pair != 0) {` | Null object pointer initialization or presence check. |
| map_object_pool_update | 304 | `0` | `if (timer == 0) {` | First swing-opening update plays the sound. |
| map_object_pool_update | 305 | `0x77` | `if (object->object_id == 0x77) {` | Authored object resource selector: gold drop 39, door sound variant 119, or enabled restore point 123; asset identity not inferred. |
| map_object_pool_update | 313 | `1` | `if (timer == MAP_SWING_DOOR_OPEN_UPDATES - 1) {` | Last zero-based opening update, when the collision edge opens. |
| map_object_pool_update | 322 | `1` | `if (map_object_probe_forward(object, object->rotation.y - KF_ANGLE_QUARTER_TURN) != -1) {` | Established negative-one distance-query miss result. |
| map_object_pool_update | 327 | `0x77` | `if (object->object_id == 0x77) {` | Authored object resource selector: gold drop 39, door sound variant 119, or enabled restore point 123; asset identity not inferred. |
| map_object_pool_update | 336 | `0` | `if (pair != 0) {` | Null object pointer initialization or presence check. |
| map_object_pool_update | 345 | `0` | `if (elapsed == 0) {` | First lift-opening update plays the sound. |
| map_object_pool_update | 349 | `1` | `if (elapsed == MAP_LIFT_DOOR_OPEN_UPDATES - 1) {` | Last zero-based opening update, when the collision edge opens. |
| map_object_pool_update | 358 | `1` | `if (map_object_probe_forward(object, object->rotation.y) != -1) {` | Established negative-one distance-query miss result. |
| map_object_pool_update | 370 | `0` | `if (object->action_timer == 0) {` | Zero timer selects falling before floor contact. |
| map_object_pool_update | 380 | `1` | `object->action_timer = 1;` | One selects tipping after floor contact; same halfword now carries angular velocity. |
| map_object_pool_update | 400 | `1` | `object->action_timer = 1;` | Completion write before the spin action becomes idle. |
| map_object_pool_update | 409 | `0` | `if (object->action_timer == 0) {` | Zero timer selects positive bounce pitch direction. |
| map_object_pool_update | 420 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_update | 423 | `1` | `object->link.vertical_velocity = -(object->link.vertical_velocity >> 1);` | Arithmetic division by two before sign reversal at a bounce. |
| map_object_pool_update | 424 | `0` | `object->action_timer = object->action_timer == 0;` | Boolean inversion switches bounce pitch direction. |
| map_object_pool_update | 431 | `0` | `if (object->action_timer == 0) {` | Zero countdown permits a new range check and emission. |
| map_object_pool_update | 433 | `0` | `object->position_x, KF_COLLISION_IGNORE_HEIGHT, object->position_z, MAP_EMITTER_PLAYER_RANGE, 0)` | Zero vertical extent of the height-ignored distance/collision query. |
| map_object_pool_update | 434 | `1` | `== -1) {` | Established negative-one distance-query miss result. |
| map_object_pool_update | 438 | `137` | `case 137:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_update | 439 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_update | 444 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| map_object_pool_update | 445 | `0xe` | `0xe,` | Effect kind 14 emitted by object 137; independent behavioral identity unresolved. |
| map_object_pool_update | 451 | `124` | `case 124:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_update | 452 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_update | 460 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| map_object_pool_update | 467 | `125` | `case 125:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_update | 468 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_update | 476 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| map_object_pool_update | 481 | `1` | `1);` | True requests Wind Cutter sound through the seventh constructor argument. |
| map_object_pool_update | 484 | `115` | `case 115:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_update | 488 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_update | 492 | `0` | `case 0:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| map_object_pool_update | 504 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| map_object_pool_update | 519 | `0` | `if (object->link.link_id == KF_MAP_LINK_NONE && object->action_timer == 0) {` | Zero means the one-shot effect-release request has not been sent. |
| map_object_pool_update | 521 | `1` | `object->action_timer = 1;` | One records that the release request was sent. |
| map_object_pool_update | 529 | `1` | `effect_pool_records[object->link.action_parameter].visual.animation_phase = (KF_FIXED12_ONE - 1);` | Greatest animation phase below Q12 unity; also the reverse departure endpoint. |
| map_object_pool_update | 535 | `0` | `if (record->visual.animation_phase == 0) {` | Zero interpolation endpoint: departure sound test or reset after reverse underflow. |
| map_object_pool_update | 541 | `1` | `record->visual.animation_phase = (KF_FIXED12_ONE - 1);` | Greatest animation phase below Q12 unity; also the reverse departure endpoint. |
| map_object_pool_update | 551 | `1` | `if (record->visual.animation_phase == (KF_FIXED12_ONE - 1)) {` | Greatest animation phase below Q12 unity; also the reverse departure endpoint. |
| map_object_pool_update | 557 | `0` | `record->visual.animation_phase = 0;` | Zero interpolation endpoint: departure sound test or reset after reverse underflow. |
| map_object_pool_update | 563 | `0` | `if (object->link.link_id == KF_MAP_LINK_NONE && object->action_timer == 0) {` | Zero means the one-shot region copy has not been processed. |
| map_object_pool_update | 567 | `1` | `object->action_timer = 1;` | One records completion even when the copy parameter was absent. |
| map_object_pool_update | 574 | `0` | `if (object->action_timer == 0) {` | Zero selects the initial reveal displacement. |
| map_object_pool_update | 576 | `1` | `object->action_timer = 1;` | First settling timer; updates 1 through 5 restore the overshoot. |
| map_object_pool_update | 581 | `3` | `if (player_state.progress_state.current_floor == 3) {` | Authored floor number selects its persistent reveal-completion script. |
| map_object_pool_update | 588 | `2` | `map_apply_copy_region(2);` | Authored copy-region table index applied when floor 3 completes four reveals. |
| map_object_pool_update | 589 | `3` | `map_apply_copy_region(3);` | Authored copy-region table index applied when floor 3 completes four reveals. |
| map_object_pool_update | 594 | `1` | `} else if (player_state.progress_state.current_floor == 1) {` | Authored floor number selects its persistent reveal-completion script. |
| map_object_pool_update | 606 | `0x7b` | `object->object_id = 0x7b;` | Authored object resource selector: gold drop 39, door sound variant 119, or enabled restore point 123; asset identity not inferred. |

### `src/game/map_object_pool.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| initializers | 25 | `55` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| initializers | 25 | `33` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| initializers | 25 | `50` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| initializers | 25 | `39` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| initializers | 25 | `3` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| initializers | 25 | `3` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| initializers | 26 | `47` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| initializers | 26 | `16` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| initializers | 26 | `30` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| initializers | 26 | `20` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| initializers | 26 | `3` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| initializers | 26 | `3` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| initializers | 27 | `58` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| initializers | 27 | `44` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| initializers | 27 | `15` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| initializers | 27 | `48` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| initializers | 27 | `3` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| initializers | 27 | `3` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| initializers | 28 | `64` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| initializers | 28 | `44` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| initializers | 28 | `37` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| initializers | 28 | `45` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| initializers | 28 | `3` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| initializers | 28 | `3` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| initializers | 29 | `0` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region source X in map cells; explicit table data. |
| initializers | 29 | `0` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region source Z in map cells; explicit table data. |
| initializers | 29 | `36` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region destination X in map cells; explicit table data. |
| initializers | 29 | `4` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region destination Z in map cells; explicit table data. |
| initializers | 29 | `7` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region width in map cells; explicit table data. |
| initializers | 29 | `1` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region height in map cells; explicit table data. |
| map_apply_copy_region | 50 | `0` | `while (height-- != 0) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_apply_copy_region | 54 | `0` | `while (width-- != 0) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_mark_collision_edge | 83 | `3` | `case 3:` | Behavior 3 shares lift-door collision geometry; independent interaction/action identity unresolved. |
| map_object_mark_collision_edge | 86 | `0x000` | `case 0x000:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| map_object_mark_collision_edge | 103 | `0x000` | `case 0x000:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| map_object_mark_collision_edge | 104 | `1` | `map_collision_grid[cell_z][cell_x + 1] =` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 105 | `1` | `map_collision_grid[cell_z - 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 105 | `1` | `map_collision_grid[cell_z - 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 108 | `1` | `map_collision_grid[cell_z + 1][cell_x] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 109 | `1` | `map_collision_grid[cell_z + 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 109 | `1` | `map_collision_grid[cell_z + 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 112 | `1` | `map_collision_grid[cell_z][cell_x - 1] =` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 113 | `1` | `map_collision_grid[cell_z + 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 113 | `1` | `map_collision_grid[cell_z + 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 116 | `1` | `map_collision_grid[cell_z - 1][cell_x] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 117 | `1` | `map_collision_grid[cell_z - 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_mark_collision_edge | 117 | `1` | `map_collision_grid[cell_z - 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| map_object_probe_forward | 137 | `0` | `point_x, KF_COLLISION_IGNORE_HEIGHT, point_z, MAP_DOOR_CLOSING_PROBE_RADIUS, 0,` | Zero vertical extent of the height-ignored distance/collision query. |
| map_object_probe_forward | 142 | `0x000` | `case 0x000:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| map_object_pool_clear | 164 | `1` | `u16 index = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| map_object_pool_clear | 172 | `1` | `link_words[1] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| map_object_pool_clear | 172 | `0` | `link_words[1] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| map_object_pool_clear | 173 | `0` | `link_words[0] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| map_object_pool_clear | 173 | `0` | `link_words[0] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| map_object_pool_clear | 175 | `0` | `} while (index-- != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_pool_clear | 176 | `0` | `map_object_effect_sequence_180 = 0;` | Reset the transient group allocation sequence to its initial value. |
| map_object_pool_clear | 177 | `0` | `map_object_effect_sequence_170 = 0;` | Reset the transient group allocation sequence to its initial value. |
| map_object_pool_clear | 178 | `0` | `map_object_effect_sequence_160 = 0;` | Reset the transient group allocation sequence to its initial value. |
| map_object_definitions_load | 190 | `0` | `} while (--count != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_pool_load | 203 | `0` | `u16 ended = 0;` | Boolean end-of-placement-list flag; after the sentinel, remaining slots are marked free. |
| map_object_pool_load | 207 | `2` | `u32 effect_output[2];` | Existing two-word constructor output scratch extent. |
| map_object_pool_load | 210 | `1` | `remaining = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| map_object_pool_load | 212 | `1` | `if (ended == 1) {` | Boolean end-of-placement-list flag; after the sentinel, remaining slots are marked free. |
| map_object_pool_load | 220 | `0` | `object->rotation.z = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_load | 221 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| map_object_pool_load | 231 | `0` | `if (definition->collision_radius != 0) {` | Zero collision radius disables occupancy or skips a non-colliding definition. |
| map_object_pool_load | 232 | `1` | `collision_adjust_cell_occupancy(object->cell_x, object->cell_z, 1);` | Increment occupancy by exactly one loaded colliding object. |
| map_object_pool_load | 235 | `136` | `case 136:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 238 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| map_object_pool_load | 245 | `115` | `case 115:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 246 | `124` | `case 124:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 247 | `125` | `case 125:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 248 | `137` | `case 137:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 251 | `138` | `case 138:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 254 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| map_object_pool_load | 262 | `139` | `case 139:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 265 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| map_object_pool_load | 273 | `135` | `case 135:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 276 | `0` | `0, KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, KF_EFFECT_KIND_MAP_SWITCH, &object->position_x,` | Stored effect ID zero for the switch visual. |
| map_object_pool_load | 281 | `56` | `case 56:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 282 | `63` | `case 63:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 283 | `64` | `case 64:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 284 | `68` | `case 68:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 285 | `69` | `case 69:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 289 | `111` | `case 111:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 290 | `123` | `case 123:` | Authored object ID selecting the loader/emitter binding in the original dossier; asset identity unresolved. |
| map_object_pool_load | 300 | `1` | `ended = 1;` | Boolean end-of-placement-list flag; after the sentinel, remaining slots are marked free. |
| map_object_pool_load | 304 | `0` | `if (remaining-- == 0) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| map_object_distance_to_point | 329 | `1` | `return -1;` | Established negative-one distance-query miss result. |
| map_object_pool_find_near_point | 339 | `0` | `for (index = 0; index < KF_MAP_OBJECT_CAPACITY; index++, object++) {` | Start scanning at the first pool slot. |
| map_object_pool_find_near_point | 344 | `0` | `if (radius == 0) {` | Zero collision radius disables occupancy or skips a non-colliding definition. |
| map_object_pool_find_near_point | 348 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| map_object_pool_find_near_point | 352 | `1` | `return -1;` | Established negative-one distance-query miss result. |

## Final verification

All sixteen function scores in the plan remain unchanged: thirteen exact,
three partial. Both units were force-compiled, then the full dependency
comparison and full `kf build` ran. Frozen before/after compilation of all
112 units preserves every runtime section, symbol and ordered relocation;
only the two intended source objects change `.debug_line`.

The sixteen reviewed bodies preserve all 1937 emitted words, 66 direct
call references and 63 data/address references. All 1014 words in the thirteen
exact functions independently match retail and their delinked targets.
The thirty authored copy-region bytes also match retail, source and target.

| Function | Final verdict | Words / calls / addresses |
| --- | --- | ---: |
| `map_apply_copy_region` | Exact, unchanged | 121 / 0 / 8 |
| `map_object_mark_collision_edge` | Exact, unchanged | 143 / 0 / 7 |
| `map_object_probe_forward` | Partial, unchanged | 49 / 1 / 1 |
| `map_object_pool_clear` | Exact, unchanged | 24 / 0 / 4 |
| `map_object_definitions_load` | Exact, unchanged | 11 / 0 / 1 |
| `map_object_pool_load` | Exact, unchanged | 274 / 11 / 8 |
| `map_object_distance_to_point` | Exact, unchanged | 42 / 1 / 0 |
| `map_object_pool_find_near_point` | Exact, unchanged | 51 / 1 / 2 |
| `map_object_pool_find_interaction_from` | Exact, unchanged | 112 / 5 / 2 |
| `map_object_start_action_if_idle` | Exact, unchanged | 8 / 0 / 0 |
| `map_object_effect_pool_acquire` | Exact, unchanged | 36 / 0 / 1 |
| `map_object_spawn_effect` | Partial, unchanged | 100 / 5 / 2 |
| `map_object_spawn_actor_debris` | Exact, unchanged | 99 / 6 / 1 |
| `map_object_pool_trigger_link` | Exact, unchanged | 60 / 1 / 2 |
| `map_object_pool_clear_link` | Exact, unchanged | 33 / 0 / 1 |
| `map_object_pool_update` | Partial, unchanged | 774 / 35 / 23 |

The first retail differences remain: forward probe at GAME `80030ec4`
loads position X into `$a3` instead of `$t0`; spawn effect at `80031838`
saves `$ra` at stack +0x20 instead of +0x24 (100 source words versus 101
retail); updater at `80031dcc` emits `lui $a0,0x8005` where retail loads
the object ID with `lbu $v1,0($s3)`. These are unchanged observations,
not attributed compiler mechanisms. No new exact result is banked here.

The live 484-score comparison has exactly one independent concurrent change:
GAME `audio_initialize` at `800328e0` improves from 95.121950 to 100.
Its separately captured source was compiled under both this campaign's old
and new headers at the same path; both runtime objects are identical and
equal the live audio object. The other 111 live objects agree with the frozen
naming controls. All map-object scores and all other live scores are unchanged.
The audio source, its documentation and the generated score panel are outside
this commit.

Inventory validation, `ruff check scripts tests` and `git diff --check` pass.
All 680 existing tests pass (86.522 seconds); no tests or size assertions were
added. Modern diagnostics preserve the same 300 errors and 65/112 passing
source/image variants. Full `kf build` retains existing data-ownership and
placement failures: source data matches PSX 0/1, GAME 9/42 and OPEN 2/19;
target relink verifies PSX 1/1, GAME 75/77 and OPEN 34/38, with six conflicting
section bases and no artifact failures. No tooling or flake files changed.
The broader naming objective remains open, including ten unresolved
`unknown_` occurrences in `src`.
