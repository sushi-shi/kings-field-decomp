# GAME save/message presentation matching

The [status return-contract follow-up](game-status-result-traces.md) closes
GAME `8002c510` at strict 100%. The inferred explicit success return was not
supported by its five callers, all of which discard the result. The retained
C returns -1 only on loader failure and otherwise falls through. The dialogue
path remains open at 98.780490%; the earlier trials below are historical.

## Function Match Plan

Continue the GCC 2.5.7 instrumentation campaign with the two remaining
functions in the shared `game.save_system` unit. All 22 exact neighbors are
controls. Retail hashes, the six image-qualified semantic views, complete
disassemblies, all constraining call sites, source history and the existing
save-status/source-layout dossiers were reviewed before editing. These are
game message-selection and asset-path routines; the SDK file/GPU providers
remain external vendor boundaries.

| GAME function | Retail / current bytes | Strict baseline | Evidence snapshot |
| --- | --- | --- | --- |
| `8002c510 memory_card_show_status_message` | 208 / 212 | 97.980770% | Signed-halfword input and selected/default message narrowing; 24-byte frame, saved ra; one direct call, one table address pair, twelve validated internal jumps. Fifteen table-pointer rows remain candidates and indirect reachability is incomplete. Five callers pass status words or literal 12 and ignore the result. The body maps loader result 1 to -1 and forwards other results. |
| `8002c9d4 talk_show_dialogue_page` | 164 / 164 | 98.780490% | Four map-event call sites pass floor/stage/character/page bytes; signed word division by 10 establishes the character arithmetic, while byte stores define path encoding. One direct call, seven validated address pairs, signed-divide trap branches, 24-byte frame and saved ra. No candidate outgoing references. |

For the status message, the first mismatch is the extra `move a0,v0` after
`menu_load_message_image`, at `8002c5c0`. Retail keeps the loader's actual
result in v0 and conditionally replaces it with -1 before the shared return.
Test updating that result before one return, and the equivalent conditional
expression, instead of two source return statements. Preserve all status
cases, the unlisted/default halfword path, call target and both narrowing
pairs. Trace the real result's expansion and allocation; no extra carrier is
introduced.

For the dialogue path, the first mismatch is `mfhi v1` versus retail's v0 at
`8002ca04`; the directory pointer occupies v0 instead of a0. Retail forms
`talk_image_path_template+6`, writes the directory/filename digits in order,
then derives the full path in the call delay slot by subtracting six. Test
expressing that final call through the existing directory pointer. Keep the
same complete 19-byte string owner, signed arithmetic, seven referents,
store order, byte parameters and word character-ID domain. Inspect whether
the pointer's call-argument preference changes before considering any other
source hypothesis.

Strict objdiff 100%, complete relocation-resolved retail words and preserved
exact siblings are required before retaining either correction. A partial
trial only supplies evidence for the next step.

## First observations

Updating the status result gives 96.538460%; the conditional expression gives
91.730770%. The original remains 97.980770%. Before allocation its return
value has already been copied back to v0 ahead of the comparison, leaving
the named result simultaneously live for the branch. Test consuming the
actual call assignment directly in that comparison, preserving both return
values and the same statement-level data dependency.

The explicit directory-based call is byte-identical. The first scheduler
places `a0 = directory_character - 6` before the remaining directory stores,
so the pointer and outgoing argument overlap during allocation. Test direct
accesses to the known owning string, without the separately declared interior
pointer. These accesses existed in the original reconstruction and preserve
the same offsets and store order. Trace whether the root and call argument
remain simultaneously live; no scheduling attribution to Sony follows.

The assignment-in-test form is identical, and direct owning-array accesses
fall to 84.829270%. Next test the status function with the exceptional `==1`
return first, followed by forwarding the loader result. Retail's `bne` skips
the -1 assignment; this orientation can preserve the source result through
the comparison without moving its return-register assignment ahead of it.

For dialogue, the four observed character inputs are bytes and every use of
the quotient/remainder ends in an encoded path byte. Test byte numeric
components and, separately, actual ASCII-byte components initialized with
`'0'` already included. Preserve signed word division and all field-write
ordering. This tests the width and identity of real path components; it
does not introduce a register carrier or change the character-ID API.

The exceptional return first gives 93.076920%. Numeric byte components are
byte-identical; ASCII-byte components give 94.512190%. Neither source change
is retained. Next, express the loader-result mapping as a switch with a
forwarding default, and separately test updating the existing directory
pointer back to the full pathname before the image call. These preserve the
observed result mapping and retail's final subtraction from the directory
address; inspect the branch return copy and pointer lifetime after scheduling.

