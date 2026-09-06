# GAME save/load hub: confirmation and exit-state flow

## Function Match Plan at `8c1ad49`

GAME `80024e64 menu_save_load_hub`, 608 bytes, belongs to the contiguous
four-function `game.menu_item_drop` unit. Its current strict score is
93.611840%; a fresh pinned `probe-gcc257-o2-g0` compile reproduces the
remaining dispatch, confirmation and return differences. The three unit
neighbors remain partial: drop 98.854780%, save 96.797874%, load 92.613640%.
Do not change those bodies or the shared save types in this campaign.

Hash-checked retail, all six semantic views, the complete hub body, its
caller and adjacent boundaries, source/history, the two-option dialog and
texture-loader bodies, and the sound/pad interfaces were inspected. The sole
caller at `224a4` supplies no arguments, retains v0 as a signed result, and
maps -1 to its own ongoing -99 sentinel. The hub is custom menu control, not
vendored code; PadRead remains the revisioned LIBETC PAD v1.17 entry and is
not reconstructed or counted here.

The frame is 10288 bytes, with the shared 640-byte save header at sp+16 and
the 9600-byte save payload at sp+656. Their addresses are published through
the two existing pointer globals; no overlapping storage is introduced.
The hub saves ra and s0..s6. It contains twenty direct calls, two validated
address pairs and seven reviewed internal jumps, no strings, indirect
transfers or outgoing candidates. Input masks are 1000/4000/20/40, movement
wraps through rows 0..2, cues are 0/0/1/2, and both PadRead sites pass 1.

Action 0 invokes the load panel and maps a zero result to -3. Action 1
invokes the two-option dialog `(2,3,cursor,NULL)`; a zero response loads
texture 3e6, stops map music, then enters a proven non-returning redraw loop.
No claim that this branch saves a slot is supported by its actual call set.
Normal cancellation resets the result to -99 only when a sub-action was
pending. A result other than -99 reaches the shared return at `25098`.

Two direct corrections are visible independently of the percentage:

1. `2503c` branches on exit row 2, but its delay slot `25040` sets confirm
   to 1 on **both** arms. Existing C sets it only on non-exit rows. Move the
   assignment before that test; the following ordinary and exit redraws
   consume it, so this is a real visible-state correction.
2. `24fa8` clears the pending action in the result-exit branch's delay slot,
   on both the return and continuing paths. Move the existing reset before
   that exit guard after verifying correction 1. Do not move the reset above
   the cancellation test that still consumes the old action.

The remaining two-arm dispatch difference is recorded separately. An older
switch spelling was already tried; do not repeat a byte-flat experiment or
name a compiler wall from the old source comments. Preserve complete calls,
numeric referents and delay slots, compare from the first divergence after
each focused compile, then run strict/raw/all-function and full-build checks.
Only a verified 100% result may be banked.

The confirmation correction puts `li s5,1` into the exit-row branch slot
and removes the unsupported nop. The shifted downstream addresses lower
the normalized text similarity, but the decoded visible-state fact is now
correct and is retained. Next apply the independently observed action reset
before the result guard, as already verified in the related option menu.

The reset now occupies the right branch slot. However, the in-loop `return`
still creates a separate jump carrying the result into v0. Retail branches
directly to the final `move v0,s1` followed by the shared epilogue. Express
that exit as a loop `break` and return the actual result once after the loop;
this preserves the return condition and avoids introducing a result carrier.

The post-loop return recovers retail's direct exit branch and final result
move. The only structural difference now is dispatch: retail tests both
case values before either body, then jumps to the no-action join. The old
switch experiment predates the corrected confirmation/reset/return paths;
it cannot establish the result for this changed CFG. Test one ordinary
`switch (action)` with cases 0 and 1 and a no-op default on this corrected
base. This directly expresses the observed case/default split; do not vary
case order, introduce synthetic cases, or search alternative switch shapes.

The corrected switch produces a focused **exact** match. Both forward case
branches and the no-action jump now match; the state reset occupies the exit
branch slot, confirmation is set on both row-test arms, and the result move
is in the final epilogue block. This closes a real source reconstruction gap,
not a proved code-generation limitation. Native objdiff and raw relinking
remain required before banking.

## Resume on newer master (`a199baf`)

The interruption was followed by independent naming/type work. The current
hub source again has the pre-correction control flow, now using named SDK
pad masks and menu cues. Its fresh focused diff reproduces the original
93.611840% source's dispatch/reset/highlight/return differences. Preserve
all newer names and reapply only the four individually verified corrections
above; do not overwrite the other dirty player/magic/type work. The retained
raw-word control must pass against a fresh build of this actual source before
claiming closure. Original trial results above describe the earlier base.

## Verified result on `a199baf`

The current named source reaches strict native objdiff **100.000000000%**
for all 608 bytes. A fresh pinned compilation and the delinked target both
relink to all 152 original retail words, including the twenty ordered direct
calls, the pointer publications to 800668e0/800668d8, and internal jumps.
Controls explicitly cover the 10288-byte frame, both state-update delay
slots, the final result move, and a deliberately wrong load-panel referent.

All 484 scored functions were compared against the pre-edit snapshot. Only
this hub changes (93.611840% to 100%); all other scores, including its three
partial unit neighbors, are unchanged. Eligible exact counts become
GAME 297/362, OPEN 98/108, and PSX 1/1: **396/471** overall.

Independent verification uses a temporary detached checkout of committed
`a199baf` plus only this campaign. Its full score vector is identical to
the working master result. This separates the banked inputs from the
unrelated, unstaged player/magic type work; no dirty-bank override is used.

Full builds in both trees complete the object comparisons but fail the
existing whole-project closure gates: source data 8/61, config SDK data
4/4, and target relink 110/116, with six conflicting-section-base failures
and no data artifact failures. Known-reference coverage remains incomplete.
This is one exact function, not closure of the unit or either linked image.

Verification: all 656 repository tests pass on the working master. The
isolated checkout passes 647 and skips nine oracle checks whose generated
runtime prerequisites are absent there; the hub's fresh raw-word test runs
and passes in both trees. Ruff and whitespace checks pass in both trees.
