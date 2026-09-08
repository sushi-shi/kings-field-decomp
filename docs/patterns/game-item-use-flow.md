# GAME item-use panel control flow

Subsequent strict closure: the
[root/item compiler-trace campaign](game-root-item-use-traces.md) reaches
**100%** by placing the fruit's HP addition before its independent status
clear in source. Compiler scheduling restores the retail store order, and
all 477 words now agree. The earlier seven-word residue below is resolved.

## Function Match Plan

Start at `a665d54` on master with hash-verified Japanese retail and the existing
`probe-gcc257-o2-g0` profile. Review GAME `0x80022608`,
`menu_use_item_panel`, 1908 bytes, in `game.menu`; starting strict objdiff is
92.821800%. The other claims in this contiguous WIP module are controls:
`menu_save_confirm` (`800222b4`, 148 bytes, 100%) and `menu_root`
(`80022348`, 704 bytes, 96.863640%). Do not change their source.

The six-view dossier, complete retail disassembly and focused baseline are
under `build/item-use-*.log`. The sole direct caller is `menu_root` at
`8002244c`: no arguments, full-word selected item or negative return sentinel.
The source already uses `s32`; the inventory's `u32` return is stale. The
preceding root return owns its `80022604` stack-restoration delay slot; this
panel ends after its own `80022d78` slot, before `menu_map_viewer` at
`80022d7c`. The model loader/releaser, map viewer, list initializer and shared
list confirmation sources constrain their existing argument and return forms.

This inventory/menu policy is absent from the vendored roster and references
game-owned item names, quantities and player fields. It is not an SDK wrapper,
libc-shaped routine, startup body or GTE family. PadRead remains a vendored
LIBETC/PAD.OBJ boundary with the existing v1.17 archive/retail-marker evidence.
History `2c95649` introduced this reconstruction; its claim that the CFG was
already correct needs re-evaluation against the raw branches below.

Retail has 24 direct call sites, 16 validated internal MIPS26 jumps and 26
validated HI16/LO16 pairs. There are no unresolved indirect transfers or local
strings. Data references select player stock, four ten-halfword item-name
rows, and u16 HP/MP/status fields. Preserve the 1208-byte frame and existing
40-byte list context, 50 ten-halfword labels and two 56-byte arrays. List
geometry, quantities and item codes are bytes. Preserve the unsigned
`(selection - 42) < 6` test, stock decrement, HP additions 25/10/80/150/300,
status masks 0xb/3/0 and u16 stores before unsigned clamps.

First restore each independently decoded semantic fact, with a focused build
and first-divergence comparison after each edit:

1. `8002296c..80022990`: an empty list skips navigation. Nonzero raw input
   plays cue 0 and sets selection to -1; zero input goes directly to redraw.
2. `80022940` owns `move s4,zero` at `80022944`: clear confirmation on both
   the continuing and exiting paths, including cancelled prompts.
3. `80022a10` owns the selected-index store at `80022a14`: upward wrap sets
   `entry_count - 1` before either the short- or long-list viewport branch.

Then review source shape against the remaining direct evidence: scroll-first
inner navigation arms, confirmation/input initialization order, code-counter
initialization before name pointers, and separate first/post-input presentation
sites. Track actual return paths rather than just return-instruction counts:
both model-load failures bypass ordinary release/effects and reach the shared
restore tail at `80022d58`; normal exit waits for input release first and calls
the releaser at `80022bd0` before optional item effects. Do not add artificial
locals, register permutations or compiler attribution to explain a residue.

Before handoff, rebuild the affected unit and full project, compare all strict
scores, audit raw constants/calls/relocations/delay slots, run existing lint,
tests and whitespace checks, and preserve unrelated dirty files. Bank only a
verified 100% result from clean campaign inputs.

## Results

The empty-list guard restores the raw `entry_count` load, branch, cue-0 call
and -1 selection store. Strict objdiff improves from 92.821800% to
94.737946%; both neighboring controls are unchanged. The next focused edit
moves `confirm = 0` before the exit guard and recovers exactly the missing
`move s4,zero` delay-slot instruction, replacing the candidate's nop.
That reset raises strict objdiff to 94.886795%. Upward wrap then recovers
the common selected-index store and both viewport arms. Scroll-first inner
branches restore their retail ordering. Confirmation-before-input declaration
initialization restores the prologue; separate first/post-input presentation
restores the second static call and loop target. Assigning the selected result
after the confirmation call is the next directly supported lifetime control.