The frame feature previously omitted the first transfer's delay slot and
mistook an unseen prologue for a zero-byte frame. The reader now includes the
slot and reports an unavailable frame when a branch precedes allocation.
For these functions that recovers the status routine's saved ra, while the
dialogue routine's frame remains manually established by disassembly. Two
unavailable observations do not establish agreement.

The result switches reproduce the earlier 93.076920%/96.538460% return trials.
Rewinding the directory pointer is byte-identical at 98.780490%. Final verdict
for this batch: both functions remain non-exact, with no retained source
change or new bank entry. Their frame observations now distinguish measured
entry saves from the manually decoded prologue behind division guards.

## Verification checkpoint

The four new frame-reader controls pass, including the actual first-branch
save pattern and the pre-prologue division guard. Fresh native/traced GAME
trials confirm the status frame is 24 bytes with ra at 16, while dialogue
frame agreement is unavailable. Both examined units were rebuilt; full
`kf build` retains GAME 317/362 and OPEN 106/108 strict exact functions, with
all thirteen vendor controls exact and no stale or missing score reports.
The build still exits nonzero on the existing data ownership, placement and
reference-closure gaps. No partial function was banked.

Ruff, `git diff --check`, all 703 local tests (nine optional skips), and
`nix flake check -L` pass. Nix's test suite has 144 local-artifact skips and
its compiler parity/trace checks also pass. Completing the remaining 45 GAME
and two OPEN functions at strict 100% remains the campaign's success criterion.

## Follow-up at `81fa559`

A fresh six-view/caller/target audit rechecked both functions. Explicitly
updating the real signed `ones` value with `'0'` immediately before its two
byte stores leaves every linked word unchanged: CSE still removes the named
digit copies and retains the divmod remainder in v1. The return joins,
conditional return, cursor rewind and direct path-member trials reconfirm the
rejected source families above; they do not constitute new owner coverage.
No source spelling is retained from this follow-up.

All 24 current unit bodies were freshly compiled and numerically relinked:
1665 source words versus 1664 retail words, with all 22 exact siblings preserved.
Native/traced whole ELFs agree per source. Wrong-callee controls change exactly
one call, and a one-byte path shift changes seven address instructions while
preserving its call. The direct-member trial recovers v0 for the remainder but
retains template+10 instead of +6 as its shared root and changes the middle
address-pair order; the complete comparison rejects it.

Current evidence is under `build/gcc257/save-ui/`, including complete raw
comparisons and allocation snapshots. Both strict scores remain unchanged.
Ruff, whitespace checks and all 703 tests pass (135.803 seconds, nine skips).
Full build retains the existing data/ownership/placement failures and zero
artifact failures. GAME remains 324/362 and OPEN 106/108 exact; forty functions
remain open, with no new bank entry from these trials.

## Signed-word digit-expression control

Function Match Plan: refresh GAME 8002c9d4/164-byte identity, disassembly,
all four caller windows, callee, strings and strict state after hash-validating
the configured retail files. Current baseline is 98.780490%; six blocks, three
division-guard branches, one return, one image-display call and seven validated
path address pairs agree. Retail reads the remainder into v0; current source
uses v1 and retains the directory pointer in v0 rather than a0. This remains
game path formatting, with the existing image-display helper as the boundary.

Test a three-state JSON digit-expression axis: existing signed numeric word
locals; signed ASCII word locals initialized with quotient/remainder plus
48; and signed quotient/remainder expressions at the two chained stores.
The latter removes the numeric locals without changing the character-ID API,
store ordering, byte destinations, pointer expression or signed division.
The ASCII-word form is distinct from the earlier narrowed ASCII-byte trial.

All three states remain 98.780490%. Independent recompilation proves the
complete function raw listings, including relocation annotations, identical.
Thus neither moving the signed digit arithmetic to its stores nor representing
those actual digits as ASCII words explains the retail remainder/pointer
allocation. Keep source unchanged and leave the function open. No result banked.

Results: `build/hypotheses/20260908-183427-game-save_system-talk_show_dialogue_page`;
manifest: `build/dialogue-digit-hypotheses.json`; independent listings:
`build/dialogue-digit-objects`. Fresh baseline comparison is
`build/dialogue-current-compare.txt`; retail references and callee/caller
windows are in `build/dialogue-evidence.txt` and `build/dialogue-callers.txt`.
