# Modern type checking and retail storage

## Direction

The modern compiler should verify domain types through fields, locals,
parameters and returns while the pinned compiler continues to build the
retail C objects. HoMM2's local `decomp-gold-2.1-buka` branch demonstrates
this separation in `include/Ints.h`: C++20 uses scoped enums and explicit
storage types; the old compiler sees the corresponding integer types.
King's Field needs its own C-compatible declarations, not HoMM2's C++ ABI.

A Clang 21.1.8 C++20 syntax probe of the 111 editor-command sources at
`ecb078c` passes 59 sources and fails 52. This is a feasibility census,
not an installed strict check. Common failures are C implicit `void *`
conversions, incomplete SDK declarations, missing game declarations and
incompatible vector pointers. Fix known game types and propagate them;
reserve explicit conversions for actual allocation or encoded-data
boundaries. SDK compatibility declarations must retain checked parameter
types. Neither permissive pointer shims nor casts between unrelated enum
domains establish type safety.

Actor action and lifecycle are distinct byte-sized domains. Action progress
needs a more careful model: the same byte holds jump phases, pursuit retreat
counts and a post-death counter. It cannot truthfully become an enum of just
the three jump phases. Modern scoped enums can prohibit unrelated-domain
assignments, but cannot by themselves prove that a phase is used under the
correct runtime action tag.

A bounded storage probe confirms the proposed language split with Clang
21.1.8 and both pinned GCC 2.5.7/2.6.0 probes. C++20 sees `enum class
Action : u8`; legacy C sees `typedef u8 Action` and the corresponding
anonymous enum constants. The same source declares an `Action` field,
local, parameter and return, with one-byte domain and two-byte aggregate
size assertions. All three compilers accept the valid chain. Clang rejects
three independent negative controls: assigning a different enum, assigning
the raw integer 16, and passing an argument after erasing its enum type
to `u8`. This proves the declaration mechanism, not a completed migration
of the game fields or a runnable modern game build.

## Function Match Plan: effect direction arguments at `ecb078c`

Clang rejects the six-byte `KfVec3s *` direction passed to
`effect_pool_construct`, whose `SVECTOR *` contract copies eight bytes.
`magic_cast` already hides that mismatch behind two casts. The boss-death
caller also passes a twelve-byte position to the constructor's sixteen-byte
`VECTOR` copy and calls an uninitialized direction buffer `effect_output`,
although the constructor reads it. Correct these object extents before
adopting strict enum fields.

Hash-identical retail was initialized. All six semantic views, source
history, adjacent math functions, callers and constructor references were
inspected separately for GAME and OPEN. The functions below are game-owned
helpers/policy, not entries in the vendored inventory. Their SDK calls
retain the authentic Psy-Q types and implementations.

| Image | VA / bytes | Function | Initial strict % | Evidence and first hypothesis |
|---|---|---|---:|---|
| GAME | `80014d34 / d4` | `pitch_yaw_to_forward_vector` | 100 | Three halfword stores at output +0/+2/+4; all three callers subsequently pass the object to the eight-byte constructor copy. Use `SVECTOR *`, retaining those three stores. |
| GAME | `80014e48 / 5c` | `vector3s_scale_shift12` | 100 | Signed halfword loads, multiply and arithmetic shift by 12; final store remains in the return delay slot. Propagate the same direction type. |
| GAME | `8002edd4 / 454` | `actor_spawn_action_effect` | 100 | One direction object feeds both math helpers and the constructor, inside the 200-byte frame. Declare that object as `SVECTOR`. |
| GAME | `80018880 / 1a1c` | `player_update` | 96.945540 | The direction at sp+40 flows through calls at `800197c4/800197d0` and the constructor's fifth-argument slot. Propagate `SVECTOR` without changing its initialization. |
| GAME | `8003a2a0 / 4c0` | `magic_cast` | 98.875000 | Direction sp+32 flows through both helpers at `8003a510/8003a51c` to both constructor arms. Replace the undersized local and remove its casts. |
| GAME | `8002f8cc / 1bc` | `actor_update_boss_death_sequence` | 100 | A 64-byte frame passes direction sp+24 and position sp+32. Position writes +32/+36/+40; the constructor also reads +44. Model `SVECTOR` and `VECTOR`, preserving the untouched lanes. |
| GAME | `80036f44 / 82c` | `effect_pool_construct` | 100 | Loads position +0/+4/+8/+12 and direction +0..+7 before kind dispatch. Keep this exact consumer as the width control. |
| OPEN | `80015b0c / d4` | `pitch_yaw_to_forward_vector` | 100 | Same six-byte mutation and SDK call shape, with no decoded callers. Share the declaration consistently; full-object extent comes from GAME consumers, not invented OPEN callers. |
| OPEN | `80015be0 / 5c` | `vector3s_scale_shift12` | 100 | Same scalar instruction sequence; no decoded callers. Preserve the shared type and unchanged six-byte mutation. |

