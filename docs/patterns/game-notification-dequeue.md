# GAME notification dequeue addressing

## Function Match Plan

GAME `0x8001fafc notify_effect_update`, 716 bytes, `game.notify_queue`,
existing `probe-gcc257-o2-g0`. At `f6756ac`, strict objdiff is 97.452515%.
The six semantic views, raw compiled/retail instructions, the only caller,
adjacent functions, shared state/sprite types and source history were
inspected before editing. The signature remains `void (void)`:
`render_frame` calls at 0x8002018c and consumes the notification state, not
a return value. The preceding digit helper ends at 0x8001faf4/8 (jr and
its `sb` delay slot); buffer flip begins at 0x8001fdc8.

The state machine is custom game UI, absent from vendor/FID inventories.
Its five direct calls are one menu formatter and four digit-helper calls;
there are no SDK calls, strings, indirect transfers or switch tables.
Five validated internal jumps and 36 validated address pairs complete the
retail text relocation contract. All source ownership remains unchanged:
six 14-byte sprite records, eight independent message-ID bytes and the
22-byte payload/control state. Keep the 56-byte frame and existing 24-byte
digit workspace, byte queue/phase/hold fields and signed short angle.

Preserve phases 0/2/3, ID sentinels 0xff and 0x13, the 15-frame hold,
128-unit angle steps clamped at 512, nibble-derived UV coordinates, four
unsigned digit reads, descending sprite resets and modulo-eight dequeue.
The duplicate-ID loop always consumes one entry, continuing while the next
ID agrees and the current message is not numeric (0x13).

History `37b3915` replaced invalid cross-object byte arithmetic with the
complete `KfNotificationState` owner. Do not restore raw `phase-2`/`tail[2]`
source or enlarge that owner merely to steer registers. The first existing
divergence is the early tail-pointer formation in the angle-guard slot;
later differences include a phase-relative first tail load and the final
phase reset's newly materialized symbol. Retail forms the tail pointer only
after the six sprite clears, loads the initial tail directly, and clears
phase at offset +2 from the tail pointer.

First test the final reset as `*phase = 0`, reusing the real local already
used by dispatch and phases 0/2. This is one address-lifetime correction,
not a new pointer or storage model. Compare the first actual raw divergence
after the focused build before considering the separate tail lifetime.
Require canonical strict comparison, ordered referents, all-image regression
checks, full build and existing lint/tests before committing. Bank only a
verified strict-100% result; preserve every existing data/placement gate.

## Pointer lifetime controls

Using `*phase = 0` removes a direct symbol materialization, but leaves the
reset based on the phase pointer, not retail's tail-relative store. It also
exchanges the phase and case-zero tail register roles throughout the body.
Revert that control. Next isolate the tail assignment: retain its declaration
but assign the pointer immediately after the six sprite clears and before
reading the initial ID, where retail defines it. Keep all other expressions
and the direct final phase reset unchanged; do not force scheduling with
volatile or an artificial dependency if the compiler moves it anyway.

The phase-pointer reset control scored 97.703910% but failed the retail
base/register facts and was reverted. Moving the tail assignment after the
sprite clears restores their entire instruction prefix and the guard slot.
The remaining mismatch starts at the dequeue base itself (a0 versus v1),
followed by the initial tail-load form and final reset address.

Test a typed pointer to the existing complete state for the dequeue operation:
use its named `queue_tail` member for queue updates and `effect_phase` for
the final clear. This is a local object-lifetime model, with no new structure,
header, offsets or inventories. The existing owner proves that both members
belong together; retain the direct initial ID read as its own expression.
Keep this control only if its raw addressing supports retail more closely.

The complete-owner pointer reproduces the retail instruction sequence and
register roles, but its six address immediates are based sixteen bytes before
the dequeue cursor. The remaining difference is not a relocation discrepancy:
retail forms a cursor/control base and uses both its tail at +0 and phase at
+2. Test a named `KfNotificationControl` subobject containing the existing
tail/head/phase/hold/angle fields at state +0x10. The payload array remains in
the same 22-byte complete owner, and the message-ID array stays independent.
This models the directly observed control base without pointer casts, interior
globals, new storage or an artificial offset accessor. The historical nesting
and typedef spelling remain hypotheses, not recovered symbols.

