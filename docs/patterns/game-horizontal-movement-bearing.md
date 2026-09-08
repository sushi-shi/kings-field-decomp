# GAME horizontal movement: bearing sign and short-vector storage

## Function Match Plan

At `cd679e4`, GAME `0x800171fc player_move_horizontal` is a 2088-byte
retail function in `game.player_core`, strict 93.538315%. The manifest selects
`probe-gcc257-o2-g0`; historical compiler attribution remains open. All six
image-qualified semantic views, the full retail body/CFG, source history,
shared player/collision layouts, callers, helpers and neighbors were reviewed.

Retail has seven direct calls, 58 conditional branches, nine internal jumps,
100 blocks and one shared return. The return delay slot restores its 88-byte
frame. The baseline has the same ordered successors but a 2112-byte body and
80-byte frame. Equal graph structure does not establish equal computed values.
The signed-halfword retry counter starts at one and reaches minus one after
the second collision. Both the exhausted-retry and normal exits return one.

The sole caller is `player_update`: retail sites 80018de4, 80018e0c and
80018e4c implement four source arms; both strafe arms share the last call.
Heading comes from a sign-extended camera halfword or a full-word masked
calculation. The mover preserves it full-word in s7 without re-extension.
Keep the existing source/header `s32` signature and reconcile the stale
identity and campaign-evidence `s16 heading` entries; source-type confidence
remains supported.

Game-specific collision grids, camera state and hit-response policy exclude
a vendored body. Called rsin/rcos remain Sony LIBGTE GEO archive matches in
`functions_vendored.tsv`, backed by the exact Psy-Q Release 2.5 section.
The separately reconstructed angle helpers retain their full-word arguments.
Adjacent distance/view-bob functions are exact and remain unchanged.

First correct the evidenced Z subtraction, then test the independently
supported short-vector layout. Preserve constants, widths, owners, ordered
calls and the retry/diagonal CFG. Do not use padding, forced registers or
dead stores to reproduce the frame.

## Bearing sign

Retail loads hit X at 800172d8/dc, camera X at 800172e0, camera Z at
800172e4 and hit Z at 800172e8/ec. At 800172f0 it subtracts camera X from
hit X. At 800172f8, word `00a32823`, it instead computes camera Z minus
hit Z. Both are narrowed to signed halfwords for the 8001730c bearing call;
the Z sign extension occupies that call's delay slot.

The old source used hit-minus-camera for both axes. The corrected Z input
changes the bearing helper's sign-dependent quadrant choice: this is a
semantic correction, not a register-allocation difference. Its isolated
strict score drops, which does not invalidate the decoded subtraction.

## SDK short-vector layout

The four ordered retail stack-halfword stores are `a7a40018`, `a7a5001c`,
`a7a2001c`, `a7a20018`. X/Z occupy sp+24/sp+28, first as bearing inputs,
then as the cosine/sine-derived slide displacement. Z is reloaded with
signed `lh` after the second sine call.

Release 2.5 `LIBGTE.H` defines `SVECTOR` as short vx/vy/vz/pad: X/Z at
offsets 0/4 in eight bytes. Using the authentic SDK type instead of two
scalar locals reproduces all four stores, the signed reload, the 88-byte
frame, all eleven frame/save prologue words and old-cell spills at sp+32/40.
Only vx/vz are used; no artificial initialization or padding is introduced.
This supports the source layout, not the original local typedef's spelling.

## Focused controls

| Source | Strict % | Candidate bytes | Verdict |
| --- | ---: | ---: | --- |
| Original scalars, reversed Z input | 93.538315 | 2112 | Incorrect Z semantics. |
| Corrected Z only | 92.839080 | 2112 | Independently supported correction. |
| Corrected Z with SVECTOR | 96.568960 | 2128 | Retain: recovers the evidenced local layout/stores. |
| Add a starting-cell pointer | 96.329500 | 2128 | Restores the later cell reload but changes index association/order; does not close that access region. Not retained. |
| Explicit angle updates, then shared mask | 96.344826 | 2132 | Adds an internal jump/block: 101 versus 100. Rejected. |

