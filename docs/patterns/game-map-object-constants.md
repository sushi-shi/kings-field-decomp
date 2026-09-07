# GAME map-object constants

The later [motion-constant pass](game-map-object-motion-constants.md) names
the interaction distances and motion/timing units left inline here. Its
ledger (now 170 occurrences after the shared-identity follow-up) and
verification supersede this historical census.

## Function Match Plan

Continue from master `20bf14c` with hash-verified GAME retail and the unchanged
`probe-gcc257-o2-g0` profiles. Audit all constants in `map_object.c` and
`map_object_pool.c`, carrying shared interaction/action names into their
renderer, interaction dispatcher, actor-drop callers and save/restore code.
The map's 190 custom 44-byte objects, eight-byte definitions, link policy and
effect-pool coupling establish game ownership. SDK matrix, square-root, RNG,
copy and audio providers remain separate; no vendored body is reconstructed.

Before editing, refresh each function's address/extent, disassembly/CFG,
callers, callees, strings, strict score and source history. Independently
resolve all exact functions' complete source and target words and ordered
referents to retail. Inspect partial dispatchers' byte selectors, halfword
timers, signed movement, jump-table bindings and concrete callees. Keep
candidate pointer rows distinct from decoded direct calls and validated
address pairs; this campaign does not promote their inventory status.

Name the decoded interaction types, running actions, reusable-link boundary,
switch phases and three transient object bands. Separate the free object,
idle action, absent link and absent parameter meanings of `0xff`. Preserve
the existing byte/halfword types, raw content IDs, exact predicates, operation
order, unknown fields and unsupported-input behavior. Reuse established angle,
Q12, collision and volume constants only in their actual consumer domains.

First hypothesis: each substitution expands to exactly the original source
tokens and values. Require fresh compilation of affected units, unchanged
code/data in all 112 source objects, all 484 strict scores and the complete
objdiff report. Verify every exact function against retail again, and run
lint, repository tests, whitespace checks and full `kf build`. Existing
partial functions and full-build closure failures remain separate work.

## Function snapshots and verdicts

Every initial and final score in the table is identical. Sixteen functions
remain exact; eight remain partial. The last column counts complete retail
words and ordered call/data references independently resolved from both
objects for exact functions. A dash makes no whole-function retail-equality
claim. All partial functions retain their complete pre-campaign emitted code.

| GAME VA / extent (hex) | Function | Initial = final strict % | Words / calls / data addresses |
| --- | --- | ---: | ---: |
| `0x8001ebb8 / 180` | `render_map_object` | 95.604164 | — |
| `0x8002fa88 / d90` | `actor_update_current_action` | 98.938940 | — |
| `0x80030a98 / 1e4` | `map_apply_copy_region` | 100.000000 | 121 / 0 / 8 |
| `0x80030c7c / 23c` | `map_object_mark_collision_edge` | 100.000000 | 143 / 0 / 7 |
| `0x80030eb8 / c4` | `map_object_probe_forward` | 93.755104 | — |
| `0x80030f7c / 60` | `map_object_pool_clear` | 100.000000 | 24 / 0 / 4 |
| `0x80030fdc / 2c` | `map_object_definitions_load` | 100.000000 | 11 / 0 / 1 |
| `0x80031008 / 448` | `map_object_pool_load` | 100.000000 | 274 / 11 / 8 |
| `0x80031450 / a8` | `map_object_distance_to_point` | 100.000000 | 42 / 1 / 0 |
| `0x800314f8 / cc` | `map_object_pool_find_near_point` | 100.000000 | 51 / 1 / 2 |
| `0x800315c4 / 1c0` | `map_object_pool_find_interaction_from` | 100.000000 | 112 / 5 / 2 |
| `0x80031784 / 20` | `map_object_start_action_if_idle` | 100.000000 | 8 / 0 / 0 |
| `0x800317a4 / 90` | `map_object_effect_pool_acquire` | 100.000000 | 36 / 0 / 1 |
| `0x80031834 / 194` | `map_object_spawn_effect` | 94.455444 | — |
| `0x800319c8 / 18c` | `map_object_spawn_actor_debris` | 100.000000 | 99 / 6 / 1 |
| `0x80031b54 / f0` | `map_object_pool_trigger_link` | 100.000000 | 60 / 1 / 2 |
| `0x80031c44 / 84` | `map_object_pool_clear_link` | 100.000000 | 33 / 0 / 1 |
| `0x80031cc8 / c18` | `map_object_pool_update` | 98.966410 | — |
| `0x80033f64 / 288` | `map_ambient_script_floor1` | 100.000000 | 162 / 5 / 19 |
| `0x800346a8 / 38c` | `map_floor5_transition_cutscene` | 100.000000 | 227 / 12 / 14 |
| `0x80034de4 / 904` | `map_interaction_dispatch` | 83.012130 | — |
| `0x80035b5c / 2b8` | `map_world_state_persist` | 94.821840 | — |
| `0x80035e44 / 69c` | `map_restore_floor_state` | 99.964540 | — |
| `0x80036554 / a4` | `map_load_floor` | 100.000000 | 41 / 4 / 4 |

