# GAME world-state layout investigation

## Later exact reader closure

`map_restore_floor_state` now matches all `0x69c` retail bytes at strict
**100%**. The final source correction keeps the established world owner and
spells the stream address as two short-lived components: `floor_offset` is the
current-floor byte multiplied by 1,700, while `records_base` is the world-state
anchor minus 1,690. Assigning their sum to the input cursor emits retail's
`addiu v1,a0,-1690; addu s0,v0,v1` without changing the address, call set,
five-floor switch, or any later instruction. A single combined expression
associates `-1690` with the multiplied offset instead; assigning the adjusted
base directly to the long-lived cursor moves it too early. The exact form uses
no padding, volatile state, forced register, or compiler-profile change.

## Function Match Plan

The full goal remains 29 strict-exact GAME parsers/serializers. This campaign
starts at clean `8ffaaa6`, with 23 exact. The preceding
[inline-helper pass](game-parser-inline-helpers.md) leaves a specific ownership
question: a four-byte anchor currently represents an 8,500-byte saved region.
Retail is hash-verified again before the image-qualified evidence refresh.

| Function | GAME VA / size | Starting strict score | Evidence and first hypothesis |
| --- | --- | ---: | --- |
| `map_restore_floor_state` | `80035e44` / `69c` | 99.964540% | One direct caller, `map_load_floor`; complete inverse byte walk and five-way scripted tail. Four `rand` sites and 15 other direct call sites; no strings. Every instruction/ordered referent agrees except the final two address instructions. Test the real 1,700-byte floor slot with a ten-byte persistent prefix, not a manually reassociated byte-offset expression. Preserve all read/store order and the inherited current-floor byte width. |
| `map_world_state_persist` | `80035b5c` / `2b8` | 94.821840% | Three direct callers: death restart, interaction dispatch and unload. Leaf with no calls or strings; eight reviewed address/branch references. The 8/128/190/160/10/20 walks and sparse eight-byte link payload are preserved. Test the same typed floor-slot expression, then the independently evidenced event/world owner relationship. |

Both functions are game-specific state policy, absent from the vendored
inventory; their direct call/reference sets and custom runtime pools exclude
an SDK implementation. Retail CFG/disassembly, callers, adjacent functions,
data references, match state and source history are reviewed before edits.

The restore's first remaining divergence is:

```text
retail:     addiu v1,a0,-1690; addu s0,v0,v1
candidate:  addiu v0,v0,-1690; addu s0,v0,a0
```

Here `v0` is the current-floor byte times 1,700 and `a0` is `8009ddb4`.
The reader subsequently reaches the event pool at `a0 - 556`. The serializer
uses the same anchor for both regions. These are direct address-generation
facts, not an inferred compiler mechanism.

## Layout evidence and staged experiment

Save read/write copy `0x2134` bytes (8,500); initialization clears the same
extent. The per-floor stream starts at `base + 10 + (floor - 1) * 1700`.
Persistent script bytes occur at offsets 0 through 3, 3,400, and 6,800 through
6,803. This supports a five-element slot view with a ten-byte persistent
prefix followed by 1,690 bytes available for the variable-length runtime
stream. It does not establish semantic names for every prefix byte.

The first source trial uses that shared slot type as a temporary view of the
existing anchor. It tests the addressing hypothesis before changing curated
identities or every consumer. Such a view is experimental, not an acceptable
final substitute for proper global ownership. If useful, review all interior
identity xrefs, alignment and adjacent event fields before promoting a shared
owner; do not create overlapping globals. The save routines' word-copy
alignment must remain supported, and no banked function may regress.

No compiler/profile change, forced register, dummy local, artificial padding,
volatile carrier or assembly body is authorized by this hypothesis. Rebuild
each focused experiment, compare the first raw divergence and ordered
relocations, then require strict objdiff and complete differential checks for
any retained result. Final handoff also requires full build/check, tests/lint,
whitespace checks and banking only newly verified 100% functions.

## Complete-owner evidence

The direct retail call at `8001472c` invokes `memset` with destination
`8009db88`, zero and length `0x2360` (9,056) in its delay slot. That extent
ends at `8009fee8`, exactly the saved world block's end. Combined with the
pool-relative accesses in the reader/writer and `map_event_update_spinner`,
this supports the complete owner independently of the field-view trials.

The next trial curates that single owner and converts former global names to
member aliases. Existing function bodies keep their operations; all compiled
consumers must retain banked exactness. World storage keeps its established
word alignment and 2,125-word save view. The floor-slot view did not improve
either parser and is not required to establish the enclosing object.