The other consumers receive field-path-only changes: GAME enqueue
0x8001fa44/160 bytes, renderer initializer 0x8001bce0/728 bytes, frame renderer
0x8001fde4/1304 bytes and interaction dispatcher 0x80034de4/2308 bytes.
Their existing call/width/ownership snapshots remain in the notification-state
evidence dossier; fresh interface, outgoing-reference and score queries precede
the change. Preserve the exact enqueue and renderer controls and the partial
interaction dispatcher byte-for-byte. Add compile-time extent/offset checks,
and update the shared layout inventory only if the focused experiment succeeds.

The control subobject restores all six base/offset instructions. One initial
tail read still reuses the phase register (`lbu -2(t0)`) instead of the retail
symbol pair, shortening the body by four bytes; downstream branch offsets
then differ by the same amount. The mutable dequeue-control pointer is not
needed to snapshot the first message ID. Test taking that snapshot before
binding the pointer, then check whether the independent initial load is kept;
do not introduce volatile loads or a duplicate state view to force it.

Moving the ID snapshot before the pointer binding emitted identical code.
Revert that no-effect ordering control and retain the evidence-backed
control-subobject model for canonical comparison and consumer checks.

## Retained verdict

Strict objdiff improves **97.452515% to 99.385475%** relative to `f6756ac`.
The late-tail-pointer intermediate was 98.541900%. The kept typed control
matches retail's dequeue accesses and final reset; no function is newly exact
or banked.
The compiled body is 712 bytes/178 words versus retail's 716 bytes/179 words.

The sole source-form residue is at retail +0x240/+0x244: a symbol-based
`lui v0` / `lbu v0,16(v0)` pair becomes `lbu v0,-2(t0)`. The initial phase
base remains state +0x12, so both read the same byte at state +0x10. All other
aligned words agree except five internal-J addends and three forward-branch
displacements shortened by one instruction. The back edge, delay slots,
constants, widths, call targets and final `sb zero,2(v1)` agree. There are
80 versus 82 text relocations: removing exactly that initial tail-read pair
from the retail sequence leaves the same ordered relocations and adjusted
sites. This is an unattributed address-materialization residue, not an excuse
to remove a real relocation from the target model.

All other 483 native report function rows remain unchanged, including the
13 vendored controls. GAME stays 282/362 exact; OPEN stays 97/108 and PSX 1/1.
Raw consumer bodies are unchanged, not merely equal in score: the renderer
initializer's 182 words/97 relocations, frame renderer's 326/152, and interaction
dispatcher's 575/114 all compare byte-for-byte with their pre-change builds.
Enqueue, digit UV and buffer flip remain strict 100% in the rebuilt unit.
Compile-time checks preserve state size 22, control size six, control offset
16 and every control member offset; the inventory test retains the unique
complete data owner and all seventeen existing state references.

The full build retains its pre-existing data/ownership/placement failures:
source data 7/60, SDK complete contributions 4/4, target relink 110/116, and
zero artifact failures. No banked function regresses. The layout adds one
curated structure, so the inventory census assertion is updated from 88 to
89; no test campaign or tooling change is introduced.

Final verification: Ruff and `git diff --check` pass; all 591 existing tests
pass in 61.243 seconds with the final structure and field census assertions.

## Phase-access Function Match Plan at `333f135`

Refresh the six GAME views, complete retail body/CFG, sole caller at
`8002018c`, digit/buffer-flip boundaries, shared headers and source history.
The unchanged baseline is 99.385475%, 712 compiled versus 716 retail bytes.
The supported no-argument state machine retains its 56-byte frame, five
game-helper calls, 36 retail address pairs, five internal jumps, byte
phase/queue/hold fields, signed narrowed angle test and common return with
its frame-restoring delay slot. There are no strings, candidates, indirect
transfers or SDK bodies to reconstruct. Keep all phase guards, modulo-eight
dequeue, numeric-message exception, digit scratch and sprite side effects.

