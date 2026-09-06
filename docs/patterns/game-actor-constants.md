# GAME actor constants and retained literals

## Function Match Plan

Review all literals in `actor.c`, `actor_behavior.c` and `actor_pool.c` as one
campaign, starting at master `85843db`. Keep the existing action/state names,
all signatures and field widths, claims, globals, tables, CFG and ordered
referents. Name animation phase boundaries, sound slots, placement/effect bits,
damage units and credit, movement query policies and their activation ranges.
Reuse shared angle, fixed-point, map, collision and audio constants only at
sites belonging to those domains. Document every retained literal by its
actual consumer; do not infer a creature or spell identity from a number.

`kf init` verified the retail files. All 43 functions in these contiguous
actor units have refreshed image-qualified address, disassembly/CFG, caller,
callee, string and strict-match snapshots, plus source history. The existing
actor-core/AI/action evidence inventories and the animation, state and volume
distance dossiers constrain the shared ABI and neighboring functions. The
actor state, definition, map and player references establish game ownership;
SDK routines and `rand` remain library calls. Candidate relocation evidence
and the unresolved wrapper `func_8002cab4` retain their current confidence.
All three units use the `probe-gcc257-o2-g0` probe.

The first source hypothesis is constant substitution without a code change.
The two quadrant conversions may express a quarter-turn multiplication instead
of a ten-bit shift because their input is an unsigned byte. Require identical
non-debug sections in all 112 objects, an unchanged full strict report and
independent retail-word/ordered-address checks for the exact actor functions.
Preserve the dispatcher's high-half comparison with `fff1`, the return-home
uninitialized read, nonuniform table accesses and random-call order. Rebuild
the three units before comparison, run lint, tests, whitespace checks and full
`kf build`, then record a verdict for every reviewed function. No banking is
intended.

