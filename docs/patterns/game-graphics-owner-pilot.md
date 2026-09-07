# GAME graphics owner: complete-clear pilot

This is a tested ownership hypothesis, not an adopted DATA claim or a recovered
original C declaration. The complete GAME graphics clear is larger than the
current fragmented globals. The pilot tests one shared declaration against
independent retail instructions before changing production ownership.

The initial map-cell result below is superseded by
[the byte-index/switch/XYZ source correction](game-map-cell-source.md).
Production is now strict 96.743240%; the unchanged shared-owner declaration
emits all 592 bytes with only eighteen stack operands different. Its ownership
limitations still apply; neither form is a new exact claim.

## Independent bounds and access evidence

At GAME `800146cc/800146d0`, `game_main_loop` materializes destination
`80070e98`. The `memset` call at `800146dc` receives zero and length `249cc`,
with the length's low half in the call delay slot at `800146e0`. The six raw
words are `3c048007 24840e98 00002821 3c060002 0c0140c3 34c649cc`.
The half-open clear ends at `80095864`; `audio_state` begins four bytes later
at `80095868`. Neither that four-byte gap nor audio storage belongs to this
candidate. Five subsequent clears have separate destinations and lengths.
A single memset span proves its accessed range, not a historical C aggregate.

The map-cell matrix chain supplies separate ownership evidence:

- `8001e7dc/8001e7e0` forms `80095760`, the view-quadrant matrix base.
- `8001e7e4` derives the view matrix at base minus 192; the next instruction
  derives the selected quadrant from the orientation times 32.
- `8001e804` adds 128 to the same saved base, producing `800957e0`, and the
  `SetLightMatrix` delay slot adds the same orientation offset.

Conversely, the exact initializer uses four independent absolute pairs for
`800957e0`, `80095800`, `80095820`, and `80095840`. Any proposed owner must
explain both functions together. Absolute reference formation alone does
**not** prove four matrices are a separate complete object. The old inventory
rationale asserting that conclusion is corrected; its name, extent, type,
linkage status, and all relocation sites/targets remain unchanged.

The complete declaration retains the existing typed objects at their physical
positions. Important groups, as offsets from `80070e98`, are:

| Offset | Current interpretation in the candidate |
| --- | --- |
| `00000..20028` | Double-buffered display state and ordering tables. |
| `20028..20108` | Two authentic SDK DRAWENV and two DISPENV records. |
| `20108..20110` | Unresolved eight-byte interval. |
| `20110..20134` | Eight TMD slots and selected asset. |
| `20134..20224` | Unresolved registry interval; not a proved 60-pointer array. |
| `20224..20318` | Current vertex pointer and twelve typed pool records. |
| `20318..241a0` | Unresolved projection/morph interval; no invented capacities. |
| `241a0..241c0` | Three effect texture pages, three CLUTs, and unresolved intervals. |
| `241c0..241d6` | Active material and HUD/notification texture state; unknown byte at `241cd`. |
| `241d6..241f4` | Eight notification message IDs and typed queue state. |
| `241f4..24200` | Floor texture state/count and unresolved six-byte interval. |
| `24200..24800` | 64 typed floor-item records. |
| `24800..24808` | Two reset words whose roles remain unresolved. |
| `24808..24948` | Existing view/lighting/fog/quadrant state. |
| `24948..249cc` | Four light-quadrant matrices and active visibility-window pointer. |

All individual offsets and the complete size are measured from the pinned C
compiler's output, without adding C size assertions. Opaque intervals are
explicitly unresolved storage, not claimed original padding. GAME's existing
projected-vertex and morph identities still describe only their referenced
eight- and 24-byte prefixes. Neighbor distances of 1000/1001 vectors and OPEN's
separate capacity evidence do not prove those GAME capacities; see
[tmd-projection.md](tmd-projection.md).

## Function Match Plan and evidence snapshots

