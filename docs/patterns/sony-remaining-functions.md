# Sony helper investigation of the 18 remaining functions

## Function Match Plan

This campaign starts from master `48ff2dc8` and covers every non-exact function
in that build: seventeen in GAME and one in OPEN. Its scope is supplied Sony
SDK helpers, types, macros and library boundaries. The
[previous helper pass](sony-helper-near-misses.md) supplies tested controls;
generic source permutations and compiler-profile searches are outside this
campaign.

Before source trials, `kf init` validates all three retail images. The complete
evidence pass captures the six image-qualified semantic views for all eighteen
functions, their neighbors and histories, and 41 direct call sites in 22
callers. A separate audit resolves target-object relocations back to retail
and compares complete bodies. All eighteen target controls reproduce retail,
all existing ordered call sequences agree, and none of these game bodies has
COP2, LWC2 or SWC2 instructions. This excludes missing inline GTE instructions
within these claimed extents; direct matrix/trigonometric APIs remain calls.

The primary helper definitions and SDK provenance are in the
[header census](sony-header-helpers.md). `MEMORY.H` declares memory functions;
the current item loader demonstrates that this compiler can inline `memcpy`.
Consequently, call absence alone does not reject a compiler-recognized memory
helper. New copy controls compare the actual emitted loops and access widths.

| Image / VA | Function | Initial strict % | Helper investigation |
| --- | --- | ---: | --- |
| GAME `800171fc` | `player_move_horizontal` | 96.56896 | Pure signed X/Z absolute values; retain the observed trigonometric call order and halfword direction narrowing. |
| GAME `80018880` | `player_update` | 99.96709 | Unsigned magic-charge clamp and camera vector-copy forms; existing one-sided velocity limits are not symmetric clamps. |
| GAME `8001e5ec` | `render_map_cell` | 99.87838 | Authentic `RotTrans` output ownership and existing SVECTOR construction; preserve MATRIX translation/rotation call order. |
| GAME `8001f218` | `render_entities` | 99.517044 | CVECTOR versus packet RGB helpers, two-axis culling and existing matrix calls. |
| GAME `80020cfc` | `item_load_database` | 99.746666 | Existing compiler-inlined memory copies, SDK CD file boundary, decimal filename construction versus BCD helpers. |
| GAME `8002430c` | `menu_status_panel` | 99.96217 | Reflected UV rectangle macros and genuine RECT input capture inside the existing inline draw helper. |
| GAME `80027b7c` | `menu_draw_item_detail` | 97.80875 | SDK memory copy of the ten-glyph row, matrix zero-row initialization, and actual MATRIX field extents. |
| GAME `800292f8` | `menu_draw_item_name_frame` | 99.97571 | Glyph memory copy, reflected UV rectangles and matrix initialization. |
| GAME `8002a510` | `menu_draw_window_backdrop` | 99.97176 | Reflected UV rectangle macros and genuine RECT inputs. |
| GAME `8002c9d4` | `talk_show_dialogue_page` | 98.78049 | Decimal ASCII digits versus SDK BCD and character helpers; existing screen call. |
| GAME `80031834` | `map_object_spawn_effect` | 94.50495 | Existing position setter and the already tested rotation setter's pre-random store dependency. |
| GAME `80034d54` | `map_show_screen_image` | 88.888885 | Decimal ASCII digits versus BCD helpers; global floor read and real screen call. |
| GAME `80034de4` | `map_interaction_dispatch` | 99.202774 | Two-axis probes, angle predicates and call-preserving SDK boundaries. |
| GAME `80035b5c` | `map_world_state_persist` | 97.52873 | Eight-byte complete link copy versus SDK `memcpy`; sparse and packed records retain their game serialization. |
| GAME `80037850` | `effect_map_collision` | 99.87369 | Signed cell bounds, geometry predicates, effect-mask extraction and the real collision call. |
| GAME `80038298` | `effect_projectile_update_2d` | 99.93421 | VECTOR distance components and setter input capture preserving sin/cos/sin call order. |
| GAME `80038a38` | `effect_update_dispatch` | 99.82781 | Existing helpers, complete padded vector copy versus `memcpy`, and scalar scale bounds; reuse previous ordered-vector controls. |
| OPEN `80014e28` | `opening_ending_scroll_run` | 99.917694 | Transition vector, bounded blend clamp, full MATRIX copy and genuine color/material types. |

Each trial changes one source-level helper hypothesis in a complete alternate
unit under its unchanged pinned profile. Compare from the first real raw
divergence, resolving section-symbol addends before judging referents. Preserve
the SDK interfaces, widths, side-effect order, complete-object extents and every
exact sibling. Do not add unconsumed fields, artificial stack storage, incompatible
packet casts, or calls absent from retail to improve a score.