## Interaction types and running actions

The byte at definition+0 selects interaction behavior; object+40 selects its
running action. Some behavior values are passed directly to the action
starter, but many are not. The starter at `80031784` only writes the action
and clears the halfword timer when the old action equals 255. Free object ID,
idle action, absent link, absent parameter and absent copy-region ID have
distinct constants despite sharing that byte value.

The interaction dispatcher loads its table base `80012a7c` at
`800351b4/351b8` and indexes it by the definition byte. The complete 84-word
table binds the following decoded cases; all other entries go to default
notification at `8003561c`. Behavior 255 is beyond the table guard and takes
that default too; `BEHAVIOR_NONE` does not bypass the notification code.

| Behavior, decimal | Table destination | Supported meaning |
| --- | --- | --- |
| 0 / 1 | `80035408` | Hinged door / partner leaf: find a nearby leaf, store reciprocal indices and start the definition's action. Only behavior 0 checks its own lock here. |
| 2 | `800353a8` | Lift door: require facing tolerance, idle action and absent lock before starting action 2. |
| 8 | `800351d0` | Hinged item container: animate pitch, then offer stored item IDs to the inventory menu. |
| 9 | `80035328` | Item container: offer its four stored item bytes without the hinge animation. |
| 11 | `80035594` | Restore point: an absent link permits `player_restore_vitals_with_color_cycle`. |
| 13 | `800355b4` | Screen image: object IDs 130/131 choose image group 0/1 and the link byte selects the image. |
| 14 | `80035604` | Save point: persist world state, then enter the save confirmation menu. |
| 64 | `8003551c` | Item pickup: offer the object ID itself to the inventory menu and free it on success. |
| 65 | `8003554c` | Gold pickup: add the first link halfword to player gold, notify, then free the object. |
| 83 | `8003557c` | Effect switch: a present link permits setting its timer to the forward phase. |

Behavior 10 is named from the loader's explicit comparison and action-10
starter at `800313b0..800313d0`, not an interaction-table case. Behavior 3
remains numeric: the collision-edge marker treats it like a lift door and
the renderer assigns the same depth bias, but the reviewed code establishes
neither an independent action nor a more specific object identity. No name
is inferred solely from sharing that geometry profile.

The action table at `80012888` is loaded at `80031d34/31d38` and indexed by
the action byte after a `<=98` guard. All 99 retail pointer words were checked:

| Action, decimal | Destination | Supported meaning |
| --- | --- | --- |
| 0 | `80031d50` | Swing the door and any linked partner in opposite directions, wait, then close. |
| 2 | `80031eb0` | Lift the door, wait, then lower it. |
| 10 | `800326f0` | Apply the parameter's map-copy region once the link is absent. |
| 11 | `8003286c` | Enable the restore point's visible object 123 and continue rotating it after unlinking. |
| 12 | `80032734` | Reveal a map piece from its hidden Y offset and perform floor-specific completion work. |
| 80 | `800321a8` | Emit projectiles near the player while the link is present. |
| 81 / 82 | `80032534` | Request release of the orbit/short-swing or long-swing effect once, after unlinking. |
| 83 | `80032584` | Advance the linked effect's switch animation, fire its link, and optionally reverse. |
| 96 | `80031f8c` | Fall to the floor, then tip to a quarter turn. |
| 97 | `8003205c` | Fall at fixed speed while spinning. |
| 98 | `800320dc` | Fall, bounce with reduced vertical velocity, and alternate pitch direction. |

Every other table entry, including **action 1**, points to the loop tail at
`80032894`. The partner leaf has no independent updater: action 0 changes
the paired record's yaw. Do not add an action-1 body or change its state as
part of naming. The serializer, renderer and lock/interaction consumers retain
their original predicates and byte domains.

For actions 11 and 80..82, the trigger helper clears matching links. For
other actions, linked behaviors below 8 can start when link ID is at least
128. The clear helper separately admits behaviors 0..8. Valid linked IDs
128..254 also make an effect switch reverse and become reusable; 255 retains
its absent-link meaning. The original predicates do not gain extra validity
guards. The orbit helper continues orbiting after the release request;
the [effect audit](game-effect-literals.md) documents that asymmetry.