| GAME VA / size | Function | Initial and final strict % | Retail words / calls / data addresses |
| --- | --- | ---: | ---: |
| `0x8002ca78 / 3c` | `actor_pool_find_free` | 100.000000 | 15 / 0 / 1 |
| `0x8002cab4 / 20` | `func_8002cab4` | 100.000000 | 8 / 1 / 0 |
| `0x8002cad4 / 70` | `actor_set_player_transform` | 100.000000 | 28 / 0 / 2 |
| `0x8002cb44 / 74` | `actor_update_cell_from_position` | 100.000000 | 29 / 0 / 0 |
| `0x8002cbb8 / 9c` | `actor_set_position` | 100.000000 | 39 / 0 / 0 |
| `0x8002cc54 / 10` | `actor_set_rotation` | 100.000000 | 4 / 0 / 0 |
| `0x8002cc64 / c4` | `actor_initialize` | 100.000000 | 49 / 2 / 1 |
| `0x8002cd28 / a4` | `actor_initialize_current` | 100.000000 | 41 / 4 / 1 |
| `0x8002cdcc / bc` | `actor_initialize_slot` | 100.000000 | 47 / 4 / 1 |
| `0x8002ce88 / 40` | `actor_pool_clear` | 100.000000 | 16 / 0 / 1 |
| `0x8002cec8 / c` | `actor_set_action` | 100.000000 | 3 / 0 / 0 |
| `0x8002ced4 / b0` | `actor_pool_spawn` | 100.000000 | 44 / 4 / 1 |
| `0x8002cf84 / f4` | `actor_pool_begin_death_by_definition` | 100.000000 | 61 / 1 / 1 |
| `0x8002d078 / a8` | `combat_calculate_damage_component` | 100.000000 | 42 / 0 / 0 |
| `0x8002d120 / 388` | `actor_apply_damage` | 89.986725 | Unchanged residue |
| `0x8002d4a8 / 1f8` | `actor_pool_apply_radial_damage` | 86.103170 | Unchanged residue |
| `0x8002d6a0 / 158` | `actor_try_attack_player` | 100.000000 | 86 / 5 / 7 |
| `0x8002d7f8 / 184` | `actor_pool_find_target_in_cone` | 100.000000 | 97 / 2 / 2 |
| `0x8002d97c / f0` | `actor_distance_to_point` | 99.333336 | Unchanged residue |
| `0x8002da6c / 144` | `actor_pool_find_overlap` | 100.000000 | 81 / 1 / 1 |
| `0x8002dbb0 / 8c` | `actor_bind_current` | 100.000000 | 35 / 0 / 4 |
| `0x8002dc3c / 34` | `actor_advance_animation_wrapped` | 100.000000 | 13 / 0 / 0 |
| `0x8002dc70 / 5c` | `actor_advance_animation_clamped` | 100.000000 | 23 / 0 / 0 |
| `0x8002dccc / 30` | `actor_animation_crossed_phase` | 97.500000 | Unchanged residue |
| `0x8002dcfc / 98` | `actor_play_sound_at_phase` | 100.000000 | 38 / 3 / 2 |
| `0x8002dd94 / 120` | `actor_try_select_action_distance_facing` | 100.000000 | 72 / 4 / 3 |
| `0x8002deb4 / 164` | `actor_try_select_ground_action` | 100.000000 | 89 / 4 / 5 |
| `0x8002e018 / d8` | `actor_try_select_facing_action` | 100.000000 | 54 / 3 / 3 |
| `0x8002e0f0 / 1f8` | `actor_try_select_profiled_action` | 96.333336 | Unchanged residue |
| `0x8002e2e8 / 3c0` | `actor_select_next_action` | 100.000000 | 240 / 11 / 2 |
| `0x8002e6a8 / 2ac` | `actor_update_awareness` | 97.736840 | Unchanged residue |
| `0x8002e954 / 3ac` | `actor_move_xz_with_collision` | 100.000000 | 235 / 6 / 2 |
| `0x8002ed00 / d4` | `actor_move_along_heading` | 100.000000 | 53 / 5 / 2 |
| `0x8002edd4 / 454` | `actor_spawn_action_effect` | 100.000000 | 277 / 11 / 3 |
| `0x8002f228 / f4` | `actor_prepare_charge_toward_player` | 100.000000 | 61 / 5 / 5 |
| `0x8002f31c / 14c` | `actor_apply_horizontal_movement` | 100.000000 | 83 / 1 / 2 |
| `0x8002f468 / f0` | `actor_update_effect_action` | 100.000000 | 60 / 5 / 4 |
| `0x8002f558 / 374` | `actor_apply_random_movement` | 100.000000 | 221 / 7 / 2 |
| `0x8002f8cc / 1bc` | `actor_update_boss_death_sequence` | 100.000000 | 111 / 13 / 7 |
| `0x8002fa88 / d90` | `actor_update_current_action` | 98.938940 | Unchanged residue |
| `0x80030818 / a8` | `actor_pool_update` | 100.000000 | 42 / 4 / 1 |
| `0x800308c0 / 1ac` | `actor_pool_load_placements` | 100.000000 | 107 / 1 / 1 |
| `0x80030a6c / 2c` | `actor_definitions_load` | 100.000000 | 11 / 0 / 1 |

## Named domains and measured units

Animation phase has 4096 positions per cycle, numbered 0..4095. The wrapped
advance masks to twelve bits; the clamped advance caps at 4095. A delta is
phase units per update, not an elapsed-frame count. Thus an attack at phase
2700 does not imply a fixed 2700-frame delay. The animation phase names are
separate from the equally valued angle wrap mask and Q12 unity.

The three definition sounds are attack, hit reaction and death. Parameter
slots 0..2 contain encoded effect/profile IDs, slots 3..5 their selection
chances, slot 6 the optional death-drop object and slot 7 its chance. These
now have shared index names; the byte-array layout and dynamic effect indexing
are unchanged. Effect index 2 also reaches beyond the current two-element
`attachment_offsets` declaration, into the following modeled fields. This
review preserves that access and does not assert a proved three-vector owner.
A parameter's low five bits select the effect kind and index the 25-row
action-profile table; indices 25..31 are not range-checked. Bit 5 requests
two attachments mirrored by 1500 units.
The placement stream's separate low-five-bit definition index and variant
bit have their own names. No bounds check or new meaning for the other bits
is invented.

