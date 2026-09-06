# GAME magic-panel control flow

## Function Match Plan

Start from `44bee6c` on master with hash-verified Japanese retail and the
existing `probe-gcc257-o2-g0` probe. The selected-spell type campaign landed
during the initial read-only evidence pass; preserve its explicit enum
boundary. No source edits preceded this plan.

Target GAME `8002317c menu_magic_panel`, 1328 bytes, strict 95.897590%.
Its sole unit neighbor `800236ac menu_option_root`, 556 bytes, is strict 100%
and must remain unchanged. Preserve the unit's eight-row option switch table
at `RODATA(0x800122f0, 0x20)` and all existing data owners.

The six semantic views and fresh compile comparison are recorded under
`build/magic-panel-*.log`. Read the complete disassembly, caller, adjacent
boundaries, shared list/texture/confirmation helpers, typed magic/player
records and source history. The old source-shapes note is historical, not
proof of correct source flow or a compiler mechanism.

Retail has 20 proven calls, 14 validated internal jumps, 20 validated address
pairs, no local strings and no unresolved indirect transfers. The sole call
at `8002245c` consumes no arguments and compares the full-word result with -1.
Source/header already use `s32(void)`; correct the stale u32 inventory and
ledger, retaining candidate body confidence. The preceding viewer return owns
`80023178`; this function's return owns `800236a8`.

Frame size is 312: list at +24, ten ten-halfword labels at +64, sixteen byte
codes at +264, saves from +280, ra at +304. Retail builds learned records 0..3
using an exact-one learned-byte predicate, twenty-byte record/name strides,
and ten-halfword copies. MP cost/current MP and HP/MAGIC/status fields are
unsigned halfwords. Effect state owns the magic records; the player owns its
vitals and status fields. The game-specific learned-spell and casting policy
has no vendored roster match or SDK wrapper shape; PadRead remains a provider
boundary, and no library body is reconstructed.

Focused hypotheses, each followed by a rebuild and first-divergence comparison:

1. Index `magic_name_rows[code].codes[j]` directly, removing the duplicate
   advancing name pointer. Retail increments the derived name cursor before
   the code and its record offset in the back-edge slot (`80023238..248`).
2. Assign the pending sentinel after the confirmation call in a
   cancellation-first if/else (`800232e8` owns the `800232ec` assignment).
3. Set selected index before both upward-wrap viewport arms (`800233e0`),
   and restore scroll-first up/down arms from the decoded branch polarity.
4. Put the normal casting path under `selection != -1` and return selection
   after it. Cancellation targets the shared return move at `80023684`, while
   texture-load failure returns constant -1 directly through `80023688`.
   Preserve the separate insufficient-MP early return and its delay-slot move.

Keep the existing frame loop and reset location: one loop-head present at
`800232b0`, frame begin at `80023324` with confirmation reset in its slot,
then input. Do not transplant the shop's unconditional pre-exit reset. Keep
the empty-list guard, exact-one texture failure checks, unsigned MP comparison,
two MP-cost reads, four spell effects, status masks and post-effect HP clamp.
The long-list branch is present even though this panel admits only four rows.

Verify raw words, ordered calls/referents, all control targets and delay slots,
the exact neighbor, all-image scores, full build, existing tests, Ruff and
whitespace before commit. Only strict 100% may be banked. No tooling changes,
register permutations or artificial carriers are part of this campaign.

## Results

The panel is strict **100%**, up from 95.897590%, and is banked individually.
The neighboring option menu remains 100%. Both fresh compiles (the live
worktree and an isolated `44bee6c` checkout with only this campaign's build
inputs) reproduce all 332/139 complete retail instruction words.

The kept corrections follow the plan. Direct spell-name indexing first
removes the preheader and paired-cursor increment-order differences. The
cancellation-first if/else restores its post-call pending-sentinel slot;
the selected-index store precedes both upward-wrap viewport arms, and the
scroll-first branch arms restore retail layout. Last, guarding the casting
body with `selection != -1` and retaining a trailing `return selection`
recovers the shared normal-return move and its branch delay slot. No register
permutation, artificial carrier, assembler body or compiler change was needed.

The initial texture-failure branch still returns constant -1 through the
restore tail. The navigation texture-failure jump does the same. Normal
cancellation skips every spell-record access and enters `80023684`, while
the insufficient-MP branch keeps its `move v0,s1` delay slot and returns
without debiting MP or applying an effect. Sufficient MP is debited before
the four-way effect dispatch and the halfword HP clamp. These distinct paths
must not be collapsed merely because they end at one `jr ra`.

The magic body SHA256, including its return delay slot, is
`0177dc29f9880a650a169dc41c72ddec89d73df524a157c9b973b05662662312`.
The unchanged option body is
`2b9ecf559e8beda999ec0b0a0869f4c7bd7960d75d40ac876e56c769cd1e40d4`.
The target object was independently resolved and checked against retail,
then each freshly compiled candidate was resolved without masking differences.
Every one of the magic panel's 69 transfers matches its raw instruction,
numeric target and owned delay slot: 20 calls, 14 internal jumps, 34 branches
and one return. All twenty ordered address pairs and ordered call targets
agree. No relocation or data identity needed changing.

The corrected function extent also restores the following option menu's
switch-table addends. Its entire 32-byte RODATA now passes byte, referent,
addend and placement comparison. The unit is therefore two exact function
bodies plus matching claimed data, without asserting an original TU boundary.

All 484 score rows agree between the isolated and live worktree builds;
compared with the preceding shop campaign only this function changes. Eligible
exact counts rise from 399/471 to **400/471**: GAME 301/362, OPEN 98/108,
PSX 1/1. Source-data matches rise from 8/61 to **9/61** (GAME 7/41).
SDK/config-data matches remain 4/4 and target relink remains 110/116
(PSX 1/1, GAME 75/77, OPEN 34/38), with six existing section-base conflicts
and zero artifact failures. Full builds still fail on the remaining data,
placement and known-reference coverage requirements; whole-image closure
is not claimed.

Ruff and all 657 existing tests pass (80.848 seconds); no tests or tooling
were added. The spell literal ledger is refreshed for the new source layout,
correct cursor predicates and shared return. It also corrects the old claim
that this panel appends a none entry: it does not.

The concurrent learning-state campaign's enum/header/consumer changes are
preserved, including its working-tree `KF_MAGIC_LEARNED` comparison. Banking
uses the isolated source with the pre-existing literal-one learned predicate,
not those uncommitted inputs. Both forms reproduce the same full retail body.
Only the new magic-panel baseline row is updated; all other bank rows remain
untouched.