Completion requires a final verdict for all eighteen rows, results for each
plausible compiled control, reviewed raw constants/referents, focused rebuilding
of retained source, the full build, repository checks and a focused commit.
Unexplained frame sizes remain non-exact; investigation completion does not
mean all eighteen functions must become exact.

## Result

Three functions reach strict 100% through the supplied `setUVWH` helper:
`menu_status_panel`, `menu_draw_item_name_frame` and
`menu_draw_window_backdrop`. Their complete resolved instruction bodies,
ordered calls and data referents equal retail. Two further SDK substitutions
are retained with whole-object byte and relocation equality: the unsigned
charge `limitRange` in `player_update` and the transition `setVector` in
`opening_ending_scroll_run`.

All thirty compiled controls use the existing unit profiles:
`probe-gcc257-o2-g0`, except `player_update` and `save_system`, which retain
`probe-gcc257-o2-g8`. No compiler setting, function extent, identity,
relocation inventory, data owner or vendored classification changes.

### Reflected rectangles explain three frame differences

The SDK macro represents a reflected rectangle directly. For horizontal
reflection, use origin `u + width` and span `-width`; for vertical reflection,
use `v + height` and `-height`. These produce the same eight GPU packet bytes
as the former four explicit corners. The descriptor inputs are pure reads,
the packet stores retain SDK vertex order, and all screen-coordinate, page,
CLUT and primitive submission operations remain intact.

Each single reflected-tile control in the item-name and backdrop functions
changes only the frame and saved-register offsets by 16 bytes. The displayed
strict percentage stays unchanged despite these real instruction changes.
Composing all three reflected tiles makes both complete functions exact;
the resulting frame change is 64 bytes, so the isolated effects must not be
treated as an additive model of the compiler.

| Function | Initial frame | Retained / retail frame | Body bytes |
| --- | ---: | ---: | ---: |
| `menu_status_panel` | 48 | 112 | 1692 |
| `menu_draw_item_name_frame` | 160 | 224 | 1976 |
| `menu_draw_window_backdrop` | 40 | 104 | 1700 |

The status panel already has one inline helper used by all four tiles.
Replacing its corners with a reflected `setUVWH` fixes the frame but changes
eight UV extent loads from retail `lbu` to `lhu`. The source descriptor has
halfword screen dimensions; the packet has byte texture coordinates.
Narrowing only the spans prevents cancellation against the wider reflected
origins and emits extra loads/subtractions. Expressing both UV origins and
spans in the same `u8` domain restores the actual byte loads and reaches
100%. The screen dimensions keep their original halfword reads.

This narrowing preserves every possible descriptor value: replacing an extent
`w` with `w mod 256` changes either reflected corner by a multiple of 256,
which disappears at the packet-byte store. It does not rely on an assumed
small texture width. No stack object or unused local was added. These are
observed results of the SDK expressions under the pinned probe, not an
attribution of the original compiler or a backend mechanism.

### Final verdict for every function

Offsets below identify the first differing resolved instruction in the
starting object, relative to the function VA in the plan. Frame-only rows
were checked across the complete body, including epilogues and delay slots.
Unchanged rows remain non-exact; a rejected control is not banked.

