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
