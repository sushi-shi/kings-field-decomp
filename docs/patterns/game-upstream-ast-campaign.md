# Upstream AST search of the twelve remaining GAME functions

## Function Match Plan

The user requested upstream decomp-permuter on all twelve remaining GAME
functions after merging the two exact filename builders. Start from
`8bc12ab7`, which includes current master's source models and the verified
filename matches. This campaign uses upstream's random C AST transformations;
the preceding filename successes used its finite `PERM_*` combinations.

Refresh the retail hashes, six semantic views, neighboring functions,
callers/callees and source history for each function. Preserve the shared
types, curated identities, authentic SDK boundaries and existing per-unit
profiles. The twelve functions remain game-owned; their library descendants
retain separate vendored ownership. A fragmented `SquareRoot0` CFG remains
unavailable in the navigator and is recorded as such, without inventing a
function extent.

| GAME function | VA | Starting strict % | Retail bytes | Starting raw word differences |
| --- | --- | ---: | ---: | ---: |
| `map_object_spawn_effect` | `80031834` | 94.50495 | 404 | 55 |
| `player_move_horizontal` | `800171fc` | 96.56896 | 2088 | 468 |
| `map_world_state_persist` | `80035b5c` | 97.52873 | 696 | 84 |
| `menu_draw_item_detail` | `80027b7c` | 97.80875 | 732 | 3 |
| `map_interaction_dispatch` | `80034de4` | 99.202774 | 2308 | 166 |
| `render_entities` | `8001f218` | 99.517044 | 1408 | 30 |
| `item_load_database` | `80020cfc` | 99.746666 | 1500 | 16 |
| `effect_update_dispatch` | `80038a38` | 99.82781 | 6156 | 42 |
| `effect_map_collision` | `80037850` | 99.87369 | 1900 | 1 |
| `render_map_cell` | `8001e5ec` | 99.87838 | 592 | 18 |
| `effect_projectile_update_2d` | `80038298` | 99.93421 | 608 | 10 |
| `player_update` | `80018880` | 99.96709 | 6684 | 55 |

Raw counts compare words at the same offsets, including unequal-length tails;
they do not align instructions or prove semantic similarity. All target
objects independently resolve to their complete raw retail bodies. Every
parser-normalized source also reproduces its original function and siblings
when reinserted into the original full TU and compiled with its existing
GCC 2.5.7 probe. Menu detail and player update retain G8; the others retain G0.

The initial sampling target is 2,048 newly compiled source hashes per function,
with a pause for source review at strict 100% and complete raw-retail equality.
Four functions run concurrently with four workers each. A 0.985 continuation
probability permits chains through worse intermediate scores. Retain source,
parent, RNG seed, mutation, depth, native object, strict score and full resolved
word/call/reference audits. Preserve every unaffected sibling, including the
new exact filename builder in `game.map_scripts`.

## Upstream engine and adapter