The retained body still has 43 HI16/LO16 materializations versus retail's
33. Its ordered reference sequence and seven call targets are unchanged
from baseline. Extra materializations concern camera Z (five), camera X
(two), map-cell X/Z (one each) and floor height (one). No relocation/data
inventory was changed to accommodate the source.

The first remaining raw difference is +0x2c: heading enters s7 in retail,
s6 in the candidate. The first paired control difference is +0xd4, using
t4 versus t3 in the no-collision comparison with a moved destination.
All 100 ordered successor lists still agree. Other residue includes an
extra angle-mask result move, player-field base reuse, the starting-cell
reload and final-coordinate register roles. These remain unattributed;
neither a compiler wall nor semantic equivalence is established.

## Verification

Retail body SHA-256:
`ca5a66bc66c74066417dfb6cf82067b279aaa3201087363de3974c5ce2523319`.

A fresh compile of the retained source reproduces the successful trial.
All fourteen delinked targets independently relink to raw retail. The
thirteen unchanged exact siblings reproduce all 923 retail words; every
function preserves its ordered calls/references versus baseline. The mover's
raw Z subtraction, four ordered halfword stores and frame saves are checked
directly. It remains partial and is not banked.

Canonical `kf match` rebuilt the unit and records 96.568960%. Across all
484 recorded function results, only this score changed; GAME remains
313/362 exact and OPEN 98/108 exact, with all thirteen vendor controls exact.
The initial suite caught the stale campaign-evidence signature; reconciling
it with the audited identity fixes that consistency failure without changing
any tests. The final 680 tests pass (94.682 seconds), as do Ruff and
`git diff --check`.

Full `kf build` remains non-green on the existing data/ownership/placement
gates: data PSX 0/1, GAME 9/42, OPEN 2/19; target relinks 1/1, 75/77,
34/38; six conflicting section bases and zero artifact failures. No new
exact function is claimed or banked. Concurrent OPEN naming and unrelated
documentation changes are excluded from this campaign.

## Typed owner pointer controls

A nine-state JSON campaign tested `player_move_horizontal` (GAME
`0x800171fc`, retail 2088 bytes), keeping the established signature, field
widths, operations and declaration order. Controls introduced pointers to
its camera `VECTOR`, two-byte `KfMapCell`, floor-height word, their combinations,
and the complete player object. These are source-shape hypotheses, not new
ownership claims.

| View | Strict score | Bytes | HI/LO pairs |
| --- | ---: | ---: | ---: |
| Existing direct fields | 96.568960 | 2128 | 43 |
| Camera | 92.360150 | 2080 | 29 |
| Map cell | 97.113030 | 2116 | 39 |
| Camera and cell | 92.927200 | 2076 | 28 |
| Floor height | 95.143680 | 2124 | 44 |
| Camera and floor height | 92.390810 | 2080 | 29 |
| Cell and floor height | 95.250960 | 2072 | 29 |
| All three | 92.927200 | 2076 | 28 |
| Complete player | 91.086205 | 2024 | 15 |

Independent compilation and relocation resolution confirmed the same seven
ordered direct calls and unchanged sibling function outputs in all states.
Retail has 33 address pairs. The map-cell control first changes the existing
entry address sequence at `+0x30`: it replaces the retained cell-Z pointer
with a direct byte load and loses the floor-height anchor derived at offset
`-22`. Its higher score does not establish a better reconstruction. The
other views also remain non-exact. No variant was retained or banked; the
existing first raw difference at `+0x2c` remains an unattributed residue.

The narrower diagonal-block lifetime was also tested independently. Scoped
player, camera and cell pointers scored 95.076630%, 95.890810% and 95.335250%
with 2116, 2116 and 2120 bytes, respectively; address-pair counts were 40,
40 and 41. None reproduced retail's cell-X anchor with camera accesses at
`-31` and `-39`: camera Z still materialized separately, while the camera
view only reused its own base for X. All three preserve the seven ordered
calls and thirteen sibling outputs, but retain the first raw mismatch at
`+0x2c`; their first new raw change is the initial cell-byte destination at
`+0x38`. No scoped view is retained. Together these controls rule out these
simple pointer spellings under the probe, not the underlying shared owner.