| Function | Final strict % | Initial first difference / final verdict |
| --- | ---: | --- |
| `player_move_horizontal` | 96.56896 | `+0x2c`: direction is held in `s6` instead of retail `s7`; later address materializations and branches also differ. Both pure X/Z `ABS.H` controls diverge further. Existing trigonometric calls, halfword direction narrowing and game collision boundaries are retained. |
| `player_update` | 99.96709 | `+0x0`: 216-byte frame versus retail 224; all other differences are stack operands. The charge is stored as `u16`, so `limitRange(charge, 0, 5000)` preserves every input and the whole object. A camera `copyVector` followed by X/Z negation adds two instructions and is rejected. |
| `render_map_cell` | 99.87838 | `+0x0`: 88-byte frame versus retail 120; only stack operands differ. Existing SVECTOR setter and matrix APIs are correct. A separate genuine VECTOR for `RotTrans`, followed by publishing its three words to `MATRIX.t`, adds six instructions; it does not explain retail's direct output destination. |
| `render_entities` | 99.517044 | `+0x5c`: culling-origin register differs. Culling consumes two axes and game visibility cells; a three-component setter would introduce an unused component. The B/G/R writes target a CVECTOR, whose `r/g/b` members are not the packet `r0/g0/b0` fields required by `setRGB0`. No compatible missing helper was found. |
| `item_load_database` | 99.746666 | `+0x4bc`: decimal division operands use different registers. Its six existing `memcpy` operations already inline as word-copy loops. Filename digits are ASCII decimal, not BCD. The file table has the proven older 20-byte stride; replacing it with the supplied 24-byte CdlFILE would be wrong. No source change. |
| `menu_status_panel` | **100** | `+0x0`: the 64-byte frame difference is removed by reflected SDK UV rectangles with consistent byte-domain inputs. Retained, including all twenty ordered calls and the full body. |
| `menu_draw_item_detail` | 97.80875 | `+0xec`: glyph cursor base/setup; retail starts at the containing string and stores at `+4`, while source starts at its glyph array. Only three aligned words differ. `memcpy` emits an unrolled word copy rather than the retail ten-halfword loop; zero-row `memset` adds a call absent from retail. Both rejected. |
| `menu_draw_item_name_frame` | **100** | `+0x0`: composing the three reflected SDK rectangles removes the frame difference. Glyph-copy and matrix-zero alternatives are rejected; the retained glyph loop and matrix setup already agree with retail. |
| `menu_draw_window_backdrop` | **100** | `+0x0`: the same three reflected SDK rectangles remove the frame difference. No new local RECT is needed. |
| `talk_show_dialogue_page` | 98.78049 | `+0x30`: decimal remainder register/order differs. Four byte-valued caller arguments feed ASCII filename digits and one game screen call. `btoi`/`itob` and character classification/case macros do not implement these digit stores. No source change. |
| `map_object_spawn_effect` | 94.50495 | `+0x4`: saved RA offset differs and source omits retail's S4 save; the body is four bytes short. Its position already uses `setVector`. The previous rotation-setter control moves the required pre-`rand` Z clear past the call; that rejected evidence still applies. No new compatible helper. |
| `map_show_screen_image` | 88.888885 | `+0x30`: remainder register differs; the floor load and filename-address materialization also exchange order. The only call is the existing game screen API. ASCII decimal stores have no matching BCD/character helper. No source change. |
| `map_interaction_dispatch` | 99.202774 | `+0x2c`: position pointer uses S8 instead of retail S7, with later branch/layout differences. Both probe pairs consume only X/Z and already call `rsin`/`rcos` in retail order. The modular pitch and game angle predicates are not SDK vector construction or saturation. No supported missing helper. |
| `map_world_state_persist` | 97.52873 | `+0x16c`: definition-base setup appears early; later type predicates differ. A complete eight-byte link `memcpy` changes the retail byte-loop shape and is rejected. Sparse records and packed flags remain explicit game serialization. |
| `effect_map_collision` | 99.87369 | `+0x718`: source puts `li v0,1` in a branch delay slot where retail has `nop`; it is the only differing word. The branch belongs to game effect-mask/collision handling. Bounds are predicates, not clamps; no Sony helper explains this remaining instruction. |
| `effect_projectile_update_2d` | 99.93421 | `+0x0`: 56-byte frame versus retail 120; only stack operands differ. A genuine VECTOR for all three distance components adds instructions. Capturing orbit coordinates in sin/cos/sin order before `setVector` preserves calls but adds instructions too. Both rejected. |
| `effect_update_dispatch` | 99.82781 | `+0x38`: global-address register differs. Complete padded SVECTOR `memcpy` and the unsigned scale `limitRange` controls change body length/instructions. Previous ordered component-setter controls also disagree with retail. Existing compatible helpers are retained. |
| `opening_ending_scroll_run` | 99.917694 | `+0x398`: phase dispatch uses different registers; one join target also differs. Transition `setVector` is whole-object identical and retained. Full MATRIX `memcpy` is identical but less direct than assignment, so not retained. Adding the blend clamp's lower-bound arm changes branches and is rejected. |

All eighteen ordered call sequences agree before the trials. After resolving
section symbols and addends, sixteen ordered data-reference sequences also
agree. The two exceptions are accounted for without changing identities:
`player_move_horizontal` rematerializes ten already referenced addresses,
and `map_show_screen_image` exchanges the floor load's address with the
filename prefix address. No distinct target address is missing or invented.
These order/count differences remain part of those non-exact verdicts.

### Compiled controls

Every score below is strict; body size includes the return delay slot.
`Identical` means equality of all allocated section bytes/sizes/attributes,
function and object extents, and ordered symbol-named relocations with addends
against the initial whole unit. All siblings are checked in every trial.

