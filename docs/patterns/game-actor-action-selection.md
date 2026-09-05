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
