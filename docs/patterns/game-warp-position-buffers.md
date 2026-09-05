# GAME warp position buffers and argument handoff

## Function Match Plan

Start on master `23d1367`, GAME 285/362 strict exact. This campaign follows
the seven proven calls into `player_warp_shimmer` and its direct forwarding of
the input pointer to `effect_pool_construct`; it is not an address-only group.
Use the pinned `probe-gcc257-o2-g0`, without historical compiler attribution.
Retail hashes, all six semantic views of each caller, their raw bodies and
source history have been inspected. Preserve every banked function.

| GAME function | Size | Initial strict score | Retail frame / saved registers |
| --- | ---: | ---: | --- |
| `80014674 player_warp_shimmer_at_player` | 68 | 100% | 40; ra at32 |
| `80017cf8 player_warp_to_floor_entry` | 324 | 100% | 40; s0 at32, ra at36 |
| `80036850 player_warp_change_floor` | 348 | 99.770120% | 48; s0..s2 at32..40, ra at44 |
| `800369ac player_warp_same_floor` | 324 | 99.629630% | 56; s0..s3 at32..44, ra at48 |

All four put the position at sp+16, write x/z/y at16/24/20, pass that address
to the shimmer and leave the fourth word at28 unwritten. Every frame has a
full sixteen-byte extent before its first saved register. The wrapper forwards
its full mode word; the entry warp has no arguments. The dispatcher is the
only caller of the last two helpers: one shared change-floor site passes a
destination 1..5 and variant0/1; five same-floor sites pass variants1..3 and
explicit signed-coordinate arguments. Keep all values and their call order.

The change-floor helper has five calls and five validated address pairs.
It retains the signed floor for highest-floor comparison, publishes the floor
and variant as bytes, divides X/Z by2000, and reconstructs cell centres with
*2000+1000. Its only current raw difference is the order of the second argument
save and `a0=0` setup at+18/+1c. The same-floor helper has seven calls and
twelve validated address pairs. It removes old occupancy with -1, preserves
the previous variant byte, conditionally refreshes floor-five audio, sets
X/Z to cell*2000+1000, then synchronizes the player. Its three input-save moves
at+18/+1c/+20 have a different order; all remaining words/referents agree.
Both return with their original frame release in the jr delay slot.

The wrapper has one call and three address pairs; game_main_loop calls it with
modes1/2. The entry warp has six calls and thirteen address pairs; its no-argument
caller is player_use_item. The floor-entry table is x/z byte order, unlike the
player's z/x map-cell record. Neither exact control has strings or indirect
control flow. All four are absent from the vendored inventory and implement
player/map/effect policy, not SDK library bodies.

## Complete position-object evidence

The shimmer itself loads only three coordinates, but forwards the original
pointer unchanged at `800366d4`. Its callee copies **four** words:
`80036f90..80036f9c` load offsets0/4/8/12, then `80036fa0..80036fac` store them
at effect-record offsets0c/10/14/18. The supplied Release2.5 `LIBGTE.H:104..107`
declares `VECTOR` with `long vx,vy,vz,pad`, sixteen bytes in the pinned PS1 ABI.
The existing shimmer and effect-constructor interfaces already use this type.
Three-word arrays and `KfVec3i` callers therefore understate the actual input
object; a twelve-byte type cannot justify the fourth-word read.

First reconstruct all four local buffers as actual `VECTOR` objects, keeping
the x/z/y publication order, every call and the retail-uninitialized pad.
This is a complete-object/interface correction, not artificial stack padding.
Do not change the already-correct constructor copy, add a pad initializer, or
replace the shared API with a three-coordinate pointer. Build the partial
helpers first, then retain the two exact caller conversions only if their
raw words and scores remain exact. Compare the first real divergence before
considering a separate, evidence-backed argument-width hypothesis.

## Focused sequence

Actual VECTOR locals preserve both helper bodies exactly as compiled before;
the two entry-move residues remain. The exact wrapper and floor-entry caller
also keep their complete bodies. Retain the complete-object correction even
though it does not increase the score.

Next inspect the distinction between incoming value width and stored width.
Both helper bodies preserve the incoming variant in a full saved register
across calls and only consume it with `sb` into `player_state.map_variant`.
They never perform a signed variant comparison, shift or extension. Every
dispatcher call supplies a full O32 word; its values0..3 cannot determine the
historical signedness. The current u8/char parameter spellings conflate that
full-word handoff with the destination byte's width.

Test a shared unsigned-word variant input, narrowing only at the existing
typed byte assignment. This models the observed word handoff and low-byte
publication; unsigned spelling reflects the variant identifier, not recovered
historical signedness. First change only `player_warp_change_floor` and its
shared declaration, then compare all raw words before applying the same
hypothesis to the same-floor helper. Do not add a copy local or force saves.

The first word-argument correction closes `player_warp_change_floor` at strict
100%. All 87 raw words and fifteen ordered relocation rows are identical,
including the two previously exchanged entry words. No added instructions,
frame changes or source-order tricks are involved. Apply the same full-word
variant/byte-publication contract to `player_warp_same_floor`, whose original
three full-register input saves are currently reordered by the char prototype.

## Final verdicts

The same-floor word input also reaches strict 100%. All 81 raw words and
31 ordered relocation rows agree, including the three formerly reordered
entry moves. The destination remains the same unsigned byte; all signed
coordinate arithmetic, calls, constants, saved registers and delay slots are
unchanged. The constructor's full four-word copy remains untouched.

The four-function raw audit is complete:

| Function | Final strict score | Exact raw words | Ordered relocation rows |
| --- | ---: | ---: | ---: |
| `player_warp_shimmer_at_player` | 100% retained | 17 | 7 |
| `player_warp_to_floor_entry` | 100% retained | 81 | 32 |
| `player_warp_change_floor` | 99.770120% → 100% | 87 | 15 |
| `player_warp_same_floor` | 99.629630% → 100% | 81 | 31 |

The entry warp starts at source-module offset14c8 versus target14b0 because
an earlier, unchanged function in that unit is non-exact. Its 81 instruction
words are equal; all 32 relocation rows agree at function-relative offsets.
Do not compare enclosing-module offsets as if they were instruction bytes.

Canonical objdiff now reports all five `game.player_warp` functions exact:
2072 code bytes, plus equal 32-byte DATA and 20-byte RODATA contents. That does
not prove complete TU or linked-image closure: the unit still has the existing
source DATA/RODATA placement failures. Its name/boundary remain a curated WIP
module.

Only the two helper scores change in the full 484-row census. GAME advances
from 285/362 to 287/362, leaving 75 partial and no unstarted functions. OPEN
stays 97/108 and PSX 1/1; overall is 385/471. Every other function score,
including the complete-object controls, stays unchanged. No vendored body,
claim extent, data owner, relocation target or evidence tier is changed.

Focused `kf try` compiles, canonical GAME comparisons, and the full `kf build`
were run in the pinned environment. The first full build stopped scheduling
remaining GAME jobs after known PSX/OPEN gate failures, so GAME comparisons
were completed explicitly and the full build was rerun. Ruff and all 612
existing repository tests pass without skips; only the existing identity-test
row count changes to cover the two newly documented helper signatures.

The final full build retains source data 5/60, independent SDK data 4/4,
target relinking 110/116 and six conflicting section bases. There are zero
artifact failures and no banked-function regressions. Selectively banked only
GAME80036850 and GAME800369ac. The GAME-wide 100% goal remains open.