## Retained model and relocation audit

`KfMapRuntimeState` owns the entire `8009db88..8009fee8` interval in
`game.map_events`: eight 68-byte events, the current-event pointer at `+220`,
the variant allocation pointer at `+224`, two halfword timers at `+228/+22a`,
and the 8,500-byte world block at `+22c`. The previous consumer spellings are
member aliases, not separate global identities. The eight unresolved word
identities at `8009ddb8..8009ddd4` had no direct xrefs and lie inside this
saved block; script flags likewise become byte views inside the same owner.
The saved bytes' complete internal semantics remain WIP.

All 85 curated GAME address-pair rows into the owner keep their original
sites, paired sites, decoded target addresses, evidence and status. Only the
target symbol changes to `map_runtime_state`; the delinker encodes each
interior address as an owner-relative addend. Leaving the removed symbol
names in those rows initially caused apparent exactness regressions; updating
the actual referents restores every previously exact consumer. No retail
instruction, target value or evidence tier is changed to accommodate source.

The pinned-compiler fixture checks the complete size, four-byte alignment,
world-block size and all six offsets. A deliberately wrong `0x235f` size
must fail compilation. Separate integration controls decode all 85 retail
HI/LO pairs, reject removed globals, and independently check both retail
clear extents. The parser harnesses now derive field addresses from the
complete owner rather than looking up removed interior identities.

## Experiments and final function verdicts

The five-element floor-slot access moves the `-1700` adjustment into the
multiplied index and uses a ten-byte load/store displacement. Retail instead
materializes the stream pointer and accesses offset zero. This reduces raw
similarity in both functions (restore 99.6 to 99.2; persist 84.9 to 82.8).
A temporary enclosing-struct pointer view worsens restore further. Both
access experiments are removed; these raw diagnostic percentages are not
strict objdiff results. The retained owner comes from the independent clear
and cross-member evidence, not from those rejected addressing expressions.

The auxiliary consumer reviewed during that owner investigation is
`map_event_update_spinner`, GAME `800358e0`, extent `8c`. Its sole direct
caller is `map_event_pool_update` at `800359dc`; it takes no O32 arguments
and returns void. Its body has one direct audio call, no strings, three
conditional branches, a 32-byte frame and the stack restoration in the
return delay slot. The `lhu`/`sh` pair updates the halfword angle by 200 and
masks to `fff`. The floor-five/first-event test derives the event pool from
the current-event pointer address minus 544. The sound call derives its
position from that same address minus 508 and passes `7f`, `4650` and a
fifth stack argument `c350`. These custom event/floor predicates and the
curated call set exclude an SDK body; no vendored function is reconstructed.

| Function | Final strict score | Verdict |
| --- | ---: | --- |
| `map_restore_floor_state` | 99.964540% | Unchanged. Correct owner, same two remaining address-arithmetic instructions. |
| `map_world_state_persist` | 94.821840% | Unchanged. Correct owner; base arithmetic, constant placement and register/order differences remain unattributed. |
| `map_event_update_spinner` | 100% | Newly exact auxiliary consumer. Its C body is unchanged; the real shared owner enables the retail cross-member address reuse. Raw instructions and ordered relocations agree, including delay slots and the five-argument sound call. |

The explicit parser/serializer census stays **23/29 strict-exact**, with
99.493564% size-weighted similarity over 13,672 bytes. The other four
non-exact parser scores are unchanged from the inline-helper pass. Overall
GAME exactness increases from 261 to 262 of 362 eligible functions; the
spinner is not added to the parser denominator. This is a corrected source
ownership model, not evidence that the remaining parsers have a behavioral
bug or that their codegen differences cannot be resolved.

## Verification and Linux execution

The affected units are rebuilt with the pinned `gcc257-native` profile,
followed by a full reconfigured `kf build`. Strict focused matches reproduce
the table above; raw `kf try --unit game.map_events` reports the spinner
exact. Every banked-exact GAME function remains strict 100%, checked even
when its input hash changed. Only the newly exact spinner is banked with
`kf bank --function game:0x800358e0`.

Python passes 363 tests plus 229 subtests; the pinned positive/negative layout
controls and all 85 decoded-reference subtests run locally, not skipped.
`ruff check scripts tests`, whitespace checks and `nix flake check -L` pass.
The isolated flake test run also includes the new test module, but correctly
skips proprietary-image and unavailable-compiler controls. Rust passes its
96 ordinary tests and all five explicitly enabled retail-corpus tests;
library-only checking and formatting also pass.