## Starting-cell read and recentering boundaries

Function Match Plan at `2ae04f0`: refresh hashes and all six GAME views for
`800171fc`/2088 bytes/96.568960%. Read all 522 retail words, 49 outgoing
rows (seven proven calls, 33 validated data pairs and nine internal jumps),
three caller windows, adjacent functions, callee source, shared player and
collision layouts, history and prior pointer controls. The 88-byte frame,
SDK SVECTOR stores, signed-halfword retry counter, 58 conditional branches,
word heading/distance and common return remain fixed. The routine is custom
movement policy; rsin/rcos retain exact Release 2.5 LIBGTE GEO attribution.

Retail reads the starting cell for the diagonal-range guard, then reads it
again after both coordinate remainders at `8001763c`. Canonical carries its
byte snapshot and emits an `andi` there. First compare canonical with an
explicit indexed reread after the remainders and with a direct grid-range
predicate followed by that reread. Both new states are identical: 2148 bytes /
92.111115%, 46 data pairs, first difference +0 (80-byte frame). They recover
a second load but reconstruct the cell address with seven arithmetic words;
retail retains that address. They also add a separate floor-height pair at
the collision call. Reject both; recovering one read does not recover the
complete address and frame evidence.

Next test inline recentering, either the camera-coordinate operation alone
or that operation plus the following map-cell update. For each scope use
global fields, the actual SDK VECTOR, or the complete KfPlayerState pointer.
Preserve the signed remainders/half corrections, case predicates and store
order. All seven states including canonical are identical in all 532 linked
words, seven calls and 43 ordered data pairs. None explains the first +0x2c
heading-register difference or the extra player-field materializations.

A four-state follow-up isolates the retained starting-cell address: canonical,
read-only cursor, cursor with explicit reread, and cursor passed into the
recentering helper that reads it. All three cursor states are identical at
2128 bytes / 96.176250%, with the same 43 pairs. Only four words differ from
canonical, at +0x394..+0x3a0: the saved-cell load moves and index/base additions
associate differently. The reread still disappears. No result is retained.

The grid has real mutable writers in map_object_pool and the resource loader.
A five-state control therefore compares canonical, those three cursor forms
with mutable pointee type, and direct cursor reads in the initial predicate
and first recentering dispatch. Mutable/read-only pairs are completely
identical; no qualifier mechanism is established. The direct-read form reaches
96.329500%, reproducing the earlier pointer control: its second lbu replaces
the canonical andi, but it retains the changed index association, 2128-byte
body and 43 pairs. It is not a new closure and is rejected again. All remaining
arithmetic, calls, owners and source are unchanged; nothing is banked.

All nineteen JSON states compile. Separate disposable recompilation proves
all thirteen siblings equal their complete raw retail bodies and ordered
references in every state; every delinked target also reproduces raw retail.
The canonical source SHA256 remains
`04a98ff5e573d89d3d5f19057b4dd6b75f6735ffe30a8320a849e7a54178cd05`.
Generated dossiers, manifests, objects and verification logs use the
`horizontal-{type-read,recenter-inline,cell-cursor,mutable-cursor}` prefixes
under `build/`. The four JSON result directory names begin with
`20260908-220928`, `20260908-221116`, `20260908-221313` and
`20260908-221503` followed by `-game-player_core-player_move_horizontal`.

Full `kf build` retains GAME 337/362 exact / 99.428% aggregate, with
zero artifact failures. Data matches remain GAME 11/41, OPEN 3/19 and
PSX 0/1; target relinks remain 75/77, 34/38 and 1/1. Existing incomplete
data/closure checks keep the build nonzero. Production source, owners,
profiles and bank entries are unchanged by this batch.
Ruff, all 713 repository tests (102.725 seconds) and `git diff --check` pass.
