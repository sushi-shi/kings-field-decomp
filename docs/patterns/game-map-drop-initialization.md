# GAME dropped-object initialization

## Function Match Plan at `16ba53e`

Target GAME `80031834 map_object_spawn_effect`, 404 retail bytes in
`game.map_object`, strict 94.455444%. Fresh native/traced/repeated compilation
emits 400 bytes and preserves six exact siblings; `map_object_pool_update`
is the other partial in the eight-function unit. Keep the pinned
`probe-gcc257-o2-g0` profile and every shared type, owner and referent.

All six GAME views, full retail/compiled bodies, sole external caller's two
argument-producing paths, acquisition/action callees, exact debris sibling,
adjacent boundaries, shared record layout and source history were inspected.
The function has five proven calls, two validated address pairs and three
validated internal jumps, with no strings, candidate outgoing references or
unresolved indirect transfers. Its 40-byte frame saves ra and s0 through s4;
the candidate instead saves through s3 and restores the same frame size.
Every branch, call, jump and return retains its owned delay slot.

The caller at GAME `8002fdf0` supplies byte kind/ID, the actor's word position
at +28 and negative half collision height. Definition drops select group 170,
placement drops group 180, each with ten records and a halfword sequence.
The allocator returns a free record or the greatest wrapped sequence age.
Initialization publishes the sequence and ID, position/cells, rotation and
idle action. The three ID bands below 43, 48 and 65 select actions 96, 97 and
98. This is game drop policy; rand at GAME `8005049c` remains the separate
LIBAPI/C47 signature-backed provider. No vendored body is reconstructed.

Retail's final `beqz` at `80031994`, raw `10400003`, targets `800319a4`:
`sh zero,36(s1)`. Thus IDs at or above 65 also clear the signed-halfword
vertical velocity at record+36, while retaining the idle action. Current C
places the clear inside each action branch; its corresponding `10400004`
skips the last store and goes directly to the epilogue. This scope error is
present in the original reconstruction commit `3839393`. The caller excludes
specific disabled/unset IDs, not the entire range at or above 65.

Move the velocity clear after the whole action-selection chain, preserving
its position after any selected action call. This expresses the common
initialization directly and removes the three duplicated source stores.
Do not change sequence publication or introduce pointer/register carriers
to address the earlier save-register difference. Compare the first raw
divergence after a focused rebuild, all words, ordered calls/referents and
every other unit function. Native/traced parity and deterministic traces
remain controls; only strict 100% permits banking. Run the full build,
repository tests, Ruff and whitespace checks before committing the kept fix.

## Retained result

The common velocity clear changes exactly one source instruction: function
+160 (hex) becomes `10400003` instead of `10400004`. It now reaches the zero
store for IDs at or above 65, matching the retail CFG. The three action-call
paths retain their calls and clearing delay slots. Strict objdiff improves
**94.455444% to 94.504950%**; the constructor remains 400 bytes versus retail's
404 and is not exact or banked.

The first raw difference remains the saved-ra offset at +4. Trace allocation
still assigns the sequence pointer and acquired object to s0: the former has
five references, one crossed call and live length fourteen; the latter now
has eighteen references instead of twenty, four crossed calls and live length
forty-nine. No new source hypothesis follows from those register symptoms.
The existing exact debris constructor legitimately reuses s0, so that sibling
does not establish a universal requirement to keep these pointers separate.

All eight unit functions were freshly compiled and numerically relinked:
1222 source words, 52 calls and 32 address materializations. Every delinked
target independently reproduces retail. All seven siblings are unchanged,
including the six strict-exact functions and partial pool updater. The unit's
RODATA bytes also remain unchanged. Native/traced/repeated whole ELFs agree
per source, and repeated traces are byte-identical. A wrong sequence address
changes one instruction; a wrong action-callee address changes three.

The regression test separately anchors the retail branch and store words,
then freshly compiles the correct source and the original faulty scope. The
correct >=65 edge lands on the velocity store; the old edge skips it and
lands on the restore block. Its opcode/offset check deliberately leaves the
unmatched base register outside this bounded CFG claim; it is not an exactness
mask or whole-function closure criterion.

Focused `kf try` and canonical `kf match` perform real compilation. Ruff,
whitespace checks and all 704 repository tests pass (118.708 seconds, nine
optional skips). Full `kf build` retains the known data/ownership/placement
failures: source data PSX 0/1, GAME 10/42, OPEN 3/19; target relinks 1/1,
75/77 and 34/38 respectively, with zero artifact failures. Current state has
no comparison failures or lost exact functions. GAME remains 324/362 and
OPEN 106/108 exact; all forty remaining functions still require strict 100%.
Generated reproduction inputs, traces and raw audits are retained under
`build/gcc257/map-drop-trace/`.
After staging the correction, `kf bank` refreshes only the six already-exact
sibling input hashes. Both partial functions remain outside that selection.

## Acquisition-branch control at `382cca6`

A fresh six-view dossier, all caller/callee constraints, native/traced ELF
parity and complete raw audit reproduce the retained 94.504950%, 400-byte
constructor. The pool updater has since been independently closed, so all
seven siblings now provide exact controls. The velocity-clear scope above
remains correct and is preserved.

Test acquiring the effect inside each of the two existing pool-selection
arms, keeping their sequence/index assignments and the shared post-increment
after the join. This is an observed call/selection-boundary hypothesis for
retail's distinct sequence and object lifetimes, not a new value carrier.
The isolated source reaches 93.712870%, 408 bytes, with the same five ordered
calls and two physical references. It recovers separate pointer registers
but assigns sequence/object to s1/s0 instead of retail's s0/s1. The post-call
object copy precedes the sequence halfword load and adds a nop, whereas
retail puts the copy in that load's delay slot. Reject the control.

Its SCHED1 trace still has both acquisition calls; they merge only later.
The sequence pseudo has six references, two crossed calls and live length
22; the object has nineteen references, four crossed calls and live length
54. The retained baseline has five/one/fourteen and eighteen/four/forty-nine,
respectively. These measured lifetimes do not justify forcing either source
value to stay live. The exact debris sibling remains a control showing that
reuse of one register can be legitimate.

Native/traced whole ELFs agree per source, and all seven siblings retain
literal retail words. No production source, type, owner, profile or bank
entry changes. The isolated plan, source and raw/trace evidence remain under
`build/gcc257/game-drop-revisit/`. The unchanged constructor is included in
the 725-test and full-build verification recorded in the
[interaction campaign](game-map-interaction-loops.md#verification-and-unit-verdicts).

A later clean-clone control composed that branch-local acquisition with the
declaration order used by the exact debris constructor (`object` before
`sequence`). It reproduced the same 408-byte result: the probe kept the two
pointers separate but still assigned the acquired object to `s0` and the
sequence to `s1`, opposite retail, and inserted a load-delay `nop` after the
post-call sequence reload. Declaration order therefore does not supply the
missing source fact; the canonical constructor remains unchanged and partial.
