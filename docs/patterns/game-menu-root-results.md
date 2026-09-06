# GAME main-menu result lifetime

## Function Match Plan

GAME `0x80022348 menu_root`, 704 bytes, `game.menu`, existing
`probe-gcc257-o2-g0`. At `45f70b3`, strict objdiff is 96.551140%; the
compiled body has 175 words versus retail's 176. The six semantic views,
raw object streams, switch bytes, only direct caller, neighboring entries,
panel interfaces and source history were inspected before editing.

The signature stays `s32 (void)`. `menu_enter_mode` calls at 0x80036eac
without arguments and retains the complete v0 result in s0 across cleanup
calls. Retail uses a 48-byte frame, saving ra and s0..s6; the current object
saves ra and s0..s5 in the same frame extent. Cursor/confirmation/input state
is word-sized. Startup draws three frames and waits for input release;
the eight-row cursor wraps through 0..7 with edge masks 0x1000, 0x4000,
0x20 and 0x40. Result begins at -99 and selection at -1. A real panel result
exits after input release; cancellation keeps the hub active.

All 28 direct calls are proven: frame-begin, stats-header draw, window draw
and presentation three times each; PadRead four times; input sound five
times; the seven panel handlers once each. Their call order and interfaces
already agree. The custom menu is absent from vendor/FID inventories;
PadRead retains the existing revisioned Sony LIBETC PAD interface/provider.
No SDK body is reconstructed or counted as game progress.

There is no direct global access or string literal. The sole address pair
selects this unit's `RODATA(0x800122c8, 0x1c)` switch table. The seven raw
words target function offsets +0x104, +0x114, +0x12c, +0x13c, +0x14c,
+0x15c and +0x178, in case order. The navigator retains candidate pointer
rows and reports `jr v0` at 0x80022444 as unresolved; the bounded index/load
chain and table bytes independently support these switch destinations.
Eleven internal absolute jumps and the address pair join the 28 calls in
the ordered text relocation contract. The neighboring `menu_save_confirm`
returns at 0x80022340 with its frame restoration at 0x80022344;
`menu_use_item_panel` starts at 0x80022608 and supplies case zero's result.

History `2c95649` introduced explicit `result = -99` after the void panel
calls in cases 2, 3 and 4. Retail's calls at 0x80022474/84/94 have nop delay
slots and leave s1 (result) alone; only v0 receives -99 for the following
comparison. Current C emits three extra result writes in those call slots.
Delete those three unsupported assignments as one result-lifetime
correction. Keep the case-0/5 shared cancellation join and case-1 semantics,
types, all other expressions and inventory inputs unchanged.

The missing retained -1 in s6 is a separate observed symptom, not a proven
compiler mechanism. Do not add a constant carrier, fake local, forced
register or padding to reproduce it. Compare from the first real divergence
after the focused build. Require strict canonical comparison, raw words,
ordered referents, all-image regression checks, full build and existing
lint/tests before committing; bank only if the function reaches 100%.

## Cancellation-condition control

Removing the three assignments makes their call slots nop as retail requires.
The first remaining body divergence is the missing retained -1 followed by
the exchanged cursor/result registers; the ordered call set remains intact.
Test the direct cancellation condition `result == -1` in the redraw guard.
The second operand is reached only with selection equal to -1, so this is
equivalent to the existing `result == selection` and makes the cancellation
meaning explicit without introducing a constant carrier. Retail compares
result to selection on precisely that constrained path. Keep this control
only if the raw comparison supports it; do not change other expressions.

The direct-literal condition produced the same focused instruction diff and
was reverted. The kept three-assignment removal improves canonical strict
objdiff from 96.551140% to 96.863640%; the compiled body remains 700 bytes
versus retail's 704. Cases 2/3/4 now have retail's nop call slots. Remaining
symptoms are the exchanged s1/s2 cursor/result roles, missing s6 save/restore
and retained -1, repeated -1 materialization, and their shifted branches.
No constant carrier or save-slot padding is justified. This remains partial
and is not eligible for banking.