Retail forms the dequeue control base from the phase address but loads the
first tail independently at +240/+244. Current source keeps an explicit
phase pointer throughout the switch and the compiler reuses it for that
load. Test removing this pointer and using direct accesses to the existing
phase member for dispatch and its two writes. This removes a source-level
address lifetime without adding state, aliases, volatile loads or new field
views. It is distinct from the earlier final-reset-through-pointer trial.
Check canonical and unchanged complete-owner temporary declarations
separately; neither guessed scratch capacities nor production ownership
migration is authorized by this trial. Compare full linked words and ordered
references, and preserve all three exact unit siblings. Only a canonical
strict-100% result verified with full build and raw bytes may be banked.

The four-way trial (explicit/direct phase access, each with canonical or
unchanged complete-owner storage) produces identical linked updater bytes:
712 bytes, still missing the independent tail address pair at +240/+244.
All ordered calls/references and eighteen known successor lists agree across
the trials; all three exact unit siblings retain their complete retail
words. Moving the fixture root by four bytes breaks the reference control.
Direct phase access therefore does not explain this residue. No production
source, owner declaration, score or bank entry changes from this trial.

## CSE address selection revisit at `c3700f8`

Fresh six-view GAME evidence and complete resolved source/target comparisons
retain the 99.385475%, 712-byte baseline. The current complete graphics owner
replaces the older standalone state identity without changing the numeric
references. Enqueue, digit UV and buffer flip remain strict 100% and reproduce
all 53 retail words. The sole caller, shared byte fields, five calls, 56-byte
frame, sprite clears and duplicate-message policy are unchanged.

Native, instrumented and debugger compiler runs produce identical complete
ELF objects. During CSE1, phase pseudo 71 anchors both the idle tail read
(UID 19) and the rotation-exit snapshot (UID 354) at offset -2. In CSE2,
`find_best_addr` first accepts the absolute tail address for both reads. It
then replaces the rotation-exit address with phase minus two again, while the
idle read retains the absolute address. The phase quantity is known in both
observations. Thus neither an unknown phase value nor an invalid absolute
load explains this difference. The preceding control-pointer definition
supplies an address equivalence which is absent at the idle read.

The Function Match Plan tests defining the existing control pointer at its
first use inside the unconditional dequeue iteration. The initial ID snapshot
still precedes the loop; the same pointer serves the loop and final reset.
This adds no memory access or synthetic state. CSE1 removes the initial tail
address definition, and loop optimization hoists the control definition only
to the position after the ID snapshot. CSE2 now retains the desired absolute
tail read. However, CSE1 also converts the final phase reset to an absolute
address, and the control/ID registers exchange roles. This trial is rejected:
99.156425%, 720 bytes, with an extra final phase address pair.

A separate follow-up snapshots the byte tail before binding the control
pointer, then reads the indexed ID after binding. Unlike the earlier byte
trial, the pointer is not defined before the byte read. Unlike the earlier
ID-before-pointer trial, the array read remains after the binding. CSE1 still
keeps the first tail-address pseudo because the later control definition
copies it. CSE2 therefore again sees that equivalence before the snapshot.
The complete linked body and ordered referents are identical to baseline:
99.385475%, 712 bytes. This composition is also rejected.

Both trials have native/instrumented whole-ELF parity and preserve all three
exact siblings. Production notification C and its bank entries are unchanged.
The recorded observations describe this pinned GCC 2.5.7 probe; they do not
establish the original source spelling or compiler. Generated dossiers,
traces, debugger observations and raw comparisons remain under
`build/gcc257/game-notification-revisit/`.

## Message-value scope control at `8e26b18`

Refresh all six GAME views, the complete 179-word retail body and 178-word
candidate, the caller at `8002018c`, both adjacent functions, the numeric
formatter, shared headers and source history. The baseline is unchanged at
99.385475%, with five calls, a 56-byte frame, five internal jumps and one
missing address pair. All three exact siblings reproduce their 53 retail
words. No candidate, string, indirect transfer or SDK body needs promotion.

