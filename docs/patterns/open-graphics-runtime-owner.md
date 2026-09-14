# OPEN complete graphics-runtime owner

This is an ownership hypothesis, not an original-source or compiler claim.
The pilot below led to canonical adoption; see the final migration verdict.

## Independent extent and access evidence

OPEN `opening_run` loads destination `80049a48` at `800156dc/800156e0`, zero
at `800156e4`, and length `0x24788` at `800156e8/800156f0` for the `memset`
call at `800156ec`. The region ends exactly at `8006e1d0`, the independently
modeled audio-state start. The following call clears the separate `0x510`-byte
entity state. These are proven calls with validated address pairs; a clear
region alone does not prove one historical C aggregate.

Three other access chains cross the current separate graphics owners:
display setup derives display-environment and DFE fields from a draw field;
TMD emission derives projected vertices, ordering-table pointer and material
RGB from the current-asset slot; map-cell rendering derives light-quadrant
matrices from view-quadrant matrices. The smaller render-state extension
previously failed the exact initializer control. A complete-region model is
a distinct hypothesis, not permission to ignore that control.

The temporary shared declaration preserves every existing supported nested
type and each known member's physical address. Offset checks cover all fields,
and a size check covers `0x24788`. The unresolved intervals are `+20108..20110`,
`+2011c..20120`, `+20124..20138`, `+22078..23fe0`, and `+24786..24788`.
Their byte views do not imply recovered objects or additional padding.
Projection scratch remains 1000 eight-byte records, not an invented larger
array spanning the following gap. The allocation counter at `80075928` and
visibility windows at `800439d8` lie outside the region and remain separate.

## Function Match Plan and pre-edit snapshots

At `2560194`, all six queries, full retail disassembly/CFG, callers, adjacent
boundaries, source/history, shared types and SDK provider boundaries were
checked for the affected witnesses. No signature, algorithm, literal, call,
compiler profile or GAME source changes are part of this pilot.

| OPEN function | Retail size; strict baseline | Relevant control |
| --- | --- | --- |
| `80018344 render_enqueue_unlit_triangles` | `0x2a4`; 93.455620% | Eight calls, one internal jump, one address pair; signed bias and byte/halfword packet fields; current-asset/projected/material/OT chain. |
| `80016908 render_initialize` | `0x1d4`; 100% | Eleven calls, 28 address pairs, 32-byte frame; independent light-array pairs must remain exact. |
| `80016adc display_initialize` | `0x1d8`; 89.203390% | Eighteen calls, two internal jumps, sixteen address pairs; mode `0xfe`, real SDK environment fields and converged lighting policy. |
| `80016cb4 primitive_buffer_allocate` | `0x84`; 100% | `u16` request, one printf call, one internal jump, five address pairs; buffer overflow print loop and separate counter. |

The same initializer unit contains `800168dc lighting_set_active_color_matrix`
(`0x2c`, 100%); its independent load-image matrix table is not rewritten.
The controller supplies extent evidence only and is not edited. The functions
remain game-owned policy around the separately identified SDK/archive services.

Build alternate sources under `build/open-runtime-owner-probe/` using one
coherent header, with original global accesses replaced by real fields of the
candidate owner. Do not use field aliases, out-of-bounds pointer arithmetic or
different per-file views. Resolve the candidate root to `80049a48` when auditing
instructions; symbolic-name differences alone cannot decide the result.
Reject migration if the owner fails to jointly explain the emitter and exact
initializer. Do not curate new retail relocations to fit the candidate.

## Pilot result and next discriminator

The complete-region model preserves every linked instruction of
`render_initialize` (468 bytes), `primitive_buffer_allocate` (132 bytes),
and `lighting_set_active_color_matrix` (44 bytes), including all address
targets, calls and delay slots. In particular the four independent light-array
pairs survive. This contrasts with the earlier small render-state extension;
its failure does not rule out the complete-region model.

The unlit candidate is 688 bytes rather than the current 692 (retail 676).
It forms the current-asset slot and derives projected vertices at +32, and
derives the material-color address at +16076 from that slot. Only two absolute
pairs remain, targeting `80069b60` and `80069a6c`, versus four in the current
source. All eight calls retain their ordered physical targets. It still saves
the narrow bias on the stack, keeps a separate color pointer, and uses an
absolute ordering-table load; the first raw divergence is still the bias save.
The source model is not an exact emitter reconstruction.

