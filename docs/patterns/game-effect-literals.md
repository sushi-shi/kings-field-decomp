# GAME effect literal audit

## Function Match Plan

Continue from master `92f01ef`. Audit every numeric literal in `effect_pool.c`,
`effect_update.c`, `effect_dispatch.c`, `effect_map_collision.c` and `magic.c`,
plus the affected actor and map-object call sites. Hash-identical GAME retail
is initialized; 21 functions have refreshed address/extent, disassembly/CFG,
callers, callees, strings, strict match and source-history snapshots. All use
the unchanged `probe-gcc257-o2-g0` probe. Their custom effect/actor/map data and
policies establish game ownership; matrix, trigonometry, square-root and RNG
SDK bodies remain external calls. Adjacent helpers, the constructor and the
render consumer constrain the shared 60-byte record and authentic SDK types.

Name darkness, curse and emerging projectiles, the two swinging hazards,
shared projectile phases, ground-trail phases and the hazard release request.
Keep phase intervals, unsigned comparisons, byte wrapping and the asymmetric
pitch clamp exactly as decoded. Express scatter pulse phase and homing turns
with the established angle units; reuse the world-query exclusions and the
actor-cone tolerance at their existing call sites. Document retained values
by consumer, units and measured consequences, without inventing an original
tuning rationale or spell/asset identity.

Correct the existing `effect_projectile_velocities` identity: the two vectors
are rotated collision-probe offsets, not per-update displacement. Rename the
single 16-byte owner at GAME `80057b88` to `effect_swing_probe_offsets`, correct
its source owner to `effect_dispatch`, and rename the 3D helper's parameter to
`probe_offset`. Preserve the function identity, signature types, two decoded
address pairs, every reference's confidence, the initializer and its extent.
Update the existing reference test's expected spelling; do not weaken it.

First hypothesis: all source changes expand to the original statements,
types, values and operation order. Require freshly compiled affected units,
the same 484 strict scores and complete objdiff report, and identical emitted
code/data across all 112 objects after accounting for just the one symbol
rename and debug-line changes. Resolve all reviewed exact functions back to
complete retail words and ordered physical referents. Verify the probe table
and its two materialized addresses independently. Run lint, repository tests,
whitespace checks and full `kf build` before commit. No banking, profile
change or relocation promotion is intended.

## Function snapshots

| GAME VA / size (hex) | Function | Initial → final strict % | Retail words / calls / data addresses |
| --- | --- | ---: | ---: |
| `0x8002edd4 / 454` | `actor_spawn_action_effect` | 100 → 100 | 277 / 11 / 3 |
| `0x80031008 / 448` | `map_object_pool_load` | 100 → 100 | 274 / 11 / 8 |
| `0x80031cc8 / c18` | `map_object_pool_update` | 98.966410 → 98.966410 | — |
| `0x80036f00 / 44` | `effect_pool_find_free` | 100 → 100 | 17 / 0 / 1 |
| `0x80036f44 / 82c` | `effect_pool_construct` | 100 → 100 | 523 / 7 / 8 |
| `0x80037770 / ac` | `effect_pool_spawn_typed` | 100 → 100 | 43 / 1 / 0 |
| `0x8003781c / 34` | `effect_pool_set_current` | 100 → 100 | 13 / 0 / 2 |
| `0x80037850 / 76c` | `effect_map_collision` | 97.452630 → 97.452630 | — |
| `0x80037fbc / 24` | `effect_magic_power` | 100 → 100 | 9 / 0 / 1 |
| `0x80037fe0 / 2b8` | `effect_projectile_update_3d` | 100 → 100 | 174 / 10 / 2 |
| `0x80038298 / 260` | `effect_projectile_update_2d` | 99.934210 → 99.934210 | — |
| `0x800384f8 / 1cc` | `effect_floor_deform_line` | 100 → 100 | 115 / 1 / 3 |
| `0x800386c4 / 68` | `effect_scatter_triple` | 100 → 100 | 26 / 3 / 0 |
| `0x8003872c / 90` | `effect_rotate_scale_offset_y` | 100 → 100 | 36 / 2 / 0 |
| `0x800387bc / f8` | `effect_spawn_trail_kind13` | 100 → 100 | 62 / 2 / 1 |
| `0x800388b4 / 184` | `effect_spawn_ground_kind6` | 100 → 100 | 97 / 3 / 1 |
| `0x80038a38 / 180c` | `effect_update_dispatch` | 96.939570 → 96.939570 | — |
| `0x8003a244 / 30` | `effect_pool_reset` | 100 → 100 | 12 / 0 / 1 |
| `0x8003a274 / 2c` | `magic_load_records` | 100 → 100 | 11 / 0 / 1 |
| `0x8003a2a0 / 4c0` | `magic_cast` | 98.875000 → 98.875000 | — |
| `0x8003a760 / 7c` | `effect_pool_sweep` | 100 → 100 | 31 / 2 / 1 |

