# Complete Clang field review after enum integration

The enum commits `ffcf78cf` and `234e55be` were integrated with master's three
concurrent matching commits at `dafd1048`. This review examines that resulting
source state. It found further candidates: the earlier claim that all remaining
integer fields had been fully covered was too broad. **F1–F9 are now implemented**
with shared enums, typed flag operations and behavior-selected union members.
The findings below retain the original evidence; the implementation results
follow the campaign plan.

The subsequent [array audit](array-enum-review.md) extends the current field
ledger to 1,048 declarations, adds typed overlay slots and corrects the two
fixed-bank floor-sprite selectors. Counts below describe this earlier campaign.

## Original audit coverage and method

The review uses the flake's Python `clang.cindex` bindings and libclang 21.1.8,
with each source's strict MIPS C++20 arguments from `compile_commands.json`.
It parses all 111 project C files, walks every project `FIELD_DECL`, records
member references by declaration identity, and separately parses project headers
that contribute no declarations to those translation units. All 54 project
headers are accounted for; there are no parse errors. Four headers contain no
field-bearing declarations: `kf/lib/address.h`, `kf/game/game.h`, `sys/fcntl.h`, and
`sys/types.h`.

There are **1,030 unique field declarations in 165 record definitions**, counting
structs, unions, anonymous nested records and the checking template. This covers
all 854 fields in `config/retail/structure_fields.tsv` plus **176 additional
fields**. Arrays count as one declaration; union alternatives and nested record
members have separate rows. The 165 definitions include eight modern effect
argument records and the generic enum-storage template; they are not 165 retail
layout claims. SDK records imported from outside the repository retain their
SDK declarations and are not counted as project-owned definitions.

The complete, manually reviewed ledger, refreshed after implementation, is
[`enum_field_review.tsv`](../../config/evidence/enum_field_review.tsv).
Every row identifies its declaration, type, disposition, source evidence and
rationale. Raw AST extraction and member-use dossiers are local products under
`build/field-audit/`; `extract.py` there reproduces the extraction with
`nix develop -c python build/field-audit/extract.py`. Extraction proves coverage;
the semantic dispositions come from reviewing the declarations, use chains,
curated evidence and the focused cases below. Macro-based whole-object copies,
raw word access and initializers were also checked where a candidate required
them; a lack of AST member references is not proof that a field is unused.

| Disposition | Field declarations |
| --- | ---: |
| Already enum-typed, including enum storage wrappers | 93 |
| Numeric quantities, counts, offsets and resource-relative indices | 409 |
| Nested records or arrays of records | 207 |
| Pointers or pointer arrays | 54 |
| Unresolved meaning | 86 |
| Padding/reserved storage | 19 |
| Packed, encoded, format or SDK storage without a proposed selector conversion | 144 |
| Generic enum wrapper's private backing field | 1 |
| Candidates in F1–F9 below | 17 |
| **Total** | **1,030** |

The table above describes the original audit. The current ledger contains 1,046
fields and records the F1–F9 closures and their new child members.

A `typed_enum` verdict means the field has strict type identity. It does
not certify that every runtime value is enumerated, or that every explicit
decode in its callers is justified. Likewise, `numeric` is the current
source-evidenced interpretation, not a claim about unreconstructed code.

## Findings

### F1

**`KfAnimKeyframe.reverse`: missing interpolation-selector domain.**

This private struct in `src/game/pool.c:23` escaped the checked-layout inventory.
`render_bind_animated_instance` chooses the normal fraction for zero and
`4096 - fraction` for nonzero. GAME retail loads the unsigned halfword at
`0x80020774`, tests zero at `0x8002077c`, and performs the reverse subtraction
in the jump delay slot at `0x80020788`. This is a selector, not a counter.

