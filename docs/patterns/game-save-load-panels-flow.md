# GAME save/load panels: exit state and row dispatch

## Function Match Plan at `c041095`

GAME `8002552c menu_load_panel`, 880 bytes through its return delay slot at
`25898`, is the final member of the contiguous `game.menu_item_drop` unit.
Its strict score starts at 92.613640%; a fresh pinned `probe-gcc257-o2-g0`
compile reproduces the old source's 112-byte frame, rematerialized constants,
exit-result move and reversed empty-slot branch. The retail frame is 120
bytes, saving ra/s0..s6. The newly exact hub and the drop/save neighbors must
not regress. No shared type, compiler profile or data owner change is planned.

Hash-checked retail, all six semantic views, the complete body, sole hub
caller, adjacent save/config boundaries, original source/history, summary
layout and save/dialog interfaces were inspected. The caller passes no
arguments, retains the signed result and maps zero to -3. The panel is
game-owned card/menu policy, not a library body; the revisioned LIBETC
PadRead interface and SDK masks remain external. The vendored ledger does
not attribute the panel or its game-specific save services to an SDK.

Retail contains 42 direct calls, nine validated internal jumps and no
HI16/LO16 address pairs, strings, indirect transfers or outgoing candidates.
Three 24-byte KfSaveSlotSummary records occupy sp+16..87. The occupancy read
is their word current_hp at offset 8, not a byte flag. Window arguments are
5/4/cursor/confirm; cursor wraps 0..3. The three-frame load-status animation
uses texture 0x67, then calls save_system_read_slot(cursor+1), whose signed
halfword argument is proved by its callee. Catalogue/slot success is 1;
dialog cancellation is -1 and continuing state is -99. PadRead always takes
1; edge masks are 1000/4000/20/40 and cue meanings remain 0/1/2.

The catalogue-error path waits for press/release and returns -1 directly.
The main path returns the dialog result, not the slot-read worker status:
retail saves the dialog result in s2, compares the worker's v0 with 1 and
discards that status. A read error or dialog cancellation resets s2 to -99.
Exit-row confirm and cancel assign -1; an empty slot plays the error cue
without setting confirm. Both return paths share the restore block at 25874.

Three bounded source corrections are directly supported:

1. The result guard at 25740 clears confirm in its delay slot at 25744,
   on both paths. Move the existing reset before the guard, preserving its
   position after the sub-action body that still consumes confirm.
2. Retail's guard branches to the final result move at 25870. Use a loop
   break followed by the ordinary result return, as established in the
   exact caller, retaining the catalogue-error early return.
3. The occupancy test at 257f0 branches to the populated-slot confirmation;
   its empty-slot error block comes first. Express the zero-HP arm before
   the populated arm, without changing the exit-row precedence or read.

Compile after each correction and compare the first real divergence as well
as the affected edges and slots. Do not add locals to hold constants, change
types to steer allocation, or treat the old constant-hoisting explanation
as proved. Preserve raw constants, all calls and numeric relocation targets;
use native objdiff and a full raw-word comparison before any exact claim.

The reset correction emits the required clear in the guard's delay slot,
but the in-loop return adds a jump carrying the result into v0 and reverses
the continuing guard. The first divergence remains the frame/save set.
Keep the evidenced reset and apply the planned post-loop return next.

The post-loop return recovers the entire 120-byte frame, ra/s0..s6 saves,
the naturally hoisted 1/3 constants, initial dispatch and final result move.
The sole focused difference is now the inverted occupancy branch and swapped
empty/populated blocks. No constant carrier or compiler change was needed.
Apply the planned zero-HP-first source arm to recover that last CFG fact.

The final arm correction reaches strict native objdiff 100% for the load
panel. The hub stays exact and the untouched save/drop scores are unchanged.
Full raw-word and campaign-wide verification remain required before banking.

## Save-panel Function Match Plan

