# Match-preserving port readability backports

## Contract and baseline

Start from decomp `8408235c`, including the reviewed item-detail row improvement.
The port cleanup is a source-design reference, not a patch to merge wholesale.
Retain all three retail images, C compilation, authentic SDK interfaces, image
ownership, layouts, arithmetic widths, call sets and control flow. Preserve every
existing strict-exact function and every non-exact function's score. Compare
allocated object bytes, symbols and ordered relocations, not only aggregate
percentages. Do not alter compiler profiles, bank partials or repair inherited
behavior as part of readability work.

Work is isolated from the dirty master and port worktrees. Validate retail with
`kf init`; capture fresh full-build/analysis artifacts before source edits.
Existing data-ownership/closure failures are recorded separately from source
regressions. Each retained batch receives correctness and holistic reviews,
followed by a full build and existing repository checks before commit.

## Transfer queue

| Port family | Decomp treatment |
| --- | --- |
| Explicit state ownership (#23) | Start with the fourteen GAME effect/map aliases; use their exact existing member expressions without new pointer caches or accessors. |
| Descriptive names and long lines (#12, #18) | Retain already-shared semantic names; wrap expressions and declarations without changing tokens or literal types. Review genuinely missing names against their retail consumers. |
| Domain helpers (#18, #21, #24, #25) | Reuse C-compatible operations only after focused compiler comparisons. Preserve live reads, argument evaluation and real call boundaries. Existing matching helpers are not new work. |
| Pool traversal and object copies (#17, #19) | Trial conventional C loops and typed copies where the operation is independently supported; retain original forms when emitted instructions regress. |
| Vector values and simplified representations (#18, #22) | Keep authentic SDK vector types and their padding contract. Trial direct complete-object assignments or removal of unused views separately; do not import methods or wider runtime layouts. |
| Bounds and distance operations (#28) | Preserve truncation, signed shifts and quantized roots. Existing rejected bounded-distance trials are evidence, not a reason to repeat them unchanged. New natural C forms need their own focused result. |
| File/TU organization (#11, #26) | Reuse organization only where image-qualified contiguous ownership supports it. Retain necessary shared fragments and per-image compilation; no one-executable/state-parameter conversion. |
| Native rendering, audio/files/saves and behavior repairs | Port-only. Do not replace the retail SDK call graph or import intentional fixes into matching source. |

## Batch 1: direct effect and map state access

The four effect aliases and ten map/script aliases are pure object-like macros
over existing typed owners. Remove those declarations and replace their uses
with the same expressions. Preserve parentheses where they affect binding,
array extents and address-taking. Expand the nested boss/floor-five alias all
the way to its existing owner. No new local, call, state snapshot, type, field,
storage, signature or curated identity is needed.

Before changing callers, inspect image-qualified retail address/disassembly/CFG,
incoming/outgoing references, strings, current match, adjacent owners and source
history. Compare a full native baseline after the substitutions. Test source
assertions may need to name the explicit owner; their byte/layout checks remain.

Retained: 157 identifier substitutions across 24 sources, with the fourteen
definitions removed from two headers. All 97 allocated comparison objects,
native object hashes and 484 strict function scores are identical to baseline.
Independent correctness and holistic reviews found no behavior/ownership issue;
their stale-comment findings were applied. Historical oracle labels and tests
that reject old names as independent globals remain intentionally unchanged.

## Batch 2: names and expression layout

Backport the port's consumer-based names for the twelve used gameplay sound
slots, the collision flag-grid bit rejected by actor movement and wandering
events, seven player initial/restart/revival/fatal-height values, the moonlight
control-byte initializer, and three packed dialogue shifts. Use C enum constants
with the original integer values; do not import C++ constant objects or change
literal signedness. Sound slot 9 remains unnamed because no caller establishes
its role. The collision bit's broader authored meaning remains unresolved.

The GAME sound owner remains `map_object.c`, `80056188`, 13 three-byte records;
every call still selects the same offset. The initial camera stores are at
`80016e44..80016e60`; restart and revival retain their distinct coordinates.
The attribute-52 comparison at `80017b4c` retains signed threshold -6999.
The event+8 word still encodes stage/page/delay at bits 8/16/24; its unsigned
trigger mask and evaluation order are unchanged. The moonlight byte name claims
initialization only, not an inferred use for those currently unread bytes.

Wrap long conditions, calls and array accesses in the touched sources. The
formatting pass is required to have an identical non-comment token stream;
it introduces no temporary variables or helper calls. The named-constant batch
also preserves all 97 objects and all strict scores before formatting.

## Natural C trials not retained

These are bounded source-design trials, not compiler permutation. Each was
built with its manifest profile, compared from the first real instruction/CFG
divergence, and restored when matching regressed. Scores below are strict
whole-project objdiff values, not `kf try`'s listing-similarity percentage.
No flags, layouts, relocations or target inventories were changed to rescue a
trial. Unchanged siblings are checked in the full object snapshots.

| GAME function | Port-inspired trial | Strict baseline → trial | First relevant difference |
| --- | --- | --- | --- |
| `actor_pool_find_free`, `8002ca78` | Return the free slot directly; remove the local result and goto | 100 → 79.333336 | Adds an internal jump and relocates the common result/return tail. |
| `actor_definitions_load`, `80030a6c` | Assign the typed definition table | 100 → 0 | The 44-byte function's 456-word copy loop becomes an alignment-tested, chunked copy with unaligned paths. |
| `player_update_vertical_motion`, `80017a80` | Inline `player_current_map_attribute()` | 100 → 89.11392 | Repeats coordinate/address calculations instead of sharing the existing grid index. |
| `actor_pool_update`, `80030818` | Conventional remaining-count `for` traversal | 100 → 90.57143 | Changes the initial counter, decrement placement and branch delay slots. |
| `effect_pool_find_free`, `80036f00` | Conventional remaining-count `for` traversal | 100 → 80.882355 | Changes the result/return join and removes the retail internal jump. |
| `collision_query_world`, `8001a5ac` | Typed inline full-floor predicate | 100 → 98.2243 | Materializes a Boolean and adds control flow to the two shape tests. |
| `effect_map_collision`, `80037850` | Same typed inline predicate | 99.87369 → 93.378944 | Boolean materialization and repeated neighbor checks alter control flow and address reuse. |

## Remaining transfer decisions

- Menu previous/next selection, camera Q4 publication/advancement, radial
  attenuation, angle folding and shifted 3D length already have shared C
  helpers. Do not count their existing implementations as new backports.
- The full bounded-distance and save-retry extractions already have failed
  controls in [common-code-review.md](common-code-review.md). Preserve narrow
  arithmetic and live buffer reads; the port's generic bounds facilities do
  not replace those retail contracts.
- SDK `VECTOR` and `SVECTOR` assignments already express whole-object copies
  where retail copies the complete object. Component updates often use X/Z/Y
  or Z/Y/X ordering, preserve the SDK pad field, or interleave other writes.
  Keep those facts; do not replace them with the port's methods or zero-padding
  value constructors. The pinned `setVector`/`copyVector` macros write X/Y/Z
  only and are already used where that operation is modeled.
- `KfMapObjectLink.words` supplies four-byte alignment as well as a word view;
  the other union alternatives have at most halfword alignment. An unused
  member is not automatically layout-neutral. The player motion union still
  has real packed-word readers in warp selection. Neither is removed blindly.
- Camera segment computation differs between images: GAME selects the current
  point then increments, OPEN increments first. The existing shared pose helpers
  retain those caller policies. Moving all camera code to a new out-of-line TU
  changes retail calls and is not the same operation as the port's #25.
- The 64 already-shared GAME/OPEN implementations and remaining interleaved
  fragments are documented in [shared-game-open-code.md](shared-game-open-code.md).
  Retail still has three independently linked programs. Keep image selection,
  TU order and authentic SDK types; the port's namespaces, single-executable
  state and removal of `KF_OPEN` do not transfer to reconstruction.
- C Boolean/enum adapters support the pinned pre-C99 compiler and the modern
  checking mode. Removing them for C++ `true`/`false`, methods or enum imports
  would not be a C backport. Host rendering, file/save/audio backends, timing
  policy and deliberate undefined-input repairs remain port-only.

## Final verification

The [per-function ledger](../port-readability-functions.tsv) records 80 directly
edited retail bodies (including layout-only edits) and three additional consumers
of the renamed collision bit, with strict before/after scores and final verdicts.
All 97 complete comparison objects retain their
allocated bytes, section sizes/alignments, symbol contracts and ordered
relocations. All 484 scores are unchanged: **458/471 game functions** and all
**13/13 vendored controls** remain strict-exact. Native object hashes are not
claimed identical after adding enum names: unallocated metadata is outside the
allocated-object comparison. No new exact function or banking is claimed.

A fresh full `kf build` succeeds for PSX.EXE, GAME.EXE and OPEN.EXE. `kf analyze`
still exits nonzero for existing data-owner/target-relink gates; this is not a
claim of whole-image matching. GAME still has 23/41 matched data owners and
57/64 verified target-relink units, with seven conflicting-section-base gates.
The game was not run. Retail was initialized and hash-validated before queries.

Both retained batches received independent correctness and holistic reviews.
The final review additionally checked compound C tokens, literal spellings and
logical preprocessor directive boundaries: no unaccounted code changes remain.
The reported stale comments, label indentation and pointer-spacing issues were
corrected. Ruff, `git diff --check` and all 97 modern source/image type checks
pass. All 880 Python checks were exercised successfully: the full run passed
with twelve missing-Rust-driver skips, then those twelve passed without skips
after building the existing support binary. `cargo test --offline` passes with
five preexisting ignored cases. No tests, tooling, flake, inventories or matching
baselines were changed, and no generated reports or local retail paths are
committed.
