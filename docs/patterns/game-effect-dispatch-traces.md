# GAME effect-dispatch value lifetimes

## Function Match Plan

GAME `effect_update_dispatch` (`0x80038a38`, 6156 retail bytes) starts at
strict **96.939570%** on `probe-gcc257-o2-g0`. The source fingerprint is
`41e5cf7f55105fba3aed08df2f31f6b95407b74e53c32e7c6285511e8efd7a98`.
The WIP `game.effect_dispatch` unit owns this single function, its 49-row
switch table at `0x80012cf8`, and the two SDK `SVECTOR` swing probes at
`0x80057b88`. This work changes no ownership or relocation inventory.

Before editing, the six image-qualified semantic views, complete retail and
compiled disassembly/CFG, sole caller, helper interfaces, shared effect and
magic layouts, neighboring functions, and source history were reviewed.
The existing [dispatcher map](effect-update-dispatch-map.md) and
[constant review](game-effect-dispatch-constants.md) remain the semantic
baseline. Generated plans were recorded before each isolated source edit.

Preserve `void(void)`. `effect_pool_sweep` publishes the current effect and
magic record, then calls at `0x8003a7a4` without consuming a return value.
The dispatcher has a 168-byte frame, 69 ordered direct calls, 135 conditional
branches, 69 internal jumps, one table-dispatch `jr`, twelve division traps,
and one shared return with its delay slot. Its starting object has 21 address
pairs against retail's 22. There are no direct string references. The 49
switch candidates are audited as pointer values without promoting the
navigator's evidence tiers.

The effect policies, actor/player damage, child creation and floor behavior
are game code. The sine/cosine and matrix routines remain separate SDK
providers; no library body or raw COP2 sequence is reconstructed here.
The current effect pointer first differs in a saved-register operand at
`+0x30`. The substantive initial differences include scatter countdown
control flow, a retained lightning-component address, halfword scale copies,
and the homing height subtraction. Correct those values and operations before
interpreting allocation symptoms.

The first experiments distinguish the scatter generation scale and its real
`SVECTOR` scope, then recover the two countdown assignments. Subsequent
experiments isolate directly observed scale widths, pointer-row ownership,
collision snapshots, radius calculations and scalar updates. Every trial
uses the actual unit, checks native/traced object parity and ordered calls,
and compares resolved instructions against a target independently checked
against retail. Composition requires a separately meaningful source fact.

## Retained source

The retained source reaches **99.827810%**, at retail's 6156-byte extent.

- Scatter countdown alternatives assign the field in their own branches.
  This restores the conditional countdown store, the existing kind passed to
  the constructor, the stack-vector address and the invalidation delay slot.
- The sine result becomes its signed full-word pulse offset before adding
  the base scale. Both post-call base loads and the final halfword stores
  remain. The product/shift/base-add words now match retail.
- Common projectile impact owns its `u16` power snapshot. The ground-trail
  case separately owns its collision result, decoded halfword kind, power
  and parent pointer. The ground power now reaches the damage call in `a1`,
  while common impact power survives the audio call in retail's `s7`.
- Ground shrink stores X from the signed word, then assigns Z/Y through the
  halfword assignment result. The signed guard still reads the original word.
  The complete shrink run, including its copy and Y store in a branch delay
  slot, matches retail.
- Ordinary and lightning radial scale growth use X's assigned halfword for
  Z and Y. Ordinary radial damage owns its `u32` radius from the entry phase;
  it remains distinct from the 100/150-unit projectile collision radius.
  Its multiply now interleaves with the scale stores in retail order.
- Lightning radial damage takes a typed pointer to its actual magic row
  after computing power. The four component accesses now reproduce all four
  retail address pairs, recovering the previously missing second-component
  pair after the actor-damage call.
- Player homing computes the full-word aim height before subtracting player
  Y. The emitted subtraction uses the required negative 800 immediate.
  Homing initializes local-motion Y before X. The original uninitialized
  player-target distance read remains documented and unchanged.
- Actor spawning increments its existing byte phase snapshot before
  publishing it. Exactly the two previously differing add/store words change
  to retail; byte wraparound and the converging scale paths are preserved.

## Compiler observations and controls

Native, traced and uninstrumented debug compilers emit identical complete
objects for the original source and retained source at their respective
source paths. The retained debugger run also has whole-object parity.
Seventeen small controls have native/traced/debug parity. The instrumented
compiler fingerprint remains
`222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`.
These observations describe the pinned probe, not its historical attribution.

The small scatter controls reproduce the countdown distinction at CSE1:
the ternary permits a fixed kind and retained stack address; the explicit
field-assignment branches keep the entry kind and recreate the address for
the constructor. Separating only the generation-scale pseudo changes its
ownership but does not predict the real function's hard registers. Narrowing
the real `SVECTOR` scope alone produces the same complete baseline object.

The first small lightning controls use `current_effect` and retain an earlier
address into the same global owner. Both direct indexing and a typed row
rematerialize all four component addresses; this is a negative control for
the full dispatcher's missing pair. With the effect supplied as an argument,
the direct form instead keeps the first component address across the actor
call and emits three pairs. The explicit row dies during CSE2 and emits four
absolute component pairs. Both entry contexts are retained as evidence;
the successful context is not presented as a universal pointer rule.

The small ground-store pair exposes the halfword value at EXPAND: after X
is stored from the signed word, the chained Z/Y expression creates a
distinct HI result while retaining the SI value for the guard. Independent
stores lack that assignment-result copy. This agrees with the recovered
retail shrink run without requiring a fake temporary or forced instruction.