Display setup becomes 484 bytes instead of 496 (retail 472). Its 18 calls
and numeric targets remain correct; the independent display-environment
reference on the mode-`0xfe` path becomes a shared-base access. Four absolute
DFE references and the 40-versus-48-byte frame discrepancy remain. Neither
candidate's raw comparison is being reported as a new strict objdiff score.

`tests/fixtures/open_runtime_owner_probe.h` preserves the coherent candidate
and checks every field offset and the full extent using the pinned compiler.
`tests/test_open_runtime_owner_probe.py` reconstructs the initializer-unit
candidate in a temporary directory and resolves real ELF REL addends before
comparing every word with retail. A four-byte gap error fails the layout
checks; shifting the proposed root by four bytes fails the instruction
comparison. No byte masks, symbol-only equality or game execution are used.

This is positive evidence for continuing the ownership investigation, not
enough to migrate the canonical inventory. The next discriminator is whether
the same complete declaration also recovers the map-cell matrix-base chain
and preserves all other banked consumers. No production C, identity, claim,
relocation or baseline is changed, and no new exact function is banked.

## Verification checkpoint

The restored canonical OPEN sources were rebuilt, with unlit and floor-item
strict scores unchanged at 93.455620% and 98.795180%. All 398 workspace tests
pass without skips, as do Ruff and `git diff --check`. The staged
`nix flake check -L` passes all checks (398 tests, 48 optional controls skipped
in isolation). Full `kf build` still fails the existing strict data gates for
one OPEN and thirteen GAME units, plus the four GAME historical-best deficits.
OPEN remains 91/108 exact; no GAME reconstruction or baseline is changed.

## Map-owner extension: Function Match Plan

At `0794850`, the six semantic queries, full disassembly/CFG, callers,
adjacent boundaries, source history and shared declarations were inspected
again for both map functions. `opening_scene0_render_frame` calls traversal
without arguments; traversal supplies signed column/row and a byte cell to
the renderer. The supplied LIBGTE archive independently attributes matrix
operations to `MTX` and `RotTrans` to `SMP`; their authentic header signatures
remain unchanged. Grid selection and traversal are OPEN policy, not those
library bodies. Neither function references strings or unresolved candidate
relocations.

| OPEN function | Retail size; strict baseline | Pre-edit evidence and hypothesis |
| --- | --- | --- |
| `80018bbc render_map_cell` | `0x1d0`; 84.094826% | Nine calls, nine reviewed address pairs, one internal jump, 80-byte frame. Byte attribute wraps before the unsigned 99 check; orientation selects 32-byte matrices; positions use 2000, -100 and unsigned low halfwords of view coordinates. At `80018d2c..80018d5c`, the quadrant base `8006e0c8` supplies view at -128 and light matrices at +128. Test whether the complete owner recovers that cross-field chain without changing coordinate expressions or call delay-slot semantics. |
| `80018d8c opening_render_map_cells` | `0x140`; 84.062500% | Two calls, nine reviewed address pairs, one internal jump, 48-byte frame. Signed high yaw byte selects one of sixteen 204-byte windows. Retail reloads the stored active pointer at `80018dec/80018df0`; canonical code omits it. Row/column counters narrow to bytes, skipped rows advance by full halfword width, and unsigned bounds are 100. Test the same owner declaration, including its active-window field, without manufacturing a reload. |

Create only an alternate map-unit source with real owner-field expressions.
Remove its separate active-window definition from that temporary source;
the window table at `800439d8` remains independently owned. Rebuild and audit
numeric relocation targets, call order, first raw divergence and the exact
initializer controls. A positive map result must precede the wider banked-
consumer audit; no inventory migration or exact claim follows from a fuzzy
score or a raw-only candidate comparison.

### Map extension result

The same complete declaration makes `opening_render_map_cells` equal all
320 linked retail bytes. Both calls, all nine absolute data targets, the
active-pointer reload, branch targets and delay slots agree without changing
its C statements. The old 312-byte code hoisted the view-cell reads and
omitted that reload. This is a full numeric-relocation comparison, not a
banked strict objdiff result for the unadopted root identity.

