# GAME list-confirmation input flow

## Function Match Plan

GAME `0x80028380 menu_list_interact`, 852 bytes / 213 words, is the sole
claim in `game.menu_list_interact`, using `probe-gcc257-o2-g0`. Starting
strict objdiff is 87.244130%; the candidate has 206 words. The six semantic
views, complete raw disassembly, all seven caller windows, adjacent
boundaries, shared types, render interfaces and source history precede edits.
The earlier [protocol dossier](game-menu-confirmation-protocol.md) establishes
the glyph sequences and the two complete 24-byte `MenuGlyphString` objects.

Retail saves ra and s0..s8 in a 104-byte frame. Labels start at sp+16 and
sp+40; saves start at sp+64. The first four arguments are the list pointer,
label kind, preview mode and full-word item ID. Shop ID and price mode are
stack words at entry+16/+20, loaded from frame+120/+124. Callers pass kind
0/1/3/4/5 and preview 0/1/2; their item IDs are byte loads promoted to words.
All seven callers compare the full returned result against -1, keeping their
own selected row or item separately. Keep the shared typed interface intact.

Caller sites are `0x8002179c`, `0x80021d20`, `0x80022918`, `0x800232dc`,
`0x80023af0`, `0x80024060` and `0x80024bbc`. The exact dialog-frame helper
ends with its restore delay slot at `0x8002837c`; the two-option prompt
starts at `0x800286d4`. No overlapping function extent is indicated.

All 21 direct calls are proven and all 14 internal MIPS26 jumps are
validated. No address pairs, switch tables, strings, indirect transfers or
candidate references occur in this body. Label words are inline authored
glyph values, not strings requiring separate data owners. The menu-specific
protocol and caller family exclude a vendored body; the called PadRead
remains separately attributed to the SDK PAD provider.

The first real source discrepancy is input-state lifetime. Retail retains
the current button word in s3, initialized to zero. Each input iteration
copies it to s0 and publishes the new PadRead result to s3 at
`0x800285fc/0x80028600`, before all edge tests. The source instead retains
`prev_pad` between frames and copies the current word at the loop tail.
First initialize `pad` and move `prev_pad = pad` before the next read, keeping
all masks and priority unchanged. No artificial local or declaration search.

Retail then handles movement, confirm and cancel in that order; the current
negative-predicate nesting emits them in the reverse order. Test a positive
movement-edge predicate followed by confirm/cancel else-if arms. Retail's
selection toggle uses explicit zero/one branch arms rather than the current
single XOR instruction. Test that source branch separately if needed.

Keep -99 pending, 0 accepted and -1 cancelled, the first and final redraws,
initial/final release waits, preview 2's item!=255 guard and labels untouched.
Do not assume matching call counts prove the complete CFG. Rebuild after each
focused change and compare raw instructions and ordered relocations. Full
build, all-image scores, lint/tests and whitespace checks precede commit;
bank only strict 100% without unrelated dirty inputs.

## Focused source controls

The current-pad lifetime restores the copy-before-publication sequence, but
alone leaves reversed handler layout and changes register roles. Positive
edge predicates recover movement/confirm/cancel order; the explicit zero/one
toggle recovers its branch and delay-slot stores. Together they improve
strict objdiff to 96.690140%, without changing the 104-byte frame or labels.

Retail's remaining loop-tail discrepancy is independently visible at
`0x8002868c..0x800286a0`: the post-input path checks preview 0/1 then joins
partway into the existing preview body. Current source jumps to the whole
preview preheader. As in the inspected configuration panel, test an initial
draw before the modal loop and the same ordinary draw after its input arms.
Preserve the exceptional final redraw and all dynamic call sequences. This
source structure lets common continuations merge; do not force extra calls,
split the interface or introduce a register carrier.

The initial-draw/post-input-draw structure recovers the preview tail and all
register roles naturally. The focused textual instruction/relocation stream
is identical and native objdiff reports **100%** for all 852 bytes. The
profile, interfaces, labels and initial/final waits are unchanged. The old
source-history description of a register/control-flow limitation did not
establish a compiler cause; source lifetime and loop structure suffice here.

## Final verification

Two independent fresh pinned compilations, one in the working tree and one
in a clean checkout of `7fa14ec` with only this campaign's source and identity
changes, reproduce every one of the 213 raw retail words. The delinked target
also relinks to those same words. Raw body SHA256 is
`4d587553e85758306806757df9b8f0db237c3ac7d13113e1615c266e5abac2a5`.

All 21 ordered call targets, 14 internal jump targets, all conditional
branches and their complete delay-slot instructions agree. There are no
HI16/LO16 address pairs in either object. The current-pad snapshot at
relative +0x27c/+0x280 is `02608021 00409821`; the six-word post-input
preview tail at +0x30c is `1220ff94 34020001 1222ff99 02402021 0800a144
34020002`. All other raw constants, label stores, stack argument loads and
the single restore/return are covered by complete word equality, not masks.

The native strict result is **87.244130% -> 100%**. Both all-image score
rosters have 484 rows and agree exactly; only this function changes from the
campaign-start snapshot. Eligible exact counts increase from 398/471 to
**399/471**: GAME 300/362, OPEN 98/108 and PSX 1/1. The single-function unit
is text-exact. Its identity now records the verified C match; no other
function or data inventory is promoted.

Focused recompilation and full `kf build` were run in the working tree and
the isolated checkout. Both retain the pre-existing closure failures:
source data 8/61 units, SDK data 4/4 contributions, and target relink
110/116 units (PSX 1/1, OPEN 34/38, GAME 75/77), with six conflicting section
bases and no data-artifact failures. Whole-image closure remains incomplete.
The working-tree run passes all 657 existing tests and Ruff; whitespace
checks pass. No tooling/profile/SDK changes or new runtime tests are included.

`kf bank --function game:0x80028380` banks exactly one function from the
isolated checkout with its two campaign inputs staged and no dirty-input
override. Only that generated baseline row is transferred back to master.
Unrelated player, magic, effect, menu and identity edits in the working tree
are preserved and excluded from the commit.
