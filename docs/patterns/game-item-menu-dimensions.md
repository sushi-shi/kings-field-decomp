# GAME item-table and menu-text dimensions

## Function Match Plan

At `8def349`, propagate the evidenced 80-item database extent through its
global definitions, declarations, shop/discard workspaces and scan bounds.
Name the two shop-price columns and the three stock banks separately from
item IDs. Give live and serialized stock the same dimensions; derive the
startup byte-clear countdown from the complete live object while preserving
its decrement-to-minus-one loop.

Name the common ten-halfword text-row format in the positioned label,
unpositioned name row and list-title prefix, every name-copy bound and all
seven list callers' glyph strides. The window's capacity for ten positioned
rows gets a separate name. The magic panel's ten-row workspace remains a
local capacity, distinct from the ten glyphs in each row. The list initializer
still sets an eight-glyph stride which all confirmed callers overwrite.

Before editing, capture all six GAME semantic views, histories, adjacent
functions, 112 source objects and 484 strict scores under
`build/constant-names/item-menu-dimensions/`. Inspect the initializer,
database-copy and save-copy raw instructions, all glyph-copy bounds and their
callers. The 5708-byte hash-identical `COM/STAT.DAT` independently supports
the six bank extents. The item/stock ownership inventory is curated evidence;
direct loads, stores, copy endpoints and file contents constrain these names.

This is game data and menu policy, absent from the vendored inventory and
unlike SDK wrappers. SDK memory, CD and graphics calls remain external.
Keep all signed/unsigned widths, row ordering, default values, predicates,
calls, delay slots and ordered relocation targets. Do not add size assertions
or alter the existing partial functions to protect or improve a score.

The concurrently edited window renderer is controlled by compiling the same
saved source under the old and new headers. Its owner handles any independent
source change; this campaign only changes shared numeric dimensions.
Force all affected units, compare object sections and all strict scores,
independently resolve reviewed function words/calls/addresses, then run modern
checking, inventory, Ruff, existing tests, whitespace and full `kf build`.
Refresh existing literal ledgers and document the complete database/startup
functions' retained numeric uses. No new test, bank or compiler change is
proposed.

The same reviewed bodies expose two further existing dimensions: the world
clear at `8001535c` covers the complete 8500-byte `world_state`, and the item
file-size rounding at `80021298..2a8` uses 2048-byte CD sectors. Derive the
world clear count from that owner and use the already shared CD sector size
and shift, retaining the partial-sector guard and its unsigned shift sequence.

| GAME address | Function | Bytes | Strict match (before = after) | Per-function evidence and hypothesis |
| --- | --- | --- | --- | --- |
| `0x80020cfc` | `item_load_database` | 1500 | 99.746666% | Copy endpoints: 912 assets, 2376 windows, 1600 item names, 180 spell names, 320 bytes per price bank; model-search loop has exclusive bound 80. |
| `0x80021538` | `item_menu_buy` | 1476 | 99.769646% | Two item-order bands cover IDs 42..79 then 0..41; ten signed halfwords per name; eighty-row workspaces and two-column halfword prices. |
| `0x80021afc` | `item_menu_sell` | 1280 | 99.609375% | Eighty-row workspace; only allowed unequipped stock rows appended; ten-halfword copies and two-column sale prices. |
| `0x800151cc` | `game_state_initialize` | 740 | 100.0% | Raw count 0xef at 8001537c clears all 240 bytes by decrementing through zero; all player/shop seeds stay unchanged. |
| `0x80022608` | `menu_use_item_panel` | 1908 | 99.48218% | Fifty label rows and two 56-byte workspaces remain distinct; only name width and exclusive item bound 80 change spelling. |
| `0x8002317c` | `menu_magic_panel` | 1328 | 100.0% | Ten-row label workspace and sixteen code bytes remain; copy/stride uses the shared ten-halfword name format. |
| `0x800238d8` | `menu_equip_select` | 1476 | 100.0% | Twenty-row workspace remains; category/type boundary and all equipment stores remain; name copy/stride is ten halfwords. |
| `0x80023e9c` | `menu_spell_select` | 1136 | 99.54225% | Twenty-row workspace and typed selected-spell byte array remain; name copy/stride is ten halfwords. |
| `0x800249a8` | `menu_drop_item` | 1212 | 99.54785% | Full eighty-entry stock scan and parallel workspace extents; ten glyphs per copied row. |
| `0x800292f8` | `menu_draw_item_name_frame` | 1976 | 95.37652% | Copies ten name halfwords to a positioned label; keep model/GTE call set and transforms. |
| `0x8002ad6c` | `menu_list_init` | 140 | 100.0% | Copies ten title halfwords from the loaded window row, then retains its distinct default stride eight in the return slot. |
| `0x800279c4` | `menu_item_model_preview` | 440 | 98.181816% | Item-name copy uses ten signed halfwords; quantity formatting remains width two. |
| `0x80027b7c` | `menu_draw_item_detail` | 732 | 91.28416% | Ten-halfword name copy; pointer to a two-column price row remains u16 with the named shop dimension. |
| `0x8002b73c` | `save_file_write_slot` | 1268 | 100.0% | Header-only control: copies 240 stock bytes at payload +0x2440; full object and byte alignment retained. |
| `0x8002beb0` | `save_file_read_slot` | 972 | 100.0% | Header-only control: copies payload stock +0x2440..+0x2530 to the same live owner. |
| `0x80025da0` | `menu_config_panel_draw` | 408 | 100.0% | Header-only ABI control: two complete 24-byte positioned labels passed by value; private Y updates and s32 settings preserved. |