The engine is upstream
[decomp-permuter](https://github.com/simonlindholm/decomp-permuter) revision
`059609d4aec73eb0650726772954e1ad575825f8`. Local compatibility and
observability changes retain the existing Python/MIPS adapter and add lineage
records. The project compiler adapter replaces only the chosen definition
inside its original complete TU. Upstream's heuristic score guides its search;
the original public objdiff target and resolved retail bytes decide exactness.

The first four functions enable thirteen passes over existing expression
shape, declaration/statement order, assignment chains and conditions. The
remaining eight additionally enable consumed expression temporaries. This
pass keeps the inferred type: temporary-type randomization and reuse of a
similar-type existing variable are disabled through the local
`KF_PERMUTER_TYPED_TEMPORARIES` preset. Dummy locals, padding, dead-expression
passes and external/internal type randomization remain disabled.

After the renderer's reviewed success, give the first four functions a second
2,048-state round with this same typed-temporary preset. Queue this work within
the existing four-function limit. Keep the first round's logs and candidate
hashes, and count only newly compiled definitions in the second round. This
extension tests the successful mutation class on the four functions whose
first round did not enable it.

The collision function's existing static inline `effect_collision_in_cell`
contains most of its compiled instructions. Its search mutates that helper
while preserving the public wrapper. Copies of target/output objects alias
the public symbol to the helper name solely for upstream's single-function
heuristic scorer. The native archive object, strict comparison, retail
identity and source all retain `effect_map_collision`; the alias is not a
banking artifact. The helper's normalized baseline reproduces the original
public function exactly.

These transformations are not a semantic-equivalence proof. Call/reference
agreement permits further exploration, but adoption also requires review of
initialization, scopes, side effects, widths and readable source. Generated
state remains under `build/upstream-permuter/`; it is not committed.

## Exact entity renderer

`render_entities` produces an exact candidate after 850 new compiled source
hashes, with 883 attempted hashes and 278 distinct resolved bodies. Its search
explores chains up to depth 137. The winning chain itself is only two steps:
`perm_reorder_decls`, then `perm_temp_for_expr`, using RNG seed
`61179441706014579949`. Its original function hash is
`45c8f3efab411e832c0dfdd3f4e47e3a9b64d9b2e711e657a02e2c11f074d722`.

The useful change is one real SDK argument pointer. Bind
`&render_light_matrices[KF_RENDER_LIGHT_EFFECT]` before the floor-item loop,
after the floor texture-page assignment, then pass that pointer to the later
`SetLightMatrix` call for actor sprites. This computes an address within a
fixed global array; it does not capture matrix contents or move a memory read
or side effect across the loop. The pointed-to object and API type remain
unchanged. The final source calls the pointer `effect_light_matrix`.

| Reduction control | Strict % | Bytes | Raw word differences |
| --- | ---: | ---: | ---: |
| Early matrix pointer, original declaration order | 100 | 1408 | 0 |
| Early pointer plus upstream's event declaration shuffle | 100 | 1408 | 0 |
| Pointer assigned immediately before its call | 99.517044 | 1408 | 30 |

The declaration shuffle is unnecessary and is discarded. The cleaned-up
source preserves all nine ordered calls, all 24 ordered references, every
instruction/delay slot and the 48-byte frame. Fresh compilation of the actual
source passes strict 100%, independent raw equality, listing comparison and
the modern shared/SDK type check.

The compiler still materializes the effect matrix address at retail
`0x8001f588`/`0x8001f58c`, after the floor-item loop. The earlier C pointer
definition affects the emitted body elsewhere; it does not prove that retail
executed an earlier address load. No backend mechanism or historical compiler
attribution is inferred from this control.

## Reviewed partial improvements

The first-round player search reaches 99.96948% with a single declaration
reorder: move the existing `KfRotation effect_rotation` above `launch_direction`
and `spawn_offset`. No declaration, type, expression or statement is added.
The body remains 6,684 bytes, the frame remains 216 versus retail's 224 bytes,
and all 66 calls and 210 ordered references are unchanged. Raw differing words
decrease from 55 to 51. Fresh compilation of the actual source reproduces the
reviewed candidate, and the modern type check passes. It is still partial and
is not banked.

The typed spawn round later reaches 97.128716%, 408 bytes and 37 differing
words. Eight reduction controls discard unused declarations, an empty
statement, a redundant predicate copy and the divisor temporary. The only
necessary change is computing `object_id < KF_MAP_DROP_BOUNCE_ID_END` before
`rand`, then using that result for the final action-band test. The predicate
reads an unchanged, unescaped byte-valued parameter; it has no side effects,
and all earlier action bands keep their original priority. Reapplying only
that change to the original readable source preserves the observed gain.
The five calls, two references and seven exact sibling functions are unchanged.
It remains four bytes longer than retail; no exactness claim follows from the
score or from the matching saved-register set.

The actual source names the predicate `within_drop_range` and uses the shared
`KfBool` type, whose native representation is `int`. Fresh full-TU compilation
reproduces the reduced candidate and preserves all seven sibling bodies; the
modern type check passes. This partial function is not banked.

A final four-control probe follows the retail X-division-before-Y/Z-load
sequence using a consumed X-cell local while preserving the field-write order.
Both `s32` and `u16` locals yield 90.544556% / 392 bytes with the original
predicate, or 93.069305% / 400 bytes with the early predicate. Calls, references
and seven siblings remain unchanged; none improves the kept 97.128716% body.

## Rejected apparent improvements

The movement search observes 96.63602% versus 96.56896%, but its assignment
chain replaces `position_z += half` with `position_z = half`. Restoring the
addition while preserving the other candidate changes returns to 96.56896%
and 2,128 bytes. The higher score is not retained.

The spawn search observes 95.19802% versus 94.50495%, with unchanged calls and
references, but moves the object-ID write before the object pointer is
initialized and into only one branch. Restoring the original write lifetime
gives 93.46535% and 404 bytes. Matching the retail extent does not validate
the remaining instructions. Neither source is adopted.

The map-interaction search observes 99.36742%, but initializes a new event
pointer in unreachable code before a switch label, then passes that
uninitialized pointer to `map_event_interact`. This candidate is rejected
despite retaining all 54 calls and thirteen references. Restoring the original
event argument and removing an unused assignment carrier returns to
99.202774%, 2,296 bytes and the original 166 differing words. Its fifteen
siblings, including the newly exact map filename builder, remain unchanged.

The typed spawn round also observes a semantically plausible 94.80198%
candidate using a sequence-value temporary and a second alias of the acquired
object. It emits 396 bytes and 100 differing words versus the baseline's 400
bytes and 55 differing words. Removing the duplicate object alias restores
the baseline; removing both temporaries, retaining only the independent
store reorder, or retaining only the sequence-value temporary also reproduces
94.50495%. All four controls preserve the five calls, two references and seven
exact siblings. No independently useful source change survives this reduction,
so the candidate remains an explored intermediate rather than a production
improvement.

The typed menu round observes 98.46448% after chaining three zero matrix stores
and retaining the constant glyph count in a temporary. Calls, references and
34 exact siblings agree, but the body grows from 732 to 736 bytes and raw
differing words increase from three to 134, starting earlier at `+0x28`.
This higher fuzzy score does not justify keeping the loop-bound carrier or
changing the established source; the original three-word residue remains.

## Residual evidence

For functions that remain partial, the refreshed starting comparison records the
following first divergences. These are observations, not compiler-mechanism
attributions or proof of historical source form.

| Function | First differing GAME address | Observed baseline difference |
| --- | --- | --- |
| `map_object_spawn_effect` | `0x80031838` | Saved-register set and offsets differ; body is 400 versus 404 bytes. |
| `player_move_horizontal` | `0x80017228` | Argument register assignment differs; body is 2,128 versus 2,088 bytes. |
| `map_world_state_persist` | `0x80035cc8` | Address setup, zero initialization and constant setup differ in order. |
| `menu_draw_item_detail` | `0x80027c68` | Three words differ in stack-interior-pointer setup and the corresponding halfword-store offset. |
| `map_interaction_dispatch` | `0x80034e10` | Argument register assignment differs; body is 2,296 versus 2,308 bytes. |
| `item_load_database` | `0x800211b8` | Register carrying divisor 30 differs, followed by its dependent instructions. |
| `effect_update_dispatch` | `0x80038a70` | Registers carrying a global load and the effect byte differ. |
| `effect_map_collision` | `0x80037f68` | Sole word difference: generated `li v0,1` versus retail `nop`. |
| `render_map_cell` | `0x8001e5ec` | Frame is 88 versus 120 bytes, with corresponding stack offsets. |
| `effect_projectile_update_2d` | `0x80038298` | Frame is 56 versus 120 bytes, with corresponding stack offsets. |
| `player_update` | `0x80018880` | Frame is 216 versus 224 bytes; other differences remain in the body. |

Every baseline preserves the ordered calls. All except horizontal movement
also preserve the ordered references. Movement has 43 emitted reference
materializations versus 33 in retail, so no candidate is credited with an
ordered-reference pass merely for reproducing that baseline. The other
unresolved differences remain unattributed codegen residue under the existing
source models. No unused storage is added to imitate a larger frame.

## Final campaign verdicts

All searches finish normally. The first round compiles 23,404 new source
definitions; the typed follow-up adds 8,197, for **31,601** total. The small
reduction and review controls are separate from those upstream search counts.
The few counts above 2,048 reflect in-flight workers finishing during shutdown.
Chains reach depth 209; the exact renderer's winning chain is only depth two.
These counts describe sampled source states, not exhaustive C-program coverage.

| Function | First-round new states | Typed follow-up new states | Kept strict % | Final verdict |
| --- | ---: | ---: | ---: | --- |
| `map_object_spawn_effect` | 2049 | 2050 | 97.128716 | Reviewed partial improvement |
| `player_move_horizontal` | 2051 | 2049 | 96.56896 | Unchanged partial |
| `map_world_state_persist` | 2051 | 2048 | 97.52873 | Unchanged partial |
| `menu_draw_item_detail` | 2049 | 2050 | 97.80875 | Unchanged partial |
| `map_interaction_dispatch` | 2049 | — | 99.202774 | Unchanged partial |
| `render_entities` | 850 | — | 100 | Exact and banked |
| `item_load_database` | 2051 | — | 99.746666 | Unchanged partial |
| `effect_update_dispatch` | 2050 | — | 99.82781 | Unchanged partial |
| `effect_map_collision` | 2052 | — | 99.87369 | Unchanged partial |
| `render_map_cell` | 2052 | — | 99.87838 | Unchanged partial |
| `effect_projectile_update_2d` | 2051 | — | 99.93421 | Unchanged partial |
| `player_update` | 2049 | — | 99.96948 | Reviewed partial improvement |

All affected sources pass fresh focused compilation and modern type checks.
`kf build --reconfigure` recompiles all three changed units and reports PSX
1/1, GAME 351/362 and OPEN 107/108 exact functions: **459/471** overall.
All thirteen vendored source controls remain exact. The full command retains
its pre-existing failure for separate data-ownership and placement gates:
data owners match 1/1, 22/41 and 9/20; target relink verifies 1/1, 57/64 and
34/38. There are zero artifact failures. No data or executable closure is
claimed by the function match.

The full local suite passes 785 tests. Ruff, Rust tests, whitespace checks and
`nix flake check -L` pass. The initial full run after the spawn change exposed
a register-specific assertion in the velocity-reset CFG witness. The test
now accepts a nonzero predicate register in `beq rs,zero` while retaining
the exact retail-instruction assertions, branch-destination/store check and
the deliberately incorrect old-scope control. Both subtests and the subsequent
full suite pass; the source's unconditional velocity reset is unchanged.

`kf bank --function game:0x8001f218` updates exactly one ledger row. The two
partial improvements are not banked. Eleven GAME functions and one OPEN
function remain non-exact in the broader final-fifteen goal.
