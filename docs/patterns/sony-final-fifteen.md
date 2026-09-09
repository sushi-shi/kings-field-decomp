# SDK expressions and the final fifteen functions

## Function Match Plan

Starting point: `66af94a4`, 456/471 strict-exact functions. This campaign's
objective is to make all fifteen remaining functions strict 100% using the
SDK-expression, data-width and source-boundary approach that closed the
three reflected-menu functions. An investigated but non-exact row remains
unfinished. The previous thirty controls and their raw rejection evidence
are recorded in [the eighteen-function investigation](sony-remaining-functions.md).

Refresh the current roster, retail hashes, image-qualified semantic views,
call sites, neighboring functions, source histories and resolved object
controls before editing. Every hypothesis must preserve actual object
ownership, API types, calls, constants, arithmetic results and delay-slot
dependencies. Keep all banked functions exact. No artificial frame storage,
unused coordinates, forced calls, assembly or compiler permutations.

The first related family comprises the remaining frame-only differences:

| Image / VA | Function | Starting strict % | Evidence-backed next control |
| --- | --- | ---: | --- |
| GAME `80018880` | `player_update` | 99.96709 | SDK SVECTOR launch/effect construction and explicit component widths; frame 216 versus retail 224. |
| GAME `8001e5ec` | `render_map_cell` | 99.87838 | `setVector` inputs in the halfword domain proved by retail camera-coordinate `lhu` reads; frame 88 versus retail 120. |
| GAME `80038298` | `effect_projectile_update_2d` | 99.93421 | Authentic vector/scalar boundaries around orbital coordinates and distance calculations, preserving sin/cos/sin call order; frame 56 versus retail 120. |

The full campaign retains these other twelve targets and their image identity:
GAME `player_move_horizontal` (`800171fc`), `render_entities` (`8001f218`),
`item_load_database` (`80020cfc`), `menu_draw_item_detail` (`80027b7c`),
`talk_show_dialogue_page` (`8002c9d4`), `map_object_spawn_effect` (`80031834`),
`map_show_screen_image` (`80034d54`), `map_interaction_dispatch` (`80034de4`),
`map_world_state_persist` (`80035b5c`), `effect_map_collision` (`80037850`),
`effect_update_dispatch` (`80038a38`), and OPEN `opening_ending_scroll_run`
(`80014e28`). Their earlier helper, call, referent and raw-difference dossiers
are controls, not grounds to assume a new source shape.

Each new source fact is first tested in an alternate complete unit with its
existing pinned profile. Compare resolved instruction words and ordered
relocations from the first real difference, including frame operands when
the displayed percentage is unchanged. Compose only independently supported
facts. Record results here as they become verified; require focused rebuilding,
full build, repository checks and exact-only banking before each commit.

## First follow-up results

All fifteen retail dossiers were refreshed against the current tree, including
six semantic views per function, seventeen distinct callers, thirty-two call
sites, source history, adjacency and complete resolved instruction words.
Each delinked target resolves back to its retail bytes. All fifteen retain
their game classification; none contains COP2, LWC2 or SWC2 instructions.
Every ordered call list still agrees. The existing repeated-reference residue
in `player_move_horizontal` and exchanged floor/path materializations in
`map_show_screen_image` remain, with no missing or invented referent identity.

Twenty new alternate-unit controls compile. Nine reproduce the complete
starting object; eleven change instructions or object layout without closing
a function. No production source or bank entry is changed. **456/471 remains
the strict count, and all fifteen targets remain unfinished.**