The idle and rotation-exit cases both snapshot the current message ID from
the same queue. Test one function-scope `KfNotificationId` for these mutually
exclusive uses, preserving the byte type, every load and its source position.
This asks whether separate case-local value ownership explains the remaining
address selection; it does not change the control pointer or repeat the
earlier pointer-binding and tail-snapshot trials. Preserve all fields, phase
guards, modulo-eight updates and the numeric-message exception. Compare the
first raw difference, ordered referents and exact siblings with native/traced
parity before retaining anything.

The shared-ID control emits the same 178 resolved words and ordered targets
as the baseline. Both have native, traced and uninstrumented-debug whole-ELF
parity. All three exact siblings remain byte-exact. The initial tail load is
still phase-relative, so reject this control and leave production C unchanged.
Generated evidence is under `build/gcc257/game-notification-cursor-traces/`.

## Dequeue-boundary closure on master

Function Match Plan at `9ee18fde`: refresh retail hashes, all six GAME
views, all 179 retail words, the sole caller at `8002018c`, neighboring
digit/flip bodies, formatter implementation, shared notification types and
source history. Master starts at 338/362 exact; this function remains
99.385475%, 712 bytes. The separate worktree's successful boundary is a
candidate to revalidate against master's current typed storage, not a reason
to import its other source changes.

The initial message snapshot and duplicate-consumption loop form a complete
dequeue operation after the six sprite clears. Compare four JSON states:
the local block, a global-access inline helper, control/ID-array arguments,
and the initial ID passed by value. Preserve phase dispatch, the narrowed
angle, modulo-eight tail update, numeric-message exception, final phase
clear and all object ownership. This remains game-specific UI policy;
no SDK body, forced inline attribute or new machine call is introduced.

| Source boundary | Strict match | Bytes |
| --- | ---: | ---: |
| Local block | 99.385475% | 712 |
| Global-access helper | **100%** | 716 |
| Control and ID-array arguments | 92.067040% | 712 |
| Captured ID argument | 99.385475% | 712 |

The global helper restores the independent tail-address pair at +240/+244,
the five internal jump addends and affected forward-branch displacements.
Keep that operation as `notify_dequeue_group`, called from the completed
rotate-out case. This identifies a matching source boundary under the pinned
profile; the original helper name remains unknown.

Five independent whole-unit compilations cover all four trials and the final
canonical source. All three exact siblings retain their complete retail
bodies. Both the winning and canonical sources equal the independently
delinked retail target in all 232 unit words, including 179 updater words,
and all ordered call/address targets. The canonical production object agrees
with the independent compile. The updater retains five calls and 36 address
pairs; shifting the graphics owner by four bytes changes fourteen words,
including +244, so the corrected referent is not masked.

The existing graphics-owner test now requires the updater's full retail
body and wrong-owner sensitivity; its required exact count grows from 156
to 157. The literal ledger keeps its occurrence count and assigns the two
ring-arithmetic literals to the helper. JSON trials are in
`build/hypotheses/master-notify-inline`; independent objects, full word
arrays and verification results are in `build/master-notify-inline-objects`.

Full `kf build` reports GAME **339/362 exact**, OPEN 106/108 and PSX 1/1,
with all thirteen vendored controls exact. Existing data/ownership/placement
checks still fail: source-data comparisons are GAME 12/42, OPEN 3/20 and
PSX 1/1; target relinks are 75/77, 34/38 and 1/1. There are no artifact
failures. Concurrent tooling and OPEN changes are outside this campaign.
Ruff, all 735 repository tests (124.410 seconds) and `git diff --check` pass.
Function-scoped banking records only GAME `8001fafc`; the other baseline
rows and unrelated staged files remain untouched. Twenty-three GAME
functions still require strict closure.

## Completed-case exit control

Function Match Plan: refresh GAME 8001fafc/716-byte six-view evidence,
all 179 retail words and the render-frame caller at 8002018c. Fresh focused
comparison still reports 18 matching block successor lists, ten branches,
one return and the previously identified independent-tail-address omission.
The no-argument game state machine retains five game-helper calls, 36 retail
address pairs, five internal jumps and the 56-byte frame. Shared control/sprite
types, digit helper, buffer-flip boundary and prior pointer/loop controls are
unchanged. Preserve phases, widths, all stores and modulo-eight dequeue.