The snapshot also includes exact GAME `matrix_set_rotation_yxz`,
`vector2s_scale_shift11`, `vector2s_scale_shift12`, and
`actor_update_effect_action`, plus `actor_update_current_action`
(98.938940%) as adjacent/caller controls. Every reviewed function will get
an unchanged-or-diverged final verdict after forced compilation.

The new SDK object extent does not justify initializing padding or the
boss effect's untouched direction. Retail performs those reads without
caller writes. Rotation arguments passed through the variadic tail remain
a separate extent audit; fixing the fixed arguments does not certify the
entire constructor interface or complete the modern compilation cleanup.

Require identical non-debug object sections and ordered relocations, all
484 current function comparisons unchanged, focused Clang diagnostics,
repository lint/tests and full `kf build` before committing. Preserve the
known partial functions and existing data/ownership failures; no banking or
compiler-profile change is part of this correction.

## Vector correction result

The retained source uses authentic `SVECTOR` direction objects in all three
callers, with that type propagated through both math helpers and the shared
declarations. Both OPEN implementations use the same declaration without
inventing OPEN callers. The boss-death caller now owns the complete
`VECTOR` position and `SVECTOR` direction. No initialization or numeric
value was added. The four curated math signatures follow the source types.

Forced compilation of all six affected units preserves every non-debug
section of 111 of the 112 current objects. The sole code difference is five
stack operands in `magic_cast`'s other, ground-branch arm: the corrected
direction extent changes reuse of its block-local storage. Each new word
equals the corresponding retail instruction; source length, branches,
call sites and all relocation sections are unchanged.

| Retail instruction VA | Operation | Previous stack offset | Corrected / retail offset |
|---|---|---:|---:|
| GAME `8003a64c` | Store spawn X in the `rcos` call delay slot | 56 | 32 |
| GAME `8003a6ac` | Reload spawn X for cell division | 56 | 32 |
| GAME `8003a6e4` | Pass spawn position in `$a3` | 56 | 32 |
| GAME `8003a6ec` | Store spawn Z | 64 | 40 |
| GAME `8003a738` | Store spawn Y | 60 | 36 |

The initial expectation of wholly identical code was therefore too narrow.
Keep the independently supported extent correction: `magic_cast` improves
from 98.875000% to 98.891450%, and the other 483 comparisons are unchanged.
It remains partial; the outstanding instruction differences are not
attributed to a compiler mechanism. All 14 evidence snapshots receive a
final verdict: 13 unchanged and the five-operand improvement above. Eleven
are exact, verified against 1,181 complete linked retail words with their
ordered calls and data references. No exact-count movement or banking.

The same Clang C++20 probe now passes 60 of 111 sources. The five
actor-behavior errors disappear, and player update's three direction
argument errors disappear; player update still has its existing SDK
`PadRead` declaration error. This step does not install the modern mode
in clangd or finish its remaining 51 failing sources.

Final verification: Ruff and `git diff --check` pass. The repository runs
654 tests in 92.085 seconds: 653 pass and only the pre-existing untracked
save/load-hub instruction control fails. The first run additionally found
a stale final-signature field in the semantic evidence; the GAME and OPEN
evidence now agrees with the four curated signatures, and that test passes.

Full `kf build` retains the existing data/ownership failures: source data
7/60 complete contributions match; the four config SDK contributions pass;
target relink remains PSX 1/1, GAME 75/77, OPEN 34/38. No compiler profile,
banked score, build tooling, SDK header or unrelated input is changed.

## Function Match Plan: actor enum domains at `1d8a364`

Adopt `KfActorAction` and `KfActorLifecycle` as scoped one-byte enum types in
the modern view, with `u8` aliases and the same enumerators in legacy C.
Propagate action through the field, both selection locals, the setter's
parameter and all four eligibility helpers' parameters and returns. Keep
lifecycle typed in every update, render and map-script consumer. The floor
save stream is the only raw lifecycle boundary: explicitly encode its two
written states and decode its input byte without adding runtime validation.

