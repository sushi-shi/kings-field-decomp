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