| Control | Function | Strict % | Bytes | Result |
| --- | --- | ---: | ---: | --- |
| Camera inputs explicitly unsigned halfwords | `render_map_cell` | 99.87838 | 592 | Whole object unchanged |
| Both relative-coordinate operands in the halfword domain | `render_map_cell` | 99.87838 | 592 | Whole object unchanged |
| Complete setter inputs converted to SDK `short` | `render_map_cell` | 99.87838 | 592 | Whole object unchanged |
| World coordinates followed by relative-coordinate setter | `render_map_cell` | 83.63513 | 604 | Reject changed instructions and referent order |
| Inverse camera origin followed by cell coordinates | `render_map_cell` | 91.02702 | 612 | Reject changed instructions and referent order |
| Reflected camera inputs as unsigned halfwords | `player_update` | 99.96709 | 6684 | Whole object unchanged |
| Reflected rotation inputs converted to SDK `short` | `player_update` | 99.96709 | 6684 | Whole object unchanged |
| SDK limits for yaw | `player_update` | 99.21604 | 6656 | Reject changed branches/stores |
| SDK limits for pitch step | `player_update` | 99.52424 | 6680 | Reject changed branches/stores |
| SDK limits for camera pitch | `player_update` | 99.844406 | 6688 | Reject changed branches/stores |
| SDK limits for forward movement | `player_update` | 98.95272 | 6640 | Reject changed branches/stores |
| SDK limits for strafe movement | `player_update` | 98.95272 | 6640 | Reject changed branches/stores |
| Actual horizontal components in `DVECTOR` | `player_update` | 98.95871 | 6716 | Retail frame recovered, but eight instructions added; reject |
| Triple Fang inputs as unsigned halfwords | `player_update` | 99.96709 | 6684 | Whole object unchanged |
| Complete Triple Fang pitch expression converted to SDK `short` | `player_update` | 99.96709 | 6684 | Whole object unchanged |
| Existing two-component collision delta in `DVECTOR` | `player_move_horizontal` | 96.56322 | 2128 | Three local offsets change; retail's Z slot is lost; reject |
| SDK copy of spawn position, then vertical offset | `map_object_spawn_effect` | 89.95049 | 408 | Two instructions added and register roles changed; reject |
| Complete typed glyph-row assignment | `menu_draw_item_detail` | 73.39891 | 892 | Halfword loop replaced by unrolled copy; reject |
| SDK `long` distance components | `effect_projectile_update_2d` | 99.93421 | 608 | Whole object unchanged |
| Complete squared-distance expression at `SquareRoot0` argument | `effect_projectile_update_2d` | 99.93421 | 608 | Whole object unchanged |

The clamp controls use the supplied `LIBGPU.H` definition and `LIMITS.H` type
bounds. `limitRange(x, SHRT_MIN, upper)` and
`limitRange(x, lower, SHRT_MAX)` preserve the actual one-sided cap for each
signed-halfword result. They do not introduce a second reachable cap. Their
changed conditional stores and joins, rather than their percentages alone,
reject them. All five retain the 216-byte frame.

The `DVECTOR` control in `player_update` contains exactly the two consumed
horizontal components; magnitude remains scalar. It reaches the retail
224-byte frame but grows the body from 6684 to 6716 bytes. This independently
repeats the earlier three-component motion-owner rejection: matching frame
size does not validate a source owner. In `player_move_horizontal`, changing
the existing delta to a two-component owner only moves the three Z-component
stack operands from `sp+28` to `sp+26`, away from retail. Neither is retained.

Every trial preserves its existing ordered calls and all unaffected sibling
bodies. Resolved audits retain each candidate's exact reference list, including
interior offsets: the unrolled glyph assignment changes them as expected for
that rejected copy form. The other candidates preserve the set of referenced
addresses, even where an altered expression changes order or repetition.

## What the successful UV control actually reserves

The preceding three exact menu functions provide a real matching control for
investigating the remaining frame symptoms. A fresh before/after comparison
of `menu_draw_window_backdrop` uses the same pinned profile and complete unit:

| Observation | Original UV assignments | Reflected SDK `setUVWH` |
| --- | ---: | ---: |
| Strict result | 99.97176% | 100% |
| Body size | 1700 | 1700 |
| Frame size | 40 | 104 |
| Positive-size automatic allocations during expression expansion | 0 | 0 |
| Additional allocations during reload | 0 | Eight 8-byte slots in QI mode |
| Accesses to those additional slots in final code | 0 | 0 |

For both states, recompilation with the repository's observational compiler
produces an object byte-identical to the native probe. The trace records the
eight QI intermediate values through FLOW. Their uses disappear at COMBINE;
the GLOBAL-ALLOC records still report three references for each, with no
assigned hard register. RELOAD then records eight allocations and the
64-byte frame increment. The final instructions contain no accesses to
these slots. This is an observation about the pinned probe and these actual
SDK expressions, not an attribution of the historical compiler.

