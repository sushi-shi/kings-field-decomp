# GAME world-state writer: floor address components

## Function Match Plan

GAME `map_world_state_persist`, `80035b5c` / 696 retail bytes, begins at
strict 94.821840% in `game.map_events`. The configured GCC 2.5.7 probe is
unchanged. Hash-validated retail, all six image-qualified semantic views,
complete disassembly/CFG, three caller windows, both neighboring functions,
shared layouts and source history were inspected before generating variants.

This argument-free leaf has 28 blocks, seventeen conditional branches, one
internal absolute jump and a single `jr ra; nop` return. It has no calls,
strings or unresolved outgoing references. Seven address materializations
select the established map-runtime, player, actor and map-object owners.
Its game-specific event/actor/object serialization excludes vendored ownership.
Callers at `800154fc`, `80035604` and `80035e2c` provide no arguments and
consume no return value; indirect reachability in the interaction caller is
still unresolved and is not promoted by these direct-call observations.

The [complete owner and reader investigation](game-world-state-layout.md)
establishes five 1700-byte floor slots and the ten-byte persistent prefix.
The now-exact reader expresses the floor multiplier and adjusted records base
as separate consumed values. The writer uses the same address relationship:
world base minus 1690 plus current-floor byte times 1700. Retail adjusts the
base in the floor-load delay slot; the candidate instead subtracts 1690 from
the multiplied offset just before adding the base.

Test three JSON source forms: the existing combined expression, the reader's
separate `floor_offset` and `records_base`, and a cursor initialized to the
adjusted base before adding the floor offset. Preserve the unsigned-byte
floor read, signed-word multiplier, complete data ownership, all loop bounds,
sparse filtering and every serialized byte store. No new source object is
added solely to occupy a register.

## Result

| Address expression | Strict score | Candidate bytes |
| --- | ---: | ---: |
| Combined baseline | 94.821840% | 700 |
| Reader components | 96.005745% | 700 |
| Staged cursor | 91.885056% | 700 |

Retain the reader components. Only eleven linked words change, at relative
byte offsets `10,14,18,1c,20,24,28,2c,30,34,38` (hexadecimal). They restore
retail's event-field anchor placement, floor-byte load, base adjustment and
multiply sequence. The complete first 196 bytes now agree with retail; every
later candidate word is unchanged. The staged cursor changes the root/output
registers and does not reproduce that prefix, so it is rejected.

The first remaining difference is the active-count initialization at +`c4`:
retail uses t1, while the candidate uses t0. The sparse object loop also
retains different behavior/count registers and constant materialization,
and the candidate remains four bytes larger. No compiler mechanism or
historical attribution is inferred. This is a retained source correction,
not an exact function or a bankable result.

## Verification

Disposable and canonical objects agree in complete relocation-resolved words
and ordered call/address targets. The delinked target independently relinks
to all retail words. All seven writer address targets remain unchanged, with
zero calls. All five sibling bodies remain fully exact, including their
numeric targets. The focused CFG comparison retains 28 blocks, seventeen
branches, one return and the same known successor lists.

The existing retail/C/Rust world-persistence oracle rebuilds the source and
passes all five deterministic floor cases, each comparing the complete
8500-byte world state. This finite corpus does not prove equivalence over
all possible inputs.

Generated evidence remains under `build/`: the JSON manifest is
`persist-address-hypotheses.json`, scores and retained variants are in
`hypotheses/20260908-174250-game-map_events-map_world_state_persist`, and
`persist-raw-verification.json` records changed words and sibling checks.

Ruff, `git diff --check` and all 713 repository tests pass (115.246 seconds).
Full `kf build` confirms GAME remains 336/362 strict exact, with the writer
at 96.005745% and its five siblings at 100%. The build still exits nonzero
on existing data/ownership/placement gates: GAME data 11/41, OPEN 3/19,
PSX 0/1; target relinking GAME 75/77, OPEN 34/38, PSX 1/1. There are zero
artifact failures. No function is banked from this partial correction.

## Sparse-filter follow-up

A three-state JSON control compares the retained combined behavior/action
predicate, a nested action guard after the same five behavior comparisons,
and a switch over those five behavior values. The planned filter, byte
widths, sparse-copy loop and serialized output are unchanged. The nested
form emits the same complete raw function listing as baseline at 96.005745%.
The switch scores 90.798850%, changes the comparison structure and already
changes the count-slot register at +0xbc. Neither alternative is retained.
Results are under
`build/hypotheses/20260908-180755-game-map_events-map_world_state_persist`;
independently recompiled listings are under `build/persist-filter-objects`.
This rejects these source forms without attributing a compiler mechanism.

## Link countdown follow-up

Function Match Plan: compare the existing predecrement do-loop, an explicit
copy/decrement/break loop, and a for-loop testing the same -1 sentinel. Retail
initializes seven, decrements once per byte and branches against -1; all
forms copy exactly eight bytes in order. The same evidence snapshot, caller
ABI, owners and sparse filter remain applicable.