Test two JSON states: the existing three completed-case breaks, and direct
returns at those same sites. Since the switch is the final function operation,
the exits have identical behavior for every phase, including unlisted values.
Neither changes loop tests, call sites or any source object. Both compile at
99.385475%. Independent complete raw listings, including relocation annotations,
are identical. The compiler still reuses the phase base for the initial tail
load, emitting 712 bytes rather than retail 716. Reject the exit-form hypothesis;
keep source unchanged and leave the function open. Nothing banked.

Results: `build/hypotheses/20260908-184454-game-notify_queue-notify_effect_update`;
manifest: `build/notify-exit-hypotheses.json`; independent raw listings:
`build/notify-exit-objects`. The fresh dossier is `build/notify-exit-evidence.txt`
and focused baseline comparison is `build/notify-current-compare.txt`.

## Strict closure through the dequeue boundary

Function Match Plan at `d8be864`: refresh all six GAME views for `8001fafc`,
716 bytes / 99.385475%, and inspect the frame-renderer caller, both adjacent
functions, shared notification types, source history and previous controls.
The game-authored state machine retains five calls, 36 validated address
pairs, five internal jumps, ten branches and a 56-byte frame. No vendor body,
new storage or relocation correction is involved. Preserve the initial phase
snapshot, signed narrowed angle, sprite-reset order, modulo-eight tail update,
numeric-message exception and final phase clear.

Test the complete dequeue operation as an ordinary inline helper. Its initial
message snapshot and duplicate-consumption loop form a self-contained operation
following the six sprite clears. Compare global access, explicit control/ID
array arguments, and the initial ID passed by value against the local block.
These are source-boundary hypotheses, not evidence of an original helper name.
No forced inlining, new machine call or changed object view is requested.

| Dequeue boundary | Bytes | Strict score |
| --- | ---: | ---: |
| Local block | 712 | 99.385475% |
| Global-access inline helper | 716 | **100%** |
| Control and ID-array arguments | 712 | 92.067040% |
| Captured ID argument | 712 | 99.385475% |

The global-access helper alone restores the independent `lui v0` / `lbu v0`
queue-tail load at `+0x240/+0x244`, retaining the control base in v1 and the
phase-relative pointer formation. This also restores all five internal jump
addends and three affected forward-branch displacements. The captured-ID form
is identical to the local block across all linked words and references. The
pointer-argument form keeps only 712 bytes and produces 38 address pairs;
it does not explain retail.

Keep `notify_dequeue_group` with the exact existing operations and call it
from the completed rotate-out case. Independent compilation of all four JSON
states preserves all three exact siblings. Fresh canonical compilation then
matches both the winning trial and retail in all 232 unit instruction words:
40 enqueue, six digit setter, 179 updater and seven buffer flip. The updater
retains all five numeric call targets and 36 ordered address pairs. Moving the
graphics owner's address by four bytes changes fourteen updater words,
including `+0x244`, confirming sensitivity to the corrected referent.

This establishes a matching source boundary under the pinned profile; it does
not prove historical helper spelling or attribute a compiler optimization.
GAME advances **336 to 337 / 362 exact**, and `game.notify_queue` is **4/4**.
Generated evidence: `build/notify-inline-evidence.txt`; four-state manifest:
`build/notify-inline-hypotheses.json`; scores and trial sources:
`build/hypotheses/20260908-200603-game-notify_queue-notify_effect_update`;
canonical raw checks: `build/notify-inline-canonical-raw.json`.

Full `kf build` confirms 337 GAME, 106 OPEN and one PSX exact. It retains
existing data/ownership/placement failures: GAME data 11/41 and target relink
75/77, OPEN data 3/19 and relink 34/38, PSX data 0/1 and relink 1/1; there are
no artifact failures. Ruff, all 713 repository tests (108.119 seconds) and
`git diff --check` pass. Scoped `kf bank --unit game.notify_queue --dirty` records
only these four exact rows, refreshing the three unchanged sibling hashes.
Unrelated dirty source inputs and generated trial files remain unbanked.
