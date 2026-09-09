# Sony helper near misses

The follow-up to the [147-site campaign](sony-header-helper-campaign.md)
examines source that resembles a helper but differs in component order,
object view, chained assignment, or control flow. It retains three additional
helper sites in **GAME `player_update`, `80018880`, 6684 bytes**. The Triple
Fang rotation setter improves strict objdiff **99.69479% → 99.96709%** and
removes the remaining operation-order differences. The function is still
non-exact: 55 instructions retain an eight-byte stack displacement difference.

The other tested substitutions provide negative evidence. None justifies
replacing the exact sprite/clamp/absolute-value source, and neither the effect
dispatcher nor map-object spawner improves. Matching supports the retained
source form; it does not prove that the original author used these macros.

## Function Match Plan

The campaign starts from `273accaa`. Before edits, the announced plan was to
inspect reordered vector triples and the existing rotation union, then test
the non-exact player/effect/map-object functions and related exact controls.
Each substitution is compiled independently against its unchanged source;
composition follows only after inspecting the individual results.

A lexical screen of 111 C sources found 54 adjacent three-component shapes,
including reordered axes and expressions with dependencies. These are leads,
not 54 ready substitutions or an exhaustive semantic census. Nine vector
sites were selected in the three non-exact functions. Rectangle, zero-vector,
clamp and absolute-value controls bring the total to 25 compiled trials in
nine functions, plus two dependency controls rejected before compilation.

For all 11 contexts, the evidence pass captured image-qualified `addr`,
`disasm --blocks`, incoming/outgoing `xref`, `strings`, and `match` queries,
adjacent bodies, source history, and 22 direct call sites in 13 callers. Direct
calls are proven decoded edges; validated data references and candidate
identities retain their existing evidence levels. None of these enclosing
game bodies is promoted to vendored code. Supplied SDK headers, already
identified in the [header census](sony-header-helpers.md), define the helpers.

The existing per-unit compiler profiles are preserved. In particular, the
player unit uses `probe-gcc257-o2-g8`, including `-O2 -G8 -mcpu=r2000`, with
the pinned GCC 2.5.7 rebuild and maspsx ASPSX 1.07 profile. No signature, object
layout, constant, function/data claim, relocation inventory, compiler option,
or library body changes in this campaign.

## Per-function evidence and verdicts

Addresses below are qualified by image; sizes are retail body bytes and
include the return delay slot.

| Image / address | Function / bytes | Relevant evidence | Final verdict |
| --- | --- | --- | --- |
| GAME `80018880` | `player_update` / 6684 | Camera rotation is an SDK `SVECTOR`; the existing `KfRotation` union exposes the same three halfwords to vector helpers and the Euler-angle consumer. Triple Fang adds pitch 64 and position Y 300 before the second constructor call. | Retain two setters and one copy; 99.96709%, with only the frame residue described below. |
| GAME `80038a38` | `effect_update_dispatch` / 6156 | Position copies use word lanes; motion/rotation stores use the existing halfword lanes. Several copies interleave Y work with X/Z arithmetic or `rsin`/`rcos`. | All five vector variants diverge at those operations; retain 99.82781% source. |
| GAME `80031834` | `map_object_spawn_effect` / 404 | Record rotation Z and X are cleared before `rand`; Y receives its result shifted by three. The body calls pool acquisition and action selection with their existing arguments. | Setter moves Z clearing past `rand`; retain 94.50495%, 400-byte reconstruction. |
| GAME `8001e480` | `render_screen_sprite` / 364 | Unsigned halfword rectangle and byte UV descriptor fields feed chained corner stores after the real `SetPolyFT4` call. | Direct and captured-input XY/UV helpers all diverge; retain 100%. |
| GAME `8001e230` | `render_enqueue_sprite` / 592 | Projected XY corners are packed word writes; UV corners use chained byte stores. The local zero-vector initialization stores lanes in reverse order. | UV and zero-vector helper controls diverge; retain 100%. |
| OPEN `800189a0` | `render_enqueue_sprite` / 540 | Same UV store shape as GAME, with independently checked OPEN calls and graphics data. | UV and zero-vector helper controls diverge; retain 100%. |
| GAME `80032cf0` | `audio_play_spatial` / 712 | Signed computed level is clamped to 0..127 before the existing sound/angle work. Retail branches assign the level conditionally. | `limitRange` changes that CFG and adds eight bytes; retain 100%. |
| GAME `8002dc70` | `actor_advance_animation_clamped` / 92 | Signed halfword delta, signed intermediate phase, upper-limit-first conditional phase stores, then a conditional absolute delta store. | Both helper variants change CFG; retain 100%. |
| GAME `800384f8` | `effect_floor_deform_line` / 460 | Signed word range is conditionally negated before the existing floor deformation traversal. | `abs` changes the conditional-negation CFG and removes eight bytes; retain 100%. |
| GAME `800308c0` | `actor_pool_load_placements` / 428 | Z and X are stored before `map_floor_height_at_position(&actor->position)` computes Y using that position. | Reject a naive XYZ setter before compilation: Y would read the old Z. Source unchanged. |
| GAME `8002f8cc` | `actor_update_boss_death_sequence` / 444 | Consecutive random draws populate X, Z, then Y. | Reject a naive XYZ setter before compilation: it would assign the second and third draws to different axes. Source unchanged. |