## Three transient groups and switch phases

Each transient group has ten slots. The gold group begins at 160, definition
drops at 170 and placement drops at 180. Actor death supplies computed gold
to the first group, definition drop parameters with kind 1 to the second,
and placement drop IDs with kind 0 to the third. The floor-5 cutscene also
borrows the third group, so that name describes its usual ownership rather
than excluding temporary scripted users. The generic spawn helper treats
every nonzero kind like definition-drop kind 1; no new range check is added.

The gold spawner stores its historical `source` argument across the first
two link bytes and selects object 39. The actor caller supplies a gold amount,
and the gold interaction consumes that same unsigned halfword. Save/restore
preserves those two amount bytes for slots 160..169. The other two groups
share twenty serialized position/yaw records. These relationships support
the group names independently of their numeric addresses.

An acquire operation returns the first free object or the object with the
greatest unsigned-sequence age. The literal 65536 corrects a negative
subtraction across the stored halfword wrap. Initial best age zero and strict
`<` preserve the first winner and can leave a null result when every occupied
entry has age zero; this campaign does not add a fallback or caller guard.

The switch timer has four named states: ready 0, forward 1, disabled 2 and
reverse 3. Forward animation adds 128 Q12 units until it reaches 4096, clamps
to 4095, and triggers the link. A reusable link selects reverse; a one-shot
link becomes absent, causing the next update to disable the switch. Reverse
subtracts 128 from the unsigned halfword; after 32 steps from 4095 it wraps
to 65535, passes the preserved `>4096` test, and resets phase/timer to zero.
It must not be replaced with a signed-negative test or the actor's yaw mask.
Interaction can write forward again whenever the link is present; no extra
ready-only condition is invented.

## Remaining literal census

After the [saved-script update](game-map-script-state.md), the two complete
source modules retain **259 numeric occurrences** outside
retail claims and constant definitions: 30 initialized map-copy bytes and
229 function literals. The census excludes comments, strings and digits
inside identifiers. Other modules receive only the shared substitutions
listed above; their unrelated literals remain in the wider audit.

| Module / function | Remaining occurrences |
| --- | ---: |
| `map_object.c` total | 162 |
| `map_object_pool_find_interaction_from` | 8 |
| `map_object_start_action_if_idle` | 1 |
| `map_object_effect_pool_acquire` | 5 |
| `map_object_spawn_effect` | 9 |
| `map_object_spawn_actor_debris` | 8 |
| `map_object_pool_trigger_link` | 2 |
| `map_object_pool_clear_link` | 2 |
| `map_object_pool_update` | 127 |
| `map_object_pool.c` total | 97 |
| Map-copy initializer | 30 |
| `map_apply_copy_region` | 2 |
| `map_object_mark_collision_edge` | 15 |
| `map_object_probe_forward` | 3 |
| `map_object_pool_clear` | 9 |
| `map_object_definitions_load` | 1 |
| `map_object_pool_load` | 32 |
| `map_object_distance_to_point` | 1 |
| `map_object_pool_find_near_point` | 4 |

| Consumer | Retained values and reason |
| --- | --- |
| Arithmetic and loops | `0`, `1`, `-1` are null pointers, zero components/velocity, boolean flags, initial counters, exhausted-count tests, unit increments and last-index adjustments. Zero timer initialization belongs to several different actions, not one universal lifecycle. |
| Link reset / loader scratch | Word indices 1/0 clear the complete eight-byte link in retail order; scratch extent 2 supplies the existing two-word effect output. Zero stores and cursor origins are explicit storage operations. |
| Empty placement / occupancy | `ended` 0/1 tracks the sentinel-terminated input. Nonzero collision radius admits occupancy and delta 1 adds that object. The fill arm only writes the object ID; it does not acquire extra clearing behavior. |
| Door interaction point | Both `0x226` values are local Z offset 550 world units. X is respectively minus/plus one 2000-unit map tile, Y is zero. The rotated offset is added to the query point before distance testing; it is not a translation of the object. The original choice of 550 is unknown. |
| Distance queries | `-1` is the established no-result return; zero radius skips a non-colliding definition. The forward probe retains radius 3000 world units and zero vertical extent, with the named ignore-height and terrain/object exclusions. |
| Collision-edge geometry | The remaining behavior 3 has the limited profile evidence above. Two cardinal `0x000` cases are zero yaw. Twelve `+1/-1` indices describe neighboring map cells; zero/one values passed by loader and door actions mark the original blocked/open cells. They are geometry values, not action IDs. |
| Drop routing | Object-ID boundaries 43,48,65 select fall-and-tip, fall-and-spin and bounce for IDs below each boundary. Higher IDs retain idle action. These are content-table bands whose individual asset identities remain unresolved. |
| Random drop yaw | `rand() >>3` maps the SDK's 0..32767 result into the 0..4095 angle range. The shift is RNG-range extraction, independent of the distance helper's named downshift by three. |
| Gold scatter | Object ID 39 remains an explicit resource selector. Two factors 600 set the X/Z scatter radius before Q12 rounding. Initial velocity -120 is world units per update; it raises the drop before the bounce action's gravity takes over. |