The shared aim tolerance is 341 angle units (29.970703125 degrees on either
side), while the two selection tolerances are 398 (34.98046875 degrees) and
455 (39.990234375 degrees). These are measured tolerances, not proof of an
original degree conversion. Authored heading quadrants multiply by 1024;
blocked steering advances by 4096/64 = 64 units, or 5.625 degrees. Half-turn
reversals, cardinal slide headings and eighth-turn attack offsets use the
existing angle constants.

Walk collision flags retain `8040`: reject flag-grid bit 7 and skip map
events. Velocity collision flags retain `8060` and additionally skip map
objects. Both still check terrain, actors and the player. The retained `80`
in the flag construction selects that exact grid bit; its broader level-data
meaning is unresolved. It must not be confused with the low-byte skip-player
flag. Result tags identify terrain, actors and the player; low-half terrain
codes remain distinct from their high-half class. Sixteen-bit extraction
shifts/masks express the wire layout and stay inline.

Retail `80030168` performs an arithmetic right shift by 16, then compares
with positive `fff1` at `8003016c/70`. That value cannot be the result of a
signed 32-bit arithmetic shift by 16. The actual ceiling result `1fff1`
would have high half 1. Preserve the instruction sequence and its literal
comparison rather than silently repairing this apparently ineffective guard.

Activation range 28000 and active range 32000 provide a 4000-unit gap;
the non-respawning near-spawn exclusion is below 26000 unless the player
flag permits it. These limits are world distances, not fixed-point values.
Gravity adds 20 world units per update to vertical velocity. For an
unobstructed run starting with velocity `v`, before halfword overflow or a
state change, after `n` updates its displacement
is `n*v + 10*n*(n-1)`. The first update uses the initial velocity before
adding gravity. The original choice of these distances and acceleration
remains unknown; the names identify their actual jobs and units.

Actor damage computes each channel in tenths of an HP, adds half a unit
before division by ten, then multiplies by `scale/5000`. Full weapon charge
and ordinary effect callers supply 5000, so this scale's unity is distinct
from Q12 radial falloff and incoming player-damage scaling. Only the exact
masked credit nibble `10` awards player training/experience; weapon attacks
pass that value and effects forward their type byte. Keep the existing
asymmetry: all three physical channels empty train magic, while nonzero
channel 1 or 2 trains physical power. A cutting-only hit does not enter
either training call in this function.

## Retained literal inventory

At this campaign's completion (`01c741c`), the three C files retained 517
numeric occurrences outside constant definitions and retail claims: 90 in
initializers and 427 in functions. The following
consumer groups account for them. Signed values include the unary sign in
their source expression. An authored value is retained because no additional
semantic identity or original derivation is established; this is an explicit
limit of the evidence, not a claim that its design rationale has been recovered.

### General arithmetic, ABI and data

