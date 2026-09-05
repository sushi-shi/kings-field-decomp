# OPEN transition position snapshot

## Function Match Plan and pre-edit evidence

OPEN `80014608`, `opening_entity_transition`, owns 508 bytes through the return
delay slot at `80014800`. At `072b3c4` its complete C scores 94.685036% strict
objdiff. This follow-up changes only the missing, directly observed stack copy;
the signature, entity owner, mode dispatch, animation, and deactivation remain
unchanged. The current compiler profile is a probe, not historical attribution.

The six image-qualified semantic queries were read again, together with both
caller bodies, the preceding scene-1 controller, the following scene-3
controller, the render-frame callee, source history, and the previous evidence
ledger. There are four proven incoming calls, two proven outgoing calls, and
six validated outgoing relocation rows (three internal jumps and three entity
addresses). There are no candidate references or strings. The transition is
game-owned scene policy, not a Psy-Q provider: it selects custom entity indices
24 through 27, id `13`, and an opening-specific 48-frame animation. Existing
vendor/FID controls do not attribute this body to a library. VSync remains an
SDK boundary, not reconstructed game code.

The scene-3 caller supplies modes 0 and 1 at `80014aa8` and `80014b0c`, with a
VECTOR at caller sp+24. The ending-scene caller supplies modes 3 and 2 at
`80014bcc` and `80014c18`, with a VECTOR at caller sp+16. Both initialize x/z
from the camera and y to -10000. The callee sign-extends mode to 16 bits and
copies all four position words into each 40-byte entity, including the SDK
VECTOR padding word. Its scale bounds use an unsigned halfword; frame/index
operations use signed halfword extension. The two calls are
`opening_render_frame(0, 0)` and `VSync(0)`. The three data pairs resolve to
`opening_entity_state+3c0`, `+3da`, and `+3c0` in that order.

Before activating entities, retail also does this separate copy:

| Input load | Stack store | Coordinate |
| --- | --- | --- |
| `80014684`: `lw v0,0(t1)` | `8001468c`: `sw v0,16(sp)` | x |
| `80014690`: `lw v0,8(t1)` | `80014698`: `sw v0,24(sp)` | z |
| `8001469c`: `lw v0,4(t1)` | `800146a4`: `sw v0,20(sp)` | y |

Each load has an independent instruction before its store, satisfying the
R3000A load delay. There is no later consumer of those stack coordinates. The
earlier reconstruction omitted them as dead source; this pass distinguishes
stores actually present in retail from fabricated dead statements introduced
only to adjust code generation. The same observed-but-unconsumed VECTOR form
also occurs in the independently reconstructed ending scroll.

The focused hypothesis is an ordinary SDK VECTOR local with three field
assignments, not volatile, a forced call, or an artificial aggregate. Do not
initialize its padding lane: no corresponding retail operation exists. Keep
the entity's four-word copy from the original input, not from this snapshot.
Retail reserves 56 stack bytes and saves ra/s2/s1/s0 at 52/48/44/40; the old
source reserves 32 and saves the same registers at 28/24/20/16. Any remaining
frame-space discrepancy must stay unresolved without independent evidence.

## Verdict

The single source correction raises strict objdiff to **99.921260%**, without
changing any source type or control-flow construct outside the observed copy.
Both bodies are now 508 bytes, with all 11 text relocation entries agreeing in
site, kind, symbol, and addend. All non-frame instructions, including the six
snapshot loads/stores and their intervening load-delay instructions, match.

Exactly ten words still differ: the stack decrement/increment and four saved
register stores/loads. The probe now reserves 48 bytes and saves ra/s2/s1/s0
at 44/40/36/32, versus retail's 56 and 52/48/44/40. The snapshot lives at the
same sp+16/+20/+24 in both. No instruction explains the remaining eight bytes;
no padding or unused second local was added. This is an unattributed frame
residue, not a proved compiler mechanism or an exact result. It is not banked.

`tests/test_open_transition_snapshot.py` checks the retail snapshot and its
load delays, the original input's separate four-word entity copy, caller ABI,
raw relocation targets, and the actual object comparison. Its precise list of
ten differing words is explicitly a non-exact residue control, not a mask used
by the matching or banking tools.

The focused real compile/match, full `kf build`, OPEN strict check, Ruff,
`git diff --check`, and all 379 repository tests pass (no workspace skips).
`nix flake check -L` passes too; its isolated 379-test run skips 39 controls
requiring local retail or generated artifacts. All 353 previously exact
functions, 13 vendor-source controls, and 63 data-owning units retain their
exact status. OPEN remains **91 exact / 108 eligible**, with 17 partials and
zero unstarted functions. No banking rows change. The four pre-existing GAME
historical-best deficits documented in `open-format-display-sdk.md` remain the
only all-image strict-check failures; this pass changes no GAME inputs.
