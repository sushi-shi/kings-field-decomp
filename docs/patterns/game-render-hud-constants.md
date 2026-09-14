# GAME HUD slots and rendering constants

## Function Match Plan

Baseline `cb1e59d`, hash-verified GAME retail, unchanged `probe-gcc257-o2-g0`.
Eight function snapshots include address/extent, disassembly/CFG, incoming and
outgoing references, strings, history and strict scores. Adjacent renderers,
enqueue helpers, animation binding and notification update constrain the
consumers. These functions manipulate game actor/map/player pools and authored
HUD resources; the SDK matrix/GTE providers remain external vendored calls.

Name HUD slots and visibility/sentinel codes, the six subsystem lighting
matrices, actor descriptor packing, door/menu/weapon depth biases, variant
culling bounds, floor-item brightness and notification placement/brightness.
Propagate existing projection, Q12, angle and notification-slot names. The HUD
atlas supplies an independent check of slot identities. Preserve byte/halfword
storage, signed shifts, unsigned windows, boolean predicates, branch/return
delay slots, calls, relocation order and authored data. Keep the effect-sprite
iterator's stop-at-first-non-1 rule separate from HUD skip-hidden iteration.

Hypothesis: constant names preserve every emitted runtime byte. Freeze all 112
source/image variants and 484 scores; force four affected compiles and compare
isolated before/after runtime sections, symbols, alignment and relocations.
Resolve each reviewed body's words and referents; compare exact functions with
retail and targets. Account for every remaining numeric/character token in the
four source files. Run inventory, modern checking, Ruff, existing tests,
whitespace checks and full `kf build`. Add no tests or size assertions; no new
exact result is expected or banked.

| GAME VA / bytes | Function | Initial strict % | Evidence and preserved contract |
| --- | --- | ---: | --- |
| 0x8001e9a4 / 532 | `render_actor` | 94.751880 | Byte descriptor, low-nibble asset and high-nibble one-based texture cache; actor lighting; caller `render_entities`. |
| 0x8001ebb8 / 384 | `render_map_object` | 100 | Byte behavior switch selects signed depth bias; shared TMD enqueue; behavior 3 remains unresolved. |
| 0x8001ed38 / 88 | `menu_render_item_model` | 100 | Projection 200, first object in menu TMD, enqueue bias 1000. |
| 0x8001f218 / 1408 | `render_entities` | 97.380684 | Pool capacities and signed countdowns, unsigned cell windows, five entity emitters; variant interval adds 12 and compares below 24. |
| 0x8001f798 / 280 | `render_weapon` | 100 | Unsigned projection at +16, signed translations +28/+30/+32, rotation +36; bias 50 minus signed Z shifted by 5. |
| 0x8001f8b0 / 292 | `render_effect_sprites` | 100 | State byte equals 1, otherwise terminates; Q12 Z scale; animation binding and GTE calls. |
| 0x8001f9d4 / 112 | `render_hud_gauges` | 100 | Byte state 255 terminates, 1 draws; stride 14 and sprite payload +2; called from frame. |
| 0x8001fde4 / 1304 | `render_frame` | 100 | HUD stores/widths and status priority, compass yaw, notification rows/materials and translation, scene/weapon/present call order. |

## Named roles and retail evidence

The identities describe observed uses. The original design reasons for tuning
most magnitudes are not known. Shared definitions live in `game/render.h`;
module-specific tuning stays beside its consumers.

| HUD row | Name/consumer | Atlas evidence |
| ---: | --- | --- |
| 0 / 8 | HP gauge / panel | Current/maximum HP width; HP frame at UV (0,0). |
| 1 / 9 | MP gauge / panel | Current/maximum MP width; MP frame at UV (0,16). |
| 2 / 10 | Attack charge gauge / panel | Attack charge divided into pixels; POWER frame at UV (0,32). |
| 3 / 11 | Magic charge gauge / panel | Magic charge divided into pixels; MAGIC frame at UV (0,48). |
| 4 | Poison icon | Poison flag and POISON at UV (0,80). |
| 5 | Slowed icon | Existing `KF_PLAYER_STATUS_SLOWED` flag; atlas reads PARALYZE at UV (0,96). This pass does not rename the flag. |
| 6 | Darkness icon | Darkness flag and DARK at UV (0,64). |
| 7 | Curse icon | Curse flag and CURSE at UV (0,112). |
| 12 | Compass | Compass setting and ring at UV (0,128), 33×32 texels. |
| 13 | End row | State 255 stops the iterator; the remaining bytes are never drawn. |

The independent texture check decodes concatenated `COM/MIX.TIM` image index 6
with the retail HUD palette at VRAM (0,500), selected by GAME `80055dac`.
The file SHA256 is
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
The four gauge strips and status/panel coordinates agree with the typed
14-row table at GAME `80055c5c`; each row occupies 14 bytes. See also the
[material identity evidence](game-render-material-identities.md) and
`config/evidence/game_semantic_hud_sprites.tsv`.

HUD state 0 hides a row, 1 draws it and 255 ends traversal. The effect-model
iterator instead stops at the first state unequal to 1. Compass visibility is
copied into both tables. Status priority remains curse, darkness, poison, then
slowed, with at most one status icon visible. The frame keeps its pointer-based
HUD base recovery: compass pointer minus its row index, matching the retail
`addiu a0,s0,-168` at GAME `80020178`.