The original C remains the retail build input. Add a C++20 checking command
and modern clangd mode so domain erasure becomes a compiler error. The
layout inventory must read each enum's declared storage and retain its
semantic type name. Header assertions guard field sizes and offsets; modern
compilation checks propagation through the real consumers. A small negative
control verifies that the compatibility layer enables enum restrictions.
The inventory must not silently assume that an unrecognized type
occupies one byte. Action progress retains its current multi-purpose byte
until its phases and counter uses can be represented faithfully.

The GAME retail files were revalidated. All six semantic views and source
history were refreshed for 27 functions: every action/lifecycle consumer
listed in `game-actor-states.md` except the unrelated vertical-only/motion
functions, plus the floor-5 cutscene, floor-item renderer and effect
constructor controls. Each has an image-qualified snapshot and will retain
its own final verdict. The actor-state dossier supplies the preceding
adjacent-function and constant-domain evidence.

Key ABI controls are `actor_set_action` at `8002cec8` (12 bytes), which
stores +8 and clears +56 in its return delay slot; the action helpers at
`8002dd94`, `8002deb4`, `8002e018`, `8002e0f0`, which load the action with
`lbu` and mask arguments/results to 255; and the +6 lifecycle byte in the
pool, dispatcher, renderer and saved-state streams. All action numbers,
sentinels, branch conditions and effect-profile indices remain unchanged.
The profiled helper's `profile_index` is not an actor action.

Two existing errors prevent modern checking of the full consumer family.
In the floor-5 cutscene (`800346a8`, 908 bytes, exact), the fifth constructor
argument is an uninitialized eight-byte stack object at sp+160. Replace its
`s32 aux[2]` declaration with `SVECTOR direction`, preserving all untouched
bytes, its address and the constructor's eight-byte read. For
`render_entities` (`8001f218`, 1408 bytes, 91.25%), add the missing shared
declaration of `render_floor_item(KfFloorItem *)`; its body at `8001ed90`
and sole caller establish that pointer and unused return. These repairs
must not introduce permissive overloads or casts over the errors.

Before committing, force the seven consuming units to rebuild, verify all
112 objects and 484 comparisons against `1d8a364`, resolve exact functions
to raw retail words and ordered references, run modern checks on every
consumer, run the whole-tree modern census without suppressing failures,
and complete repository tests, Ruff, full `kf build` and `nix flake check`.
The remaining modern compilation errors are migration work, not a passing
whole-tree check or an excuse to relax the enum constraints.

## Actor enum result

`KfActorAction` and `KfActorLifecycle` now retain their distinct domains in
the modern view, including selector parameters/returns and the selection
locals. Both still occupy one byte; `KfActor` remains 0x48 bytes with lifecycle
at +6 and action at +8. Encoding and decoding are explicit at the saved-floor
byte stream. The conversion helpers accept only integer/enum boundaries,
not direct conversion between unrelated enums; they do not validate saved
values at runtime. The shared `SVECTOR` cutscene argument and floor-item
declaration repair the existing errors in the remaining consumers.

Clangd now defaults to modern checking, with a persistent `--mode retail`
option. `kf check-types` uses the same modern arguments but compiles every
selected source/image variant, including both images of shared sources.
All seven actor-consuming units pass. The full census passes 62 of 112
variants and fails 50, reporting all diagnostics and a failing exit status.
The remaining failures include implicit `void *` conversions, missing game
and SDK declarations, and incompatible primitive/SDK pointer types. These
remain work to resolve through actual types; no permissive shim was added.

Forced compilation preserves all 112 object files byte for byte and all
484 strict function comparisons. The 27 reviewed functions below all retain
their previous result. The 18 exact functions additionally agree with 2,025
complete linked retail words, including delay slots and ordered calls/data
references. The nine partial functions have unchanged objects and retain
their unattributed differences. No exact-count movement or banking.