`render_map_cell` recovers the quadrant/view/light chain and the exact saved
register set. Its last 96 bytes, starting with the quadrant-base load through
the return delay slot, equal retail after relocation. All nine calls retain
their ordered targets and the extra independent light-array pair disappears.
It is still 472 bytes versus retail 464 (canonical 468). Ignoring early branch
displacement changes caused by that size difference, the first divergence is
the empty delay slot at `80018c1c`: retail loads constant one there, whereas
the candidate loads it later into `a2`. Coordinate arithmetic and view-load
interleaving also differ. No new source-order or register-steering experiment
is justified by that symptom alone.

The initializer, allocator and color-matrix controls still preserve every
linked instruction. The regression test now also checks the complete map
traversal, the renderer's exact suffix and its literal ordered calls/data
targets; an incorrect root address fails the traversal comparison. The two
map verdicts are **raw-exact candidate traversal** and **partial renderer with
recovered shared base**, respectively. Canonical sources remain untouched.

The next required step is the broader banked-consumer audit, starting with
the display/TMD/projection unit. There are seventeen OPEN source files with
references to candidate members; they must share this declaration if the
ownership migration is adopted. Do not migrate only the newly raw-exact
traversal while retaining incompatible separate globals elsewhere.

### Map checkpoint verification

The canonical map unit was freshly compiled with its pinned manifest profile;
its strict scores remain 84.094826% and 84.062500%. All 399 workspace tests
pass without skips, Ruff and `git diff --check` pass, and `nix flake check -L`
passes (399 tests, 49 optional controls skipped in isolation). Full `kf build`
still fails only the recorded strict gates: one OPEN and thirteen GAME data
units, plus four GAME historical-best deficits. OPEN remains 91/108 banked
exact, with no production source, inventory or baseline changed by this pilot.

## Display/TMD/projection regression plan

At `1673dbe`, retail was hash-validated and all six semantic queries, full
instructions/CFG, signature-constraining call sites, neighboring boundaries,
source history and shared SDK/data types were inspected for `open.render`.
The shared slot/vertex state and confirmed select/project/enqueue call chains
justify this next consumer set; adjacency alone does not establish a source
file boundary. The sole literal is the existing projection overflow message.
The preparation dispatch has 29 candidate pointer rows, not 29 proven calls.

| OPEN function | Size; strict baseline | Pre-edit constraints |
| --- | --- | --- |
| `80016d38 display_begin_frame` | `98`; 100% | No arguments; one call, seven address pairs, 24-byte frame. Byte buffer toggle, 12-byte allocator records, 0x4000-entry OT, three counter resets. |
| `80016dd0 display_present_frame` | `98`; 100% | No arguments; five calls, four pairs, 24-byte frame. SDK draw/display environment strides 92/20; final OT index 0x3fff. |
| `80016e68 tmd_select` | `2c`; 100% | Halfword slot mask, two pairs, eight-byte frame, no calls; selected pointer stored before return. Confirmed callers select zero or one. |
| `80016e94 tmd_get_object` | `24`; 100% | Halfword index, 28-byte record stride plus 12-byte header, one pair; pointer addition in return delay slot. |
| `80016eb8 tmd_set_current_vertices` | `10`; 100% | Pointer store, one pair, no frame/calls, return with nop; no confirmed caller. |
| `80016ec8 tmd_select_object_vertices` | `3c`; 100% | Halfword index passed in call delay slot; one call, two pairs, 24-byte frame; word vertex offset relative to asset plus 12. |
| `80016f04 render_set_view_transform` | `12c`; 100% | Nullable VECTOR/SVECTOR pointers; word position and halfword-aligned rotation copies; signed division by 2000; two calls, seven pairs, 32-byte frame. |
| `80017030 tmd_prepare_primitive_indices` | `300`; 98.333336% | No direct calls, four pairs, seven internal jumps and one indirect dispatch; eight-byte retail frame. Word disk counts narrow to halfwords, mode mask 0xfd, eight packet types, halfword indices shifted three. |
| `80017330 tmd_register` | `3c`; 100% | Halfword slot and asset pointer; one call, one pair, 24-byte frame; current-asset store in preparation-call delay slot. |
| `8001736c tmd_release_last_allocation` | `20`; 100% | One allocator call, no pairs, 24-byte frame. No confirmed callers or reads of the tentative slot parameter; retain candidate signature status. |
| `8001738c tmd_project_vertices` | `cc`; 100% | Signed count, 1000 limit, three calls, four pairs, one internal jump, 56-byte frame. Eight-byte cursors, signed global shift, SDK output pointers, low-halfword depth. |
| `80017458 tmd_project_vertices_perspective_right` | `a4`; 100% | Word count from object; two calls, three pairs, 64-byte frame. Signed perspective shift right; count-to-minus-one loop and eight-byte cursors. |
| `800174fc tmd_project_vertices_shift` | `ac`; 100% | Signed count/byte shift from body; no confirmed callers. Two calls, two pairs, 64-byte frame; low-halfword perspective doubled, full-word depth shifted arithmetically. |
| `800175a8 tmd_transform_vertices` | `a4`; 100% | Signed count from body, no confirmed callers; one call, two pairs, 72-byte frame. Non-perspective output halfwords and duplicate low depth loads. |

