# GAME effect kinds and control values

## Function Match Plan

Start from master `01c741c`, with hash-verified GAME retail and the unchanged
`probe-gcc257-o2-g0` probe. Follow the effect constructor, dispatcher, collision
helper, render consumer and actor/player/map callers as one naming campaign.
The proposal has 27 per-function snapshots covering image/extent, retail
disassembly and CFG, callers, callees, strings, current strict match and source
history. The shared effect owner, adjacent helper bodies and previously
decoded constructor argument windows constrain the ABI. The reviewed functions
use custom game objects and policies; SDK matrix, trigonometry and RNG calls
remain external library calls. No reviewed function has a vendored identity.

The first hypothesis is constant substitution with identical emitted code.
Name eight effect behaviors, collision target classes, the player-magic bit,
billboard/hidden selectors, homing selectors, ground-branch roles and the three
floor-deformation phases. Reuse the established player status, angle, Q12,
damage, collision and volume constants only at consumers of those domains.
The 120-word magic load may derive its count from the existing 24-row array.
Keep every signature, field width, variadic slot, operation order, claim,
switch table, data owner and physical referent. Do not promote candidate
references, add bounds checks or repair the homing uninitialized read and
asymmetric 3D pitch clamp.

Require a fresh compile of all affected units, identical non-debug object
sections across all 112 units, the same 484 strict scores and complete report,
and independent retail-word/ordered-address checks for each reviewed exact
function. Compare private section references at their actual owning bases.
Run lint, the repository tests, whitespace checks and full `kf build` before
commit. Existing partial functions remain partial; no banking is planned.

The constructor/state, projectile and floor-deformation dossiers describe the
existing ownership and reconstruction evidence. The new names below identify
decoded behavior, not recovered Japanese spell names or original C symbols.

## Function snapshots and final verdicts

| GAME VA / size | Function | Initial and final strict % | Direct calls / CFG blocks | Retail words / ordered addresses |
| --- | --- | ---: | ---: | ---: |
| `0x80018880 / 1a1c` | `player_update` | 96.945540 | 66 / 325 | Unchanged residue |
| `0x8001eedc / 1e8` | `render_actor_sprite` | 100.000000 | 20 / 8 | 122 / 6 |
| `0x8001f218 / 580` | `render_entities` | 91.250000 | 9 / 61 | Unchanged residue |
| `0x8002e0f0 / 1f8` | `actor_try_select_profiled_action` | 96.333336 | 4 / 23 | Unchanged residue |
| `0x8002edd4 / 454` | `actor_spawn_action_effect` | 100.000000 | 11 / 51 | 277 / 3 |
| `0x8002f8cc / 1bc` | `actor_update_boss_death_sequence` | 100.000000 | 13 / 16 | 111 / 7 |
| `0x80031008 / 448` | `map_object_pool_load` | 100.000000 | 11 / 43 | 274 / 8 |
| `0x80031cc8 / c18` | `map_object_pool_update` | 98.966410 | 35 / 118 | Unchanged residue |
| `0x800346a8 / 38c` | `map_floor5_transition_cutscene` | 100.000000 | 12 / 32 | 227 / 14 |
| `0x80036f00 / 44` | `effect_pool_find_free` | 100.000000 | 0 / 6 | 17 / 1 |
| `0x80036f44 / 82c` | `effect_pool_construct` | 100.000000 | 7 / 52 | 523 / 8 |
| `0x80037770 / ac` | `effect_pool_spawn_typed` | 100.000000 | 1 / 3 | 43 / 0 |
| `0x8003781c / 34` | `effect_pool_set_current` | 100.000000 | 0 / 1 | 13 / 2 |
| `0x80037850 / 76c` | `effect_map_collision` | 97.452630 | 1 / 126 | Unchanged residue |
| `0x80037fbc / 24` | `effect_magic_power` | 100.000000 | 0 / 3 | 9 / 1 |
| `0x80037fe0 / 2b8` | `effect_projectile_update_3d` | 100.000000 | 10 / 28 | 174 / 2 |
| `0x80038298 / 260` | `effect_projectile_update_2d` | 99.934210 | 9 / 15 | Unchanged residue |
| `0x800384f8 / 1cc` | `effect_floor_deform_line` | 100.000000 | 1 / 13 | 115 / 3 |
| `0x800386c4 / 68` | `effect_scatter_triple` | 100.000000 | 3 / 1 | 26 / 0 |
| `0x8003872c / 90` | `effect_rotate_scale_offset_y` | 100.000000 | 2 / 1 | 36 / 0 |
| `0x800387bc / f8` | `effect_spawn_trail_kind13` | 100.000000 | 2 / 11 | 62 / 1 |
| `0x800388b4 / 184` | `effect_spawn_ground_kind6` | 100.000000 | 3 / 11 | 97 / 1 |
| `0x80038a38 / 180c` | `effect_update_dispatch` | 96.939570 | 69 / 257 | Unchanged residue |
| `0x8003a244 / 30` | `effect_pool_reset` | 100.000000 | 0 / 3 | 12 / 1 |
| `0x8003a274 / 2c` | `magic_load_records` | 100.000000 | 0 / 3 | 11 / 1 |
| `0x8003a2a0 / 4c0` | `magic_cast` | 98.875000 | 13 / 37 | Unchanged residue |
| `0x8003a760 / 7c` | `effect_pool_sweep` | 100.000000 | 2 / 5 | 31 / 1 |