The initial five-witness campaign at `37551a6` inspected the six semantic
queries, complete retail CFG/disassembly, callers, adjacent boundaries, source
history, SDK declarations, and current strict match reports. The shared-owner
experiment changes only these witnesses in temporary copies of their three
TUs, using the same declaration and real field expressions. Production claims
and unrelated function bodies are not rewritten. The startup clear uses the
whole candidate's address and `sizeof`, not an out-of-bounds prefix view.

| GAME function | Hex size; baseline | Signature, references, and first hypothesis |
| --- | --- | --- |
| `800146b8 game_main_loop` | `2e4`; 100% | No arguments from `main`; 32-byte frame, 43 proven calls, 23 validated references including three internal jumps, no strings/candidates. Preserve all six clears, callback pointer, loop policy, calls and epilogue while expressing the graphics clear through the complete owner. |
| `8001bce0 render_initialize` | `2d8`; 100% | No arguments from `display_initialize`; 32-byte frame, 15 calls and 41 address pairs, no strings/candidates. Preserve allocation `32c80`, buffer stride `19640`, yaw values 0/c00/800/400, all nine light constants, four absolute output pairs, SDK texture calls, and notification byte stores. |
| `8001c184 render_set_view_transform` | `12c`; 96.8% | Nullable VECTOR/SVECTOR pointers passed unchanged by `render_frame`; 32-byte frame, two calls and seven pairs, no strings/candidates. Copy full typed inputs only when non-null, divide position x/z by 2000 into halfword cells, then rebuild both matrices unconditionally. Correct the independently disproved branch boundary before using this as an exact owner control. |
| `8001e5ec render_map_cell` | `250`; 76.783780% | Word column/row and byte cell from its sole external caller, traversal; 120-byte retail frame, nine calls, fifteen validated references including five internal jumps, no strings/candidates. Preserve door IDs 44/45/46 -> 17/18/19, signed light timer -1, byte-wrapped object selection, 2000/-100 coordinates and low-halfword view reads; test the cross-matrix base chain without changing local types/CFG. |
| `8001e83c render_map_cells` | `168`; 100% | No arguments from `render_frame`; 48-byte frame, two calls, thirteen validated references including two internal jumps, no strings/candidates. Preserve pitch `(vx+511)` halfword check against 1023, signed yaw high-byte indexing, 204-byte windows, active-pointer store/reload, byte loop counters and full-width skipped-row stride. |

All return instructions include their delay slots. Source history includes
the visibility-state campaign `c17c8a2` and display/parser refinements
`110ec5b`; the OPEN complete-owner investigation is sibling evidence, not
proof of GAME ownership. These functions implement game policy, not vendor
bodies. The curated archive/FID evidence separately attributes SDK operations:
LIBGTE GEO (`RotMatrix`), MTX (`MulMatrix0`, matrix register setters), SMP
(`RotTrans`), LIBGPU texture APIs, and LIBAPI C43 (`memset`). The existing
authentic SDK types and signatures are retained. The pinned GCC 2.5.7 probe
does not establish historical compiler attribution.

## Branch correction and pilot verdicts

Retail `8001c23c` branches to `8001c26c` when the rotation pointer is null.
That destination is **before** the first `RotMatrix`, not after it. The local
pitch vector's y/z components are subsequently zeroed on both paths. Moving
the first call and these two stores outside the source's non-null block makes
the canonical `render_set_view_transform` strict **100%**, with all 300 linked
bytes exact. The old block incorrectly skipped rebuilding the view matrix and
left two local components uninitialized on the null-rotation path.

| Witness | Shared-owner result |
| --- | --- |
| `game_main_loop` | All 740 linked bytes exact, including the complete `sizeof` clear, callback address, and all other clear/call sites. |
| `render_initialize` | All 728 linked bytes exact, including all four independent absolute light-array pairs. |
| `render_set_view_transform` | All 300 linked bytes exact after the canonical CFG correction. |
| `render_map_cells` | All 360 linked bytes exact, including active-pointer publication/reloads and local static-window address. |
| `render_map_cell` | Partial: candidate 576 versus retail 592 bytes. All nine ordered call targets and ten materialized data addresses agree. The 18 words from quadrant-base formation through the final enqueue call's delay slot match exactly. The first divergence is still the 88-byte candidate frame versus retail 120; the epilogue and earlier byte-narrowing/CFG differences remain. No whole-function exact or new objdiff score is claimed for this alternate source. |