| Sites | Retained values and reason |
| --- | --- |
| Pool scans, initialization and getters | `0`, `1`, `-1`: null pointers, zero rotation/phase/velocity, Boolean variant/finished state, occupancy increment/removal, index origin, inclusive last index and exhausted-count sentinel. The discarded free-slot wrapper has no literals and retains its unresolved identity. |
| Distance, overlap and cone queries | `-1` means no in-range result/index; zero height arguments have no vertical extent when the separate ignore-height sentinel is used. Half-height shifts by 1 compare volume centers. Output/best-pointer zeros represent no candidate. |
| Animation advance/predicate | `0` is the lower phase/sign boundary and the Boolean predicate seed. Taking a negative delta's magnitude preserves its existing signed-halfword behavior. |
| Five damage and three attack array positions | `0..4` and `0..2` retain the fixed argument order. Physical positions are cutting/striking/piercing; the player weapon caller supplies holy/fire in its remaining positions, while effect callers supply kind-dependent magic payloads. This review does not assign one universal elemental identity to every generic payload. |
| Action-profile initializer | All 25 rows retain measured distance bands and Q8 weights. Weights 32/48/64/256/768/1024 represent 1/8, 3/16, 1/4, 1, 3 and 4 in the multiply-then-shift formula. Distances 1000..20000 are authored world distances. Twelve `{0}` rows zero the complete row. Per-row aliases would obscure the table without explaining its tuning. |
| Boss sound initializer | Four packed three-byte records occupy the complete 12-byte claim. Program 27, tones 1/2/3 and note 88 are asset selectors; the final `{88,88,88}` is preserved. The handler reads indices 1..3, so do not shift its indexing or call the final bytes padding. |
| Boss temporary vector | `effect_output[2]` supplies eight bytes to the constructor's direction-copy argument. Its existing uninitialized contents are preserved; no zero initialization or fabricated padding is added. |
| Result class/detail extraction | `16` and `ffff` split the encoded result into two halfwords. The exceptional high-half `fff1` comparison is explained above. |
| Q12/random formulas using 1 and 2 | `RAND_MAX + 1` counts the SDK's 32768 possible result values; dividing by 2 splits them into equal value intervals. This does not establish a uniform sequence or a real-time event probability. Damage's `/2` is half a ten-subunit rounding unit. |

### Core combat, awareness and selection

| Function/sites | Retained values and reason |
| --- | --- |
| `combat_calculate_damage_component` | Base power `/5`, squared-attack denominator `defense*2`, zero-attack early return, nonnegative difference clamp and zero-defense replacement 1 are the actual combat formula. No alternative balancing rationale is established. |
| `actor_apply_damage` | Floor 5 / definition 7 select the existing gated boss case; the gate tests zero. Dying phase 1548 suppresses later hits. Health, damage and empty-channel zeros are arithmetic/availability tests. The boss identity and this phase choice are not inferred from their numbers. |
| `actor_try_attack_player` | Y offset 1500 converts the player camera position to the volume reference used here; point height 1700 matches the player's collision volume. `rand() >> 7` produces values 0..255 for the stored status chance. Status/component zero means absent; multiplier 10 is unity in the callee's tenths convention. |
| `actor_pool_find_target_in_cone` | Initial best error 30000 fits s16 and exceeds every folded 0..2048 angular error. It is a ranking sentinel, not a world distance; why this particular large value was chosen is unresolved. |
| `actor_play_sound_at_phase` | Floor 5 / definition 7 choose range parameters 20000 and 60000 instead of the default range. They configure that special spatial attenuation policy; they do not imply a different sound-volume scale. |
| `actor_try_select_action_distance_facing` | Distance bands at `scale`, `scale + scale/2`, and `scale*4`; odds shifts `<<2`, `<<1`, `>>2`, and the additional far `>>4` are authored selection weighting. `rand() >> 4` tests 0..2047; raw cutoff 1638 provides the separate facing-test bypass. Keep exact truncation and inequality order. |
| `actor_try_select_ground_action` | Beyond 7000 halve the chance; below 4001 reject; otherwise multiply chance by 8. The RNG downshift 4 and bypass cutoff 1638 follow the same protocol as the preceding selector. These are measured distance/weight choices. |
| `actor_try_select_facing_action` | Below 8000 reject; above 11000 divide chance by 16; otherwise multiply by 4. The draw is again `rand() >> 4`. The band and factors remain authored tuning. |
| `actor_try_select_profiled_action` | RNG downshift 4 and bypass cutoff 819; profile 9 counts active actors plus occupied kind-9 effects, accepting only when the total is below 2. Zeros/ones initialize and advance the two scans. Kind 9's broader gameplay identity remains unresolved. |
| `actor_select_next_action` | Pursuit-distance shift 8 multiplies an authored unit by 256; it is not a Q8 fractional weight. Chance `50` hex is 80 out of the helper's 2048-value draw before distance adjustment. Awareness factor 2, pursuit factor 6 and raw cutoffs 5462/1092/8193 implement the existing retreat/idle/wander policy. Do not round those cutoffs to familiar percentages. |
| `actor_update_awareness` | The spawn chance shifts left 7 before comparison with the full RNG result. Zero flags/vertical extents and `-1` query failures retain their normal protocol. The three named distance gates retain their original strict/inclusive comparisons. |

