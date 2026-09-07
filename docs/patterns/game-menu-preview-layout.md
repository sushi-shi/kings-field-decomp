# Inventory and shop preview layout

## Function Match Plan

At `06914e5`, name the shared inventory/shop preview placement and text layout
without changing either renderer's algorithm or interfaces. GAME
`menu_item_model_preview` (`800279c4`, 440 bytes) starts at 98.181816%;
`menu_draw_item_detail` (`80027b7c`, 732 bytes) starts at 92.207650%.
The hash-verified six semantic views, callers, SDK callees, adjacent functions,
history, raw objects and strict scores were captured before edits.

The retail translation stores at `800279e4..800279f8` and
`80027bac..80027bc0` set the SDK matrix's signed-word translation to
X560/Y140/Z1500 before `SetTransMatrix`. These are model-transform units;
they are not screen pixels, angles or fixed-point light coefficients.
Both paths share the rotation object, advance yaw by the existing named
16 angle units, and use the same light-matrix recipe and model renderer.
Keep the three translations as shared named values, not a new data table.

The glyph workspace's signed-halfword origin is X174/Y36 in both functions
(`80027aac..80027ab8`, `80027c74..80027c80`). The inventory quantity label is
X230 and its number X279, one 18-pixel row below the name. Shop price starts
at X200 on the next row; its currency label and the following quantity label
start at X242; the quantity number starts at X284. Name these separate
positions and share the 18-pixel row pitch. The shop gold number starts 28
pixels right of its existing window-row origin. Their roles and units are
decoded; the original aesthetic reason for each placement is unknown.

The formatter's second argument counts decimal glyph positions: quantity
uses two, while price and gold each use six. Preserve its zero argument for
blank leading padding and the source stock-bank index zero. Keep encoded
glyph sequences as literal text data; their values are atlas indices, not
domain selectors. Matrix component indices and zero coefficients remain
literal with explicit ledger reasons. Preserve all item guards, widths,
price-bank selection, load order, calls, branches and delay slots.

The seven inventory-preview and six shop-detail call sites pass item IDs;
the shop additionally supplies a one-based shop column and typed buy/sell
mode. None consumes a return. These are custom game UI bodies, absent from
the vendored inventory. SDK matrix functions stay external, and their
authentic MATRIX/SVECTOR types remain shared. No library body is reconstructed.

The other three functions in the detail unit, shared formatter/glyph/model
helpers and calling selection panels are controls. Force all affected units,
compile all 112 variants independently before/after, and compare runtime
bytes, ordered relocations and all 484 scores. Inspect each partial control's
first divergence without promoting it to exact. Verify the rotation's complete
eight-byte zero initializer, run the modern check, inventory, Ruff, repository
tests, whitespace and full build. Complete the inventory-preview ledger and
refresh the existing complete detail ledger and source-wide accounting.

## Per-function snapshots

| GAME VA / bytes | Function | Strict baseline | Calls / branches |
| --- | --- | ---: | --- |
| `0x8001ed38 / 88` | `menu_render_item_model` | 100.000000% | 7 / 0 |
| `0x80021538 / 1476` | `item_menu_buy` | 99.769646% | 21 / 35 |
| `0x80021afc / 1280` | `item_menu_sell` | 99.609375% | 20 / 36 |
| `0x80022608 / 1908` | `menu_use_item_panel` | 99.482180% | 24 / 48 |
| `0x800249a8 / 1212` | `menu_drop_item` | 99.547850% | 20 / 36 |
| `0x800279c4 / 440` | `menu_item_model_preview` | 98.181816% | 10 / 2 |
| `0x80027b7c / 732` | `menu_draw_item_detail` | 92.207650% | 17 / 3 |
| `0x80027e58 / 72` | `menu_add_marker_quad` | 100.000000% | 1 / 0 |
| `0x80027ea0 / 68` | `menu_add_frame_quad` | 100.000000% | 1 / 0 |
| `0x80027ee4 / 1180` | `menu_draw_dialog_frame` | 100.000000% | 27 / 7 |
| `0x80028380 / 852` | `menu_list_interact` | 100.000000% | 21 / 28 |
| `0x80029de0 / 1328` | `menu_draw_string` | 100.000000% | 6 / 12 |
| `0x8002a310 / 512` | `menu_draw_number` | 100.000000% | 2 / 2 |
| `0x8002adf8 / 172` | `menu_format_number` | 100.000000% | 0 / 8 |
| `0x8002aea4 / 104` | `menu_load_item_model` | 100.000000% | 3 / 2 |