Sizes above are hexadecimal. The return delay slots and load delays are part
of each control, including otherwise unused retail frames. SDK providers are
independently identified: LIBGPU `SYS`/`VSYNC`, LIBGTE `REG`/`SMP`/`GEO`.
The archive/header checks keep their actual APIs and the evidenced two-pointer
`ReadSZ2` compatibility declaration; they do not turn these OPEN array loops
or buffer policy into library work. LIBGPU `TMD` owns different OpenTMD/ReadTMD
services, not this in-place index preparation.

Apply only real field expressions from the existing complete candidate to
one alternate copy of the whole unit. Remove the separate projected-array
definition from that copy, retaining the genuine 1000-record owner field.
Preserve every algorithm, signature, literal, branch and compiler option.
Compare all thirteen banked exact functions after resolving the literal root
and every REL addend; retain the partial preparation as a separate verdict.
One exact-consumer regression blocks migration pending an independently
evidenced explanation, even if the map traversal remains raw-exact.

### Display/TMD/projection result

All thirteen exact functions in the table retain every linked retail word,
including ordered calls, all physical data targets and the eight-byte
`tmd_select` frame. Their final verdict is **raw-exact preservation**.
`tmd_prepare_primitive_indices` retains the canonical 768-byte candidate and
the same four differing words: the entry/exit frame operations and the
primitive-count branch/packet-add ordering at offsets `3c/40`. It remains
partial; no slot-layout change explains that residue. The exact initializer,
allocator, color-matrix and map-traversal controls still pass.

The committed test constructs the complete alternate renderer unit with the
same shared header and checks all thirteen exact claims against retail. A
wrong-root control on frame setup fails, as does the existing initializer
wrong-root control. This is not yet an inventory migration or a strict
objdiff claim for the hypothetical root.

## Remaining banked-consumer plan

Continue the same field-only experiment in the remaining eight consumer
units. The six-query snapshots, full retail bodies, source/history, SDK
headers and archive ownership are refreshed before their temporary copies
are made. Signature-constraining callers and adjacent boundaries are checked
separately. No new algorithm, API, material layout or instruction steering
is introduced. The controller's clear is spelled against the complete owner
and `sizeof` that owner, using the independently proved `0x24788` extent.

