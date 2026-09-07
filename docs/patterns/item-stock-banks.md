# Item-stock bank identities

## Function Match Plan

Name the three rows of GAME's shared item-stock owner: player quantities at
row zero and the first/second shop availability lists at rows one/two. Replace
all 62 direct row selectors in the sixteen functions below. Name the first-shop
origin in four conversions from shop ID to zero-based price column. Keep the
dynamic shop selector, all item IDs, counts, prices, loops and save representation.
These are shared integer array-index constants, not flags or item IDs.

Current source, history and cached image-specific retail disassembly/CFG,
callers, callees, strings and historical match records establish the family.
Each function has validated references to the same stock owner or its interior
item bytes. The startup stores and buy/sell consumers distinguish the rows;
this is game policy, not an SDK or vendored routine. Historical match records
are not post-edit verification. No builds, compiler checks, tests, post-edit
matches or banking will run until the naming pass is finished.

All addresses identify GAME.EXE. Each listed function receives only direct
stock-bank substitutions; the occurrences column counts those substitutions.
The buy panel additionally has two price-column conversions, and the sell and
item-detail panels have one each.

| Function | Address / extent | Occurrences | Stock reference snapshot |
| --- | --- | ---: | --- |
| `item_menu_buy` | `0x80021538 / 0x5c4` | 3 | `8002159c: 06 80 03 3c lui v1,0x8006 ` |
| `item_menu_sell` | `0x80021afc / 0x500` | 1 | `80021b64: 06 80 16 3c lui s6,0x8006 ` |
| `item_pickup_confirm` | `0x80021ffc / 0x2b8` | 2 | `80022030: 06 80 01 3c lui at,0x8006 ` |
| `map_restore_floor_state` | `0x80035e44 / 0x69c` | 2 | `80036440: 06 80 02 3c lui v0,0x8006 ` |
| `map_action_script_floor1` | `0x800343e0 / 0x58` | 1 | `800343e0: 06 80 02 3c lui v0,0x8006 ` |
| `map_action_script_floor3` | `0x80034610 / 0x90` | 1 | `80034610: 06 80 02 3c lui v0,0x8006 ` |
| `map_floor5_transition_cutscene` | `0x800346a8 / 0x38c` | 1 | `800346e0: 06 80 01 3c lui at,0x8006 ` |
| `map_event_interact` | `0x80034a80 / 0x2d4` | 8 | `80034ad8: 06 80 05 3c lui a1,0x8006 ` |
| `menu_use_item_panel` | `0x80022608 / 0x774` | 1 | `80022650: 06 80 02 3c lui v0,0x8006 ` |
| `menu_draw_item_detail` | `0x80027b7c / 0x2dc` | 1 | `80027d9c: 06 80 01 3c lui at,0x8006 ` |
| `menu_drop_item` | `0x800249a8 / 0x4bc` | 1 | `80024a08: 06 80 15 3c lui s5,0x8006 ` |
| `menu_item_model_preview` | `0x800279c4 / 0x1b8` | 1 | `80027b3c: 06 80 01 3c lui at,0x8006 ` |
| `menu_equip_select` | `0x800238d8 / 0x5c4` | 1 | `8002391c: 06 80 04 3c lui a0,0x8006 ` |
| `game_state_initialize` | `0x800151cc / 0x2e4` | 33 | `80015374: 06 80 03 3c lui v1,0x8006 ` |
| `player_death_restart` | `0x800154b0 / 0x19c` | 2 | `800154d8: 06 80 03 3c lui v1,0x8006 ` |
| `player_use_item` | `0x80018054 / 0x45c` | 3 | `800182c0: 06 80 01 3c lui at,0x8006 ` |

## Bank roles

The 240-byte `item_stock` owner begins at `0x800652a8`: three eighty-byte rows.
Startup clears the entire owner, then grants one Short Sword and one Medicinal
Herb in row zero. Its next fourteen stores initialize the first shop's available
items in row one; seventeen stores initialize the second shop in row two.
For example, the first shop's Short Sword store at `0x800153ac` targets owner
+0x50, and the second shop's Knight Sword store at `0x8001541c` targets +0xa2.
These agree with the current table dimensions and named item IDs.

Buying reads `item_stock[shop_id]` to construct the availability list, checks
player quantities in row zero against the stack capacity, and increments that
player row on purchase. Most shop entries are availability flags; the Gold Cross
entry alone is decremented by the current purchase path. Selling, equipping,
dropping, using, displaying quantities, scripted exchanges and revival all
consume row zero. Thus the labels identify ownership and table order without
inventing merchant names or treating all rows as interchangeable quantities.
The existing save payload copies the whole owner and needs no bank conversion.

## Remaining item-ID work

The initial review grouped byte-sized equipment IDs with the halfword selector
in floor records. The subsequent [renderer trace](floor-sprite-base-index.md)
corrects that assumption: the floor selector is `base_sprite_index`, and its
0xffff terminator belongs to the sprite-placement stream. Inventory item IDs
and their 0xff absent value form a separate domain. Propagating that domain
through equipment and shared menu payload boundaries remains open, as does the
floor placement's unconsumed byte +3.

## Source result

The three shared names now replace all 62 direct bank indices and all four
first-shop price-column origins. Reversing those substitutions reproduces the
ten C files exactly; removing the added constant definitions reproduces the
shared header. Item IDs, stock quantities, shop availability, prices, byte
accesses, loops and save payloads are unchanged by this source substitution.
No direct raw bank selector or raw price-column origin remains in `src`.

All 111 source files and 6,007 retained literal occurrences have matching
per-occurrence ledger rows with reasons. The 66 removed rows are selector/origin
tokens; equal-valued quantity and comparison tokens remain documented. Each of
the sixteen functions in the plan remains unverified after editing: no builds,
compiler checks, tests, post-edit matches or banking ran. Source substitution
and inventory accounting do not establish a binary match. The remaining unknown
fields and item-ID/type work keep the goal open.
