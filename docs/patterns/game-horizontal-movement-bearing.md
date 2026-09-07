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