`tests/test_game_graphics_owner_probe.py` keeps per-function raw comparisons
and negative controls. A four-byte wrong gap changes measured layout/extent;
a four-byte wrong owner root breaks every exact witness. Separate controls
shift the callback pointer and the static window's section base: instruction
comparison must fail even though direct call targets remain unchanged. The
shared raw linker now handles function-pointer HI16/LO16 and unnamed ELF
section symbols explicitly. It resolves genuine REL addends, without masks.
The canonical view-transform control also recompiles the old faulty block:
both SDK calls still exist, but the retail branch comparison fails.

## Remaining ownership work

This pilot does not add DATA/RODATA claims, change numeric relocation targets,
declare config-only bytes independently compared, or reduce the reachable-data
backlog. It establishes that the complete-region hypothesis can jointly
explain the initializer and matrix-chain witnesses, and removes one false
reason to reject it. No game is booted or executed by these controls.

Before canonical adoption, audit the remaining display, TMD, pool, floor-item,
notification, material, and graphics consumers against this same owner. The
registry and projection/morph boundaries still need better evidence or an
explicitly unresolved representation that preserves their real typed uses.
Preserve every banked function, remove superseded interior DATA claims and
global identities together, and retain every numeric relocation destination.
Only then can a production owner count toward strict data matching and linked
executable reconstruction. Alignment, BSS extent, and relink failures remain
mandatory checks; passing this text-only pilot cannot waive any of them.

## Verification checkpoint

The production render object was forcibly recompiled and the full `kf build`
run. Against `37551a6`, all 484 function-score rows are unchanged except
`render_set_view_transform`, 96.8 -> 100%. All 112 target objects and the other
111 production source objects retain their hashes. No exact function regresses;
GAME advances 262 -> 263/362 exact, and all images together 360 -> 361/471.
Only the view-transform function is selected for banking.

All 616 local repository tests pass without skips; Ruff and `git diff --check`
pass. `nix flake check -L` passes, with 116 expected optional native/retail
controls skipped in its isolated 616-test run. The full build remains red:
6/60 source data-owning units pass the complete strict check, all four SDK
contributions pass, and 110/116 targets relink faithfully. The same six
section-base conflicts and 620 known-referenced config-only ranges without
independent comparison remain. Production `DAT_` references do not decrease.

## Polygon-emitter owner plan (2026-09-06)

At `04ffd96`, extend the same complete-clear hypothesis to the three
polygon emitters without changing their algorithms or production ownership.
Refresh the six image-qualified views and compare the already inspected
retail bodies, callers, SDK boundaries, shared types and source history from
the focused [TMD](game-tmd-enqueue.md), [model](game-model-enqueue.md) and
[map](game-map-enqueue.md) campaigns. Their game-owned dispatch/allocation
policies are not SDK bodies. All calls retain the authentic SDK prototypes,
prepared offsets retain their unsigned halfword widths, and every return
retains its owned delay slot.

| GAME function | Hex extent; strict production match | Owner-only hypothesis and controls |
| --- | --- | --- |
| `8001c7f8 render_enqueue_tmd` | `f38`; 98.932236% | u16 selection/s16 bias; 88-byte frame, 45 ordered calls, 35 retail address pairs, eleven J relocations and a reviewed 29-word switch. Share the selected-asset, projected-base and OT field through the existing complete owner; keep twelve allocation exits and all mode-specific lighting. |
| `8001d730 render_enqueue_model` | `6e8`; 93.126690% | Same argument widths, 96-byte frame, fifteen calls, ten retail pairs, five internal jumps. Also share active CLUT/page fields through the same owner; keep all four mode entries, material publication and quad tails. |
| `8001de18 render_enqueue_map` | `418`; 98.770996% | u16 selection, 80-byte frame, sixteen calls, six retail pairs and one internal jump. Preserve the post-projection asset reload, two separate depth guards and both allocation returns. |