The JSON batch yields 96.005745% for baseline, 93.856320% for explicit break,
and 97.528730% for the sentinel for-loop. Retain the for-loop. It restores
`li t0,-1` beside the actual copy rather than before the sparse-object walk,
and restores t1 for the active counts in both sparse walks. The first 356
bytes now match retail. The candidate remains 700 bytes versus 696 retail;
behavior-filter constant placement and register operands remain different.

Canonical/trial linked words and ordered targets agree. All five sibling
functions remain raw exact, and the seven address materializations and zero
calls are unchanged. The CFG retains 28 blocks, seventeen branches, one
return and identical known successor lists. All five complete 8500-byte
retail/C/Rust world-state cases pass after a fresh canonical compile.
Generated results are in
`build/hypotheses/20260908-181010-game-map_events-map_world_state_persist`,
with raw verification in `build/persist-copy-raw-verification.json`.
No exact claim or banking follows from this partial improvement.

Countdown verification: Ruff, whitespace checks and all 713 repository tests
pass (126.107 seconds). Full build retains GAME 336/362 exact and the existing
data/placement gates, with zero artifact failures. GAME fuzzy similarity is
99.179%; this aggregate is not a closure criterion.

## Behavior local promotion control

A two-state JSON trial compares the existing byte-domain behavior local with
`KF_ENUM_PROMOTED(KfMapObjectBehavior)`. The actual definition field remains
an unsigned byte; only the consumed local's C promotion width changes, while
modern checking keeps its enum domain. All predicates and serialization are
unchanged. Both candidates score 97.528730% and independent recompilation
produces identical complete raw listings. No source change is retained.
This rules out the tested local-width distinction as the missing source fact.
Results: `build/hypotheses/20260908-181524-game-map_events-map_world_state_persist`.

## Definition lookup scope control

The retained table pointer is compared with direct access through the same
`map_object_state.definitions` owner, removing its local declaration and
assignment. Both preserve the unsigned behavior field and eight-byte stride.
The retained form stays 97.528730% / 700 bytes; direct access is 96.867810% /
704 bytes. Raw inspection shows that direct access removes the outside-loop
base subtraction and emits an additional HI16/LO16 pair inside the lookup.
That loses retail's decoded object-pool/definition-base relationship, so it
is rejected. Canonical source is unchanged. Results are in
`build/hypotheses/20260908-181746-game-map_events-map_world_state_persist`.

## Counted-section and predicate inline boundaries

Function Match Plan at `581f14e`: a fresh six-view GAME pass reconfirms
`80035b5c`, 696 retail bytes / 97.528730%, the three no-argument callers,
28 blocks, seventeen branches, seven address pairs, no calls and the shared
`jr ra; nop` return. The custom world serializer retains the established
owners, widths, five floor slots and all six record walks. Source history,
the adjacent event driver and floor-unload helper, shared actor/object layouts
and earlier address/link-copy helper trials were reviewed before editing.

Test the two counted sparse sections as complete inline operations returning
the advanced output cursor: actor records and object links, independently and
together. Each owns its actual count byte, filter, count, and serialization
loop. The four-state JSON campaign preserves the original filter and link
countdown; it does not repeat the narrower link-copy or floor-address helper.

| Inline section | Bytes | Strict score | First raw difference |
| --- | ---: | ---: | --- |
| Neither | 700 | 97.528730% | `+0x16c` |
| Actors | 700 | 96.752880% | `+0xbc` |
| Objects | 704 | 94.942530% | `+8` |
| Both | 704 | 94.971270% | `+8` |

All four independently compiled states have zero calls, the same seven
numeric address targets, and five sibling bodies identical to retail.
The actor helper changes the first count-slot register and subsequent actor
walk; the object helper moves its output cursor to a1 and restores v1 after
the section. Neither restores retail's behavior-register/constant sequence.
Reject all three boundaries and preserve canonical source.

A three-state follow-up tests the narrower five-behavior predicate, alone
or together with the conditional action-field test, as inline boolean helpers.
The action read remains conditional in both forms. Behavior-only gives
712 bytes / 92.500000%; the complete skip predicate gives 712 / 93.735634%.
Both first diverge at `+8`, preserve zero calls and all seven address targets,
and keep all five siblings raw exact. Both materialize a Boolean and branch
on it in addition to the comparison tree, unlike retail's direct joins.
Reject these boundaries too; canonical remains 97.528730% and unbanked.

Generated four-state results are under
`build/hypotheses/20260908-201332-game-map_events-map_world_state_persist`;
the three predicate states are under
`build/hypotheses/20260908-201456-game-map_events-map_world_state_persist`.
Fresh evidence is `build/persist-inline-evidence.txt`; independent raw listings
and checks are under `build/persist-{inline,predicate}-objects` and the
corresponding `*-raw-verification.log` files.

Ruff, all 713 repository tests (105.437 seconds), and whitespace checks pass.
The full build confirms GAME 337/362 exact and unchanged data/placement gates
with zero artifact failures. Neither manifest changes canonical source or
banks any partial result; baseline source hashes match the current file.