### Movement, attacks and death

| Function/sites | Retained values and reason |
| --- | --- |
| `actor_move_xz_with_collision` | Drops exceeding 600 use the long-drop policy. The allowed RNG range ends inclusively at `4000` hex (16384), or `400` hex (1024) for drops exceeding 3000. Rises of at most 1000 pass the original `<1001` test; the 400-unit split chooses upward velocity -300 versus -120. These are world-distance/velocity choices, not angle constants. Zero tests select sign, stationary state and blocked/clear returns. The steering divisor 64 expresses the measured 1/64-turn increment. |
| `actor_move_along_heading` | Shift 1 halves the repeated three-term turn-rate sum; a negative direction reverses both X/Z components. Zero is the direction's sign boundary. |
| `actor_prepare_charge_toward_player` | Length `>>2`, followed by division of each projected component by 16, produces approximately one sixty-fourth of the measured separation per update, with distinct integer rounding at both stages. Do not merge the operations or infer a guaranteed 64-update arrival. |
| `actor_apply_horizontal_movement` | `-velocity >> 1` reverses and halves blocked motion; player impact clears both components to zero. Preserve arithmetic-shift rounding for negative values. |
| `actor_apply_random_movement` | The RNG half-domain formula chooses the sign of each nudge. Marker 2 merely leaves progress nonzero; X is tested before optional Y, then Z independently. The prior comment incorrectly called it a short timer. |
| Dispatcher idle/wander/pursuit/retreat | Zero phase/motion resets, Boolean collision options and direction ±1 are ordinary control inputs. Random-heading shift 3 maps the SDK RNG to 0..4095 angle units. Raw cutoffs 2048 and 4096 govern heading reconsideration. Backoff shift 11, bias 13 and cutoff 11 give 4..19 reverse updates if pursuit remains selected and moves stay clear, as detailed in the state review. |
| Dispatcher dying/post-death | Floor/definition pairs 5/7 and 4/5 identify scripted branches. Phase `800` hex (2048) is half the animation cycle and triggers drops. Gold uses `rand()*limit >> 15`; 15 is the SDK RNG width. Drop codes `63` hex (99) and `ff` suppress the definition drop; placement drops test only 99. The chance draw uses `>>7` and an inclusive `<=` comparison. Spawn mode 1 is used for definition drops, 0 for placement drops. Post-death bound 7 is seven increments followed by the next-update transition; its original duration rationale is unknown. |
| Dispatcher melee/special/jump contacts | Melee 2700 and special 3000 test one animation-step window; jump `d48` hex (3400) uses the phase-crossing predicate. Their fractions of a cycle are known, but their authored animation alignment is not derived from a universal frame duration. Zero minimum range and yaw offset mean no lower exclusion or angular bias. |
| Dispatcher jump/vertical motion | Attribute index -1 is the literal retail adjustment. Height threshold -5000 selects velocity/animation-step pairs -220/140 or -300/110. Player bounce uses striking input 15 with other channels/status absent, Q12 unity and tenths multiplier 10; actor/player bounce velocity is -120 and ceiling response is +100. Zero resets phase or vertical velocity. These are authored inputs to the existing combat and motion formulas, not guaranteed HP loss or physical units from a manual. |
| Dispatcher drift | Each update nudges X/Y/Z by 1 within ±100 world units/update; yaw changes by ±1 within ±32 angle units/update (±2.8125 degrees/update). These limits are tuning, not resource IDs. The four uses of 32 enforce the same measured yaw bounds. |
| Dispatcher effect actions | Helper indices 0/1/2 select the corresponding definition effect and attachment; the named action cases already identify the three slots. These are positional API arguments rather than three new spell identities. |
| Dispatcher return-home | Strict coordinate bounds -200..200 and raw heading-change cutoff 2048 implement near-home alignment. Phase `4b0` hex (1200) resets the animation in the existing mixed-slot path. Preserve the uninitialized coordinate read and the differing move/melee animation accesses. |
| Dispatcher multi-hit | Phase samples `8c0/a80/c80/e00` hex = 2240/2688/3200/3584 test the 8000..11000 distance band. Samples `dac/ed8` = 3500/3800 test 0..11000 with the named ±eighth-turn yaw offsets. These six authored contact points remain explicit samples; they are not a uniform cadence. |
| `actor_update_boss_death_sequence` | Sound indices 1/2/3 use phases 500/1000/1500. Link 13 and definition IDs 0/2/3/4 select existing scripted targets. Effect cadence uses modulo `death_step*2`, sound cadence `death_step*4`; zero is the exact-divisibility test. X/Z offsets `(rand() & 1fff)-4096` span -4096..4095 world units; Y subtracts `rand() & fff`, a 0..4095 distance. These masks describe spatial scatter, not phase or angle. Sound volume 100 uses the 0..127 API scale. |