| Family | Values and preserved meaning |
| --- | --- |
| Lighting | Six MATRIX slots, at offsets 0,32,64,96,128,160 from GAME `80055f68`, feed actor, floor-item, effect, weapon, HUD and notification rendering respectively. Names identify consumers, not undocumented lighting formulas. |
| Actor packing | Low mask 15 selects the model asset. Shifting the descriptor by four extracts the one-based texture-cache selector; zero uses the native TMD material. The byte postdecrement/narrowing remains intact. |
| Map-object eligibility | IDs below 133 are renderable in this loop. The bound is a render cutoff; it does not claim a resource count or identify excluded objects. |
| Variant culling | Add 12, narrow to unsigned halfword and compare below 24 on each cell axis: accepted relative cells are -12 through +11 inclusive. This is an asymmetric 24×24 window, not a radius query. |
| Material brightness | Floor items use RGB 180 and notifications RGB 255 as lighting/texture modulation inputs. These are not opacity values. |
| Object/menu depth | Lift doors and unresolved behavior 3 add 180 ordering-table units; hinged door behaviors add 15; the menu model adds 1000. Other map-object behaviors add zero. |
| Weapon depth | Projected SZ is shifted by 3 before enqueue. Enqueue then applies the common two-bit depth-to-OT shift. The weapon bias subtracts signed translation Z shifted by 5 and adds 50, compensating the translation's contribution at the combined 3+2 scale. Saturation/rounding still apply; no exact algebraic cancellation is assumed. |
| Projection and scale | The two GAME default projection calls reuse distance 200; effect-model Z scale reuses Q12 unity. Compass yaw reuses the 4095 wrap mask. |
| Notifications | Translation is (0,160,200); only X rotation animates. Text, gold and four digit rows reuse existing slot names, with the countdown derived from thousands minus ones. |

The weapon depth operand is the same +32 signed Z translation installed in
its matrix, not the swing angle previously claimed by the source comment.
Projection at +16, translation at +28/+30/+32 and rotation at +36 still need
complete `KfWeaponRecord` render-block modeling; replacing raw offsets with
macros would conceal that debt. Behavior ID 3 likewise remains unresolved.

Thirty-nine new definitions and propagation of existing identities replace
**108 inline numeric occurrences** across four C files, plus two shared
array bounds. The [complete retained-literal ledger](game-render-hud-literal-ledger.md)
accounts for the remaining **223 occurrences**: 152 initializer tokens and
71 function tokens. Authored UV/position/scale/palette tuples remain explicit
data; booleans, nulls, component indices and arithmetic endpoints retain their
local meanings. Opaque bytes, the unresolved behavior and weapon offsets have
specific reasons and remain open debt rather than invented semantic names.

## Verification and final verdicts

All 112 independently compiled before/after source/image variants preserve
runtime sections, alignment, runtime symbols and ordered relocations. Only
four debug-line sections change. The eight reviewed functions retain their
initial strict scores: six exact and two partial, with no new banked result.

The reviewed bodies preserve **1102 instruction words**, **94 direct call sites**
and **113 ordered address pairs**. The six exact bodies reproduce all
**615 words** against retail and target objects.

| Function | Words / calls / addresses | Final verdict |
| --- | --- | --- |
| `render_actor` | 133 / 20 / 11 | Partial, unchanged |
| `render_map_object` | 96 / 15 / 6 | Exact, unchanged |
| `menu_render_item_model` | 22 / 7 / 0 | Exact, unchanged |
| `render_entities` | 354 / 9 / 25 | Partial, unchanged |
| `render_weapon` | 70 / 10 / 5 | Exact, unchanged |
| `render_effect_sprites` | 73 / 12 / 2 | Exact, unchanged |
| `render_hud_gauges` | 28 / 1 / 0 | Exact, unchanged |
| `render_frame` | 326 / 20 / 64 | Exact, unchanged |

`render_actor` first differs at entry GAME `8001e9a4`: candidate stack frame
-160 bytes versus retail -168. `render_entities` first differs at GAME
`8001f274`: candidate `move s5,v1`, retail `move s6,v1`. These unchanged
observations establish no compiler mechanism. The scene candidate contains
354 words versus the retail body's 352; the ledger does not imply closure.

All four initialized claims—HUD rows, effect rows and two palette rectangles—
match their source/target symbols and retail for **268 bytes**. Symbol-relative
ranges preserve unclaimed gaps. The variant-window interpretation was checked
over all 65536 signed halfword deltas, accepting exactly -12 through +11.

The live 484-score comparison isolates one concurrent change: GAME `800264d8`
status ratings, committed separately as `1507bbc`. Its frozen score already
included the reviewed relocation correction (95.123000%); it now reaches
98.161130%. All other 483 scores are unchanged. The current status source was
compiled independently with both rendering header versions and emits identical
runtime contents; both agree with its live object. The other 111 live objects
agree with the frozen before/after controls. No status change is attributed to
this naming batch or included in its commit.

Inventory, Ruff and whitespace checks pass. All **680 existing tests pass**
in 89.826 seconds. One existing graphics-owner compiler probe required its
literal-specific source-rewrite anchor to follow `ACTOR_MODEL_TEXTURE_SHIFT`;
its independent byte-index/owner comparisons and raw-retail checks are intact.
No tests were added. Modern checking retains the same **300 error diagnostics**
and **65/112** passing variants.

Full `kf build` ran and retains existing data/relink failures: source-data
matches PSX 0/1, GAME 9/42 and OPEN 2/19; target relinks PSX 1/1, GAME 75/77
and OPEN 34/38. Six conflicting section bases remain, with zero artifact
failures. No tooling/flake configuration changed. The ten `unknown_` source
lines and the broader constant/type audit remain open.

The [sprite-state review](game-hud-sprite-state.md) now propagates distinct
HUD/effect byte enums, names the effect compass slot and table bound, and
refreshes the [current rendering ledger](game-render-hud-literal-ledger.md).
The counts above remain historical to this constants campaign.
