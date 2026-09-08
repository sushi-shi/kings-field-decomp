# GAME ground and profiled action selection

## Function Match Plan

Related `game.actor` campaign under the existing GCC 2.5.7 probe. The common
caller, current-actor owner, probability gate and facing helpers support the
family; no unit boundary or shared layout change is assumed.

### Ground action: GAME `0x8002deb4`, 356 bytes

Starting strict match: 95.314606%. The source object is 360 bytes. Signature
is `u8 (u8 action, s32 distance, u16 chance)`: the only direct caller, at
`actor_select_next_action+0xcc` (`0x8002e3b4`), passes action 16, the incoming
signed distance and the definition's halfword at +52. The call delay slot
sets distance; the caller masks the result to a byte and compares with 255.
The body masks the incoming action and chance to eight and sixteen bits.

Retail saves `ra/s2/s1/s0` in a 32-byte frame. It preserves an unfinished
matching action, then checks actor y against minus 100 times the byte in the
100-wide floor-height grid at `[cell_z][cell_x]`. The current target gets
cleared and immediately accepts the requested action. Other paths reject
distances through 4000, halve odds above 7000, multiply odds by eight for
4001..7000, compare `rand() >> 4`, and accept either `rand() < 1638` or a
facing check with tolerance 398. The final failure is 255. Every return
joins the frame restoration at `0x8002e000`; its `jr` delay slot adds 32.

There are four proven calls: two `rand`, `vector_xz_to_angle` and
`angle_within_tolerance`. Four validated address pairs name `actor_state`
members current, player target and player x/z. Two validated internal jumps
target +0x14c and +0xdc. No strings or unresolved indirect transfers occur
inside this game function.

The first referent defect is an absent relocation pair at `0x8002def8` /
`0x8002defc`: retail words `3c048009` / `24845900` form exactly `0x80095900`,
the existing `map_floor_height_grid` BSS identity. The decoded row/column
chain and byte load establish its use; the source already names that grid.
Its 10,000-byte owner and nineteen other validated references were checked.
First add this reviewed pair, preserving its absolute target. This corrects
the target model; do not replace the source reference with an integer.

Then examine the first remaining CFG difference: retail's early successful
target-clear path returns through the shared epilogue, but the compiler
creates a separate action-return tail. Test one explicit shared accepted
exit without changing the random-call paths. Only after CFG agreement,
compare the row-base register-role difference; direct two-dimensional
indexing is the independently supported source expression.

### Profiled action: GAME `0x8002e0f0`, 504 bytes

Starting strict match: 86.047620%. Current signature is
`u8 (u8 action, s32 distance, u8 profile_index, u16 chance)`. Direct caller
sites `0x8002e41c`, `0x8002e450`, `0x8002e484` pass actions 19/20/21,
definition bytes at +5/+6/+7 and +8/+9/+10 for profile/chance, and the same
signed distance. All three mask the return to eight bits before testing
255. Byte loads establish the argument values, not the callee parameter's
source width. Retail masks the profile to five bits immediately and later
to sixteen bits; retain the halfword local and audit any signature change
across all callers and the identity/header together.

The 40-byte frame saves `ra/s3/s2/s1/s0`. A ten-byte signed-halfword profile
selects a distance weight: default near weight at +8, far weight at +2 when
distance reaches the +0 threshold, otherwise middle weight at +6 when it
reaches the +4 threshold. Retail emits the far case first. The signed
word-sized weighted odds are `(chance * weight) >> 8`, followed by the first
random gate. Facing tolerance is 341; failure may still accept with
`rand() < 819`. Profile 9 alone counts live lifecycle-1 actors and kind-9
effects, rejecting a count of two or more. The actor/effect loops use signed
halfword countdowns from 127 and 47 and strides 72 and 60, respectively.

Four proven calls are `rand` twice and the same vector/facing helpers. Five
validated address pairs name `actor_action_profiles`, actor current/player
x/z and `effect_state+0x1e0`; one validated internal jump joins weight
selection at +0x98. Retail derives the actor-array base as player-position
base minus 9216, within the existing complete `actor_state` owner. There are
no strings or unresolved indirect transfers in this function.

After ground-action work, first test the retail's far-first weight-selection
CFG. Do not change profile width, weighted-odds lifetime and owner access
simultaneously. Compare each rebuilt object from its first real difference.

### Shared evidence and gates

All six semantic views, caller argument setups, preceding
`actor_try_select_action_distance_facing`, intervening
`actor_try_select_facing_action`, and the following caller were inspected.
The two neighbors are exact controls. The angle callee implements circular
12-bit tolerance; the vector callee takes signed x/z differences. `rand` is
the vendored BIOS A0/2f jump stub, retained as an SDK/libc call. Neither game
selector occurs in the vendored inventory or available FID matches; their
actor-specific data, profile table and pool loops are not library bodies.
Source history includes `12d4f21`'s preceding core and `642f170`'s addition
of these selectors with the same row temporary and near-first branch form;
it is not original-source proof.

