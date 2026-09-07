# GAME confirmation choice domain

## Function Match Plan

Propagate a signed-word `KfMenuConfirmChoice` through the three dialog input
loops and `menu_draw_two_option`. The first label accepts the operation; the
second declines. The [retail glyph audit](game-menu-confirmation-protocol.md)
decodes use/drop/buy/sell/equip, yes/no and take/cancel labels. Choice values
zero/one are distinct from the completed confirmation results zero/minus one
and pickup results zero/one/two. Give the choice its own enum and express the
negation into the confirmation-result domain at one typed helper boundary.
Retain the pickup's explicit stock/result branches.

The renderer receives both positioned labels in a0/a1 and the full choice
word in a2. GAME `80029228` and `80029264` choose the first label for zero;
other values use the second. List input toggles s4 with explicit zero/one
arms at `80028634..80028640`; pickup does the same to s3 at
`800221cc..800221d8`. Save/load confirmation negates the choice at `80028874`
in a jump delay slot. Preserve these guards, toggles, widths, call order and
the final highlighted frame followed by the button-release wait.

The three dialog layouts place their second label twenty pixels below their
first: list 185/205, variable footer count*20+44/count*20+64, pickup 26/46.
Name this shared confirmation row step, the shared list/footer X coordinate
96, and each layout's private origins. Preserve the variable footer's
multiply/add expression and keep all authored glyphs inline as text data.
The original aesthetic rationale for these pixel values remains unknown.

Before editing, initialize hash-identical retail, capture the six GAME views
for the four changed functions, seven list callers, save/load callers and
the complete item module. Inspect source/history, adjacent claimed functions,
raw instructions and ordered references. These bodies compose game UI and
inventory policy, with no vendored identity; PadRead and GPU/SDK calls remain
external boundaries. All use `probe-gcc257-o2-g0`; its historical attribution
remains open. Each captured function keeps an independent final verdict.

Require forced affected compiles, isolated before/after compilation of all
112 variants, all 484 strict scores and direct retail word checks for exact
controls. Require compiler rejection of integers and foreign enums at the
new boundary, unchanged whole-tree diagnostics, inventory, Ruff, repository
tests, whitespace and full `kf build`. Add no size assertions or permanent
tests. Reconcile retained-literal accounting for the two previously uncovered
dialog files and update the changed renderer/pickup accounting.

## Baseline function snapshots

| GAME VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `0x80020b4c / 432` | `item_load_floor_placements` | 98.888885% |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666% |
| `0x800212d8 / 608` | `item_menu_root` | 100.000000% |
| `0x80021538 / 1476` | `item_menu_buy` | 99.769646% |
| `0x80021afc / 1280` | `item_menu_sell` | 99.609375% |
| `0x80021ffc / 696` | `item_pickup_confirm` | 100.000000% |
| `0x80022608 / 1908` | `menu_use_item_panel` | 99.482180% |
| `0x8002317c / 1328` | `menu_magic_panel` | 100.000000% |
| `0x800238d8 / 1476` | `menu_equip_select` | 100.000000% |
| `0x80023e9c / 1136` | `menu_spell_select` | 99.542250% |
| `0x800249a8 / 1212` | `menu_drop_item` | 99.547850% |
| `0x80024e64 / 608` | `menu_save_load_hub` | 100.000000% |
| `0x800250c4 / 1128` | `menu_save_panel` | 100.000000% |
| `0x8002552c / 880` | `menu_load_panel` | 100.000000% |
| `0x80028380 / 852` | `menu_list_interact` | 100.000000% |
| `0x800286d4 / 576` | `menu_two_option_prompt` | 100.000000% |
| `0x800291ec / 268` | `menu_draw_two_option` | 100.000000% |

## Final implementation and verification

`KfMenuConfirmChoice` now reaches all three local choice variables and every
renderer call. `menu_confirm_result_from_choice` is the explicit typed mapping
between choice and ordinary confirmation outcome; its negation retains the
retail instructions, including the save/load jump delay slot. Pickup continues
to branch on accept and check the inventory count before selecting its separate
outcome. The renderer still treats every nonzero encoded value as the second
label, and highlight still requires exactly one.

The shared confirmation row step is 20 pixels. The ordinary list/footer X
origin is 96 pixels; the pickup uses its own X origin of 60. Private Y origins
and derived second rows make the layouts explicit. No glyph stream, element
index, label terminator, input mask, rendering order or arithmetic grouping
changed. The new retained ledger contains 59 list, 32 prompt and 197 item-module
occurrences. It records the other item functions from their existing evidence
and full current expressions; naming the remaining shop control sentinels is
still outstanding.

Clang accepts the typed renderer call, choice assignment/comparison and result
mapping. Ten negative controls reject raw integers and foreign confirmation,
label-kind or pickup enums as arguments, initializers, assignments, comparisons
and helper inputs. The temporary controls use actual project headers; existing
legacy comment and folded-offset warnings are excluded, while enum diagnostics
remain errors. Whole-tree checking retains the same 300 errors, with 65/112
variants passing. No permanent test or size assertion was added.

All 17 captured functions keep their strict baseline in the table above:
10 exact and 7 partial. All 4,340 candidate words, 373 calls and 111 address
references agree before/after. The exact controls also match all 2,105 retail
instruction words and their delinked targets.

The partial controls retain these first divergences. They are recorded
symptoms of existing differences, without assigning an optimizer cause.

| Function | GAME site | Candidate / retail |
| --- | --- | --- |
| `item_load_floor_placements` | `0x80020b84` | `beq v1,v0,80020bb0` / `beq v1,v0,80020bb4` |
| `item_load_database` | `0x800211b8` | `ori v1,zero,0x1e` / `ori a2,zero,0x1e` |
| `item_menu_buy` | `0x80021568` | `addiu s2,zero,-99` / `addiu s1,zero,-99` |
| `item_menu_sell` | `0x80021b28` | `move s2,zero` / `move s3,zero` |
| `menu_use_item_panel` | `0x80022ccc` | `lhu v0,0x792(v0)` / `addiu v0,v0,0x792` |
| `menu_spell_select` | `0x80023ec0` | `move s3,zero` / `move s4,zero` |
| `menu_drop_item` | `0x800249cc` | `move s2,zero` / `move s3,zero` |

All 112 source/image variants were independently compiled before and after;
allocated bytes, section alignment, runtime symbols and ordered relocations
are identical. Only the three dialog-containing objects' debug line tables
changed. All current objects agree with those isolated controls, and all 484
strict scores are unchanged. There is no new exact-count movement or banking.

Forced affected builds, inventory, Ruff, whitespace and all 683 repository
tests pass (90.335 seconds). Full `kf build` still reports existing source-data
mismatches: PSX 0/1, GAME 9/42 and OPEN 2/19 units match. Target relinking
verifies PSX 1/1, GAME 75/77 and OPEN 34/38 units, with six section-base
conflicts and no artifact failures.

The complete source census falls by 31 to 6,661 retained numeric/character
occurrences. Reconciled ledgers cover 38 files and 3,799 occurrences, including
the previously reviewed zero-literal warp wrapper. The ten source lines with
fourteen `unknown_` tokens remain unresolved. Retained-literal accounting does
not prove that the entire naming and type-propagation objective is finished.