The explicit name-pointer initializer still appears before the invariant map
exclusions and stock iterator, whereas retail places the name-row base after
them. The original source used separately named interior rows before the full
`item_name_rows` owner was recovered. Test ordinary `item_name_rows[code]`
indexing with the existing ten-halfword inner copy, eliminating the manual
parallel name cursor. The row stride and code range are already independently
established by the name-table/resource audit; this is not a declaration or
register permutation.

Direct row indexing reproduces the entire list-construction prefix, including
the stock/name/destination iterators and all four ordered name-table referents.
The counter-before-pointer experiment alone had preserved the exchanged
iterators and misplaced base setup; the explicit cursor is not retained.

Result assignment after the prompt was necessary but an accepted-first
if/else emitted an extra jump around the cancelled arm. The cancellation-first
form, also present in the related drop panel, recovers `li s2,-99` in the
comparison's delay slot and the direct fallthrough after loading the item.
Together these source changes yield strict **99.482180%**, up from
**92.821800%**. The source stays ordinary C with no new register carriers,
macros, data owners, relocation inputs or compiler-profile changes.

## Raw comparison and remaining residue

A fresh pinned compilation has the same 477-word / 1908-byte extent as
retail. Relinking the curated target reproduces every raw retail word; body
SHA256 is `32e900fa0791ba49445385c38c2b02c249e670d70cdad5524d9aa869f3711f7d`.
The candidate differs in exactly seven words, all inside the fruit effect:

| Relative offset | Retail word | Candidate word |
| --- | --- | --- |
| `+0x6c4` | `24420792` | `94420792` |
| `+0x6c8` | `94430000` | `3c03800a` |
| `+0x6cc` | `3c04800a` | `94630794` |
| `+0x6d0` | `94840794` | `2442012c` |
| `+0x6d4` | `2463012c` | `3c01800a` |
| `+0x6d8` | `a4430000` | `a4220792` |
| `+0x6e0` | `a4240796` | `a4230796` |

Retail derives `&player_state.vitals.current_hp` in v0, reads/writes through
it and carries maximum MP in a0. The candidate instead uses separate HP-load
and HP-store pairs, carries the HP value in v0 and maximum MP in v1. Both
retain the +300 addition, halfword storage before clamps, and the correct
HP/maximum-MP/current-MP identities. The extra HP-store pair makes 27
candidate pairs versus 26 retail pairs; it is not a missing target relocation.
No inventory adjustment is justified by this difference. Its code-generation
cause remains unattributed; do not add a pointer carrier solely to force v0.

All 89 transfer instructions agree, including all 24 ordered direct calls,
16 internal jumps, 48 conditional branches and the shared return. Their
numeric targets and complete delay-slot words agree as well. Thus both
model-load failures still bypass the ordinary releaser/effects; normal exit
still releases input and the model first. This complete raw comparison, not
the return count or fuzzy score, verifies the restored path structure.

The neighboring `menu_save_confirm` still reproduces all 37 words exactly.
`menu_root` remains the existing 175-word candidate versus 176 retail words,
strict 96.863640%; its source is unchanged. The panel's inventory return is
corrected to `s32` to match its source, negative sentinel and direct caller;
its partial body is not promoted to exact and is not banked.

## Final verification

The full 484-row strict score snapshot differs from `a665d54` only for this
panel. Eligible exact counts remain **399/471**: GAME 300/362, OPEN 98/108
and PSX 1/1. No previously exact function regresses. The generated README
records the fuzzy improvement without changing the exact count or baseline.

Focused compilation and full `kf build` complete with the existing closure
failures: source data 8/61, SDK/config data 4/4, and target relink 110/116
(PSX 1/1, OPEN 34/38, GAME 75/77). The six section-base conflicts, incomplete
known-reference ownership and zero data-artifact failures are unchanged.
`game.menu` retains its existing root-switch RODATA addend difference.

Ruff and whitespace checks pass. The first 657-test run caught the stale
unsigned signature in the menu-list evidence ledger after the inventory
correction; synchronizing that row resolves it without changing any test.
The final existing suite passes all **657 tests** in 85.378 seconds. No new
tests, tooling, SDK or flake changes are included. Only this campaign's seven
files/hunks are staged; unrelated player/magic/effect/menu changes remain out
of the commit, including the other dirty function-identity row.
