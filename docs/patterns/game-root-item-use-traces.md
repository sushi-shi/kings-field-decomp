# GAME root and item-use compiler traces

## Function Match Plan

Continue from `9d79d24` with the unchanged `probe-gcc257-o2-g0` profile and
hash-verified GAME.EXE. This campaign follows the proven call from `menu_root`
to `menu_use_item_panel`, with the contiguous `menu_save_confirm` as an exact
control. Ownership remains the existing WIP `game.menu` module.

| Function | GAME address | Retail bytes | Starting strict objdiff |
| --- | --- | --- | --- |
| `menu_save_confirm` | `800222b4` | 148 | 100% |
| `menu_root` | `80022348` | 704 | 96.863640% |
| `menu_use_item_panel` | `80022608` | 1908 | 99.482180% |

The six semantic views for each function, complete retail CFG/disassembly,
native and traced objects, and raw baseline audit are under
`build/gcc257/item-use/`. Native and traced whole ELF files agree. Source
history and the earlier root-result and item-use-flow dossiers were reviewed;
their rejected experiments remain rejected.

The root's sole caller, `menu_enter_mode` at `80036eac`, retains the entire
return value across cleanup. Its seven switch entries at `800122c8` resolve
to the independently decoded dispatch blocks; the navigator still reports
the table jump as unresolved. There are 28 direct calls, 11 internal jumps,
one address pair and no strings. Retail saves s0 through s6 and ra in 48
bytes; the current 700-byte candidate saves only s0 through s5 and ra. It
uses the result value as the magic-cancellation comparison operand after
setting that result to -1; retail compares against a separately retained -1.
The first hypothesis consumes the actual magic-panel return through the
existing numeric result channel, then maps cancellation to pending and a
successful cast to no item. The decoded protocol and signed return width
remain unchanged. Trace result references, constant lifetime, call delay
slots and the complete switch table; reject a partial register improvement.

The item panel's sole caller is the root at `8002244c`. All 477 words except
seven inside the fruit effect already agree, including all transfers and
delay slots. Retail has 24 calls, 16 internal jumps, 26 address pairs and no
strings or indirect transfers. The current candidate has one extra HP
address pair. The shared `KfPlayerState` and four-u16 `KfPlayerVitals` layouts
identify the HP, MP and status accesses; no new owner or relocation is
justified. The return delay slot is `80022d78`, followed by `menu_map_viewer`.

Baseline CSE1 uses the fruit's first status-clear address (`player_state+42`)
as the root for HP and MP references. CSE2 replaces those derived references
with absolute addresses. The other recovery arms begin with HP and retain
that address for their read/modify/write. Test the same HP-first source order
for the fruit's independent HP addition and status clear, keeping MP refill
last. Preserve all u16 stores before the two unsigned clamps. This changes
neither effects nor object types and adds no pointer carrier. Compare the
scheduled status-clear position against retail, not just the new address
form.

These are game inventory and menu policies with game-owned data, rather than
vendored library bodies. PadRead remains the documented LIBETC PAD.OBJ v1.17
boundary; no SDK function body is reconstructed by this campaign.

Run each hypothesis separately against the whole unit, starting comparison
at the first real divergence. Retain only humane source that preserves the
exact control. For a strict 100% result, rebuild the affected unit, audit raw
words and ordered referents with native/traced parity and a wrong-referent
control, run Ruff, repository tests, whitespace checks and the full build,
then refresh eligible exact bank rows from staged campaign inputs.

## Exact source and observed compiler states

Both isolated hypotheses reach strict **100%**, and both remain exact when
combined in the production unit. The independent raw audit verifies every
word of each selected body while preserving the other two functions' words,
calls and address targets. There is no rejected intermediate source in this
campaign.

The root now assigns `menu_magic_panel()` to its existing `s32 result`, tests
that result for cancellation, and maps it to pending or no item. The
allocation trace changes as follows; reference and lifetime counts are those
reported by this pinned probe, not historical compiler facts.

| Value | Baseline refs / calls / length / register | Exact refs / calls / length / register |
| --- | --- | --- |
| result, p75 | 25 / 25 / 141 / s2 | 27 / 24 / 139 / s1 |
| cursor, p71 | 27 / 27 / 144 / s1 | 27 / 27 / 144 / s2 |
| shared -1, p80 | no retained allocation | 7 / 21 / 232 / s6 |

