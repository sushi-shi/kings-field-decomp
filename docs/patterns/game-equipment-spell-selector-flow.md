# GAME equipment and active-spell selectors

## Function Match Plan

Campaign starts at `a7b4aa1` in the contiguous WIP `game.menu_select` unit.
Use its configured `probe-gcc257-o2-g0` profile, not a claim of historical
compiler attribution. Retail images were hash-validated with `kf init`.
Each function's address, block disassembly, incoming/outgoing references,
strings and current match were read, along with the option-menu caller,
adjacent return boundaries, source history and raw resolved object words.

| GAME function | Body | Initial strict score | Evidence and first hypothesis |
| --- | ---: | ---: | --- |
| `800238d8 menu_equip_select` | 1476 | 97.899730% | Full-width category arrives from `menu_option_root` at `800237a8`; no result consumed. Stock quantities and chosen IDs are bytes; names are ten halfwords. Retail has 25 calls, 32 conditional branches, 23 direct jumps, two table jumps and one return. Thirteen HI/LO pairs include stock, names, two tables and player-slot fields. Candidate is one word short; the stock address appears after the category switch instead of before it. Test direct stock indexing, retaining category bounds and shared object types. |
| `80023e9c menu_spell_select` | 1136 | 98.063380% | The sole direct caller at `800237b8` supplies no argument and consumes no result. Learned records accept exactly byte 1 for spell IDs 4..8; selected spell storage is a byte enum, while the accepted row index is signed with cancellation -1. Retail has 20 calls, 30 conditional branches, 11 direct jumps and one return. All three ordered referents and transfer sites/targets already agree. Candidate has 32 differing words, including name-cursor setup and paired-cursor increment order. Test direct `magic_name_rows[code].codes[j]` instead of an independently advanced name pointer. |

Both bodies are absent from the curated vendored list and local FID matches.
Their inventory filtering, Full Plate policy and player writeback are game
semantics, not SDK/runtime wrappers. SDK calls remain provider boundaries.
Neither function references local strings: the none-label glyph constants
`0x59, 0x104c, 0x4c, -1` are authored stack entries, followed by byte ID 255.

Keep the equipment panel's bottom-of-loop frame begin/render/present and its
unconditional confirmation reset before the exit guard. Initial and navigation
model failures branch directly to `80023e74`, bypassing normal model release.
Normal cancellation releases the model without equipment writeback. Full Plate
clears arm and leg slots through their existing shared equipment helper.

Keep the spell panel's initial render and loop-head present, with frame begin
and confirmation reset after its exit guard. Texture failure means exactly 1
and enters `800242e8`; cancellation reaches the same restore tail without
writing active magic. Confirmation stores the row index, then resolves its
spell ID only during writeback. The return delay slots at `80023e98` and
`80024308` belong to these bodies.

The unit owns `RODATA(80012310, 0x40)`: two eight-row equipment tables.
Their pointer rows and the caller's table were decoded directly; do not trust
the current CFG viewer's unreachable labels after unresolved `jr v0`.
Do not alter table ownership or curated identities to hide shifted addends.

Rebuild after each focused source correction. Compare referents, calls, CFG,
raw immediates and all delay slots before attributing remaining code generation.
Verify full-build status, all-image scores, existing tests, Ruff and whitespace.
Bank only strict 100% functions; preserve unrelated work and all banked rows.

## Focused observations

Direct spell-name indexing restores six words: the four-word preheader order
and the two stride-20 increments, including the loop back-edge delay slot.
The remaining 26 differences are register operands; all 62 transfer sites,
mnemonics and numeric destinations, 20 calls and three ordered referents agree.

Direct `item_stock[0][i]` alone leaves stock address formation inside the
equipment loop; it does not reproduce retail's pre-switch base acquisition.
The next hypothesis is an explicit player-stock bank view acquired after
the initial pad-release wait and before the category switch, then indexed
by the category-dependent item ID. This corresponds to `8002391c..920` and
`800239c8`; do not move the acquisition across the PadRead call.

## Results

`menu_equip_select` is strict **100%**. The explicit bank view reproduces all
369 words with no source changes outside list construction. Its raw body SHA256
is `571d7e61830f40d4ac7f82644080d12d1d7805a9be868715e15344d7bc02e245`.
All 83 transfers retain their numeric destinations and exact delay slots,
including both indirect jumps. All 25 ordered calls and thirteen address pairs
match. The unit's two eight-entry switch tables now match in full, including
extent, pointer addends and placement; source-data matches increase from 9/61
to 10/61 without any inventory or relocation curation.

`menu_spell_select` is strict **99.542250%**, not bankable. Its 284-word extent
matches, and the six restored words leave 26 register-operand differences.
No opcode, immediate, numeric transfer destination or ordered referent remains
different. Six delay slots still have differing register operands. The retail
body SHA256 is
`b985d74533b14f86b21a37888f3370645da8f86fd8cb1498fcfc99fc96aa46b5`.
Retail assigns selection to s2, the loop's constant one to s3 and input to s4;
the candidate uses s4, s2 and s3 respectively. This records the observable
residue, not its compiler cause; no declaration permutations were tried.

All 484 function scores were compared against the starting snapshot: only
these two rows improve. Eligible exact progress moves **400/471 -> 401/471**:
GAME **301/362 -> 302/362**, OPEN **98/108**, PSX **1/1**. Vendored controls
remain outside that denominator.

## Verification and banking

The root worktree and an isolated `a7b4aa1` checkout with only this campaign's
source changes produce identical scores for all 484 rows. The isolated fresh
compile independently reproduces all equipment words; the partial spell
comparison asserts that its remaining differing bits are GPR operand fields,
as a diagnostic only. `kf bank --function game:0x800238d8` was run in that
checkout after staging its sole changed source input. Only the equipment row
is banked; the spell row remains unbanked.

Both full builds retain the existing data/placement/coverage failures:
source-owned data matches are 10/61, SDK/config data is 4/4, and target relink
is 110/116 with six conflicting section bases. No artifact failure was found
in the root full build. The improved equipment tables are not among failures.
Ruff and `git diff --check` pass. The isolated existing suite completes
657 tests in 75.926 seconds, with nine skipped controls and no failure.

Concurrent notification-domain changes temporarily broke global manifest
parsing, then caused three layout errors and two stale notification assertions
in the first root test run. Those files were neither reverted nor banked here.
The focused compile used the already-audited selector/profile metadata until
normal manifest loading recovered; the normal full build and isolated input
verification followed before banking.

After the concurrent inventory/test updates, the root rerun passes all
657 tests in 82.659 seconds with no skips. Ruff and whitespace checks also
pass on the current shared tree. The notification changes remain outside
this campaign's commit.