The common chain is selected-asset field `80090fc8` plus 488 to projected
storage `800911b0`, then minus 756 to the OT pointer `80090ebc`. Model's
CLUT/page are projected base plus 16040/16042. Those are decoded addresses,
not a licence for pointer arithmetic between separate C globals.

Use temporary source copies with references to the existing complete-clear
declaration. A temporary typed cursor at the start of its explicitly opaque
projection/morph interval permits checking address formation without
asserting a 1000-entry GAME capacity. Keep that byte view out of production;
neither the existing eight-byte inventory prefix nor neighbor spacing proves
the complete projected allocation. Do not change the shared declaration's
layout, external colour objects, any numeric retail relocation target, the
compiler profile, or the unselected sprite function.

Resolve emitted REL addends to their real addresses and compare complete
raw words, ordered calls, address pairs, switch entries and exit paths.
Record each pilot's first remaining divergence; partial raw agreement does
not update an objdiff baseline. Production migration still requires a
consistent typed representation of the unresolved spans and an audit of
every remaining consumer, preserving all banked functions and data claims.

### First emitter probe and next source hypothesis

The unchanged owner declaration recovers all retail address pairs and the
exact 1768/1048-byte extents for model/map. Their complete linked bodies
differ at only seven/five words, all references to exchanged stack slots:
model's count/header use 32/40 versus 40/32; map's normal-base/header use
24/32 versus 32/24. No branch destination, call, material selector or
instruction outside these accesses differs. TMD becomes 3900 bytes versus
3896 and still has 36 pairs versus 35: its OT load remains separately
materialized, and normal/projected registers remain exchanged. These raw
pilot results do not change production scores or constitute bankable 100%.

The header word is loaded and consumed exclusively within one packet
iteration. Test declaring that existing word at its loop-local load, in
both the owner pilot and canonical source. This is a bounded lifetime
hypothesis, not a local-order permutation or an assertion about GCC's spill
allocator. Do not change the word type, packet advance, count guard, calls,
or separately justified vertex lifetimes. Compare the complete raw bodies
and first divergences before retaining any production change.

Moving the header to its packet-local load is byte-identical in all three
pilot functions. It is not retained in production. The corrected adjacent
[sprite renderer](game-sprite-enqueue.md) supplies a fourth owner witness;
its separate plan records the complete retail/ABI pass and canonical source
corrections before extending this temporary reference-only experiment.

### Emitter pilot verdicts

| Witness | Final temporary-owner result |
| --- | --- |
| TMD | 3900 versus 3896 bytes; first difference at +44 is count spill 32 versus 40. All 45 call targets agree; projected/normal register roles, header/count spills and one extra OT address pair remain. The restore tail agrees but this body is not exact. |
| Model | 1768 bytes, fifteen calls and ten materialized addresses. The only seven differing words exchange the count/header stack offsets; every other linked instruction agrees. |
| Map | 1048 bytes, sixteen calls and six materialized addresses. The only five differing words exchange the normal-base/header stack offsets; every other linked instruction agrees. |
| Sprite | All 592 linked bytes, five calls, seven materialized addresses and every delay slot agree after the separately documented canonical source corrections. This temporary owner does not become a production claim. |

The durable emitter probe checks the full model/map residue sets, complete
sprite words, ordered retail calls, address-pair counts and restore tails.
Shifting the owner root by four breaks each raw result while preserving its
call set. Both packet-header scopes produce identical linked words, including
the unchanged sprite. Existing initializer, display, TMD-accessor and matrix
witnesses still pass. The final 640-test repository run passes without skips;
full production build retains the known strict data/placement failures.

The sprite also matches with the smaller, independently evidenced eight-byte
material record in a separate temporary source. That gives a bounded next
ownership campaign without inventing the projection/morph capacities or
immediately migrating the entire graphics region. Both experiments are
evidence, not objdiff banking criteria; the retained canonical sprite is
93.972980% until its real material ownership can be migrated consistently.

### Frame control supersedes the standalone-material route