## Retained player reconstruction

Three nearby rotations now use the existing `effect_rotation.vector` view:

1. `setVector` assigns negative camera X, camera Y, and negative camera Z.
2. `copyVector` copies camera X/Y/Z.
3. `setVector` assigns camera X plus `PLAYER_TRIPLE_FANG_PITCH_OFFSET`, camera
   Y, and camera Z for the second homing projectile.

The first two are individually identical in allocated bytes, symbols and
ordered relocations. The third replaces explicit Y/Z/X assignments. Its
emitted loads, stores, registers and argument preparation at the second
constructor now follow retail. The composed production object is identical
to the isolated third trial in every allocated section's bytes, size, flags,
alignment, function/object symbols and ordered relocation records.

`setVector` and `copyVector` in the supplied `LIBGPU.H` write three components
in X/Y/Z order. Neither copies padding. This uses the existing eight-byte
rotation union; a plain six-byte `KfEulerAngles` object is not silently
converted to an eight-byte SDK vector, and no cast or padding is introduced.

The natural alternative of copying all three camera components and then
adding 64 to X emits an additional initial X store. It grows the body to
6688 bytes and scores 99.64093%; it is rejected on that concrete divergence.

An independent relocation-resolution audit resolves function/data symbols,
HI16/LO16 addends, and internal jumps to retail addresses before comparing
all 1671 instruction words. Resolving the delinked target first reproduces
the original 6684 retail bytes exactly, providing a control for the audit.

| Comparison against retail | Before | Retained |
| --- | ---: | ---: |
| Unequal instruction words | 63 | 55 |
| Ordered direct calls | 66, identical | 66, identical |
| Ordered data referents | 210, identical | 210, identical |
| Frame allocation | 216 bytes | 216 bytes |
| Retail frame allocation | 224 bytes | 224 bytes |

Every one of the 55 remaining unequal words has the same opcode and register
bits as retail, uses SP as its base/source register, and differs only by eight
in the low immediate. This includes frame allocation/restoration and displaced
local/save-slot references. It is an unattributed frame-layout residue, not an
exact result or a proved compiler mechanism. Earlier independently tested
frame, motion-vector and camera-owner hypotheses are recorded in the
[low-trial campaign](game-low-trial-campaign.md); artificial storage is not a
valid way to close it.

## Trial ledger

Scores are strict objdiff percentages, with no loose comparison. Sizes are
compiled bytes. Each row tests one substitution against `273accaa`; rows are
not cumulative. All other functions in each tested unit keep their initial
strict score.

