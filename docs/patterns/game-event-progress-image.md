# GAME event-image progress update

## Function Match Plan

GAME `0x800337ac map_event_refresh_image_for_progress`, 116 bytes/29 words,
in `game.map_event`, `probe-gcc257-o2-g0`. At `80add05` strict objdiff is
70.793106%; the existing C compiles to 104 bytes/26 words. The six semantic
views, current raw body, caller argument windows, neighbor disassembly,
shared event/player types and original reconstruction history were inspected.

The signature remains `void (KfMapEvent *event)` with a0 pointing at a
68-byte live event. The two proven retail callers are `map_event_interact`
at 0x80034cd0 (event-one base derived from its image-index pointer minus nine)
and `map_refresh_event_images` at 0x8003651c (an eight-record loop, stride
68, filtering state==1). The current partial interaction source contains
additional calls; that caller discrepancy is not evidence to alter this
helper's signature or create relocation rows here. The preceding setter
returns at 0x800337a4 with its nop at 0x800337a8; the following rotation
helper starts at 0x80033820. No boundaries change.

This leaf has no frame, calls, strings, switch tables or indirect transfers.
One validated player_state+0xb address pair and two validated internal J
references form its four retail relocations. All loads/stores are unsigned
bytes: event limit/index/dirty/delay at +8/+9/+10/+11 and highest-floor byte
at player-state +11. Custom image progression and dirty-state policy, with
no provider or SDK correspondence, excludes a vendored-body attribution.

Only update when index < limit. Below the limit, highest floor supplies the
new index if different; otherwise the limit supplies it if different. The
two paths reload their chosen byte, store index, and share dirty=1/delay=0.
Retail's high-floor path branches backward into that shared post-store tail;
the delay reset occupies the jump-to-return slot. The existing C merges the
index store after the flags and emits only one internal J, so this is a CFG
and store-order difference, not a relocation gap to paper over.

First make the shared dirty/reset tail explicit, with the limit-clamp path
jumping to it only after publishing its new index. Keep predicates, widths,
field paths and constants unchanged. If that restores the shared tail,
separately inspect the first-load order: retail loads limit before index,
whereas the existing outer comparison spells index first. Do not introduce
volatile fields, fake carriers or change the common state layout.
Require focused raw/strict comparison, every-unit regression checks, full
build, existing lint/tests and diff check. Bank only verified strict 100%.

## Shared-tail control

The explicit post-index-store tail restores all 29 retail instructions apart
from the first two loads, which are reversed. Its two internal jumps and
store-owning delay slots now agree, and all six exact unit neighbors remain
exact. Next express the identical outer predicate as `limit > index`, in
the observed operand-load order, without adding a cached value or changing
the unsigned byte fields.

## Exact verdict

The limit-first predicate closes the helper at strict **100%**, up from
70.793106%. All 29 raw instruction words and all four ordered relocation
rows match, including their actual addends, branch displacements and delay
slots. No type, inventory, data owner, callee or constant changed. All seven
functions in `game.map_event` are now strict 100%.

Across all 484 native report function rows, only this helper and the separately
documented [menu title load](game-menu-window-rows.md) change; the other 482
rows, including thirteen vendored controls, are unchanged. GAME advances
282 to 283 exact out of 362, with 79 partials; OPEN remains 97/108 and PSX 1/1.
Only this new exact GAME function is eligible for banking.

The full build still reports the pre-existing data/ownership/placement gaps:
source data 7/60, complete SDK contributions 4/4, target relink 110/116,
six conflicting-base units and zero artifact failures. No banked function
regresses. This source-only campaign does not change tooling or the flake.

Ruff and `git diff --check` pass. All 591 existing repository tests pass in
65.881 seconds; no additional test suite was introduced.