Four small ground/radial ownership controls compare shared values, the
ground family, the radial radius, and their composition. The shared power
has four references and occupies `s1`. Ground ownership separates a
two-reference, no-call-crossing power in `a1` from the radial result in
`s1`, reproducing the relevant lifetime distinction. The separate radius
creates its own source pseudo; the control's original constant collision
radius then disappears. That last optimization is specific to the reduced
control and does not occur in the full dispatcher.

The pulse-update control retains the named signed-word result where the
nested expression eliminates it. Its hard registers differ from the full
function, so only the value-ownership observation is transferred. The byte
phase control preserves the existing phase pseudo through the increment.
The real phase trial changes only the two intended retail words.

A debugger capture of an earlier retained candidate finds an `s1` preference
for the shared power, excluding that register during the first search for
the impact-magic pointer. Isolated power splits also introduce an unwanted
position-address lifetime at CSE2. The ground/radius composition removes
that extra lifetime; impact ownership then restores the common call-site
transfers. These intermediate states explain why a lower isolated score is
not sufficient to reject an independently supported ownership fact.

## Trial verdicts

The generated plan states each parent; this table summarizes the retained
frontier and the separately useful ownership compositions.

| Source state | Strict % | Bytes | Verdict |
| --- | ---: | ---: | --- |
| Starting source | 96.939570 | 6156 | Baseline |
| Scatter countdown branches | 97.460040 | 6152 | Retain decoded control flow |
| Homing Y/X initialization order | 97.461340 | 6152 | Retain observed stores |
| Lightning magic row | 98.767380 | 6152 | Restore the 22nd address pair |
| Ground X then chained Z/Y | 98.871346 | 6156 | Entire shrink run matches |
| Lightning scale chain, independently from row | 99.108510 | 6152 | Retain assigned halfword |
| Both scale changes | 99.215720 | 6156 | Retain composition |
| Homing aim-height word | 99.363220 | 6156 | Restore subtraction and loads |
| Ground power alone, from aim height | 99.083170 | 6164 | Local lifetime evidence; extra common-path address |
| Ground collision family, from aim height | 99.203380 | 6164 | Local lifetime evidence; extra common-path words |
| Radial radius alone, from aim height | 99.460040 | 6164 | Restore multiply/store interleaving |
| Ground family plus radial radius | 99.671870 | 6156 | Extra common-path words disappear |
| Add common impact-power ownership | 99.766080 | 6156 | Restore common call-site transfers |
| Add ordinary radial scale chain | 99.801820 | 6156 | Restore scatter's scale temporary |
| Sine-offset word update | 99.821310 | 6156 | Restore local product/base words |
| Phase update, independently from radial chain | 99.808320 | 6156 | Exactly two words become retail |
| Sine and phase updates composed | 99.827810 | 6156 | Retained partial result |

Other controls are not retained: splitting only the early scatter scale
recovers its local words but changes other scale/address lifetimes; narrowing
the stack-vector scope is byte-identical. Moving the ground Y store after
the guard gives the wrong ordering, while chaining all three stores puts the
copy before X. An ordinary radial chain on the earlier shared-value base is
byte-identical, as is making the shared power a word with explicit halfword
argument conversions. Isolated common-impact and ordinary-radial power
splits retain an extra position address. Explicitly widening phase into the
radius before compound multiplication loses the recovered interleaving
(99.467186%); the direct phase-times-step expression is retained.

## Remaining difference and verification

All **1539 instruction positions** have matching non-register bits. All
control-transfer locations, kinds and destinations agree, including the
49 resolved table targets and the return delay slot. The ordered 69 calls
and 22 address pairs agree. The two initialized probes are byte-identical to
retail. There are **42 differing words**, all with different register fields:

- Entry magic and kind occupy `s6`/`s3` instead of retail `s3`/`s6`.
- Ground's parent and ordinary radial damage radius occupy `s3` instead
  of retail `s5`.
- The ordinary radial multiply and scale-store sequence interchanges its
  `v0`/`v1` temporaries. Its instruction order and constants agree.

The function remains partial and is not banked. Matching non-register bits
is not strict 100%, and these symptoms do not establish a compiler wall.
The production GAME compiler/profile, SDK interfaces and retail inventories
are unchanged by this campaign.

The full production build recompiles the affected object; the recorded match
and independent production audit reproduce **99.827810%**. Modern type
checking, repository lint, 730 repository tests and `git diff --check` pass.
The literal ledger independently accounts for all 133 remaining occurrences;
the explicit phase `+1` is now the byte increment.

The full build remains unsuccessful at existing data-placement and reference
ownership gates. Data comparisons pass for PSX 1/1, GAME 11/41 and OPEN 3/19
units, with no comparison artifact failures. This unit's 16 initialized bytes
match retail, but GNU ELF's 16-byte section alignment conflicts with the
eight-byte-aligned address `0x80057b88`. This campaign does not change that
profile. Target relinking verifies PSX 1/1, GAME 75/77 and OPEN 34/38 units.
The separate concurrent PSX alignment pilot is not part of this change.

No banked function regresses. Exact counts remain PSX **1/1**, GAME
**336/362**, and OPEN **106/108**: **443/471** overall, with no new exact
function from this campaign. The original strict-completion objective remains
open.

Generated plans, isolated sources, traces, debugger captures, controls and
raw audits are under `build/gcc257/game-effect-dispatch-revisit/` and are not
committed.