The supplied probe source is consistent with those events: `reload1.c`
invokes `alter_reg` for pseudo registers, and `alter_reg` can allocate a stack
slot when its reference count remains positive and it lacks a hard register
or equivalent. No compiler source, option or profile is changed in this
campaign. No artificial local is introduced to reproduce those allocations.
The observation narrows future work to real expression and width facts; it
does not justify adding an unused matrix or padding to another function.

## Per-function verdict after these controls

| Image / function | Strict % | Remaining evidence and next constraint |
| --- | ---: | --- |
| GAME `player_move_horizontal` | 96.56896 | Retail uses the existing SVECTOR Z slot; DVECTOR loses it. Direction register, later branches and repeated materializations remain unresolved. Preserve both signed-halfword collision components and trig-call order. |
| GAME `player_update` | 99.96709 | Still only stack operands: 216 versus 224 bytes. Compatible setter widths are neutral; the live DVECTOR adds instructions; SDK clamps change exact branches. An explanation must preserve all 6684 bytes outside those operands. |
| GAME `render_map_cell` | 99.87838 | Still only stack operands: 88 versus 120 bytes. SDK input widths are neutral; staged coordinate construction changes instructions. Preserve direct RotTrans output into MATRIX translation. |
| GAME `render_entities` | 99.517044 | Same culling-origin and loop-register residue. Its two-axis cell/window predicates have no three-component SDK setter counterpart; CVECTOR color members also do not satisfy packet setter field names. No fabricated third coordinate. |
| GAME `item_load_database` | 99.746666 | Same decimal-division registers. Existing memory helpers are present; ASCII digits are not BCD. Preserve the established 20-byte CD-file entry and complete call set. |
| GAME `menu_draw_item_detail` | 97.80875 | Three glyph-cursor/setup words remain. Typed whole-row assignment changes the ten-halfword loop into an unrolled copy. Preserve the established glyph object and raw store width. |
| GAME `talk_show_dialogue_page` | 98.78049 | Same decimal remainder/register sequence. The supplied character/BCD helpers do not express the required ASCII digit stores. |
| GAME `map_object_spawn_effect` | 94.50495 | Same saved-register, instruction and four-byte extent deficit. SDK copy-plus-offset adds two instructions; rotation setter still conflicts with pre-rand clears. |
| GAME `map_show_screen_image` | 88.888885 | Same remainder register, floor/path ordering and extra load-delay nop. Its only call remains the game screen API. |
| GAME `map_interaction_dispatch` | 99.202774 | Same pointer, reload and branch residue. Both trig probes consume only X/Z; the modular pitch predicate is not a saturation clamp. |
| GAME `map_world_state_persist` | 97.52873 | Same definition-base and type-predicate residue. Sparse byte serialization has no supported SDK replacement; the earlier memcpy control changes its byte loop. |
| GAME `effect_map_collision` | 99.87369 | One unequal delay-slot word remains at +0x718. It belongs to masked game collision dispatch; no missing SDK call or expression has been established. |
| GAME `effect_projectile_update_2d` | 99.93421 | Still only stack operands: 56 versus 120 bytes. SDK word types and direct squared-distance argument are whole-object neutral. The actual vector aggregate and captured trig inputs remain rejected by extra instructions. |
| GAME `effect_update_dispatch` | 99.82781 | Same entry/type and later register residue. Existing compatible vector helpers remain; previous reordered setters, complete memcpy and clamp controls disagree with retail. |
| OPEN `opening_ending_scroll_run` | 99.917694 | Same phase-dispatch registers and join target. Its transition setter is already retained; the signed blend is not equivalent to a new two-sided clamp. |

Local dossiers, twenty alternate sources, native objects, complete resolved
trial audits and the parity-checked UV traces are under
`build/sony-final-15/`. They are generated evidence and are not committed.

## Integration and verification

The campaign was integrated onto `b9d490c0`, retaining the concurrent menu,
renderer and runtime data-owner repairs. A fresh compile of its current units
resolves all fifteen remaining bodies to exactly the same words, ordered calls
and references as the starting campaign. The three previously closed UV
functions also remain strict 100% and equal to complete raw retail. They now
reside together in `game.menu_runtime`; the older separate-unit names identify
the controls' starting ownership, not the current manifest.