A fresh scan through `tmd_oracle.shipped_cases` and
`animation_oracle.parse_asset` finds 70 animated assets, 214 clips and 813
keyframes; **all 813 encode zero**. The nonzero branch is proven control flow,
but a particular nonzero encoding such as `1` is not established by the shipped
corpus. A typed forward/reverse view is warranted; any chosen nonzero authored
value must remain a documented hypothesis until supported. The existing
synthetic reverse oracle case is not retail evidence.

### F2

**`KfActorDefinition.action_parameters[8]`: an enum hidden inside a mixed array.**

All eight positions already have named roles in `game/actor.h`:

- 0–2: effect code, combining the existing `KfEffectKind` with the paired flag;
- 3–5: their chance values;
- 6: drop object, interpreted as `KfMapObjectId`, including disabled/unset values;
- 7: drop chance.

`actor_select_next_action`, `actor_try_select_profiled_action`,
`actor_spawn_action_effect`, `actor_update_effect_action`, and the death-drop
branch of `actor_update_current_action` establish these uses. The first three
bytes are masked before indexing the action-profile table or dispatching an
effect. Byte 6 is compared to encoded enum constants and decoded back to
`KfMapObjectId` at the spawn call.

The earlier description of these eight bytes as unresolved kind-dependent
payload was inaccurate. A structured view can type the drop object directly,
retain numeric probabilities, and give the effect-code bytes a shared packed
view. Applying one enum to the whole array would lose its actual layout roles.

### F3

**`KfMapObjectLinkFields.action_parameter`: a missing typed union alternative.**

The field stores effect-pool indices, paired-door object indices, and a copy
region selected by behavior. `map_object.c:554` explicitly decodes the byte as
`KfMapCopyRegionId` before `map_apply_copy_region`; its other consumers index
object/effect arrays. The copy-region branch should have a typed alternative
sharing the existing storage. The whole field cannot honestly become one
copy-region enum.

### F4

**`KfEffectRecord.base_render_id` and `render_id`: untyped named resource domains.**

`game/effect.h` already declares named billboard and model IDs, plus `0xff` for
no rendering, in anonymous enums. The constructor assigns them to raw bytes;
the dispatcher compares named IDs and advances billboard frames. Rendering
chooses the interpretation using `animation_clip`.

These are stronger candidates than arbitrary asset-table indices because the
source already attaches stable effect meanings to the values. A shared typed
render-resource representation should preserve the billboard/model distinction,
the NONE value and numeric frame arithmetic. Two overlapping ID banks should
not be silently treated as interchangeable merely because their bytes coincide.

### F5

**`map_cell_attribute_grid` through `KfMapGrid.cells/bytes`: a missing dedicated
attribute view.**

The grid carries named pitfall, poison-hole, warp, hidden-door, bottomless-pit
and missing-attribute IDs. `map_interact_at_player_view` and `player_update`
switch on these values. Other callers use the same byte as a mesh/height-table
index. This supports a map-attribute domain, with explicit indexing boundaries
and address-derived names for unresolved resource identities.

`KfMapGrid` also backs quantitative floor heights and occupancy count/flags.
Converting the generic grid would conflate those domains. The appropriate
candidate is a dedicated attribute-grid view, as already done for collision
kinds and orientations. There are two affected declaration rows because the
row/column and linear views alias the same cells.

### F6

**`KfEffectRecord.type`: packed collision/class selectors remain untyped.**

The low two bits select actors, player, both, or neither; bit `0x10` selects
player magic. Other class bits participate in actor-damage credit, while `0xf0`
and `0xff` serve specialized deformation/free-slot roles. Relevant readers are
`effect_map_collision`, `effect_magic_power`, the pool, and actor damage code.
This is an opportunity for typed extracted selectors and flags. A flat enum
containing only the three named collision constants would be incomplete.

### F7

**`KfFloorItemPlacement.facing_and_frame_count` and its runtime copy: facing is
hidden in a packed count byte.**