## Named domains

| Kind | Behavioral name | Constructor and update evidence |
| ---: | --- | --- |
| 6 | Ground branch | Optional halfword branch role; delayed quarter-turn child spawns, floor-snapped placement and radial damage. |
| 9 | Actor spawner | Movement/growth stages eventually call `actor_pool_spawn` at `80039fd0`; actor action selection counts pending instances against actor capacity. |
| 10 | Scatter projectile | Generation and frame counters, a pulse scale and repeated `effect_scatter_triple` calls producing child projectiles. |
| 17 | Orbiting projectile | Constructor saves the orbit center; dispatcher calls the 2D helper with radius 6500 and phase limit 40. |
| 18 | Radial blast | Growing visual scale and repeated actor/player radial-damage calls; kind 44 selects another render resource and becomes this kind. |
| 19 | Ground trail | Byte parent-effect index, floor-following motion and shrink/removal after the parent's phase advances. |
| 20 | Homing projectile | Separate pitch/yaw approach toward a selected direction; kind 24 selects another render resource and becomes this kind. |
| 52 | Floor deformation | Specialized constructor; forward interpolation, hold and reverse interpolation through `effect_floor_deform_line`. |

The low two type bits select downstream collision targets: 1 actors, 2 player,
3 both. Terrain is handled before that query. Class 0 returns encoded `1`,
not the no-collision sentinel; it is not named as a generic disabled query.
The separate type bit `10` selects `player_state.magic` instead of fixed power
5 (`80037fc4..80037fd4`). Actor damage credit requires the entire masked high
nibble to equal `10`; testing the power bit does not establish player credit.
Actor/map callers retain high nibble `20` alongside named target bits. Only
its observed source category is established, not an independent flag meaning.
The floor controller's `f0` remains a special live type, not a free slot.

The shared projectile impact path forwards player status bits 1/2/4 as
curse/darkness/poison. Those calls now use the established player-status names;
zero damage components in these calls remain literal zero, not invented
element or spell identities.

The same value `ff` has different meanings in separate fields: free pool
slot, no render resource, billboard animation, wandering homing mode, leaf
branch and completed branch timer. Renderer `8001eedc` first rejects a missing
render resource; its separate mode test selects billboard or animated model.
These meanings must not share one generic sentinel name.

Homing selector `ff` randomizes direction, while `fe` aims at the player
(`800396f8..80039778`). Other byte values trigger a fresh actor-cone query,
not indexing that byte into the actor pool. The player caller passes an actor
index in this category, but the dispatcher does not consume it as an index.
The `fe` pitch calculation reads `target_distance` before this path initializes
it; preserve that retail behavior.

Branch role 0 is the root. It spawns roles 1 and 2 with angle offsets 1024 and
3072; each of those emits a leaf using its respective offset. These names
describe the explicit quarter-turn/three-quarter-turn parameters without
asserting visual left/right handedness. Leaf `ff` emits no descendants.
The halfword timer starts at 6, tests its old value before decrement, then is
set to `ff` after propagation. Thus children appear on the seventh visit to
that countdown, not the sixth. Role-dependent phase assignments 16/26/36 at
old phase 15 are followed by a common increment, producing 17/27/37 next.