## Final implementation and verification

Shared constants now describe the model translation, item-name origin,
18-pixel row pitch and two-digit quantity format. Each renderer keeps its
own price/quantity origins. Shop price and gold widths have separate names
even though both currently use six decimal positions. All 22 replaced
literal occurrences preserve their exact values and expression structure.

The [inventory-preview ledger](game-menu-inventory-preview-literal-ledger.md)
accounts for all 36 remaining occurrences, including every matrix index,
zero coefficient, initializer component, glyph code and formatting argument.
The [detail ledger](game-menu-detail-literal-ledger.md) accounts for all 87
remaining occurrences across its four functions. The source census is now
6,589; 42 files have complete ledgers covering 4,057 retained occurrences.
Unresolved source fields remain 10 lines with 14 identifier tokens.

Independent before/after compiles of all 112 variants preserve allocated
bytes, alignment, runtime symbols and ordered relocations. Only the two
edited modules' debug line sections change. The fifteen GAME function
controls preserve 2,856 candidate words, 180 ordered direct calls and 140
address materializations. The nine exact controls retain all 1,094 retail
instruction words, including delay slots. Source and both candidate/target
ELFs preserve the complete eight-byte rotation initializer at GAME `80057b70`.

Two shared OPEN build artifacts were detached from their current source by
concurrent rendering experiments: `open.render` already differed at capture,
and `open.render_tmd` changed afterward. At final verification their current
source and all headers agree with the independently compiled source snapshot,
whose before/after runtime output is identical. The cached OPEN `8001764c`
score moved from 99.171080% to 98.722890%; all other 483 cached scores remained
unchanged. This campaign does not attribute that cache movement to its source
changes or overwrite the concurrent artifacts. The other 110 live objects
agree with the isolated result.

The six partial GAME controls keep their first raw difference:

| Function | GAME site | Candidate / retail |
| --- | --- | --- |
| `item_menu_buy` | `80021568` | `li s2,-99` / `li s1,-99` |
| `item_menu_sell` | `80021b28` | `move s2,zero` / `move s3,zero` |
| `menu_use_item_panel` | `80022ccc` | `lhu v0,0x792(v0)` / `addiu v0,v0,0x792` |
| `menu_drop_item` | `800249cc` | `move s2,zero` / `move s3,zero` |
| `menu_item_model_preview` | `80027a88` | `sll v0,v0,2` / `lui v1,0x8006` |
| `menu_draw_item_detail` | `80027b9c` | `move s4,a1` / `move s2,a1` |

These remain existing unattributed residues; no new exact result is banked.
All 683 repository tests pass (87.267 seconds), as do inventory, Ruff and
whitespace checks. Modern diagnostics are the same 300 errors, with 65/112
variants passing. Full `kf build` retains existing source-data and target-relink
failures: source-data PSX 0/1, GAME 9/42, OPEN 2/19; target relink PSX 1/1,
GAME 75/77, OPEN 34/38. There are six conflicting section bases and zero
artifact failures. No compiler-control tests or size assertions were added.

## Per-function verdicts

| Function | Final strict score | Verdict |
| --- | ---: | --- |
| `menu_render_item_model` | 100.000000% | Exact, unchanged |
| `item_menu_buy` | 99.769646% | Partial, unchanged |
| `item_menu_sell` | 99.609375% | Partial, unchanged |
| `menu_use_item_panel` | 99.482180% | Partial, unchanged |
| `menu_drop_item` | 99.547850% | Partial, unchanged |
| `menu_item_model_preview` | 98.181816% | Partial, unchanged |
| `menu_draw_item_detail` | 92.207650% | Partial, unchanged |
| `menu_add_marker_quad` | 100.000000% | Exact, unchanged |
| `menu_add_frame_quad` | 100.000000% | Exact, unchanged |
| `menu_draw_dialog_frame` | 100.000000% | Exact, unchanged |
| `menu_list_interact` | 100.000000% | Exact, unchanged |
| `menu_draw_string` | 100.000000% | Exact, unchanged |
| `menu_draw_number` | 100.000000% | Exact, unchanged |
| `menu_format_number` | 100.000000% | Exact, unchanged |
| `menu_load_item_model` | 100.000000% | Exact, unchanged |