The integrated full build regenerates the graph and recompiles its changed
units. Function counts remain PSX 1/1, GAME 348/362 and OPEN 107/108. It exits
nonzero for the remaining data/ownership/placement failures: data owners
PSX 1/1, GAME 29/41 and OPEN 12/20, with zero artifact failures; target relink
PSX 1/1, GAME 57/64 and OPEN 34/38. Those data-owner changes belong to the
concurrent campaign and are preserved here.

Ruff, Rust tests and whitespace checks pass. Of the integrated Python suite's
775 cases, 770 pass and five optional cases skip. The source-owner test needed
a retry after the merged-unit target objects were regenerated; that retry
passes, including all 26 subtests. No source, profile, relocation or banking
change is retained by this follow-up.

## World-coordinate boundary follow-up plan

Continue from `2f24a5cf` with GAME `render_map_cell`. Its retail matrix remains
at `sp+16`, while its SVECTOR input and flag are each 32 bytes above the current
object. This differs from a uniform shift of all automatic objects and does
not by itself identify the missing source object. The actual inputs comprise
three world coordinates and the existing camera VECTOR; all six values are
consumed by the relative-coordinate calculation before narrowing to SVECTOR.

Test one complete world VECTOR built with `setVector`, and separately a
complete camera VECTOR captured with `copyVector`. These controls introduce
only coordinates used in that calculation, keep padding untouched and retain
every matrix/GTE API boundary. A combined control is meaningful only if both
individual input captures preserve the arithmetic and object identities.
Compare their complete words, frame operands, references and siblings under
the unchanged profile; an increased frame alone cannot validate either form.

### Boundary results and observed frame layout

Both controls are rejected. They preserve the call list, referent identities
and exact sibling but change reference order and introduce stack traffic absent
from retail. The world capture emits three word stores followed by halfword
reloads; the camera capture additionally replaces the original camera halfword
reads with word loads. Combining those independently contradicted memory
boundaries is not supported.

| Control | Strict % | Body bytes | Frame bytes |
| --- | ---: | ---: | ---: |
| Existing source | 99.87838 | 592 | 88 |
| World VECTOR built with `setVector` | 82.560814 | 612 | 104 |
| Camera VECTOR captured with `copyVector` | 91.12838 | 620 | 104 |
| Retail | 100 | 592 | 120 |

Five further observational compilations cover the three frame-only functions
and these two controls. Every complete object equals its normal native-probe
counterpart byte for byte. Positive-size `stack.allocate` events distinguish
the actual local objects from reload allocations:

| Function / control | Expansion allocations | Reload allocations |
| --- | --- | --- |
| `player_update` | Three 8-byte aggregates, one 16-byte aggregate, one 32-byte aggregate, one 4-byte word | Nine 8-byte slots, SI mode |
| `render_map_cell` | One 32-byte aggregate, one 8-byte aggregate, one 4-byte word | One 8-byte slot, SI mode |
| `effect_projectile_update_2d` | None | None |
| Each rejected cell capture | The cell objects plus one 16-byte aggregate | One 8-byte slot, SI mode |

The cell matrix starts at `sp+16` in both retail and current output. The input
SVECTOR starts at retail `sp+80` versus current `sp+48`; the flag follows at
`sp+88` versus `sp+56`. The observed reload allocation follows those expanded
objects. Under this probe and these declarations, adding reload allocations
like the UV control would not insert the missing region between the matrix
and SVECTOR. That location constrains further source recovery; it is not proof
of a second matrix or any other unused object.

The player's nine slots use SI mode, unlike the successful UV control's eight
QI slots. The orbit function has no positive-size automatic allocation at all.
Neither observation identifies a missing SDK expression. All fifteen final
verdicts above remain unchanged; the two additional trials bring this campaign
to twenty-two controls with no further exact function. Generated evidence is
under `build/sony-final-15/vector-boundaries/` and `frame-trace/`.

Fresh verification after this follow-up passes all 775 Python tests and 9,223
subtests, Ruff and whitespace checks. The full build retains the same twenty
data-owner mismatches and zero artifact failures. Production sources, profiles,
inventories and banking inputs remain unchanged from `2f24a5cf`.