## Recovered dimensions

The hash-identical `COM/STAT.DAT` is 5708 bytes, SHA-256
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
The loader copies six consecutive banks, ending exactly at the file boundary:

| Bank | File offset | Bytes | Source shape |
| --- | --- | --- | --- |
| Menu assets | 0 | 912 | Complete `KfMenuAssets` |
| Window layouts | 912 | 2376 | Nine records, each one title plus ten positioned rows |
| Item names | 3288 | 1600 | Eighty rows of ten signed halfword codes |
| Player magic names | 4888 | 180 | Nine rows of ten signed halfword codes |
| Buy prices | 5068 | 320 | Eighty items by two unsigned halfword shop columns |
| Sell prices | 5388 | 320 | Same dimensions, separate price policy |

`KF_ITEM_COUNT` is the database extent, including currently unnamed item-ID
gaps. It excludes the `KF_ITEM_NONE` sentinel 255. It does not replace the
separately named CD file-table capacity merely because both presently equal
eighty. `KF_ITEM_SHOP_COUNT` names the two price columns; `shop_id - 1` selects
their zero-based column. No shop character identity is inferred.

`KF_ITEM_STOCK_BANK_COUNT` names the independently supported three banks:
player quantities at zero and shop availability at one and two. The 240-byte
live owner occupies GAME `800652a8..80065398`; its saved copy occupies payload
`+2440..+2530`. Both now share their declaration dimensions. The bank count is
the literal enum value three, which the inventory parser can resolve directly.
Ordinary shop entries are availability flags; the Gold Cross entry is the
exceptional finite stock that the buy path decrements.

`MENU_GLYPHS_PER_ROW` describes all ten-halfword item/spell names, positioned
label codes, list titles, name-copy bounds and seven caller-supplied strides.
Those loops copy all ten codes, including terminators and trailing codes.
`MENU_WINDOW_ROW_CAPACITY` separately describes the ten positioned rows after
the title. Neither name changes the list initializer's eight-glyph default.
Workspace row counts 50, 20 and 10, and parallel byte capacities 56, 20 and 16,
remain separate; their original extra-capacity rationale is not proved.

The startup countdowns derive from the complete 8500-byte saved-world owner
and 240-byte stock owner. `sizeof(owner) - 1` preserves each zero-byte store,
pointer advance and decrement-to-minus-one termination. The CD size expression
reuses `KF_CD_SECTOR_BYTES` and `KF_CD_SECTOR_SHIFT`: test the low 11 bits, and
only for a partial sector add one to the truncated sector count before shifting
back to bytes. Already aligned sizes retain their original value.

## Verification and retained literals

Each function in the snapshot table has the final verdict **unchanged**:
its strict score, resolved instruction words, ordered calls and address
referents agree with the captured pre-edit version. This checks 4502 candidate
words, 254 calls and 364 address references. The seven exact rows additionally
reproduce all 1583 retail words and the independently resolved target objects.
The other nine retain their existing non-exact results; no residue attribution
or new exact claim follows from this naming change.

Concurrent renderer work was committed independently as `8103d63`. An isolated
build of that commit, followed by an otherwise identical build with only these
dimension edits, produces identical sections in all 112 source objects.
The live build's allocated sections, runtime symbols and ordered relocations
also agree with that isolated result. Of the 484 scores captured at `8def349`,
only the two sprite blitters and string renderer change, exactly as documented
in the independent renderer commit. This campaign causes no score movement.

All eleven affected units were forcibly compiled. Inventory validation, Ruff,
all 678 existing tests (79.212 seconds) and whitespace checks pass. The existing
stock test now recognizes the named declaration bounds; its BSS initializer
check remains restricted to declarations rather than runtime element writes.
No test or C size assertion was added. Modern checking retains the same 320
error diagnostics and 64/112 passing source/image variants. Full `kf build`
was run and retains the existing data/placement gates: data PSX 0/1, GAME 9/42,
OPEN 2/19; target relinks 1/1, 75/77 and 34/38; six conflicting section bases
and zero artifact failures. These failures are not reported as a passing build.

The C census has **52 fewer inline numeric occurrences** from this campaign;
shared header declarations also use the new dimensions. The
[database/startup ledger](game-item-database-literal-ledger.md) explains all
120 retained occurrences in those complete functions (22 database, 98 startup).
Six existing ledgers are refreshed: equipment 120, spell 94, item use/discard
101, shops 139, preview 192 and window/list initialization 15. Their scopes
remain explicit; other functions in the same translation units are not
silently declared audited. Initial stock quantities, tuning choices, path
digits, zero origins, null pointers and representation arithmetic retain
individual reasons without inventing an original design rationale.
