# GAME root choices and menu panel state

## Function Match Plan

At `0d0b156`, name and type the root menu's pending choice, retain the existing
equipment-category type through the equipment root's pending selection, and
separate each selection from the menu's open/closed phase. Name the root,
equipment and configuration row limits and the four configuration bindings.
Propagate heading-row names into the item, magic, spell and drop list callers.
Reuse the established background ordering-table bucket in the config helper.

Before editing, capture all six GAME semantic views for the nine affected
functions, source history, adjacent functions, all 112 objects and all 484
strict scores. The window resource decoded in the
[window-kind review](game-menu-window-kinds.md) independently confirms the
label order. Caller arguments, field loads/stores, switch targets, comparison
values and delay slots determine the type and constant domains. These game
menus are absent from the vendor inventory; SDK calls remain external.

Root choices 0..6 dispatch item use, magic use, equipment, status, discard,
System and configuration. The return row 7 never becomes a pending choice.
Selection -1 means no action is pending, independently of root result -1
(no item to use). Keep the root's mixed integer result channel and the exact
selection/result equality used for the final redraw.

Equipment rows 0..7 retain the existing equipment-category meanings; row 8
returns. Pending selection -1 is not an equipment category passed to the
equipment selector. Preserve its reset before the phase-exit guard, including
the retail reset in the branch slot at `800237c8`. Do not repeat the earlier
equivalent-condition experiments or introduce a new dispatch shape.

The equipment root and configuration panel use -99 while open and -1 when
closing. Configuration publishes all four working values on both exit inputs,
so its closing phase is not a cancelled transaction. Keep the four-word
working array, the two by-value 24-byte labels, and the helper's pointer walk.
The four bindings are effects, music, HUD gauges and compass; the return row
is outside that array. Raw values are preserved until a toggle normalizes
one value through `== 0`.

Force the five affected units to compile, compare every non-debug section,
all strict scores and each captured function's linked words and referents.
Require existing exact functions to remain exact. Run modern checking,
inventory, Ruff, existing tests, whitespace and full `kf build`. An isolated
saved-source compile before and after the header edit controls the concurrently
changing list renderer. No size assertions, permanent tests, compiler changes
or banking are proposed.

## Retained domains and resource evidence

`KfMenuRootChoice` stores the pending choice as a signed word: NONE=-1,
USE_ITEM=0, USE_MAGIC=1, EQUIPMENT=2, STATUS=3, DROP_ITEM=4, SYSTEM=5 and
CONFIG=6. These values agree with the decoded root window and its direct
switch targets. The integer cursor includes return row 7; only rows below
that boundary are decoded into choices. The mixed item/control return value
remains `s32`, with explicit encoding for the existing selection comparison.
A magic-panel return other than -1 closes the root with NO_ITEM; this includes
the insufficient-MP path and does not prove a spell was cast.

The equipment root carries `KfEquipmentMenuCategory` through its pending local,
switch and `menu_equip_select` argument. NONE=-1 represents no pending action;
the existing category order remains weapon, magic, shield, head, body, arm,
leg and accessory. Return row 8 stays outside the category dispatch. The root
handles MAGIC through `menu_spell_select`; neither MAGIC nor NONE is passed
to `menu_equip_select`. Adding NONE to the type does not widen that selector's
valid input contract. The Full Plate guard reads `equipped_body_armor_id`
before arm/leg dispatch. The selection reset still precedes the phase guard,
preserving the instruction in the branch delay slot at GAME `800237c8`.

`KfMenuPanelPhase` gives the equipment and configuration panels separate
OPEN=-99 and CLOSED=-1 states. These signed-word locals describe modal lifetime,
not an item result or a cancelled configuration transaction. The row counts
derive from the named last row plus one: root 8, equipment 9, configuration 5.
The four configuration bindings are explicit on both import and publication:

| Working row | Player field |
| --- | --- |
| EFFECTS=0 | `audio_effects_enabled` |
| MUSIC=1 | `audio_music_enabled` |
| GAUGES=2 | `hud_gauges_enabled` |
| COMPASS=3 | `compass_enabled` |