| GAME VA | Function | Strict % before / after | Final verdict |
|---|---|---:|---|
| `8001ed90` | `render_floor_item` | 98.795180 | Unchanged partial |
| `8001f218` | `render_entities` | 91.250000 | Unchanged partial |
| `8002cc64` | `actor_initialize` | 100 | Unchanged exact |
| `8002cdcc` | `actor_initialize_slot` | 100 | Unchanged exact |
| `8002ce88` | `actor_pool_clear` | 100 | Unchanged exact |
| `8002cec8` | `actor_set_action` | 100 | Unchanged exact |
| `8002ced4` | `actor_pool_spawn` | 100 | Unchanged exact |
| `8002cf84` | `actor_pool_begin_death_by_definition` | 100 | Unchanged exact |
| `8002d120` | `actor_apply_damage` | 89.986725 | Unchanged partial |
| `8002d4a8` | `actor_pool_apply_radial_damage` | 86.103170 | Unchanged partial |
| `8002d7f8` | `actor_pool_find_target_in_cone` | 100 | Unchanged exact |
| `8002da6c` | `actor_pool_find_overlap` | 100 | Unchanged exact |
| `8002dd94` | `actor_try_select_action_distance_facing` | 100 | Unchanged exact |
| `8002deb4` | `actor_try_select_ground_action` | 100 | Unchanged exact |
| `8002e018` | `actor_try_select_facing_action` | 100 | Unchanged exact |
| `8002e0f0` | `actor_try_select_profiled_action` | 96.333336 | Unchanged partial |
| `8002e2e8` | `actor_select_next_action` | 100 | Unchanged exact |
| `8002e6a8` | `actor_update_awareness` | 97.736840 | Unchanged partial |
| `8002f8cc` | `actor_update_boss_death_sequence` | 100 | Unchanged exact |
| `8002fa88` | `actor_update_current_action` | 98.938940 | Unchanged partial |
| `80030818` | `actor_pool_update` | 100 | Unchanged exact |
| `800308c0` | `actor_pool_load_placements` | 100 | Unchanged exact |
| `80033f64` | `map_ambient_script_floor1` | 100 | Unchanged exact |
| `800346a8` | `map_floor5_transition_cutscene` | 100 | Unchanged exact |
| `80035b5c` | `map_world_state_persist` | 94.821840 | Unchanged partial |
| `80035e44` | `map_restore_floor_state` | 99.964540 | Unchanged partial |
| `80036f44` | `effect_pool_construct` | 100 | Unchanged exact |

The compiler checks real field, parameter, return and layout constraints;
separate tests for each use would duplicate that check. Keep one negative
integration control for the compatibility layer and two tests for the
Python inventory's enum storage/alignment parser. Remove the old source-text
rule forbidding `static_assert`: it conflicts with using the compiler to
verify the reconstructed layout. Legacy acceptance is exercised by the
normal retail compilation, rather than another per-field fixture.

Final verification: Ruff and `git diff --check` pass. `nix flake check -L`
passes, including the modern enum control; its isolated suite runs 655 tests
with 137 expected skips for unavailable local retail/compiler artifacts.
The local suite runs 656 tests in 80.857 seconds: 655 pass and the existing
untracked save/load-hub word comparison remains the only failure. Its source
object is unchanged. The first full run also exposed the obsolete assertion
ban described above; removing that rule resolves the new failure.

Full `kf build` still fails the existing data/ownership checks: source data
7/60 complete contributions match, all four config SDK contributions pass,
and target relink remains PSX 1/1, GAME 75/77, OPEN 34/38. These failures and
the 50 failing modern source/image variants are not claimed as resolved by
this actor-domain checkpoint.

## Function Match Plan: actor slot and movement states at `f4c7b23`

Extend scoped byte storage to `KfActorSlotState`, `KfActorVerticalState` and
`KfActorCollisionState`. The live actor and the 16-byte placement record share
the slot domain; the loader copies +0 without translating it. Keep the
placement terminator `ff` and the four policy values unchanged. Vertical
states remain the five gravity/landing policies, and collision states remain
the three horizontal steering outcomes documented in `game-actor-states.md`.
The world-query result and the mover's boolean result are different domains.

The byte load at GAME `8002e6dc` captures slot policy before the awareness
distance call, then `8002e750` masks it to 255 for the dormant branch's
comparisons with 2, 1 and 3. Propagate the enum through both existing locals;
rename the untyped `kind` copy to `spawn_policy`. Its narrowed domain follows
the decoded byte value, rather than erasing that value to `s32`. Compare the
resulting instructions explicitly: the current function is 97.736840%, not
exact, and the effect of the C local's width remains to be measured.

Hash-identical GAME retail was revalidated. Eighteen consumer/control
snapshots were refreshed with all six semantic views and source history,
plus `map_resources_load` at `8001b558` as the placement-stream caller.
The earlier actor-state dossier provides the per-function state semantics,
adjacent functions and vendor-negative evidence. Additional exact controls
are `actor_pool_find_free` (`8002ca78`, 60 bytes), `actor_pool_find_at_tile`
(`80033ee4`, 128 bytes), `map_load_floor` (`80036554`, 164 bytes), and the
resource caller (600 bytes). They expose the free-slot checks and load path,
not new library ownership or reconstruction progress.