| OPEN function | Hex size; strict baseline | Pre-edit constraints |
| --- | --- | --- |
| `80013804 sprite_add_g4` | `198`; 100% | Rectangle and four byte-color pointers, fifth argument on O32 stack; two calls/two pairs, 48-byte frame; 36-byte SDK packet and OT index 0x3fef. |
| `8001399c sprite_add_ft4` | `1c0`; 98.973210% | Rectangle/texture pointers, halfword page/CLUT, stack color/index; two calls/two pairs, 48-byte frame; 40-byte packet, byte UV fields at source offsets 0/2/4/6, 14-bit OT index. |
| `80013b5c sprite_add_f4` | `114`; 100% | Rectangle/color pointers and halfword OT index; two calls/two pairs, 40-byte frame; 24-byte packet and 14-bit OT index. |
| `800143dc opening_scene1_draw_fade` | `180`; 100% | Byte shade; eight calls/three pairs, 32-byte frame; two 40-byte packets, 192/128/320/240 coordinates, two OT-pointer reloads around AddPrim. |
| `80014b34 opening_ending_scene_run` | `2f4`; 100% | No arguments; twenty calls, 23 pairs, two internal jumps, 72-byte frame; signed halfword brightness/blend and separate SDK draw-environment RGB stores. |
| `800156bc opening_run` | `214`; 100% | Word display mode, values 1/0xfe; 31 calls, twelve pairs, four internal jumps, 40-byte frame; three resource strings and independent graphics/entity clears. |
| `800189a0 render_enqueue_sprite` | `21c`; 100% | Typed 12-byte sprite, signed halfword depth bias, word flag; six calls/five pairs, 136-byte frame; packed screen words, SDK material, biased depth >=5. |
| `80018ecc opening_entity_render` | `228`; 100% | Typed entity pointer; 22 direct calls, six pairs, six internal jumps and indirect dispatch, 136-byte frame; seventeen candidate table rows, IDs 11..27, rotation +/-64 and signed depth biases. |
| `800190f4 render_floor_item` | `14c`; 98.795180% | Typed 24-byte item; nine calls/six pairs/one jump, 80-byte frame; facing nibble plus retail byte mask, 150/200 biases and wrapped animation byte. |
| `80019240 opening_render_entities_and_items` | `298`; 92.867470% | No arguments; four calls/fifteen pairs, 48-byte frame; 32 entities, halfword-wrapped window bounds, signed /2000 item coordinates, RGB180, page-to-pool base chain. |
| `80019658 fog_interpolate_near` | `44`; 92.941180% | Three signed words, no confirmed callers; one call/one pair, 24-byte frame; 12-bit fixed-point interpolation and SDK screen constant 200. |
| `8001969c fog_set_near` | `28`; 100% | Word distance, no confirmed callers; one call/one pair, 24-byte frame; store distance and SDK screen constant 200. |

The unchanged frame wrapper, scene-1 controller and five matrix/color helpers
in these units receive byte-preservation checks too; their bodies do not
reference the proposed owner and are not rewritten. LIBGPU `PRIM`, LIBGTE
`SMP`/`CMB`/`MTX`/`GEO`, the API memset entry and PAD functions remain separate
vendored services. The real SDK `setRGB0` macro is three byte stores, not an
invented packed store. No canonical data definition changes during this audit.

### Remaining banked-consumer result

All eight already-exact owner consumers in the table retain every linked
retail instruction, as do all seven unchanged helpers. In particular,
`opening_run` clears the complete object with `sizeof` and still reproduces
the literal destination, extent, calls and delay slots. Each of these fifteen
functions has a **raw-exact preservation** verdict; the regression test covers
every one with its own retail comparison.

The four partial consumers retain their previous candidate instructions:
`sprite_add_ft4` is 448 bytes with the same register differences starting at
offset `28`; `render_floor_item` remains 328 versus 332 bytes, missing the
byte mask at `90`; `opening_render_entities_and_items` remains 660 versus
664 with the same early entity/window load ordering; and
`fog_interpolate_near` remains 64 versus 68 with the missing initial move.
Calls and physical referents remain unchanged. Their final verdict is
**unchanged partial**, not a new instruction-selection hypothesis.

Together with the display/TMD and initializer controls, all 31 banked-exact
functions across ten consumer units survive the complete-owner declaration.
The map traversal remains raw-exact too. The remaining nonbanked consumer
units and canonical data ownership still require audit before adopting this
single owner throughout OPEN and checking strict objdiff.

## Canonical migration: Function Match Plan

The final five consumers received the six-query/raw-CFG, caller, adjacent-
boundary and source-history pass at `08018b2`. Their common hypothesis is
only the already-reviewed complete graphics owner; algorithms and SDK types
remain unchanged. LIBGPU PRIM/SYS and LIBGTE SMP provide the called graphics
services; PAD, rand and audio services remain vendored, not new game bodies.

| OPEN function | Hex size; strict baseline | Snapshot and first hypothesis |
| --- | --- | --- |
| `8001764c render_enqueue_tmd` | `cf8`; 98.802410% | 58 calls, eleven internal jumps, eleven address pairs, 96-byte frame, 29 candidate switch rows. Halfword object index and signed bias from entity callers; current asset +32 yields projected vertices and projected base -276 yields OT. Preserve packed header and every polygon case; test actual owner fields. |
| `800185e8 render_enqueue_map` | `3b8`; 97.449580% | Eighteen calls, one jump, two pairs, 80-byte frame. Halfword object from cell renderer; cases 0x24/0x2c, depth +200 and <16384; same asset/projected/OT chain. Test owner only. |
| `800197e4 item_load_floor_placements` | `1b0`; 97.731480% | One rand call, three pairs, 40-byte frame; resource caller passes payload pointer. 12-byte input/24-byte output, ffff sentinel, byte tile positions, signed local positions and 2000/100 scaling. Retain independent count/items address formation under the complete owner. |
| `8001a82c display_adjust_vram_view` | `248`; 95.678085% | Nine calls, eleven pairs, one jump, 40-byte frame; no confirmed callers. Genuine SDK RECT copy and environment strides, byte flags, pad masks, four-pixel steps, 1024/512 wrap. Preserve all behavior under owner fields. |
| `80014e28 opening_ending_scroll_run` | `798`; 97.129630% | Forty calls, eight jumps, 31 pairs, 264-byte frame, infinite frame loop with retained epilogue. No-argument controller call; signed phase/blend halfwords, nine pages/CLUTs, live shared RGB bytes and shift=2. Owner-only change; no phase or stack steering. |