Both renderers extract the high nibble: zero means billboard; nonzero supplies
a biased quarter-turn yaw. The low nibble controls the frame loop. A facing
domain can be shared by GAME and OPEN after extraction; frame count remains
numeric. This is not grounds to turn the complete byte into a facing enum.

### F8

**Five TMD `mode` fields: normalized primitive-format domain is untyped.**

These are `KfTmdPacketHeader.bytes.mode` and the color/mode words in
`KfTmdF3`, `KfTmdF4`, `KfTmdG3`, and `KfTmdG4`. GAME and OPEN prepare-index
routines dispatch on eight named `KF_TMD_MODE_*` values after masking the
semitransparency bit. The header's field has no direct member reference because
the code reads its packed `word` instead.

The normalized mode can have a shared type. Raw mode bytes also contain flags,
and the body color/mode words are copied in packed form; preserve those format
facts. This is a format-domain candidate, not newly reconstructed game logic.

### F9

**`KfActorDefinition.status_effect` and `KfPlayerState.status_effect_flags`:
shared typed flags remain possible.**

Both use the `KF_PLAYER_STATUS_*` bits. `player_apply_damage` and status-update
paths independently test, set and clear combinations. They need a flag-set type
with same-domain bitwise operations if stronger typing is desired. A mutually
exclusive state enum would incorrectly exclude legal combinations. This is
separate from the missing selector domains above.

## Negative controls and unresolved fields

Several plausible names do not justify enums:

- `KfEffectSprite.asset_variant` is passed as the animation binder's **fourth
  argument, phase**. The name is misleading; the consumed value is numeric.
- `KfEffectRecord.id`, `KfActorDefinition.effect_owner_id` and
  `KfMapObjectSpawn.effect_id` propagate to player damage's
  `multiplier_tenths`, which participates in multiplication and division.
  Their ID-style names should not be used as evidence of a finite ID domain.
- `KfFloorDeformSegment.column_step/row_step` are modular coordinate increments,
  including 255 for minus one. Small observed values do not make them choices.
- Animation keyframe indices, morph-table indices, actor-definition indices and
  bank-relative audio programs are quantitative/resource-relative indices.
- `KfSavePayload.item_stock` includes player quantities and shop stock. The shop
  Gold Cross entry is decremented on purchase; the complete array is not a
  boolean availability domain.
- The three OPEN `unknown_control_50a/50c/50e` halfwords and the four named
  `DAT_*` graphics words are only cleared in reconstructed code. This does not
  establish state domains. The two Moonlight control bytes are initialized to
  `0xff`, but their meaning is still unresolved.
- The graphics registry and projection/morph byte arrays already have pointer
  and vertex uses. They need structural ownership refinement rather than enums.

The ledger explicitly retains all 86 unresolved fields. Their presence means
there is no evidence-backed claim that every possible enum in the unrecovered
program has been found.

## Integration verification

After the integration, `kf check-types` passes all **112/112** source/image
variants. The existing repository suite passes **714 tests**, with nine skips
and 9,173 subtests. The audit adds no tests or runtime source changes.

The full `kf build` was rerun. Its existing data-ownership/data-comparison
failures remain in all three images; GAME and OPEN also retain target-relink
or section-placement failures. Strict syntax checking and the test suite pass;
full executable closure still does not. The enum audit makes no new exactness
or banking claim. Concurrent changes in the primary worktree are preserved.

## Function Match Plan: implement F1–F9

Implementation baseline: `ac95ac2d`, in the isolated enum worktree. The nine
findings are one type-propagation campaign over their existing data/call
families. Before changing source, refresh all 484 per-function retail dossiers
(address/extent, CFG/disassembly, callers/callees, strings, relocation and match
state) under `build/enum-implementation/before/`, inspect the affected family
records alongside the field ledger and source history, and preserve the 116
current compiled objects. Existing library calls remain vendored controls.

- F1: add an unsigned-halfword interpolation direction. Keep the zero/nonzero
  branch, with a documented canonical nonzero source value rather than claiming
  the shipped assets demonstrate it.
