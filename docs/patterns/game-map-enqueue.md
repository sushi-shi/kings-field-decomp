# GAME map polygon dispatch and allocation exits

## Function Match Plan (2026-09-06)

GAME `8001de18` / `0x418` (1048 bytes), `void render_enqueue_map(u16)`,
starts at strict 17.954199% on `0674985`, under the unchanged
`probe-gcc257-o2-g0` profile. Hash validation, all six semantic
views, full retail body/CFG, sole map-cell caller, adjacent model/sprite
boundaries, projection/accessor callees, shared types, histories and SDK
provider evidence were refreshed. The caller masks the object selection
to a byte; the callee masks its O32 argument to a halfword. Primitive counts
and packet headers are words; prepared vertex/normal offsets are unsigned
halfwords. Projected XY is a packed word and depth/fog are signed halfwords.

Retail has an 80-byte frame, sixteen ordered direct calls, six validated
address pairs and one internal jump, with no strings or candidate outgoing
references. Each control transfer includes its delay slot. LIBGTE SMP
NormalClip/NormalColorCol/DpqColor and LIBGPU PRIM packet/AddPrim functions
have separate vendored attribution; map packet policy is game-owned.
Keep the 40/52-byte SDK GT3/GT4 packets, RGB code publication, triangle
checked division by three then shift two, quad shift four, depth bias 200,
signed upper bound 16384, mask 3fff and header stride mask 3fc.

The earliest instruction difference remains asset-base setup. Retail
reloads current_asset after projection (`8001de7c`), while the source reuses
a pre-call payload snapshot. The projection body writes projected entries,
not the selected asset; nevertheless the two source references should
preserve the observed call boundary. The broader GAME graphics owner is
still unresolved; no cross-object arithmetic or aggregate migration is
part of this focused campaign.

The decisive semantic error is later: both allocation-overflow branches
(`8001df30`, `8001e0b8`) go directly to the epilogue at `8001e200`.
The C currently skips emission and continues to the next packet. First
correct both missing returns, keeping the existing cursor increment before
the check. An overflow must not parse, allocate or render another polygon.

Next compare separately the post-projection asset reload/offset expression,
then the decoded dispatch: mode 24 branches to the later triangle arm;
mode 2c falls through to the quad arm; other modes skip to packet advance.
A two-case switch is a candidate source structure, not proof of original
syntax. Retail forms quad vertex 3 before SetPolyGT4, while source currently
does it afterward. Reconstruct these facts one at a time, preserving call
arguments and packet fields. Recheck the word postdecrement's entry and
back-edge and shared AddPrim tail after the dispatch correction.

Earlier source-shape notes attributed this low score to compiler walls.
The missing allocation exits and reversed arm layout are concrete source
discrepancies; those historical attributions do not explain this body.
Keep all other unit functions and every banked match unchanged. Rebuild
the affected unit for each source hypothesis, compare raw words/relocations
and strict scores, then run full build, lint and tests before committing.
Do not bank any partial result.

## Focused reconstruction

The two missing returns change strict objdiff to 17.973282%. Both overflow
edges now reach the function epilogue rather than the packet-advance block;
the rest of the unit is unchanged. The next isolated asset-expression change
reaches 19.404581% and restores the post-projection load plus offset-before-
base addition. The first remaining difference is the saved asset-address
register, followed by the separately owned projected-buffer address.

The two-case switch restores the triangle branch over the quad and the
sixteen-call order, reaching 94.248090%. This large movement follows a
decoded CFG correction, not register steering. The quad's fourth vertex
must also be formed before the allocation-overflow branch, including on
its return path; move its existing expression alongside cursor acquisition.

The entry count test currently branches directly on the saved count and
decrements only on fallthrough. Retail first copies the count into a
temporary, tests that old value and decrements in the delay slot, exactly
as its back-edge does. Test one `while (remaining-- != 0)` rather than the
separate zero guard/decrement/do-loop. Unsigned wrap on the final test is
unobserved; packet traversal counts remain identical. Then scope the existing
projected-base pointer to the loop, matching its retail initialization only
after the entry guard; do not change its still-separate data identity.

The fourth-vertex lifetime reaches 95.049620%; the postdecrement loop reaches
96.175575%; loop-local projected storage reaches 97.015270%. These changes
recover the quad allocation-delay-slot load/value, both count-test forms and
the guarded projection-base initialization, respectively. Current source
still merges the quad's depth guard into the triangle's guard, although
retail checks each depth separately and shares only the final AddPrim setup.
Each mode computes its own signed depth, so scope the existing depth local
inside each case. Keep both guarded AddPrim calls and their actual pointers;
do not create a common result carrier or a forced duplicate call.

## Final verdict

Case-local depths restore retail's separate quad/triangle upper-bound tests
and shared AddPrim tail, reaching strict **98.770996%**. The source is
1056 bytes versus 1048 retail. Both have sixteen calls in the same order,
thirteen conditional branches (including checked division), one internal
jump, one return and the same 80-byte frame/restoration slots. The two
allocation failures now reach that epilogue, with the original advanced
cursor left intact. Clipping and depth rejection still advance the packet.

The source has eight HI16/LO16 pairs versus retail's six. All six original
numeric targets agree in order; the two additional pairs reach the correct
projected buffer `800911b0` and ordering-table field `80090ebc`. Retail
derives them from the selected-asset field: `80090fc8 + 488`, then the
projected base minus 756. These observations support further GAME ownership
work, not an alias between the currently separate source objects.
The first raw difference is the selected-asset address register at +38;
the count/projected-base saved registers are exchanged and normal/header
spills use 24/32 instead of retail's 32/24. No fake local, register directive,
padding, assembly or compiler-profile change is retained.

Only this function changes among all 484 native comparison rows. The three
other unit functions and all banked functions preserve their strict scores;
GAME remains 288/362 exact, OPEN 97/108, with no new bank entry. GAME's
aggregate fuzzy measure moves from 93.91% to 94.45%. OPEN sources remain
unchanged. This is a corrected and substantially improved partial function,
not exact closure or compiler attribution.

The focused regression control checks both decoded buffer-overflow guards
and their complete restore/return tail in retail and the rebuilt object.
Redirecting either guard to packet advancement fails its negative control.
It is intentionally a narrow control, not general CFG-equivalence tooling.

Verification: focused recompiles and the final recorded match retain
98.770996%; Ruff, all 635 tests (75.249s, no skips) and `git diff --check`
pass. Full `kf build -j4` exits 1 on unchanged data/ownership/placement
gates: source data 5/60, config-backed SDK data 4/4, target relink 110/116,
six conflicting section bases and zero artifact failures. No partial
baseline is updated or banked.