The initializers are the following five authored copy regions. Each row is
source X/Z, destination X/Z, width/height in map cells; zero is a valid source
coordinate. The new count names the table dimension, while the thirty byte
samples remain explicit data. The copier uses the exact stored counts and
copies all five grids: attribute, floor height, orientation, collision shape
and collision flags. It preserves traversal order even if regions overlap.

| Source X/Z | Destination X/Z | Width/height |
| --- | --- | --- |
| 55,33 | 50,39 | 3,3 |
| 47,16 | 30,20 | 3,3 |
| 58,44 | 15,48 | 3,3 |
| 64,44 | 37,45 | 3,3 |
| 0,0 | 36,4 | 7,1 |

## Loader content bindings

These exact numeric object IDs select authored resources and remain inline.
The action names express their decoded operation without inventing an asset
name. The loader's default action is idle, followed by the separate behavior-10
copy-region check. Its first successful action assignment wins.

| Object ID(s), decimal | Loader effect/action |
| --- | --- |
| 136 | Create orbit effect 17; action 81 holds its release linkage. |
| 115,124,125,137 | Start the projectile emitter. |
| 138 / 139 | Create short/long swinging effect 15/16; action 81/82. |
| 135 | Create effect `0x30` (48), model selector 5, and start the effect switch. The controller changes its animation phase rather than moving the map object. |
| 56,63,64,68,69 | Start reveal-map-piece and add 10000 world Y, placing the piece below its requested height. |
| 111,123 | Start enable-restore-point, which selects visible object 123 after unlinking. |

The three loader `0x20` type bits and the four emitter `0x20` bits preserve
the existing effect damage-credit class. They are not the collision query's
equal-valued skip-map-objects flag or the player-magic power bit. The effect
protocol currently leaves that class numeric; its exact word consumers and
actor-credit distinction are documented in the [effect protocol](game-effect-protocol.md).
The switch constructor's first argument zero is its stored ID, and its
unused input/output scratch representation remains unchanged.

## Action timing, motion and authored choices

Retained tuning values below have known units and consequences. Those facts
do not establish the original designer's reason for choosing each value.