Compare these alternate units, then adopt one `KfGraphicsRuntimeOpen` in
`open/render.h` throughout all seventeen OPEN consumers. Define its BSS
storage in the render-initialization module, remove the three old interior
DATA claims and fourteen interior global identities, and retain every raw
relocation site and numerical destination. This is a supported WIP ownership
model, not recovered original symbol names or a TU boundary. Move GAME-only
projected-state declarations out of the common TMD interface without changing
GAME definitions or instructions. Require strict 100% for traversal and all
previously exact consumers before banking.

### Canonical migration verdict

Adopted the complete owner in all seventeen OPEN consumers and its real BSS
definition in `render_init.c`. All 170 affected relocation rows retain their
sites, instruction forms, numeric destinations and evidence status; only the
owner name changes. Fourteen global identities become nineteen typed fields
(including five opaque intervals), with no overlapping interior definitions.
The common TMD header now leaves GAME's two globals in `game/render.h`;
GAME source bodies and identities are unchanged. The inventory checker knows
the authentic LIBGPU DRAWENV/DISPENV layouts and reads the shared vertex type
before the enclosing OPEN type. The former alternate fixture is retired.

| Function | Final strict result | Linked-instruction verdict |
| --- | --- | --- |
| `opening_render_map_cells` | **100%**, banked | All 320 bytes exact, including the active-window reload and both calls. |
| `display_adjust_vram_view` | **100%**, banked | All 584 bytes exact; shared owner eliminates the extra saved register and restores the separate draw-environment reference. |
| The 31 previously exact consumer functions listed above | **100% each** | Every linked instruction remains exact under the same canonical declaration. |
| `display_initialize` | 92.177960% | 484 versus 472 bytes; recovered display-environment sharing, unchanged calls/writes, frame and DFE-address residue. |
| `render_enqueue_tmd` | 98.598790% | 3324 versus 3320; current-asset/projected base sharing recovered, all 58 calls unchanged; frame, OT load and four-byte default-table-target residue remain. |
| `render_enqueue_map` | 97.890755% | 956 versus 952; projected base recovered, all eighteen calls unchanged; first divergence is the normal-pointer stack slot at +58, followed by load ordering. |
| `render_enqueue_unlit_triangles` | 93.071010% | 688 versus 676; asset/projected/material sharing recovered; narrow bias spill and separate OT load remain. |
| `render_map_cell` | 87.922420% | 472 versus 464; exact matrix suffix and all nine calls; earlier coordinate/load-order residue remains. |
| `item_load_floor_placements` | 97.731480% | Unchanged 424 versus 432 bytes and ordered targets; first register difference at +20. |
| `opening_ending_scroll_run` | 97.129630% | Unchanged 1944 bytes, forty calls and numeric referents; first ordering difference at +298. |

The remaining partial consumers retain the verdicts recorded above. The two
slightly lower emitter percentages do not override the independently evidenced
complete owner; neither was banked exact, and no exact function regresses.

OPEN advances **91 -> 93 / 108 exact** (15 partial, none unstarted), with
98.798% aggregate fuzzy similarity. Full `kf build` and the keep-going graph
check retain the existing one OPEN / thirteen GAME strict data failures and
four GAME historical-best deficits. The OPEN failure is still the TMD table's
seventeen default addends, +ca0 versus retail +c9c; no gate is weakened.
All 401 workspace tests pass without skips, Ruff and `git diff --check` pass,
and `nix flake check -L` passes (58 optional controls skipped in isolation).
Only the two newly strict-exact functions are banked.