### Actor effect-constructor arguments

The effect-code switch accepts kinds 5,7,8,9,10,11,12,13,22,23,24. These are
curated dispatch/asset IDs; numerical adjacency is not proof of specific spell
names. The paired path has two iterations and offsets local X by ±1500. The
player aim query's maximum distance is 50000. Code-specific speed scales
800/250/600 are world-vector magnitudes; kind 7 overrides to 800. Without a
target, kind 23 uses pitch -32 angle units and seeds its lifetime counter with
20; kinds 9/10 also receive 20. These counters do not describe total visible
lifetime uniformly: kind 9 first grows and consumes its counter only during
the movement phase. With a target, kind 23 aims 1000 units above the
stored player reference; kinds 10 and 9 subtract respectively 4500 and 2000
from distance before dividing by speed 250, with a minimum step count of 1.
These target offsets, speeds and durations remain authored tuning.

Every call passes type `23` hex. Its low class bits permit both actor and
player queries; retain the whole encoded byte rather than infer all its
type-policy meanings from that subset. Kinds 8/22 take an angle pointer and
sound-enable 1. Kind 24 takes its separate angle pointer, target-mode byte
`fe` and sound-enable 1; the constructor converts it to kind 20. Kind 10's
optional values are propagation generations 3, computed movement counter and initial
scale `bb8` hex (3000). Other arms pass the computed lifetime and flag 1,
which only the kinds that consume those optional slots interpret. This is
not a uniform variadic signature, so the constants are documented at each
actual consumer rather than assigned one shared meaning.

The boss effect call instead supplies id 0, type `13` hex, kind `2c` hex
(44) and sound-enable 0. The constructor converts kind 44 to kind 18 with
render resource 17. The raw ID/type/kind selectors and disabled sound flag
are preserved; no creature or effect name is invented from them.

## Final verification

All 43 functions retain their initial strict score. The 36 exact functions
independently reproduce 2515 retail instruction words and their ordered call
and data addresses. The seven non-exact functions retain their existing
residues without a compiler-mechanism attribution. All 112 object sections
remain identical except `.debug_line` in `game.actor` and
`game.actor_behavior`, and the full 484-function strict report is unchanged.
After expanding the declared constants and the two unsigned-byte quadrant
conversions, all source statements, types, values and operation order agree
with `85843db`. There are no curated config, identity, relocation, claim or
bank changes and no new exact claim.

Ruff and whitespace checks pass. Full discovery ran 654 tests in 97.864
seconds: 653 passed; the pre-existing untracked save/load-hub control retains
its unrelated instruction mismatch. Its source and object are unchanged,
and the untracked test/documentation remain separate local work.

Full `kf build` retains the same unfinished ownership/placement failures:
source data matches PSX 0/1, GAME 5/40 and OPEN 2/19 (7/60 total); independent
config-provider data matches 4/4. Target relink verifies PSX 1/1, GAME 75/77
and OPEN 34/38, with the same two GAME and four OPEN conflicting section
bases and zero artifact failures. This is not a passing full build.