| Action / consumer | Values and measured consequences |
| --- | --- |
| Swing-door opening | Old timers 0..31 each add 32 yaw units to the door and subtract 32 from its partner. The net turn is 1024 units, 90 degrees, in 32 updates. Timer 0 plays the opening sound; timer 31 opens the collision edge and writes timer 250. |
| Lift-door opening | Old timers 0..40 each subtract 60 world Y: 41 updates raise it 2460 units. Timer 0 plays sound 0; timer 40 opens its collision edge and writes 250. |
| Door hold and close | Timers 250..299 give 50 stationary updates. At 300 a nonempty forward probe resets the timer to 300 and postpones closing. Once clear, collision is marked closed and sound is played. Swing closes at old timers 300..331; lift closes at 300..340. Bounds 332/341 terminate after exactly the original opening displacement is undone. There is no claim that 300 is a 300-update hold. |
| Door sound resource | Object `0x77` (119) selects sound 1; the other swinging doors select sound 7. Lift doors select sound 0. These resource choices remain explicit; volume now uses the existing maximum-volume constant. |
| Fall-and-tip | Timer zero moves Y by the old signed velocity, then adds gravity 20. Contact clamps Y to floor, replaces velocity with 16 and sets timer 1. Thereafter the same halfword is angular velocity: pitch adds it, then it increases by 16 angle units/update. From zero pitch, eleven tipping updates reach/exceed a quarter turn and clamp there. This is not one physical unit throughout the action. |
| Fall-and-spin | Each update adds 20 world Y and 256 yaw units (22.5 degrees); contact clamps Y and ends the action. The stored timer 1 is a completion write, not a second motion phase. |
| Bounce | Y uses the old velocity, then gravity adds 30. Pitch adds/subtracts 160 angle units (14.0625 degrees), selected by a boolean timer that flips on bounce. Floor contact stops if the updated signed velocity is below 120; otherwise velocity becomes negative arithmetic-half of that value. Keep signed halfword narrowing and the existing order of gravity, collision and reversal. |
| Emitter range/countdown | Player range is 30000 world units with height ignored and zero vertical extent. A positive timer decrements even when the player has left range; a zero timer waits for range before spawning. Delays `(rand() >>12)+10` are 10..17, or +20 gives 20..27. Since spawning tests the old zero timer, consecutive emissions are 11..18 or 21..28 updates apart while enabled/in range. |
| Emitter object 137 | Effect `0xe` (14), zero Y direction and no position offset. Factors `175u >>10` applied to Q12 sine/negative-cosine give nominal horizontal speed 700 world units/update before quantization. The unsigned multiplication, logical shift and halfword store are preserved. |
| Emitter object 124 | Effect 5, launch Y minus 1400. Factors `25u >>7` give nominal speed 800, with the same unsigned/narrowing contract. |
| Emitter object 125 | Effect 7, launch Y plus 600, nominal speed 700 and optional sound flag 1. |
| Emitter object 115 | Requires the floor-5 boss-encounter flag. Effect 7 travels at nominal speed 900 from `225u >>10`, at yaw plus a quarter turn. Y is minus 1000; yaw zero uses X/Z offsets +1100/+1000 and half-turn uses -1100/-1000. Other yaws leave the point's X/Z uninitialized, as in the original. A separate `rand()<4096` requests sound for 4096 SDK outcomes; it is not a Q12 phase test. |
| Release requests | Timer 0/1 records whether the single request has been sent. Both map actions write the already named effect release phase 1, then leave that timer at 1. The orbit/short/long distinction remains in the loader and enum identities. |
| Effect switch | Step 128 is 1/32 of a complete 4096-unit animation. Forward and reverse timing, clamp 4095, and unsigned underflow are described above; these are interpolation units, not a 360-degree yaw. Sound 3 plays at each end's departure. |
| Copy-region trigger | Timer 0/1 is an unprocessed/processed flag. A missing parameter skips the copy but still records completion. The parameter indexes the exact five-row table; no additional region bounds check is inferred. |
| Reveal movement | Loader +10000, first update -10200, then timers 1..5 each +40 yield net zero displacement relative to the requested placement. Timer 6 performs completion work; the next increment prevents repeating it. The 40-unit step is the 200-unit overshoot divided across five settling updates. |
| Reveal floor completion | Floor 3 increments `revealed_piece_count` unless it equals `KF_MAP_FLOOR3_REQUIRED_REVEALS` (four); completing four reveals applies region IDs 2 and 3, plays sound 7, and clamps the count. Floor 1 sets `revival_enabled` once and plays sound 5. Sounds, floor numbers and region IDs remain authored bindings; the named count and flags now carry their own meanings. |
| Enabled restore point | Object `0x7b` (123) and yaw step 8 are its visible resource and continuous rotation: 0.703125 degrees/update, one turn per 512 updates. Original visual/resource tuning rationale is unknown. |

Retail `ADDRESS`, `DATA` and `RODATA` addresses/extents remain explicit
ownership evidence. Shared layout assertions keep their literal byte offsets
and extents as independent constraints. No unresolved field was renamed
merely to remove an `unknown_` occurrence.

## Verification

All seven affected units were force-compiled, followed by a full dependency
rebuild through `kf build compare -j4`. All 112 source objects preserve their
code, relocation entries, initialized data, switch tables and symbol metadata;
only four `.debug_line` sections change. The complete objdiff report and all
484 strict function scores are identical to the `20bf14c` baseline. All 1444
words across the sixteen exact reviewed functions resolve independently to
retail, including delay slots and ordered physical referents. All thirty
copy-region bytes in both source and target equal retail. All source/header
tokens retain their types, numeric values and operation order after declared
constant expansion. Config inventories, bank state and probe profiles have
no changes.

`ruff check scripts tests` and `git diff --check` pass. The full repository
suite ran 654 tests in 86.701 seconds: 653 passed and the existing untracked
save/load-hub control failed at word 38 (151 source words versus 152 retail
words). Its source and object remain unchanged. That user test and its
companion document are excluded from this campaign's commit.

Full `kf build -j4` ran and retains the existing closure failures. Source-data
matches remain 7/60: PSX 0/1, GAME 5/40 and OPEN 2/19. All four independent
SDK config contributions pass. Target relink verifies PSX 1/1, GAME 75/77
and OPEN 34/38, with two GAME and four OPEN conflicting-section-base failures
and no artifact failures. No exact-count increase, banking, tooling change
or flake change is claimed. The broader source naming and literal audit
remains open.