The follow-up at `c518cc5` reproduced the older standalone-material
regression before changing production: the exact frame renderer becomes
1296 bytes versus retail 1304, retaining a selector pointer across calls.
The same frame body under this complete owner matches all 1304 linked
bytes, twenty calls and 64 address pairs. Its complete semantic and caller
pass, raw findings and rejected narrow-owner control are recorded in
[game-sprite-enqueue.md](game-sprite-enqueue.md#material-consumer-control-plan-at-c518cc5).

Thus the eight-byte sprite result is not an independent production migration
path. The complete owner jointly explains the sprite and frame renderer;
the typed scratch representation and remaining consumer audit are still
required. No production ownership or banked score changes in this control.

## Floor-item consumer plan at `88d0ad1`

Extend the unchanged complete-region declaration to the corrected entity
traversal and placement loader, using the exact initializer and frame renderer
as controls. The six image-qualified evidence views, full retail bodies/CFG,
caller constraints, adjacent boundaries, source history and SDK boundaries
were reviewed; the four current strict scores are rechecked before the trial.
No function signature, algorithm, local type, compiler profile, fixture layout,
production data identity or relocation target changes in this first control.

| GAME function | Retail bytes; strict score | Evidence snapshot and owner hypothesis |
| --- | --- | --- |
| `8001f218 render_entities` | 1408; 95.164770% | No arguments, sole caller at `800202c0`; 48-byte frame, 61 blocks, 40 branches, nine calls, one internal jump, eight division traps, one return. The traversal plan records the complete five-pool semantics. Retail has 24 address pairs, derives floor_items from active tpage plus 62, and derives its coordinate cursor with plus 66; separate globals emit 25 pairs. Test the real field relationship without inter-object pointer arithmetic. |
| `80020b4c item_load_floor_placements` | 432; 98.888885% | Placement pointer supplied at `8001b664` with `addiu a0,s2,4` in its delay slot; 40-byte frame, seven blocks, four branches, one rand call, three address pairs and one return. Preserve both sentinel-ffff passes, u16 count, 12/24-byte record strides, u8 tiles, signed s16 offsets, 2000/100 coordinate scales, and the full packed-byte random product shifted by 15. Retail transfers the count address before its first loop and independently forms the item base; test whether the common owner explains that transfer. |
| `8001bce0 render_initialize` | 728; 100% | Exact no-argument control from display_initialize; 32-byte frame, three blocks, fifteen calls, 41 address pairs and one return. Preserve allocation 32c80, buffer stride 19640, four quadrant matrices, all texture API arguments and notification byte initialization. |
| `8001fde4 render_frame` | 1304; 100% | Exact nullable VECTOR/SVECTOR forwarding control; 80-byte frame, twenty calls, 64 address pairs, four internal jumps and one return. Preserve HUD guards, material reloads across calls, notification loops, complete SDK matrices and final entity/weapon/display dispatch. |

Every return owns its frame-restoring delay slot. No outgoing candidate or
string evidence belongs to these four functions. Their game-specific pool,
resource and rendering policy is absent from the vendored roster; rand is
LIBAPI C47, texture APIs are LIBGPU PRIM, and matrix operations retain the
reviewed LIBGTE identities and authentic SDK declarations. These library
bodies are negative controls, not reconstructed game progress.

Compile canonical and temporary-owner copies freshly. Resolve all REL addends
to numeric destinations and verify the delinked target against hash-checked
retail bytes before comparing complete words, ordered calls, address pairs and
known successors. Shift the temporary owner root by four as a negative control.
Any remaining difference is recorded without banking the temporary source.
The typed scratch representation and all-consumer audit still precede a
production ownership migration; this trial does not invent array capacities.

### First floor-item owner result and focused source controls

Both exact controls retain every linked word and ordered address pair. The
loader remains 428 bytes with its original four-byte count-loop deficit.
The traversal keeps 1420 bytes and all 61 known successor lists; its two
saved-origin and view-pointer register roles now agree with retail, and the
extra floor_items address pair disappears. It derives the pool from the blue
member rather than the retail tpage member, however, and the material/count
address order still differs. This is not exact reference formation merely
because both sides now contain 24 pairs. The first remaining raw difference
is the actor Z subtraction/mask destination at +1b0.

The exact OPEN loader supplies an independently documented source control:
remove the explicit count pointer and access the real count member directly
for both reset and halfword increments. Earlier rejected GAME direct-global
trials did not use this complete owner. Test that spelling under the shared
owner without changing the two input walks or expansion, and compare the
canonical direct-global form separately as a negative ownership control.

For traversal, the three material stores publish one brightness value in
blue/green/red order. Test the single right-associated assignment
`red = green = blue = 0xb4`, preserving that store order, under both original
and temporary-owner declarations. This expresses the shared value without
adding state, changing widths, reordering independent side effects or forcing
registers. Retain a production change only if it recovers evidenced retail
instructions without a banked regression; neither owner-only success nor a
different register choice by itself proves the original source spelling.

Direct member accesses make the temporary-owner loader match all 432 retail
bytes, all three ordered address pairs and the single rand call. Shifting its
owner root still breaks the bytes. Direct standalone-global accesses instead
emit five address pairs and retain 428 bytes. This isolates the count-transfer
deficit to an ownership-dependent source expression, rather than establishing
a compiler limitation. It remains a temporary result, not a banked function.

The traversal's chained colour assignment leaves canonical linked words
unchanged. Under the owner it derives the pool from red rather than blue, but
still misses retail's tpage base and count ordering; do not retain it. Retail
loads the floor count at +244, before the texture-selector copies, whereas
the source reads it only in the subsequent loop initializer. Next test taking
that existing counter snapshot after brightness publication and before those
copies, then decrementing it in the loop initializer. Keep the original three
colour stores. This follows the observed halfword load lifetime without an
extra local, a new access or changes to the floor-item traversal's exit paths.

### Floor-item verdicts and verification

Retain the canonical counter snapshot. It restores the floor count's saved
register/load lifetime, removes four bytes and raises strict render_entities
from **95.164770% to 97.380684%**. The result is 1416 versus 1408 retail
bytes, with all 61 known successor lists, forty branches, nine calls, eight
division traps and the frame-restoring return preserved. Its ordered address
sequence moves the count before the two texture reads, as retail requires;
the same extra independent floor_items pair remains. The first raw difference
is still the saved-origin register at +5c, followed by the actor Z
subtraction/mask destination and the material-base/division residues. It is
partial and is not banked. The canonical chained-colour form is not retained.

Under the complete owner, that same snapshot emits 1416 bytes but does not
close traversal: saved-origin/actor register roles change again and the pool
still derives from the blue member. No production owner is inferred from a
fuzzy gain or from eliminating an address pair alone.

The loader's direct-member form supplies a new complete-owner witness:
all 432 linked bytes, one rand call and three address pairs agree with retail.
The four-way existing-pointer/direct-access and separate-global/shared-owner
control is retained in the existing probe suite; only direct shared-member
access is exact. Its retail SHA-256 is
`777284628fdec37c637aa589a75aeb6cfa392c64f5735e694a16e5a97eb87acd`.
Its canonical source stays unchanged at 98.888885% until the owner can be
migrated consistently. Initializer and frame witnesses retain all 728/1304
linked bytes and 41/64 ordered address pairs. Wrong-root controls fail without
changing calls. These text-only experiments neither bank a function nor waive
the unresolved scratch representation, remaining consumers or data-layout
checks required for migration.

Fresh production compilation reproduces both the measured snapshot and the
strict-report object's complete linked words and ordered references. Across
all 484 report rows, only render_entities changes relative to `a348881`;
no previously exact function regresses. GAME remains 311/362, OPEN 98/108,
PSX 1/1, or 410/471 total. Ruff, all 679 repository tests (84.189 seconds)
and diff whitespace checks pass. The focused match actually compiles
game.render_scene; full `kf build` retains the known source-data failures
(PSX 0/1, GAME 9/42, OPEN 2/19), target relinks 1/1, 75/77 and 34/38,
six conflicting-section cases, incomplete reference ownership and zero
artifact failures. Concurrent player/effect/map naming edits are not part of
this campaign; its only production C change is the floor-count lifetime.