| Trial | Function / change | Before → after | Bytes before → after | Decision |
| --- | --- | ---: | ---: | --- |
| V008 | GAME dispatcher, impact position X/Z/Y → setter | 99.82781 → 99.43470 | 6156 → 6156 | Reject |
| V009 | GAME dispatcher, local motion Y/X/Z → setter | 99.82781 → 99.82651 | 6156 → 6156 | Reject |
| V010 | GAME dispatcher, trigonometric spawn X/Z/Y → setter | 99.82781 → 99.26771 | 6156 → 6156 | Reject |
| V011 | GAME dispatcher, offset position X/Z/Y → setter | 99.82781 → 99.55881 | 6156 → 6156 | Reject |
| V013 | GAME dispatcher, masked rotation Y/X/Z → setter | 99.82781 → 99.82391 | 6156 → 6156 | Reject |
| V024 | GAME map-object spawner, Z/X/Y → setter | 94.50495 → 92.67326 | 400 → 400 | Reject |
| V038 | GAME player, signed camera components → setter | 99.69479 → 99.69479 | 6684 → 6684 | Retain; raw identical |
| V039 | GAME player, camera copy → setter | 99.69479 → 99.69479 | 6684 → 6684 | Equivalent; prefer copy helper |
| V039_copy | GAME player, camera copy → `copyVector` | 99.69479 → 99.69479 | 6684 → 6684 | Retain; raw identical |
| V040 | GAME player, Triple Fang Y/Z/X → setter | 99.69479 → 99.96709 | 6684 → 6684 | Retain; retail operation order |
| V040_copy_offset | GAME player, camera copy then pitch addition | 99.69479 → 99.64093 | 6684 → 6688 | Reject; extra X store |
| P001 | GAME screen sprite, direct `setXYWH` | 100 → 86.69231 | 364 → 412 | Reject |
| P002 | GAME screen sprite, direct `setUVWH` | 100 → 86.69231 | 364 → 412 | Reject |
| P003 | GAME enqueued sprite, direct `setUVWH` | 100 → 88.38513 | 592 → 636 | Reject |
| P004 | OPEN enqueued sprite, direct `setUVWH` | 100 → 89.748146 | 540 → 584 | Reject |
| P005 | GAME enqueued sprite, chained zero lanes → setter | 100 → 99.98649 | 592 → 592 | Reject; store order |
| P006 | OPEN enqueued sprite, chained zero lanes → setter | 100 → 99.98518 | 540 → 540 | Reject; store order |
| P007 | GAME screen sprite, captured XY inputs → `setXYWH` | 100 → 85.70330 | 364 → 340 | Reject |
| P008 | GAME screen sprite, captured UV inputs → `setUVWH` | 100 → 85.70330 | 364 → 340 | Reject |
| P009 | GAME enqueued sprite, captured UV inputs → `setUVWH` | 100 → 90.97298 | 592 → 568 | Reject |
| P010 | OPEN enqueued sprite, captured UV inputs → `setUVWH` | 100 → 90.10371 | 540 → 516 | Reject |
| C001 | GAME spatial audio, level `limitRange` | 100 → 95.08427 | 712 → 720 | Reject |
| C002 | GAME animation, signed phase `limitRange` | 100 → 65.434784 | 92 → 92 | Reject |
| A001 | GAME animation, signed delta `abs` | 100 → 60.217392 | 92 → 84 | Reject |
| A002 | GAME floor deformation, signed range `abs` | 100 → 92.113045 | 460 → 452 | Reject |

The sprite differences are specific. Retail stores paired corners in chained
assignment order (for example U2 then U0), then reloads the descriptor origin
when computing the far edge. Direct SDK rectangle macros instead process
vertices in order and repeat descriptor reads. Capturing four real inputs in
locals removes those repeated reads, but also removes six retail instructions
and retains the wrong vertex-store order. Both controls disagree with retail.
The packed projected XY word writes in the enqueuers remain their own form.

The dispatcher controls likewise preserve arithmetic and calls but move
particular component operations away from retail: V009 swaps the X/Y zero
stores, V010 moves Y between `rsin` and `rcos` and changes a call delay slot,
and V008/V011 move Y before Z. V013 changes the order of the existing masked
halfword lane calculations. In the map-object control, the first local change
is the missing pre-`rand` Z store, reappearing after the call. These are
instruction-level rejection reasons, rather than score-only verdicts.

For the scalar controls, `limitRange` is the SDK's lower-bound-first ternary
assignment. The actor trial retains a signed local before publishing to the
unsigned phase field; directly clamping that field would additionally lose
the negative test. The `ABS.H` macro is tested only with pure scalar arguments
in whole assignment expressions. Neither control changes a call target or
solves a missing library boundary; each changes the retail branch/store form.

## Verification

The retained object was rebuilt with `kf try --unit game.player_update`, then
the complete `kf build` ran. Strict function counts remain **453/471**:
PSX 1/1, GAME 345/362, OPEN 107/108. No banked function regresses, and the
non-exact player result is not banked. The generated README fuzzy total rises
from 99.89% to 99.90%.

The baseline and final full builds both exit nonzero for existing data and
known-reference ownership/placement failures. Data matches remain PSX 1/1,
GAME 30/44, OPEN 11/20 (23 divergent units); target relink remains PSX 1/1,
GAME 68/77, OPEN 33/38. These failures are not repaired or hidden by this
source-only reconstruction change.

`ruff check scripts tests`, all 769 Python tests, Rust tests, and all 112
source/image variants under `kf check-types` pass. `git diff --check` passes.
The local trial sources, objects, semantic dossiers, raw diffs and resolved
audit are generated under `build/sony-helper-nearmiss/` and are not committed.