Rebuild affected units, record canonical strict objdiff, audit raw words,
delay slots and ordered referents, and run full build plus existing lint and
tests. Preserve every banked function and only bank a new strict 100% result.

## Ground-action iteration

The admitted floor-grid pair passes the shared validation and changes the
strict match to 95.337080% without changing C. A shared accepted label inside
the final facing-success arm removes the extra tail but incorrectly reverses
the final branch and sinks the target-clear store into the earlier jump's
delay slot; this control is reverted. Next test the other directly observed
join: all rejection paths reach the same final 255 return. This leaves the
accepted returns in their original arms and makes only rejection sharing
explicit.

The rejection join restores the complete retail CFG and size. Replacing the
temporary row/cell sequence with direct `[cell_z][cell_x]` indexing then
closes ground selection at strict 100%.

## Profile-action iteration

Far-first weight selection preserves semantics and raises the strict match
from 86.047620% to 94.555560%. The first residual instruction is still the
extra byte mask when assigning the five-bit profile into its halfword local.
Before changing a shared signature, test assigning the incoming byte to the
existing halfword local and then masking that local. Both steps correspond
to the already established profile-value/low-five-bit contract; caller and
parameter types stay unchanged. Do not combine this with weight-lifetime or
owner-address changes.

The two-step local mask still emits two masks (`andi ...,0xff` followed by
`andi ...,0x1f`) instead of retail's single five-bit mask. It changes saved
register roles but does not resolve the first real difference; revert this
control and keep only the far-first source correction. The existing ABI is
unchanged. Remaining differences include this narrowing, separate weight
and odds lifetimes, and independent actor-array address formation instead
of retail's reuse of the player-position member base. None is attributed
to a proved compiler mechanism.

## Raw comparison and final verdicts

Ground selection is strict **100%**, up from 95.314606%. All 89 instructions
agree after resolving the two internal `R_MIPS_26 .text` targets to +0x14c
and +0xdc; their literal object addends differ by the eight-byte placement
shift caused by preceding partial functions. The other 87 raw words agree
literally. All sixteen ordered relocations agree in offset, kind and target,
including the added floor-grid pair. Constants, branch/load delay slots and
the original random-call paths are preserved.

Profiled selection is strict **94.555560%**, up from 86.047620%, and remains
partial. It is not banked. The full caller/neighbor family retains its
existing types; no SDK body was reconstructed or counted as game progress.

Final verification: GAME moves from 278/362 to **279/362 exact**;
`game.actor` is 24/29 exact. Of 484 native function-report rows across all
images (including thirteen vendored controls), only these two selectors
change; the other 482 rows retain their sizes and strict percentages. The
ground pair is also reported as validated by the semantic navigator.

Ruff, all 551 existing tests and `git diff --check` pass. Full `kf build`
recompiles the changed actor unit and regenerates all image reports, but
still exits nonzero on the existing data/ownership/placement gaps: source
data matches 11/59 units, both independent SDK data controls pass, and
target relinks verify 108/114 units. Those gates are not weakened. Only
GAME `0x8002deb4` is eligible for the new bank entry.

## Profile follow-up Function Match Plan

At `10bfa8a`, GAME `0x8002e0f0` remains 504 retail bytes and strict
94.555560%. The six semantic views, three call setups, source history and
the preceding campaign evidence were rechecked. Calls, table fields,
constants, far-first CFG and signed countdowns are unchanged. The byte
profile parameter remains an inferred contract: each caller uses `lbu`, but
O32 passes its zero-extended value in a complete register, and the callee
only consumes five profile bits. Retail retains that value in a halfword
local for the later `andi ...,0xffff` comparison with 9.

First test a `u16 profile_index` parameter, consistently in the definition,
shared header and curated signature, while retaining the existing local and
all body expressions. This is a conservative alternative to the unsupported
assumption that the caller's byte field determines the parameter width; all
three actual call values and all low-five-bit results are preserved. It
does not prove a unique original width or meaningful upper argument bits.
The exact caller must remain exact. Reject a control that merely moves the
redundant mask elsewhere without explaining retail.

Then test the independently observed weight lifetime if needed: retail loads
the signed profile weight into `s0` and writes the scaled chance back into
that same live value across `rand`. One evolving signed odds local can
represent both stages without a fake carrier or changed arithmetic. Do not
combine that test with the parameter-width experiment.

The member-base reuse remains a separate address-form question within the
already complete actor owner; do not replace it with an unrelated global,
out-of-bounds pointer or numeric address. Require focused rebuilding, raw
words and ordered referents, the exact caller and all-image regression
comparison, full build, lint and existing tests before committing or banking.