All 21 final verdicts are unchanged: 16 exact and five partial. The last column
counts independently resolved complete retail words and ordered call/data
references for exact functions, totaling 1720 words. A dash means the function
remains partial; it does not claim whole-function retail equality. Its final
emitted code and strict score are identical to the pre-campaign object.

## Collision probes and release behavior

The two caller pairs at `8003a054/58` and `8003a06c/70` materialize `80057b88`
and `80057b90`, then call the 3D helper with limits 40 and 60. The complete
16-byte initializer is two SDK vectors, `(0,2500,0,0)` and `(0,4900,0,0)`.
At `80038064`, `ApplyMatrix` rotates the selected offset. The following
instructions add the current record position into the temporary VECTOR at
sp+104..112, which is passed to `effect_map_collision` at `800380a4`.
They do not publish this temporary as the record's new position. Thus 2500
and 4900 are probe distances in world units, not velocities. The shorter and
longer hazard names describe those measured offsets; both use model selector 7.
The original object/spell identities remain unresolved.

Map actions 81/82 publish phase 1 once their link is absent. In the 3D helper,
phases 0 and 1 still update the swing and probe. At a pitch zero crossing,
phase 1 sets pitch to zero and phase to 10 (`8003821c..80038240`). Subsequent
updates subtract 60 from world Y while the phase is at most the supplied
limit. Limits 40/60 therefore admit 31/51 upward steps, or 1860/3060 world
units. The helper then stops moving the record; it does not free the slot.
The separate orbit helper keeps orbiting in phase 1 and has no transition
from that release request into its upward-motion arm. Preserve this observed
asymmetry rather than claiming every hazard releases successfully.

## Literal census and common arithmetic

The final five-module census contains **764** numeric occurrences outside
constant definitions and retail claims: 91 in initializers/layout checks and
673 in functions. Comments, strings and numbers inside identifiers are not
counted. The tables below account for the remaining literals by their actual
consumer. An authored parameter stays literal when its measured effect is
known but no additional semantic identity or derivation is established.
This does not claim the designer's reason for choosing its value.

| Module | Initializer/layout occurrences | Function occurrences | Total |
| --- | ---: | ---: | ---: |
| `effect_pool.c` | 18 | 250 | 268 |
| `effect_update.c` | 36 | 92 | 128 |
| `effect_dispatch.c` | 8 | 278 | 286 |
| `effect_map_collision.c` | 29 | 15 | 44 |
| `magic.c` | 0 | 38 | 38 |