Return row 4 never indexes the working array. Both exit inputs publish all
four values; a changed music value stops the sequence if zero and starts the
current map sequence otherwise. Working values and the helper pointer remain
`s32`: untoggled values are copied unchanged, toggling uses `value == 0`, and
the renderer highlights ON only for exact value 1. Other nonzero values
therefore highlight OFF and toggle to zero. Converting this storage to Boolean
would erase a distinction present in the retail instructions.

The local labels decode as ON (`0xf9,0xfa`) and OFF (`0xf9,0xfb,0xfb`), each
followed by `MENU_TEXT_END`. Their atlas cells were visually checked against
retail `MIX.TIM`, SHA-256
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
The glyph texture begins at byte 232096 and the used palette at 231572;
14-by-12 cells are addressed on a sixteen-column grid. Their X anchors are
180 and 240 screen pixels. Both begin at Y=41 and advance by 22 per row,
producing 41, 63, 85 and 107, in agreement with the loaded configuration
window. Its return row at Y=129 is outside the editable set. These placements
and encoded character values remain local literals with measured meanings;
the original layout rationale is not established.

The helper uses the existing `MENU_BACKGROUND_OT_DEPTH` for all four frame
packets and retains their prepend order 3,2,1,0. Item, magic, equipment-magic
and discard list headings now encode their named choice/category at the
generic integer row API boundary. No API signature, object layout, call order
or source control flow changes in this batch.

## Literal coverage and final verification

The four-function [literal ledger](game-menu-choice-literal-ledger.md) explains
all 97 retained occurrences, down from 154: root 25, equipment root 20,
configuration 44 and configuration drawing 8. Four further heading-row uses
are named in their callers, for 61 replaced numeric uses altogether. The
[equipment ledger](game-equipment-literal-ledger.md) drops from 139 to 138;
every other existing reason is preserved.

All five affected units were forcibly compiled. Across 112 objects, every
object owned by this batch is byte-identical in every section, including
debug sections. All 484 strict scores were compared: the nine reviewed
functions and 474 other scores are unchanged. The one concurrent list-renderer
change reaches 100% in its own campaign. Compiling the same saved list-renderer
source before and after this header edit produces byte-identical objects,
independently isolating that concurrent change.

Each row below also passed comparison of resolved linked instruction words,
ordered calls and data-address materializations against its pre-edit source.
The four exact functions additionally equal the retail and delinked target
words: 942 words total. Root's reconstructed body has 175 words against 176
retail words; its existing mismatch remains. Percentages are strict objdiff
function results, not whole-unit data closure.

| GAME address | Function | Strict match | Final verdict |
| --- | --- | --- | --- |
| `80022348` | `menu_root` | 96.86364% | Unchanged partial |
| `80022608` | `menu_use_item_panel` | 99.48218% | Unchanged partial |
| `8002317c` | `menu_magic_panel` | 100% | Exact retained |
| `800236ac` | `menu_option_root` | 100% | Exact retained |
| `800238d8` | `menu_equip_select` | 100% | Exact retained |
| `80023e9c` | `menu_spell_select` | 99.54225% | Unchanged partial |
| `800249a8` | `menu_drop_item` | 99.54785% | Unchanged partial |
| `8002589c` | `menu_config_panel` | 99.85981% | Unchanged partial |
| `80025da0` | `menu_config_panel_draw` | 100% | Exact retained |

Inventory validation, `ruff check scripts tests`, all 678 repository tests
and `git diff --check` pass. Modern checking retains the same 320 diagnostic
errors, with 64/112 units passing; no new error is introduced. Temporary
controls using the actual compilation-database flags accept the named values
and explicit integer boundaries, and reject three raw integer initializers
and three assignments from unrelated enum domains. These controls add no
permanent test or size assertion.

Full `kf build` ran and still fails the existing data/ownership/relink gates:
PSX data 0/1, GAME 9/42, OPEN 2/19; target relink 1/1, 75/77 and 34/38.
There are six conflicting section bases and no artifact failures. No new
match is banked by this naming batch. Reproduction artifacts, six-view
snapshots, literal censuses and comparison results remain under ignored
`build/constant-names/menu-choice-state/`.