| Control | Strict % | Bytes | Verdict |
| --- | ---: | ---: | --- |
| X/Z absolute value: `absolute_z` | 95.80268 | 2128 | Reject |
| X/Z absolute value: `absolute_x` | 96.3295 | 2128 | Reject |
| `charge_limit` | 99.96709 | 6684 | Identical; retain |
| `camera_copy_negate` | 99.685814 | 6692 | Reject |
| `glyph_detail_copy` | 73.39891 | 892 | Reject |
| `glyph_name_copy` | 89.78542 | 2132 | Reject |
| `world_link_copy` | 84.37931 | 708 | Reject |
| `rottrans_output` | 95.41216 | 616 | Reject |
| `orbit_distance_vector` | 97.960526 | 620 | Reject |
| `orbit_captured_inputs` | 93.64474 | 616 | Reject |
| `ending_transition_vector` | 99.917694 | 1944 | Identical; retain |
| `ending_blend_limit` | 95.48148 | 1948 | Reject |
| `ending_matrix_copy` | 99.917694 | 1944 | Identical; keep existing assignment |
| `scatter_memory_copy` | 99.41196 | 6140 | Reject |
| `moonlight_scale_limit` | 99.56725 | 6160 | Reject |
| `light_zero_detail` | 93.87978 | 736 | Reject |
| `light_zero_name` | 98.54656 | 1980 | Reject |
| `uv_name_x`, `uv_name_y`, `uv_name_xy` (three separate controls) | 99.97571 each | 1976 each | Each moves frame offsets by 16; compose |
| `uv_backdrop_x`, `uv_backdrop_y`, `uv_backdrop_xy` (three separate controls) | 99.97176 each | 1700 each | Each moves frame offsets by 16; compose |
| `uv_name_all` | **100** | 1976 | Retain |
| `uv_backdrop_all` | **100** | 1700 | Retain |
| `status_uv_rectangles` | 98.86525 | 1692 | Correct frame; eight wider UV loads remain |
| `status_uv_rect_capture` | 70.01418 | 1700 | Reject genuine RECT capture; changed loads and body |
| `status_sdk_tiles` | 81.054375 | 1432 | Reject direct SDK tile expansion; 260 retail bytes disappear |
| `status_uv_byte_spans` | 86.10875 | 1884 | Reject mixed-width origin/span arithmetic |
| `status_uv_byte_domain` | **100** | 1692 | Retain consistent byte-domain inputs |

The memory controls distinguish actual library boundaries. Twenty-byte glyph
`memcpy` uses unaligned word instructions (`lwl/lwr/swl/swr`) and loses the
retail halfword loop. The six-byte zero MATRIX row becomes a real `memset`
call; it changes none of the other six rotation fields in the source, but
the added call is absent from retail. The link copy changes an eight-iteration
byte loop. A complete SVECTOR copy includes its padding, so `copyVector`,
which copies only X/Y/Z, is not a substitute. The full MATRIX copy includes
its translation and padding and is genuinely identical to assignment.

The scalar controls use the actual primary-header definitions. `ABS.H`
provides a conditional expression, and `limitRange` tests its lower bound
first. Pure signed displacement absolute values change branch/register
instructions. The charge lower bound is vacuous for its `u16` storage and
compiles away; the signed ending-blend control emits an extra bound test.
The dispatcher scale clamp has equivalent unsigned input semantics but a
different comparison form. None supplies a missing call.

### Verification

The four retained units are rebuilt with `kf try` and reported with
`kf match`, followed by a full `kf build`. The final resolved audit checks
all eighteen target objects against raw retail, all eighteen final verdicts,
and every sibling in their units, including all 25 functions in the four
retained units. Only the three intended bodies change;
the other retained helpers preserve whole objects. Allocated data bytes,
extents, attributes and relocations remain unchanged.

Strict counts rise from **453/471 to 456/471**: PSX 1/1, GAME 348/362,
OPEN 107/108. Fifteen functions remain non-exact. These three exact functions
are the only new rows eligible for banking; the data-owning menu unit is not
claimed as fully reconstructed.

The starting and final full builds both exit nonzero for the existing data
and known-reference ownership/placement failures. Data matches remain
PSX 1/1, GAME 30/44 and OPEN 11/20, with the same 23 divergent units. Target
relink remains PSX 1/1, GAME 68/77 and OPEN 33/38. All game/vendor counts
retain the same denominators and classifications.

`ruff check scripts tests`, all 769 Python tests (9,218 subtests), Rust tests,
all 112 source/image variants under `kf check-types`, and `git diff --check`
pass. No tooling or flake inputs changed.

The local semantic dossiers, immutable starting objects, thirty alternate
sources, strict reports, raw diffs and resolved audits live under
`build/sony-remaining-18/` and are generated evidence, not committed inputs.