- F2: replace the eight-byte actor parameter array with its proven component
  structure. Give packed effect codes their own domain, propagate it through
  selection/spawn helpers, and use the existing map-object domain for drops.
- F3: give the action byte typed copy-region and numeric pool-index alternatives.
- F4: give billboard and model IDs typed alternatives in their shared byte;
  retain their common NONE representation and explicit frame arithmetic.
- F5: add a dedicated attribute grid and enum, keeping numeric height/occupancy
  grids distinct. Make table-index conversions explicit.
- F6: type the complete effect type/class flag byte and propagate it through
  construction, collision, power selection and damage-credit arguments.
- F7: type the packed appearance byte and its extracted facing domain; keep
  frame counts and coordinate conversion numeric.
- F8: type TMD mode flags and the normalized dispatch expression, preserving
  packed SDK transport and the semitransparency mask.
- F9: share one player-status flag domain across actor definitions, player state
  and damage arguments. Bitwise operations accept only the same domain.

After each focused family build, compare from the first real divergence against
both its preserved object and retail; inspect referents, calls, CFG and typed
instruction selection in that order. Preserve all banked exact functions and
all original data widths. Update curated identities/layouts and the field
ledger, run strict checking and the existing suite without new tests, then run
full `kf build`, lint, whitespace checks, and flake checks if tooling changes.
Final verdicts must explicitly close each of F1–F9; no score-only banking or
speculative runtime changes are part of this campaign.


## Implementation results

All nine findings are closed. The original 17 candidate declarations now have
strict enum types or typed component views; 16 new declarations make the mixed
storage explicit. Numeric probabilities, pool indices, frame counts and raw
whole-object copy views retain their numeric role.

| Finding | Kept implementation | Evidence and boundary |
| --- | --- | --- |
| F1 | `KfAnimationBlendDirection` on the private keyframe field | Unsigned halfword and zero/nonzero branch preserved. `REVERSE = 1` is a canonical source value, not an observed shipped value. |
| F2 | `KfActorActionParameters` with `KfActorEffectCode[3]`, numeric chances and `KfMapObjectId` | Eight-byte extent, drop at relative offset 6; effect selection and spawn signatures carry the same enum at their original ABI widths. |
| F3 | `KfMapObjectParameter` | Existing `KfMapCopyRegionId` shares one byte with separately named numeric effect/object indices; each behavior uses its matching member. |
| F4 | `KfEffectRenderId`, `KfEffectBillboardId`, `KfEffectModelId` | Distinct resource banks; billboard frames and both NONE sentinels enumerated. Animation clip chooses the view; frame/index arithmetic explicitly encodes it. |
| F5 | `KfMapAttributeGrid` and `KfMapAttribute` | Shared GAME/OPEN attribute type, including every shipped attribute value. Floor heights and occupancy remain in the generic grid. |
| F6 | `KfEffectType` flag enum | Target bits, power bit, credit class, deformation and free encodings share the original byte. Constructor, collision and damage arguments retain the domain. |
| F7 | `KfFloorItemAppearance` and `KfFloorItemFacing` | Serialized and live appearance bytes share the packed type; a typed component constructor/extractor separates facing from numeric frame count. |
| F8 | `KfTmdMode` flag enum | All five mode fields and normalized format dispatch use one domain; packed word transport and semitransparency masks stay intact. |
| F9 | `KfPlayerStatusFlags` | Player state and damage API use the halfword domain; actor definitions use enum-only byte storage. Same-domain bitwise operations preserve combinations. |

`KF_ENUM_FLAGS` opts individual domains into bitwise operators. Those operators
accept and return the same enum; no integer or foreign-enum overload exists.
Ordinary assignments and function arguments still reject unrelated enums and
integers in strict C++20. The pinned retail C view retains its original storage
and parameter widths. Explicit encode/decode operations remain at serialized,
packed, arithmetic and numeric API boundaries.