The halfword parameter removes the extra mask without adding another; the
three-site caller remains exact in its focused rebuild. Updating one signed
`odds` value from the selected weight through multiplication and scaling
then reproduces the whole weight-selection/calculation block, including
`mflo v0`, the two following nops and `sra s0,v0,8` in the `rand` delay slot.
Together these corrections reach strict 96.333336%. The first remaining
symptom is a profile/action saved-register exchange; the first independent
address-form discrepancy is the player-position base at +0xc4.

Next test an explicit authentic `const VECTOR *` view of
`actor_state.player_position`, assigned only after the first random gate and
used for both x/z differences. Retail explicitly forms and retains that
member address across the two angle calls, so this is a bounded source
pointer-lifetime control. The actor array still uses its normal typed member
access. Do not subtract across unrelated pointers or force the later -9216
instruction in source.

The explicit player pointer leaves the focused instruction/relocation stream
unchanged and is reverted. The kept source uses the halfword profile
parameter and one evolving signed `odds` value. The campaign's signature
record in `config/evidence/game_semantic_actor_ai.tsv` is synchronized with
the header and identity table; its old signed-byte action description is
also corrected to the already established unsigned-byte/255 contract.

Follow-up raw verdict: 126 words in both objects, strict **96.333336%**, not
exact and not banked. The first difference at +0x18 is `s3` versus retail
`s2` for the five-bit profile; the requested action uses the opposite saved
register. At +0xc4 retail retains the player-position member address in
`s0`, then derives the actor-array pointer from it. Reconstruction still
loads that member directly and later forms `actor_state+0x720` separately,
adding one HI16/LO16 pair (17 ordered relocations versus retail's 15).
All target identities and logical call paths agree, but this is not an
exact relocation/instruction stream. No compiler-mechanism attribution or
register-forcing source is claimed.

Follow-up verification: the exact caller stays at 100%, and all previously
exact functions are preserved. Ruff, all 551 existing tests and diff checks
pass. Full `kf build` regenerates the comparisons but still rejects the
pre-existing data/ownership/placement gaps (11/59 source-data units, 2/2 SDK
data controls, 108/114 target relinks). The simultaneous screen-image
closure is documented separately; this selector remains unbanked.

## Parameter-lifetime control at `258f6ad`

Function Match Plan: GAME `8002e0f0`, 504 bytes, strict 96.333336%, remains
under `probe-gcc257-o2-g0`. Refresh all six image-qualified views, the full
126 retail words/CFG, the three caller setups, neighboring facing selector
and following caller, both angle callees, BIOS rand attribution and source
history. There are 23 blocks, fifteen branches, four calls, one internal
jump and one return with its 40-byte restore delay slot. Five address pairs
and that internal jump are validated; no strings or unresolved indirect
transfers occur. All 26 exact actor functions and the three-site caller are
controls. The selector remains game-owned policy over the actor/effect pools.

The incoming profile is masked to five bits before its ten-byte table index
is formed; the unmasked value is never used again. Retail keeps the masked
profile in s2 and the byte action in s3, while canonical source exchanges
those roles. Test masking `profile_index` in place instead of introducing a
second halfword `profile` variable, then use that same parameter for the table
index and profile-nine guard. Preserve the existing u16 interface, both
narrowings, the far-first weight selection, evolving signed odds, chance and
facing gates, signed countdowns and return form. This is an input-lifetime
hypothesis, not permission to permute declarations or force registers.

Keep the distinct address question separate: retail retains player position
in s0 and derives actors by -9216, while canonical source forms the actor
array independently. The earlier explicit VECTOR pointer was byte-identical
and is not repeated. Compare the first raw divergence after the focused
compile; retain the parameter change only if it explains retail. Preserve
all sibling bytes and use strict objdiff/full verification before banking.

The in-place parameter mask emits the same instruction/relocation listing as
the starting source, including all remaining action/profile register operands
and the extra actor-array address pair. Revert the no-effect control. It does
not explain the residue or authorize another width change; the next source
investigation must address an independently evidenced difference.

### Actor-array pointer lifetime

Retail forms the player-position address at +0xc4 and keeps that register
through both angle calls and the optional second rand call. Its only later
use is deriving the actor-array cursor at +0x124. Canonical source does not
bind that cursor until after all these calls and the profile-nine guard.
Test assigning the existing `candidate = actor_state.actors` after the first
odds rejection and before the facing expression, leaving its first dereference
and both pool loops in their original positions. This is a genuine lifetime
test for the later cursor, distinct from the rejected player VECTOR pointer,
which had no post-call use. Keep the original profile local and every other
expression. No cross-member byte arithmetic or new global/type is introduced.

The early actor cursor is rejected. It keeps the actor-array address itself
in s0 across the calls, but still materializes the player-position load
independently; retail keeps the player-position address and later derives a
v1 cursor. The actor scan also changes its cursor and comparison registers.
Restore the original post-guard cursor assignment. Neither tested lifetime
form explains the shared-base residue, and the original strict 96.333336%
source is retained without a forced offset or register.

Final restored verification ran with the separately closed actor-distance
helper: fresh raw controls preserve every other actor body and ordered
relocation, and this selector remains 96.333336%. All 678 tests, Ruff and
diff checks pass; the full build retains the existing data/ownership/placement
failures. No profile-selector source or signature change is kept or banked.

## GCC 2.5.7 shared-exit closure

Function Match Plan at `695d1ae`: revisit GAME `8002e0f0 / 504` with the
current pinned compiler and unchanged O2/G0/R2000 profile. Refresh all six
semantic views, all three callers, angle callees, exact neighbors, shared
owners and source history. The actor unit now has 27 exact siblings. The
selector starts at 95.539680%, 508 bytes: the later enum-domain conversion
introduced a byte cast before assignment to the existing halfword local.
Retail requires a five-bit mask followed by a halfword use, without that
intervening byte narrowing. Decode directly to the existing promoted
`KF_ENUM_PARAM(KfEffectKind, u16)` type; preserve the signature and callers.

Then investigate the shared address and exit structure. CSE1 and CSE2 both
leave the actor cursor as an independent constant in the baseline, unlike
OPEN's address that is initially shared and later folded. Test the retail
rejection join, the observed cursor-before-count initialization, and finally
the common accepted return, one source fact per focused compile. No pointer
arithmetic, extra value carriers, register hints, profile changes or broad
source permutations are involved.

| Retained composition | Strict objdiff | Bytes | Differing resolved words |
| --- | ---: | ---: | ---: |
| Halfword enum conversion | 96.333336% | 504 | 32 |
| Shared rejected return | 98.055560% | 504 | 11 |
| Cursor before count initialization | 99.642860% | 504 | 9 |
| Shared accepted return | **100%** | 504 | **0** |

The rejected return makes the facing-success label single-use before CSE.
The uninstrumented debugger observes branch UID166 to label179 with status
`AROUND` in both passes. The former two-use success label was not followed.
`use_related_value` now returns `player_position_pseudo - 9216` for the actor
cursor; previously it returned no related value in either pass. Subsequent
path rescans can still return null, so the positive observation is checked
against the surviving RTL and final object. The player-position base remains
in s0 across three calls, and all five retail address pairs agree.

Only the s2/s3 exchange remains after correcting the two initializers.
The real allocation records explain the successful-return control:

| Value and source form | References | Calls crossed | Live length | Hard register |
| --- | ---: | ---: | ---: | --- |
| Action, separate accepted returns | 4 | 4 | 93 | s2 |
| Profile, separate accepted returns | 3 | 4 | 55 | s3 |
| Action, shared accepted return | 3 | 4 | 92 | s3 |
| Profile, shared accepted return | 3 | 4 | 55 | s2 |

`global.c:allocno_compare` orders these measured reference/lifetime values;
the shared result lowers action's priority below profile's. Final delay-slot
placement still emits the retail's separate byte masks. Source result uses
and final instruction occurrences therefore cannot be counted interchangeably.
This explains the current Decompals probe, without claiming historical GCC
attribution or a general register-allocation workaround.

Native, instrumented and uninstrumented-debugger compilations agree as whole
objects for the width, rejection and final controls. A reduced selector with
the same gate/scan shape independently acquires a related object-base
expression when its rejection exits are shared. Its retained base and
allocation differ from the real selector; it is an address-sharing control,
not an assertion that every shared exit produces the same registers.
Generated plans, all four trials, debugger observations and raw audits are
under `build/gcc257/game-actor-revisit/`.

Final verdict: all 126 words match literal retail after resolving curated
relocations, including all constants, branches, calls, loads and delay slots.
Every other actor function is unchanged; all 27 prior exact siblings remain
raw exact. The radial-damage helper was inspected as a related constant-sharing
case but is unchanged at 90.087300% and is not banked. The selector remains
custom game policy; its BIOS RNG calls are not counted as reconstructed code.

Focused recompilation and full `kf build` move GAME from **329/362 to
330/362 exact**, with OPEN 106/108 and PSX 1/1 unchanged. Ruff, all 710 tests
(nine skips), `nix flake check -L` and diff checks pass. The full build still
exits nonzero on the already documented data extent, ownership and section
placement failures; there are no function-report failures or lost exact
functions. Only GAME `8002e0f0` receives the new strict-100% bank entry.