Retain the live actor's offsets +0, +11 and +57, the placement's +0 slot byte,
all aggregate extents, values, calls, CFG and ordered referents. Layout
inventories and modern compilation check the types; no new per-field tests
are needed. Force the affected units to compile, compare all existing objects
and scores with the immutable `f4c7b23` baseline, independently resolve exact
functions against retail words, then run existing tests, lint, whitespace
checks and full `kf build`. Every reviewed function receives a final verdict.

The first compilation of two byte locals adds a copy before the first mask
and repeats the mask after `rand`, growing awareness by eight bytes. Only
that partial score changes, from 97.736840% to 96.099420%; the other 483
comparisons stay unchanged. Retail and the prior source instead retain one
promoted policy value. Model this C integer promotion explicitly with
`KF_ENUM_PROMOTED`: the legacy view uses the type of unary `+` on the enum's
integer storage, while the modern view retains the same scoped enum domain.
This preserves the existing word-valued local without an unchecked integer
escape in Clang. No arithmetic, cast expression or extra local is added to
the function body. Recompile to verify the promotion rather than accepting
the narrowed-local instruction sequence.

Per the user's subsequent direction, remove compile-time size assertions
from game sources and headers. This includes the enum-size checks, which
mostly repeat the declared byte storage, and the aggregate-size checks.
Use the existing layout inventories and retail comparisons for those facts.
Historical references above describe the checks used during their original
campaigns; they are not instructions to reintroduce size assertions.

## Slot and movement state result

The three domains now propagate through both placement/live slot fields and
the live vertical/collision fields. `spawn_policy` retains the promoted C
integer value and the modern slot enum domain. All 112 objects retain their
non-debug contents and all 484 strict comparisons remain unchanged. Only
four `.debug_line` sections change after removing size assertions from C
files. The source/header scan finds no remaining compile-time size assertion;
27 were removed, including four added during this uncommitted campaign.

All 19 reviewed functions retain their initial strict result. Fourteen exact
functions additionally match 1,157 linked retail words and their ordered
calls and data addresses. No exact count changes and no banking occurs.

| GAME VA | Function | Strict % before / after | Verdict |
|---|---|---:|---|
| `8001b558` | `map_resources_load` | 100 | Unchanged exact |
| `8002ca78` | `actor_pool_find_free` | 100 | Unchanged exact |
| `8002cc64` | `actor_initialize` | 100 | Unchanged exact |
| `8002ce88` | `actor_pool_clear` | 100 | Unchanged exact |
| `8002ced4` | `actor_pool_spawn` | 100 | Unchanged exact |
| `8002cf84` | `actor_pool_begin_death_by_definition` | 100 | Unchanged exact |
| `8002d120` | `actor_apply_damage` | 89.986725 | Unchanged partial |
| `8002dd94` | `actor_try_select_action_distance_facing` | 100 | Unchanged exact |
| `8002e0f0` | `actor_try_select_profiled_action` | 96.333336 | Unchanged partial |
| `8002e2e8` | `actor_select_next_action` | 100 | Unchanged exact |
| `8002e6a8` | `actor_update_awareness` | 97.736840 | Unchanged partial |
| `8002e954` | `actor_move_xz_with_collision` | 100 | Unchanged exact |
| `8002ed00` | `actor_move_along_heading` | 100 | Unchanged exact |
| `8002fa88` | `actor_update_current_action` | 98.938940 | Unchanged partial |
| `80030818` | `actor_pool_update` | 100 | Unchanged exact |
| `800308c0` | `actor_pool_load_placements` | 100 | Unchanged exact |
| `80033ee4` | `actor_pool_find_at_tile` | 100 | Unchanged exact |
| `80035b5c` | `map_world_state_persist` | 94.821840 | Unchanged partial |
| `80036554` | `map_load_floor` | 100 | Unchanged exact |

The full modern census remains 62/112 passing variants, including all seven
actor consumers. The other 50 retain their existing diagnostics. Ruff and
`git diff --check` pass. The existing local suite runs 656 tests in 83.694
seconds: 655 pass and the untracked save/load-hub instruction comparison
remains the only failure. No new tests were added.

Full `kf build` retains the same data/ownership failures: source contributions
7/60, config SDK contributions 4/4, target relink PSX 1/1, GAME 75/77, OPEN
34/38. Removing assertions has not fixed or hidden those discrepancies.