| Consumer | Retained values and reason |
| --- | --- |
| Pointer checks, initialization and arithmetic signs | `0` is null, the arithmetic origin, zero scale/rotation/vector components, cleared sound acceptance, or the initial age/cache accumulator. The generic constructor starts age at zero for different kinds; this is not a claim that all kinds share one lifecycle. |
| Loops, countdowns and inclusive limits | `0`, `1`, `-1`, and unsigned `1u` retain the original exhausted-count, last-index and exclusive-bound adjustments. Unit increments are arithmetic, not extra states. |
| Damage components | Array indices `0`, `1`, `2` select existing definition channels. The physical calls deliberately pass them in order 0,2,1; the other calls pass 0,1 into their existing magic channels. Zero components and zero base-power arguments remain zero. No universal element interpretation is inferred for these array slots. |
| Sound definitions | `sounds[0]` and `[1]` select the two stored SoundRefs. Their event depends on the kind: construction, impact or an intermediate phase. A universal cast/impact enum would misdescribe some consumers. |
| Optional construction sound | Flag `0` suppresses the optional sound call and `1` requests it in branches that consume that argument. These are boolean call options, separate from sound-slot indices and the result of attempting playback. |
| Packed return and storage views | `>>16`, `&0xffff`, `&0xff` extract the collision class, low-half actor index, or phase byte. Their field widths already state the contract; the literal bit operations preserve the existing promotions and narrowing. |
| Vector lanes | Scatter indices `0`, `1`, `2` are the X/Y/Z halfwords; horizontal offset helpers zero Y and unused rotation axes. These are coordinates in a small formula, not independent game IDs. |

## Initializers and layout witnesses

The 18 literals before the pool functions assert size `0xd28`, alignment 4,
and member offsets `0x1e0`, `0xd20`, `0xd24`. Pointer zero and `1/-1` in these
typedef checks implement compile-time offset/extent validation. The measured
numbers stay explicit so the assertions independently constrain the named
array dimensions; they are not additional tunable settings.

The eight probe-vector components retain the two measured distances above,
zero cross-axis components and the SDK pad halfword. The array's count and
short/long selectors are named separately from effect-kind IDs.

The floor table's 36 literals are its five-row bound and 35 initialized bytes:

| Start column,row | Column,row step | Cell count | Start,end height bytes |
| --- | --- | ---: | --- |
| 65,80 | 1,0 | 2 | 0,100 |
| 61,73 | 0,255 | 2 | 0,100 |
| 75,56 | 1,0 | 3 | 0,100 |
| 37,27 | 0,255 | 3 | 0,100 |
| 32,82 | 0,1 | 12 | 0,100 |

These are authored map coordinates, counts and heights. Step 255 wraps an
unsigned coordinate backward by one; it is not a sentinel or padding. Height
bytes multiply by 100 world units with the map's negative-Y convention, so
the end height is 10000 world units above height zero. The predecrement loop
visits exactly the stored count, not count+1. No specific floor-event name is
inferred just from these coordinates.