`python -m scripts.kf.codec_oracle` rebuilds its candidates and passes all ten
complete suites without case/event limits: 250 TMD payloads, 461 resource
cases, five outer floor walks, 1,136 animation cases, five VAB banks and nine
SEQ files / 15,880 events, five full-state VAB banks plus 16 runtime controls,
15 save-read cases, 14 save-write cases, 15 world-restore cases and five
world-persist cases. The complete declared byte states and ordered service
requests agree. No new behavioral reconstruction bug is demonstrated.

PS1 code executes on Linux using the existing isolated Unicorn little-endian
MIPS harness. One run maps hash-verified retail GAME bytes; another relocates
freshly compiled C functions into private emulated code slots. Both receive
identical input bytes, O32 arguments, stack, inherited register seeds and
poisoned destination state. Declared output regions and service traces are
compared against each other and an independently implemented native Rust
driver. Candidate execution cannot silently fall through to undeclared
retail code.

This runs isolated function calls, not a PS1 boot or playthrough. Unchanged
Sony providers and deterministic SDK, allocation and GTE hooks remain explicit
boundaries. The runner separately audits executed R3000 load-delay hazards
and guards writes; Unicorn alone is not evidence of all original hardware
behavior. Finite corpus agreement is not a proof over every possible input.

## Writer base-expression revisit at `c3700f8`

The Function Match Plan refreshes all six GAME views, all 174 retail words,
three no-argument call sites, neighboring functions, shared owners, source
history and the earlier parser/layout/inline-helper controls. The writer is
still a game-owned leaf with no calls, strings or indirect transfers; its
seven address pairs and one internal jump are validated. The production
baseline is 94.821840%, 700 bytes versus retail's 696 bytes.

Use the two short-lived address components already verified in the exact
inverse reader: `floor_offset` holds the floor byte times 1,700, and
`records_base` holds the saved-world anchor minus 1,690. Their sum initializes
the output cursor. This is distinct from the previously rejected combined
expression and adjustment of the long-lived cursor itself. It preserves the
complete owner and every byte written to the serialized stream.

The retained source reaches **96.005745%**, still 700 bytes. The first 49
retail words now agree exactly, including the base adjustment, scheduling and
complete event loop. All subsequent source words and ordered numeric
references remain unchanged from the baseline. The five exact unit siblings
retain their combined 297 retail words. No function becomes newly exact.

Two separate follow-up controls are rejected:

| Control | Strict result | Observed result |
| --- | ---: | --- |
| Eight-byte copy uses `k-- != 0` | 92.040230% / 700 bytes | The old counter copy survives through combine; it does not become retail's decrement-and-minus-one test. Registers change throughout the body. |
| Behavior comparison uses the promoted enum form | 96.005745% / 700 bytes | Complete linked instructions and ordered references are unchanged. The byte storage and original local type are retained. |

Debugger observation of `move_movables` explains where the current two
misplaced constants move. The inner copy has seven RTL instructions and
hoists -1, with lifetime/savings 1/1. The outer object loop has 37 RTL
instructions and threshold 52; 65 has lifetime/savings 1/1 and the already
moved -1 has 11/1. Both move to the outer preheader. The measured behavior
agrees with the source's profitability test, including its increased cost
for an already moved expression. It supplies no independently supported
source correction for those constants, so no carrier, extra operation,
profile change or declaration permutation is introduced.

The remaining differences are the active-count register, definition-base
placement, behavior-value register and the two constant lifetimes in the
sparse-object loop. Source remains one instruction longer overall. All six
walk extents, predicates, byte order, return delay slot and numeric referents
are preserved. No partial result is banked. Generated evidence remains under
`build/gcc257/game-world-save-revisit/`.

Final verification uses an actual affected-unit rebuild, followed by the full
`kf build`. Native, instrumented and debugger compiles have whole-ELF parity
when compared at identical source paths; production instructions match the
retained experiment. Modern type checking, Ruff, whitespace checks and all
725 repository tests pass, including the existing world-persistence oracle.
Full build retains the existing data ownership/extent/placement failures and
PSX/GAME/OPEN target relinks of 1/1, 75/77 and 34/38, with no artifact failures
or banked-function regression. The 17 writer literal-ledger rows have updated
line references; their tokens, expressions and reasons are unchanged.

Concurrent master commits `fb415a6` and `9080261` close the menu-window and
map-object updater functions. Those independent changes bring GAME to 334/362
strict exact; OPEN remains 106/108 and PSX 1/1. This writer correction adds
zero exact functions and no bank row. The remaining goal is still open.