The raw final comparison retains all 41 ordered relocation kinds/referents
and all 28 calls. Its neighboring functions retain their previous scores;
no banked function regresses. The independent indexed dialog-summary loop
closes one related renderer, but does not remove this function's residue.
The combined verification passes Ruff, all 591 existing tests and diff checks;
the full build retains the existing data/ownership/placement failures recorded
in [the dialog-summary audit](game-dialog-summary-loop.md). No gate was weakened.

## Selection reset and final-return plan at `13e081a`

Hash-identical retail, all six semantic views, the complete sole caller,
adjacent boundaries, raw seven-row switch, current fresh compile, original
history and the exact save/load hub source were refreshed. The baseline is
96.863640%, 700 probe bytes versus 704 retail; save confirmation is exact
and item use remains 99.482180%. The main-menu body is absent from the
vendor/FID inventories; the PadRead provider and its revisioned old-style
SDK declaration remain separate from this game-owned hub.

The signature stays s32(void): menu_enter_mode retains v0 across its cleanup
calls and forwards it. Keep all 28 direct calls, eleven internal jumps, the
single switch-table pair, and all seven table destinations. Its bounded
table dispatch is independently decoded; the viewer's unreachable flags
after jr v0 are not evidence that the panel arms are dead. All cursor/input
state is full-word, the frame is 48 bytes, and jr at 80022600 owns the stack
restore at 80022604. No strings or outgoing candidate relocations occur.

Retail's exit guard at 800224cc owns selection=-1 in its delay slot at
800224d0: the reset occurs on both guard outcomes. The source duplicates
that reset inside the returning arm and after the guard. First test one
unconditional reset before the guard. Then separately test breaking the
main loop after its existing input-release wait and returning the result
after the loop; retail's exit jump reaches the common epilogue. The related
exact save/load hub has this reset/guard/final-return structure. These are
bounded source hypotheses, not proof of unique source spelling. Preserve
the case-0/5 cancellation join, case-1 result policy, draw/input order and
all other declarations and constants; add no constant carrier or padding.

The reset-only trial moves that reset into several earlier panel call slots
and moves confirmation clearing into the exit guard's slot. It does not
recover the missing saved constant or the cursor/result register roles.
Before judging the composed structure, test the separately evidenced
post-loop return; the reset-only candidate is not an exact claim.

Adding the post-loop return emits the same reset-only instruction diff; it
does not repair those call slots. Reject the unconditional-reset experiment
on those concrete ordering differences, not merely its fuzzy score. Isolate
the final-return hypothesis against the original two reset sites next.

The isolated post-loop return restores the original focused instruction
diff, including the missing retained -1/save/restore and exchanged
cursor/result roles. It does not recover a retail instruction. Restore the
original source completely; neither source experiment is retained, and no
compiler limitation or new exact result is inferred from these controls.
The reset is already emitted in the correct guard delay slot by the original
duplicated form. This result limits the transfer of the save/load hub's
source correction: the same source shape is not automatically a correction
for every related menu.

A fresh restored compile matches every resolved instruction, direct call and
ordered address pair of the pre-trial canonical object for all three unit
functions. Save confirmation remains all 37 retail words exact; item use
retains its seven-word fruit-healing residue. Main-menu retail SHA-256 is
`655d2343f84b303d279e9dcdbe87a8c72a0abbf2ca66a9329367ccfb18e3658a`
(704 bytes); the restored resolved candidate is
`003389479a02f93d3db3978aebd471a0a36de3248cad94c4e7df2601b18c8f4e`
(700 bytes). The 28 numeric calls and switch-table base still agree, while
instruction positions, saved-register set and register operands remain
non-exact. No source, shared header, retail inventory or baseline change is
retained from this experiment.

The restored focused match and full rebuild retain GAME 302/362, OPEN 98/108
and PSX 1/1 exact functions. Ruff and whitespace checks pass. Repository-wide
verification is not green: the final 657-test run encounters three errors
for graphics fields renamed concurrently and one inventory-count failure
(2952 versus 2954); the full build retains data/ownership failures and now
reports GAME target relink 74/77 during those separate render/material edits.
The preceding clean-campaign 657-test run passed before commit `13e081a`.
Those concurrent source/config/test changes are preserved and excluded from
this evidence-only follow-up; no global clean-build claim is made.