The collision table's 29 literals are the seven-row bound and 28 oriented
X/Y bounds. The [collision literal review](source-constant-names.md#effect-collision-reconciliation-and-target-classes)
accounts for all seven rows and their reason: measured local X/floor-relative Y
geometry, including the last three rows' inverted Y bounds. The remaining
15 function literals are six zeros (three signed lower-bound/height tests and
three zero vertical extents), eight adjacent-cell offsets `+1/-1`, and the
default encoded result 1 for target class zero. That last result is not -1.

## Constructor IDs, fields and tuning

These are separate render-resource namespaces: billboard selectors index
sprite descriptors, while model selectors add the effect-model asset base.
The following table accounts for every remaining numeric kind, model/sprite
selector and fixed magic-row selection. Equal render IDs in different modes
are not evidence of the same asset. Numeric content IDs remain explicit until
the assets support more specific names.

| Input kind(s), decimal | Stored kind / rendering selector | Fixed magic-row use |
| --- | --- | --- |
| 5; 7 | Same kind; billboard 0; billboard 5 | Current row |
| 4; 23 | 4; billboard 6; billboard 17 | Row 4 sound 0 |
| 32; 41 | 32; billboard 11; billboard 19 | Row 4 sound 1 |
| 33; 42 | 33; model 0; model 15 | Dispatcher uses row 4 damage |
| Ground branch 6; 34 | Same kind; model 1; model 2 | Current row for the branch |
| Actor spawner 9; scatter 10 | Same kind; model 3; billboard 9 | Current row |
| Darkness 11; curse 12 | Same kind; billboard 8; billboard 10 | Current row |
| Emerging 13; 14 | Same kind; model 9; model 4 | Current row |
| 48; 8; 22 | Same kind; models 5,6,14 respectively | Current row where consumed |
| Short/long swing 15/16; orbit 17 | Same kind; model 7; model 9 | Current row |
| 36; ground trail 19 | Same kind; model 10; billboard 14 | 36 plays row 18 sound 1 |
| Radial blast 18; 44 | 18; model 11; model 17 | Row 18 sound 0 |
| Homing 20; 24 | 20; model 12; model 16 | Row 20 sound 0 |
| 21 | Same kind; model 13 | No fixed sound call |

Every model branch sets animation clip 0, the first clip. Billboard mode has
its own named selector. Optional argument indices 1/2/3 are the sixth,
seventh and eighth O32 argument slots relative to the saved direction slot;
their consumers use the actual byte, halfword, pointer or full-word flag.
They are not three interchangeable parameters. The frame count, branch,
generation, parent and target-mode fields retain their type-specific reads.

| Constructor consumer | Retained tuning and reason |
| --- | --- |
| Kind 7 orientation | Pitch `0x352` = 850 angle units = 74.70703125 degrees. Exact authored orientation; no proved degree-conversion formula. |
| Ground branch | Countdown 6 and volume `0x78` = 120. The postdecrement dispatch emits children on the seventh visit; volume is below the maximum 127. |
| Emerging projectile | All scales `0x5dc` = 1500 Q12 units; Y offset +3500. Its twenty emergence updates each subtract 175, returning to the requested spawn Y before launch. |
| Short swing | All scales `0xa28` = 2600 Q12 units (about 0.635). The long variant retains unity; the independently measured probe lengths are 2500/4900. Do not equate visual scale with probe distance. |
| Orbit | All scales `0xaf0` = 2800 Q12 units (about 0.684). Saved X/Z `>>8` selects 256-world-unit buckets, later reconstructed with `<<8`; Y is stored as a halfword. |
| Homing and kind-24 variant | X/Y scale `0x800` = one half Q12 unity; Z remains the default unity. |
| Kind 21 | X/Z `0x1800` = 1.5 Q12 unity; Y starts at zero. The dispatcher has no kind-21 update arm. |
| Long-range construction sounds | `0x4e20` = 20000 and `0xea60` = 60000 are world-distance arguments, not milliseconds or Q12 scales. |
| Kind 36 control bytes | Two `0xff` writes initialize separate bytes without a decoded behavioral consumer. Do not borrow the free-slot, homing or branch sentinel names. |
| Specialized floor constructor | Type `0xf0` is a live controller marker; zero progress starts interpolation. It is not the `0xff` free-slot marker. |

## Update helpers and magic casting

| Consumer | Retained values and measured meaning |
| --- | --- |
| Default magic power | 5 is the fixed power returned when the player-magic bit is absent. Original balance rationale is unknown. |
| Swing collision/audio | Radius `0x78` = 120 world units. Random cutoff 8192 accepts that many outcomes of the SDK's 32768-value range; it does not establish RNG uniformity. Spatial range arguments are `0xbb8` = 3000 and `0x36b0` = 14000. |
| Swing angular dynamics | `+10/-10` modifies angular velocity by 10 angle units per update, pointing it toward pitch zero. These are acceleration steps, not ten-degree angles or a duration. The preserved +/-45-degree clamp includes the asymmetric yaw test described in the protocol review. |
| Swing upward motion | 60 world units per update; the exact 31/51-step consequences and lack of slot removal are explained above. |
| Orbit geometry | Shift 8 restores the stored center; `angle<<1` gives two Y oscillations per orbit and `>>2` makes their amplitude 1024 world units. The denominator 64 in the angular step means one turn per 64 active updates. |
| Orbit collision/audio | Radius `0x96` = 150; random cutoff 8192; sound distances `0x1388` = 5000 and `0x36b0` = 14000. The separate distance >=5000 test clears sound acceptance for another attempt. The upward arm uses the same 60-world-unit step. |
| Floor interpolation sound | 3900/4096 = 95.21484375% progress. Sound applies only inside the unclamped interval `[3900,3900+abs(step))`; original choice of threshold is unknown. Zero, unity+1 and -1 express the clamp and exhausted-count bounds. |
| Scatter helper | The six raw indices are three halfword coordinates read and written once. Its named shift/bias retain perturbations -64..63 world units per update, three RNG calls and signed accumulation. |
| Trail placement helper | Divisor 800 converts requested distance into Q12 scale for the supplied direction. It scales the actual input vector; it does not normalize arbitrary vectors to length 800. |
| Branch placement helper | Two factors 1500 set radial X/Z offsets in world units through sine/cosine; floor sampling determines Y. The resulting signed angle still includes the caller's turn offset. |
| Magic selection | IDs 4/5/7/8 retain their serialized spell/constructor choices. The two cone ranges `0x4e20` = 20000 use the now-shared 341-unit aim tolerance. |
| Magic launch transform | Offset `(-200,200,400)` is in world/model coordinates before rotation. Speeds 600 and 800 are world units per update; default kind-4 pitch -128 is -11.25 degrees and its untargeted countdown is 20 updates. |
| Magic target-height choice | Attribute index `-1`, threshold `-4999`, and aim-height additions 3000/5000 preserve the decoded lookup and branches. Their exact level-dependent tuning is unresolved; no new bounds guard or level identity is inferred. |
| Magic constructor options | ID `0xa` = 10 supplies unity to the downstream player-damage tenths multiplier. Optional sound flag 1 requests sound. Zero tests are absent targets; named branch role zero is separate. |
| Untargeted ground branch | Distance 6000 is three map cells before integer trigonometric rounding. The original choice of that range remains unknown. |

The pool search/reset/sweep and loader retain only null/exhaustion/index
arithmetic: two literals in the search, one each in reset and loader, and
two in the sweep. The current-effect selector has no remaining raw literals.

## Dispatcher phases, geometry and tuning

The shared projectile phase byte distinguishes travel 0, impact start 1,
dissipation start/end 50/60, emergence start/last 100/119, fall 120 and shrink
121. Emergence assigns 255 before the common increment wraps the byte to 0.
This is deliberate launch scheduling, not a free-slot write. Darkness and
curse hits on the player carry their corresponding named status bits; actor
hits retain their existing component damage instead.

| Dispatcher consumer | Remaining values and reason |
| --- | --- |
| Kind selection and child/resource variants | Numeric kinds 4/5/7/8/14/22, 32/33/34/36 and child variants 41/42/44 retain the exact dispatch/constructor mapping above. Kind 7 continues travel after actor/player contact; kinds 8/22 skip the shared spin. Specific asset/spell identities are not invented from those predicates. |
| Shared query and impact stages | Default radius 100; emerging radius 150. Kind 5's `phase<5` selects four post-impact frames 1..4; `phase<10` handles the following invalidation interval. These numbers are frame/age boundaries, not kind IDs. |
| Shared motion and dissipation | Spin increments 200, 100 and 600 are angle units per update (multiply by 360/4096 for degrees). Kind-4 dissipation subtracts 400 Q12 scale units at ages 50..59, ten times, leaving 96 from an initial 4096; age 60 invalidates. Its resource-6 test chooses child kind 32 versus 41, and countdown `&1` alternates two render frames. |
| Emerging projectile | Twenty emergence steps of -175 restore its constructor's +3500 Y offset. Falling starts with vertical velocity zero, adds 20 before moving, and removes the slot when Y exceeds floor+3000. Before halfword wrapping, after n fall steps displacement is `10*n*(n+1)`. No constant fall duration is inferred from a possibly elevated impact origin. |
| Darkness/curse shrinking | `0x180` = 384 Q12 units are subtracted from Y/Z scale each update; removal tests signed scale <0. Starting at 4096, this takes eleven updates and leaves X unchanged. |
| Scatter generations and pulse | Multiplying scale by 3, then shifting right by 2, gives three quarters before integer rounding; new countdown is 15 for the final generation and 5 otherwise. Two scatter calls independently change parent and child directions. The original `<<9` advances pulse argument by 512 angle units per countdown step: eight samples per turn. `>>13` on sine/cosine products means Q12 output with half amplitude, giving roughly 0.5..1.5 of base scale before integer rounding. It is not an RNG bit extraction. |
| Kind-14 sound retry | Cutoff 8192, spatial distances 4000/12000 and sound slot 0 remain explicit authored sound policy. The call is attempted only while sound acceptance is zero. |
| Kind-36 movement and trail emission | Collision radius 100. While age<11, extend Z scale by `0x400` = 1024 Q12 units and cap signed overflow at `0x7fff` = 32767. Age 2 emits four trails at angles +/-`0x6ee` (1774) and +/-`0x720` (1824), with distance parameters 4000/8000. Age 10 holds until collision. |
| Kind-36 impact | Phase 20 hides the parent and begins repeated radial blasts; `(phase-20)&1` emits on even offsets and phase>23 invalidates after the possible emission. The trail waits for parent phase>19, then shrinks by 400 Q12 units; its render ID cycles across three entries. These 19/20 boundaries are related parent/child scheduling, not a 20-update delay. |
| Radial blast | Preincremented phase<13 permits twelve growth updates of `0x400` = 1024 Q12 units. Radius uses the old phase times `0x14d` = 333 world units; oddness uses the new phase. Do not conflate those two snapshots. Player radial scale 5000 is Q12 (about 1.221), unlike actor scale unity 5000. |
| Homing random direction | Pitch `(rand>>6)-0x80` ranges -128..383; yaw `(rand>>3)-0x200` ranges -512..3583 before wrapping. These are not symmetric small-angle jitters. The wander cutoff `0xccc` = 3276 selects that many SDK RNG outcomes. |
| Homing target and movement | Steering begins when phase>4. Player aim offsets Y by 800; actor aim uses half the collision height. Cone range 20000 and tolerance `0x555` = 1365 angle units (119.970703125 degrees) are authored settings. Setting phase 20 then incrementing to 21 keeps retargeting on later updates; it is not a cooldown. |
| Homing transform and impact | Both turn limits are 4096/64 = 5.625 degrees per update. Local forward motion is 650 world units; roll adds `0x100` = 256 angle units (22.5 degrees). Render selector `0x10` chooses child variant `0x2c` rather than the ordinary radial blast. The -1 query sentinel and zero unused vector components keep their separate meanings. |
| Kind-32 emitter | Ages 0..9 cycle three render entries. Ages 3/5/7 spawn kind 33 or variant 42 depending on base render 11; age 3 also plays fixed row-4 sound 1. It then invalidates and still performs the common age increment. |
| Kind-33 expansion | Ages 0..7 add `0xdff` = 3583 Q12 scale units and `0x514` = 1300 yaw units per update. Odd ages apply radial damage at age*1000 world units with height ignored. Fixed magic row 4 and player scale 5000 retain the interpretations above. |
| Ground branch | Growth is +`0x100` = 256 Q12 units for ages 0..15. At age 15, role-dependent assignments 16/26/36 precede the common increment. Before age 48, `phase&3` selects every fourth update; random angle is `rand>>3` and radius `(rand*0x145)>>13` ranges 0..1299 world units. Child kind 34 is visual; actor/player damage radius is 1500. Ages 48..63 shrink by 256; later ages free the slot. Yaw adds 500 units and age increments even on that final path. |
| Kind-34 visual | Ages 0..3 add 4000 Q12 Y-scale units; ages 4..7 subtract 4000 and move Y by -1800 world units. Later ages free the slot; yaw still adds 500 and age increments. The model's specific visual identity remains unresolved. |
| Actor spawner | Ages<17 grow scale by 220; ages 17..40 move X/Z, with radius 1000 and the named object/event exclusions. At age 40 a collision, or any exhausted movement count, assigns `0x75` = 117. Age 40 otherwise holds. This is not one simple lifetime counter. |
| Actor creation and retirement | Before age 149, age 132 spawns definition 2 if the first RNG draw<3000, else definition 4 if a second draw<3000, else definition 0. Those are distinct draws and unresolved creature IDs. The spawn adds 800 Y and faces the player. Ages 149..164 shrink by 220, then invalidate; yaw/pitch/roll increments are `0x6a4/0x578/0x76c` = 1700/1400/1900 angle units. |
| Swing/orbit helper calls | Limits `0x28/0x3c` = 40/60 and orbit radius `0x1964` = 6500 retain the measured helper contracts. The short/long probe indices now have names; the orbit's 6500 is a radius despite the helper's historical parameter name `speed`. |
| Floor-deformation phases | Remaining zeros select the first segment and arithmetic origin; `-1` terminates signed halfword/word countdowns. Step negation and +/- progress preserve forward/reverse interpolation. No extra phase enum is inferred from those countdown values. |

Raw retail claims remain explicit addresses and extents. The caller edits
outside these five modules only substitute the named kind or release value;
the earlier actor audit covers the actor literals, while the other map-object
literals remain part of the wider audit.
The broader naming objective and the unresolved field identities remain open.

## Verification

Force-compiled all eight affected/control units, then rebuilt shared-header
consumers and renamed target symbols with `kf build compare -j4`. All 112
source objects retain identical section contents except `.symtab` and
`.strtab` in the dispatcher, which contain the single probe-datum rename.
After resolving symbol strings, every other symbol property is unchanged.
All code, relocation entries, initialized data, switch tables and debug-line
bytes are unchanged. All 484 strict scores and the complete objdiff report
equal the `92f01ef` baseline.

The 16 exact functions independently resolve to all 1720 retail words,
including delay slots and ordered calls/data addresses. Both source and target
probe tables equal all 16 retail bytes, and both dispatchers materialize
exactly the ordered addresses `80057b88,80057b90` for their two calls to
`80037fe0`. Expanding the declared constants and name substitutions retains
all original statements, types, values and operation order. Curated inventory
changes are limited to the datum/parameter spelling, descriptions and the
corrected source owner; relocation sites, numeric targets, evidence status,
claims, profiles and bank state remain unchanged.

An intermediate experiment expressed the scatter pulse's `<<9` as
`*KF_ANGLE_EIGHTH_TURN`. The dispatcher grew from 6156 to 6164 bytes and its
strict score changed from 96.939570 to 96.662120. The first raw difference
was the early current-magic pointer load using `fp` instead of `s7`.
Restoring the original shift recovered all original emitted code. This is
an observed code-generation difference without an attributed compiler
mechanism; the shift's angle interpretation is documented above.

`ruff check scripts tests` and `git diff --check` pass. The repository suite
ran 654 tests: 653 passed and the existing, untracked save/load-hub test failed
at word 38 (151 source words versus 152 retail words). Its source and object
are unchanged; the test and its companion user document are excluded from
this commit.

Full `kf build -j4` ran and remains unsuccessful on the existing ownership
and placement gates: source-data matches are 7/60 (PSX 0/1, GAME 5/40, OPEN
2/19), while all four independent SDK config contributions pass. Target
relink verifies PSX 1/1, GAME 75/77 and OPEN 34/38, with two GAME and four OPEN
conflicting-section-base failures and no artifact failures. No exact-count
movement or new banking is claimed. There are no tooling or flake changes.