The shipped resource census confirms attribute IDs `0x00..0x62`, `0x66`, `0x67`
and `0xff`, packed floor-facing high nibbles `0x00`, `0x10`, `0x20`, and actor
effect bytes including `0xff`. Other attribute names remain numeric identities
instead of invented semantics. The facing enum names the decoded encodings;
unused nonzero facing values are supported by arithmetic, not claimed as
observed assets. The existing initial-frame calculation consumes the whole
appearance byte, so its expression is preserved exactly.

The map renderer originally reuses one byte first as an attribute and then as
a bank-relative mesh index. A local union now names those two phases, with an
explicit domain-to-index assignment. Both GAME and OPEN preserve the retail
instruction sequence. Two independent scalar locals changed generated register
use; that intermediate experiment was rejected after the object comparison.

The refreshed Clang pass covers **1,046 unique fields in 171 record definitions**,
all 111 C sources and all 55 project headers, with zero parse errors. All 866
curated fields are represented, plus 180 declarations outside that inventory.
The 16 added fields are four actor-parameter members, three map-object parameter
alternatives, two effect-render alternatives, three attribute-grid views, and
four local attribute/index alternatives across GAME and OPEN. The ledger keeps
all previous declarations and assigns a final verdict to each added member.
Four new layout claims bring the curated inventory to 129 records; six function
signatures and both map-attribute data identities have been updated.

## Verification

The final per-function verdicts are local at
`build/enum-implementation/final-verdicts.tsv`, alongside refreshed pre-edit
retail disassembly/CFG, callers, callees, strings, data and relocation dossiers.
All **116 preserved objects are unchanged** in allocated section bytes, sizes,
alignment and ordered relocation offsets/types/targets. This includes four
vendored data controls. All 484 owned function verdicts retain their original
match state: **459 exact**, including 13 vendored functions; the game-only
count stays **446/471**. No banked function regressed, and no new match is claimed.

Strict type checking passes **112/112 source/image variants**. Focused matching
covers the animation, actor, map-object, effect, map-rendering, floor-item,
TMD and player-damage families. The full `kf build` was run after rebuilding
source: it still fails the pre-existing ownership/data comparisons in all three
images and the existing GAME/OPEN relink/placement checks. Identical preserved
objects establish that this campaign did not introduce those differences.

The existing full test run initially found three stale type/API expectations;
those existing assertions and constructor controls were updated, with no new
tests. All 115 tests in the affected modules then passed (41 subtests); the
remaining tests had passed in the full run. `ruff check scripts tests` and
`git diff --check` pass. `nix flake check -L` passes, including the hermetic
723-test suite (140 skipped because optional/local prerequisites are absent).


## Integration with concurrent matching

The campaign commit `99bbc39` was merged with master commit `5f8b1b6d` in the
isolated worktree before handoff. The concurrent status-detail layout and
item-detail edits are retained. Resolving the status-detail overlap applies
only the five typed NONE comparisons to master's final function body;
`menu_draw_status_details` remains **100%** against retail. The integrated tree
has **460/484 exact owned functions**, or **447/471 game functions**. That one
additional exact result belongs to the concurrent matching commit.

All 114 other preserved objects remain identical; the two changed objects are
exactly the two source units changed by `5f8b1b6d`. The enum campaign itself
preserved all 116 objects before integration. Integrated per-function verdicts
are in `build/enum-implementation/integrated-verdicts.tsv`. The field ledger's
member references were refreshed for both merged source units with zero Clang
parse errors; its 1,046-field coverage and nine closed findings are unchanged.

Final integration checks: **112/112 strict variants**, **714 existing tests
passed, 9 skipped, 9,173 subtests passed**, lint and whitespace checks passed.
The full build was rerun and retains the same pre-existing image verification
failures described above. No new tests or banked match claims were added.
