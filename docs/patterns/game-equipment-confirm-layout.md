# Equipment confirmation state and status-name layout

## Function Match Plan

Review GAME `menu_select.c`, `menu_draw_name_list.c` and `equipment.c` as one
equipment selection/display family. Capture all six functions with disassembly,
CFG, callers, callees, strings, current scores and history. Check the existing
equipment-category/slot domains and the fixed equipment-name row order before
editing. SDK pad/render services remain library providers; the selection policy,
player slots and displayed item/magic names are game-owned.

Give the local pending-confirmation flag its own signed-word enum, distinct
from the widget's result enum and the equipment-category enum. Propagate both
selection panels' initializers, exact-one tests, clears and confirm-edge stores.
Preserve their different frame/clear schedules and the widget's existing
accepted/cancelled handling. This local state does not cross an ABI boundary.

Name the equipment-name column's X/Y origin and repeated row pitch. Preserve
the fixed eight-row category order, including advances past empty slots. Check
the retained glyph values, local workspace capacities and loader arithmetic
against their current occurrence ledger rather than treating older grouped
counts as complete coverage.

Force the three units, compare isolated before/after runtime objects and ordered
references, and independently compare exact bodies to retail words. Partial
bodies must retain their existing words and scores. Exercise valid and invalid
enum uses with the modern compiler, then run type checking, inventory, Ruff,
repository tests, whitespace and full build before the focused commit.

## Starting evidence snapshots

| GAME function | VA / bytes | Strict baseline | Constraint |
| --- | --- | ---: | --- |
| `weapon_records_load_and_mirror_angles` | `800150a8 / 84` | 100% | Copies 176 words, then negates the +0x26 halfword of all sixteen records; no calls. |
| `armor_records_load` | `800150fc / 44` | 100% | Copies 294 words using the pre-decrement loop; no calls. |
| `fixed6_ratio_step` | `80015128 / 60` | 100% | Signed division after the six-bit shift and span increment; final increment occupies the return delay slot. |
| `menu_equip_select` | `800238d8 / 1476` | 100% | Full-width category argument; the local confirmation register begins at zero, compares to one, clears before exit testing and becomes one on the confirm edge. |
| `menu_spell_select` | `80023e9c / 1136` | 99.542250% | No argument; same local request domain, but clear occurs in the frame-begin call delay slot after selection/exit testing. |
| `menu_draw_name_list` | `8002718c / 2104` | 100% | Halfword screen coordinates begin at 174,40; seven unconditional increments of twenty retain fixed category positions. |

The request values occupy `$s5` in both retail panels. Equipment initializes
at `80023900`, compares at `80023acc`, clears at `80023b1c` and sets one at
`80023cec`. Spell selection uses `80023ebc`, `8002403c`, `800240a0` and
`80024258` respectively. Their widget calls at `80023af0` and `80024060`
return a separate signed result; neither stores that result into the request
local. Both are called by `menu_option_root`, which supplies the category only
to equipment selection. Call edges remain proven, data references validated,
and indirect jump-table successors retain the navigator's current evidence
classification.

## Implementation and retained values

`KfEquipmentConfirmState` gives the two locals `IDLE = 0` and `REQUESTED = 1`
states. Eight initializers, tests and assignments now use their own domain.
Modern compilation rejects widget outcomes, choices, categories and implicit
integers in this state. Explicitly decoded value two still does not satisfy
the requested-state test; no runtime validity check was introduced. The legacy
view remains a signed word, and neither function signature changes.

The status column uses `EQUIPMENT_NAME_X = 174`, `EQUIPMENT_NAME_FIRST_Y = 40`
and `EQUIPMENT_NAME_ROW_STEP = 20`, replacing nine inline coordinate uses.
Weapon, magic, shield, head, body, arms, legs and accessory retain Y positions
40 through 180. The coordinates are authored layout settings; the original
reason for choosing this origin and spacing is not recovered. The source
comment now correctly describes fixed category rows, including empty slots.

The three modules retain 103 numeric/character occurrences, each reconciled
to an individual row in the [equipment ledger](game-equipment-literal-ledger.md).
The twenty-entry local workspaces remain their observed extents rather than
being relabelled as an item-count limit: the largest equipment range requires
fourteen rows including unequip, and the spell list needs at most six. The
original extra-capacity rationale is unknown. The three unequip glyphs remain
encoded text; index positions and zero-based cursor arithmetic remain numeric.

The ratio helper's six-bit shift scales the stat numerator by 64; the final
plus one sets a positive minimum for the reviewed bounded inputs. No consumer
establishes a later Q6 decoding stage, and the exact tuning choice remains
unproven. Its existing function name already identifies the scaling. Player
stock-bank naming and broader item-ID type propagation remain outstanding.

## Final verification

Three isolated before/after units retain identical runtime sections, symbols
and ordered relocations. Only the two edited units' debug line sections change.
The live objects agree with the isolated result, and every header remains
identical to the captured baseline. All six campaign functions preserve 1,226
instruction words, 53 direct calls and 117 address materializations. The five
exact controls independently agree with all 942 retail words, including their
delay slots. Spell selection retains its first difference at `80023ec0`:
candidate `move s3,zero` versus retail `move s4,zero`. This is an existing,
unattributed residue; no partial body was promoted or newly banked.

| GAME function | Final strict score | Verdict |
| --- | ---: | --- |
| `weapon_records_load_and_mirror_angles` | 100% | Exact; words and referents unchanged. |
| `armor_records_load` | 100% | Exact; words and referents unchanged. |
| `fixed6_ratio_step` | 100% | Exact; words and referents unchanged. |
| `menu_equip_select` | 100% | Exact; words and referents unchanged. |
| `menu_spell_select` | 99.542250% | Existing partial; words and referents unchanged. |
| `menu_draw_name_list` | 100% | Exact; words and referents unchanged. |

Of 484 global scores, 483 remain unchanged. Separate concurrent edits to GAME
`map_interaction_dispatch` move its score from 89.927210% to 93.020800%; its
source is outside this commit. The isolated equipment objects establish this
campaign's preservation independently of that change.

One positive and ten negative compiler controls use the actual local enum
declaration and project header. They verify its initializers, assignments and
comparisons, rejection of unrelated domains, and explicit encoding boundaries.
No production tests or size assertions were added. Modern checking retains
the same 300 errors and 65/112 passing variants. Inventory, Ruff, whitespace
and all 684 repository tests pass (89.698 seconds). Full `kf build` retains
source-data mismatches (PSX 0/1, GAME 9/42, OPEN 2/19) and target-relink gaps
(PSX 1/1, GAME 75/77, OPEN 34/38), with six conflicting section bases and zero
artifact failures.