The exact loop snapshot shares p80 among the redraw, magic-return and
item/system-return comparisons. Its definition moves before the outer loop.
The global-allocation snapshot has eliminated the temporary move from v0 to
result for the magic call, while retaining the increased source reference
count. The final object restores the missing s6 save/restore and places the
result's -1 assignment in the magic call's delay slot, exactly as retail.
The saved-register frame is still 48 bytes; the body grows from 700 to the
retail 704 bytes. Selection and input retain their original s3/s5 roles.

The fruit source now performs HP addition, status clear, then MP refill.
Baseline CSE2 commits at UIDs 1155 and 1159 replace
`p327 - 24` with absolute `player_state + 18` HP addresses under the
`cse.fold-address` context. Its status-root p327 has zero references after
allocation. With HP first, p327 instead denotes `player_state + 18`; the HP
load/store use that base through CSE2 and allocation gives it v0, with three
references, no crossed calls and live length 10. Status and MP references
become their correct absolute forms. The first scheduler moves the independent
status-clear store (UID 1159) before the HP address setup; the second scheduler
retains that order. Thus the final status-clear position agrees with retail
even though it is second in source.

This is the same observed CSE relative-to-absolute transition and allocation
reporting already checked by `member_folded`/`member_retained` and
`crossing_absent`/`crossing_present` in
`tests/fixtures/gcc257_trace_controls.c`, with scheduling-dependency controls
in `tests/gcc257_trace_smoke.py`. The full-game isolated comparisons establish
that the two ordinary source changes recover the retail stream; they do not
prove original variable spellings or the historical GCC identity.

## Raw verification

`verification.log` and `verification/audit.json` record fresh native, debug
and two traced compilations of the combined unit. All four whole ELF files
are byte-identical, and both complete trace files agree byte-for-byte.
Reconstruction and curated target objects both resolve to every raw retail
instruction, without masking immediate fields or relocation addends:

| Function | Exact bytes / words | Ordered calls | Address pairs |
| --- | --- | --- | --- |
| `menu_save_confirm` | 148 / 37 | 7 | 2 |
| `menu_root` | 704 / 176 | 28 | 1 |
| `menu_use_item_panel` | 1908 / 477 | 24 | 26 |

The three bodies total 690 words, 59 calls and 29 address pairs. The new two
matches account for 653 words / 2612 bytes. Wrong magic-panel and list-widget
call targets independently fail raw equality. The save-confirm control's
words, calls and data targets remain unchanged. All seven reconstructed
switch pointers also resolve to the raw retail targets, in order:
`8002244c`, `8002245c`, `80022474`, `80022484`, `80022494`, `800224a4`,
`800224c0`. The corrected root size therefore also closes this unit's RODATA
comparison, raising GAME data-owning units that match from 9/42 to 10/42.

## Repository verification and verdicts

`focused-match.log` records an actual compile of `game.menu`, followed by
strict reports with all three functions at 100%. `full-build.log` records
the required full build. It still exits nonzero for existing source-data
ownership/placement and known-reference closure gaps: source data matches
are PSX 0/1, GAME 10/42 and OPEN 3/19; target relink remains PSX 1/1,
GAME 75/77 and OPEN 34/38. There are no artifact failures, lost banked
functions or unchanged-input regressions.

Ruff and `git diff --check` pass. `repository-tests.log` records **703 tests
in 85.640 seconds, OK, nine skips**. There are no tooling, profile, SDK or
flake changes, and no additional repository tests are needed for these two
small source edits; raw verification uses the existing relocation helpers.

Final function verdicts are **exact** for the root and item-use panel, and
**unchanged exact control** for save confirmation. Eligible progress is now
**431/471**: GAME **324/362**, OPEN **106/108**, PSX **1/1**. This leaves
38 GAME and two OPEN functions non-exact; the full matching goal remains
open. Generated sources, objects, traces and reports remain under `build/`.

After staging the campaign inputs, `kf bank --unit game.menu` banks all three
strictly exact functions, including the refreshed save-confirm control. Their
shared input hash is
`b4d148e56aedd0ef6d90548b63fd09256a7de07801b815094c6a79d48a041bc0`.
No unrelated source, identity, owner proposal or OPEN experiment is banked.