Floor phases 0/1/2 mean advance/hold/reverse. Advance and reverse decrement a
halfword count and stop when its signed interpretation becomes -1. Hold
decrements the word in the reused position field until -1. Reverse starts
with Q12 progress 4096 and restores the iteration count. The typed fields,
wrapping and differing word/halfword reads are preserved.

## Units and retained values

Q12 unity is 4096; angles also wrap at 4096 but have distinct names. The orbit
advances 4096/64 = 64 angle units (5.625 degrees) per update, one revolution
per 64 active orbit updates. Its Y term uses twice the orbit angle and one
quarter of the sine output, giving two vertical cycles per revolution.
The saved X/Z center uses 256-world-unit buckets (`>>8`/`<<8`), not Q8 fractions.
The squared-distance helper's paired three-bit shifts reduce overflow risk
and restore world-distance units. Pitch clamps use +/-512 (45 degrees), but
the negative test intentionally reads yaw and compares it with -511 before
writing pitch -512. No correction of that asymmetric test is part of naming.

Actor damage scale 5000 is unity. The player radial API instead passes this
same number as Q12 scale: 5000/4096 = 1.220703125 before integer rounding.
Those player-call literals remain 5000; naming them actor-scale unity would
be false. Player direct hits use Q12 unity and receive the effect's byte ID
as a tenths multiplier. A caller's ID 10 therefore supplies a multiplier of
one in that API; the field must not be assumed to be only an owner identity.

The magic loader's 120 words are exactly 24 records * 20 bytes / 4 bytes.
`KF_MAGIC_RECORD_COUNT` names the array bound and `sizeof` derives the word
count without changing the four-byte copies. Constructor kind indexing still
forms a magic pointer even for kinds above 23; no new bounds check or larger
array is inferred.

This batch covers the shared protocol. Other kind/resource IDs, visual scale
and age thresholds, speeds, radii, sound ranges and random cutoffs still need
their complete per-consumer literal audit. Their numeric values are preserved;
the naming goal is not complete. Existing scatter, floor-deformation and map
collision dossiers account for their previously reviewed tuning/table values.
The explicit `20` type nibble, special `f0`, player radial scale 5000, storage
extraction shifts/masks and ordinary zero/index/count arithmetic are retained
for the domain-specific reasons above rather than given speculative names.

## Verification

All 13 directly affected units were force-compiled, followed by the full
comparison graph to rebuild shared-header consumers. All 112 objects preserve
every non-debug section; only five `.debug_line` sections differ. All 484
strict scores and the complete objdiff report are identical to `01c741c`.
The table records each reviewed function's final verdict: 19 remain exact,
eight retain their prior non-exact residues. Exact-count movement is zero.

An independent resolver compares all 2180 instruction words of the 19 exact
functions with the retail image, including branches, delay slots, ordered
calls and physical data addresses. The private floor table still matches all
35 bytes at `80056268`; private map-script data references use their original
`800561d0` section base. The renderer's unresolved `DAT_80055afc+58` retains
its decoded destination `80055b54`. Constant expansion and the documented
count/bit/angle expressions recover every original statement, type, value and
operation order in all 15 changed C/header files. No curated identity,
relocation, data owner, compiler profile or bank is changed.

`ruff check scripts tests` and `git diff --check` pass. The repository suite
runs 654 tests in 89.032 seconds: 653 pass, with the same existing failure in
the untracked save/load-hub test. It expects a 152-word retail body where the
current unchanged source emits 151 words. That test and its accompanying
untracked document are preserved and excluded from this campaign's commit.

Full `kf build -j4` completes its checks and remains nonzero on the existing
data ownership and placement gates: source data 7/60 (PSX 0/1, GAME 5/40,
OPEN 2/19), independent SDK data 4/4, target relink PSX 1/1, GAME 75/77 and
OPEN 34/38. The six conflicting section-base cases remain, with zero artifact
failures. These failures do not change or invalidate the identical function
objects; they remain required unfinished project work. No tooling or flake
change was made.