GAME `800250c4 menu_save_panel`, 1128 bytes through the return delay slot
at 25528, starts at strict 96.797874% in the same source unit. Its six
semantic views, complete body, sole save-confirm caller, neighboring hub/load
boundaries, original source/history, and additional cleanup/write/format
interfaces were inspected. The caller at 22330 supplies no arguments and
ignores the result. The summary layout, SDK interfaces and game-ownership
negative control are the same as above; memset remains the SDK entry.

Retail saves ra/s0..s5 in a 120-byte frame, with the same 72-byte summaries
at sp+16. It has 58 direct calls, ten validated internal jumps, no address
pairs, indirect transfers, strings or outgoing candidates. Catalogue states
1/3 enter the menu. Window arguments are 4/5/cursor/confirm, cursor wraps
0..4, row 3 formats and row 4 exits. Textures 72/68/69 and three-frame
animations are retained. The format path passes 1 and clears exactly 72
summary bytes; data rows call save_system_write_slot(cursor+1). Status and
dialog result are distinct values, and the function returns the latter.

The first divergence is the current 112-byte frame and missing saved s5;
the current source also omits the branch at 25308. Three corrections are
supported independently, to be compiled separately:

1. Clear confirm before the result guard, matching the 2540c delay slot.
2. Break to the post-loop return at 25504, retaining the separate catalogue
   failure return that jumps directly to the restore block at 25508.
3. Guard formatting with `cursor == 3`, not an unconditional else after the
   signed `cursor < 3` test. Retail explicitly skips formatting for other
   values and carries the existing status to the check at 25380. Do not
   discard this branch merely because normal cursor movement is bounded.

Preserve all status assignments, call arguments, input precedence, and the
newly exact hub/load bodies. No fake status carrier, padding, declaration
permutation or compiler-profile change is justified by the old register
count explanation. Compare full raw control flow after each focused build.

As in the load panel, moving the reset alone fills the observed branch slot
but leaves an inverted continuation and separate result-carrying jump. The
frame/save-set divergence persists. Retain the observed reset and test the
planned break/post-loop return without touching the format branch yet.

The post-loop return recovers the direct result-exit branch and final move,
but the missing format-row guard and shortened status lifetime remain.
Apply the independently decoded `cursor == 3` guard next; unlike a forced
status local, this restores an actual retail path that preserves old status.

The explicit format guard recovers the saved-status lifetime, full frame and
remaining instructions. Both panels now reach strict native objdiff 100%:
save 96.797874% to 100%, load 92.613640% to 100%. The unchanged hub stays
exact and the drop panel stays at 98.854780%. The old register/hoisting
limitations were not established; no compiler settings or steering locals
were changed.

## Final verification

Fresh pinned C and delinked target objects both relink to every retail word:
282 save-panel words and 220 load-panel words, including all 100 ordered
calls and nineteen internal jumps. There are no address pairs to mask.
The raw control covers both early returns, final result moves, confirmation
reset slots, the format-row guard and empty-slot branch; corrupting a state
update or a resolved catalogue-call target is rejected. The exact hub's
independent 152-word check also passes.

Across all 484 scored bodies, only these two panels improve. GAME becomes
299/362 exact, OPEN stays 98/108, and PSX stays 1/1: **398/471** eligible
functions exact. The unit is 3/4 exact; the drop panel remains partial.

The isolated checkout of committed `c041095` plus only this campaign has
exactly the same complete score vector as working master, excluding unrelated
unstaged player/magic edits from banking. Bank the two new exact rows and
refresh the verified unchanged hub for the updated unit inputs; do not bank
the partial drop panel or use a dirty override.

Focused matches, raw controls, Ruff and whitespace checks pass. Working
master passes all 657 tests; isolated verification passes 648 and skips nine
oracle checks whose generated runtime prerequisites are absent there. The
hub and both panel raw controls execute and pass in both checkouts.

Full builds still fail the same unrelated closure gates: source data 8/61,
SDK config data 4/4 and target relink 110/116, with six conflicting section
bases, zero data artifact failures and incomplete known-reference coverage.
No complete linked-image or translation-unit closure is claimed.
