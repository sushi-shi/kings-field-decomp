# Shop-ID propagation

## Function Match Plan

Introduce `KfShopId` with byte storage and the first/second shop identities,
derived from the already established stock-bank constants. Carry it through
the optional shop-byte reader and the four shop-specific signed-word interfaces
using `KF_ENUM_PARAM(KfShopId, s32)`. Encode only at stock/price indices and the
shared confirmation widget. Decode the widget's word payload in its detail
branches without introducing a byte truncation in the retail C view.

The [stock-owner review](item-stock-banks.md) establishes player bank zero and
shop banks one/two. The [shop controls](game-shop-menu-controls.md) and
[character boundary](map-character-domain.md) establish the caller chain. The
character's encoded byte enters the variadic menu dispatcher; only its shop
branch assigns shop meaning. Character fields remain character IDs.

Current source/history and cached GAME disassembly/CFG, callers, callees,
strings, data references and historical match records were reviewed. Neighboring
pickup, list and window helpers keep their separate roles and SDK callees.
All addresses below identify GAME.EXE; stored percentages are historical evidence
only. Builds, compiler checks, tests and post-edit matches remain deferred.

| Function | Address / extent | Evidence snapshot and intended change |
| --- | --- | --- |
| `item_menu_root` | `0x800212d8 / 0x260` | Frame 48 bytes; preserves a0 as a word and forwards it in the delay slots at `0x800213f8` and `0x80021408`. Type the argument and direct buy/sell calls. |
| `item_menu_buy` | `0x80021538 / 0x5c4` | Frame 1856 bytes; the sequence at `0x80021588..a4` computes shop ID times 80 into `item_stock`. Encode that index and two price-column origins; preserve the word stored for the widget at `0x80021798`. |
| `item_menu_sell` | `0x80021afc / 0x500` | Frame 1856 bytes; stock remains the player's bank, while the shop word selects a price column and is forwarded to detail drawing and the widget (`0x80021d1c`). |
| `menu_draw_item_detail` | `0x80027b7c / 0x2dc` | Frame 160 bytes; retains a1 as a word at `0x80027b9c`, then uses the one-based shop ID for the selected price table. Type the argument and encode its price-index arithmetic. |
| `menu_list_interact` | `0x80028380 / 0x354` | Frame 104 bytes; loads the unsigned word payload at `0x800283cc` and forwards its bits in a1 to item detail. Decode only in the three source detail branches. Non-shop previews ignore this parameter. |
| `menu_enter_mode` | `0x80036e38 / 0xc8` | Frame 24 bytes; spills argument homes and loads the optional byte at `0x80036ed0` before the root call. Type that byte in both legacy and modern branches; retain promoted-int variadic reading in modern mode. |

## Domain boundaries

`KF_SHOP_FIRST` and `KF_SHOP_SECOND` describe table order, not recovered merchant
names. They encode 1 and 2, exactly as the corresponding stock rows. The first
shop also defines the origin of zero-based price columns. Player bank zero is
not a shop and remains an integer stock index.

The shared list widget retains its `u32 shop_id` transport parameter: five
non-shop callers pass an ignored zero. The two shop callers encode the signed
word before the existing unsigned parameter conversion. Detail branches decode
with `KF_ENUM_DECODE(KF_ENUM_PARAM(KfShopId, s32), shop_id)`, retaining the existing
word conversion to the signed detail argument. No enum range validation or
clamping is introduced. The optional reader remains the byte-narrowing boundary.

Stock contents, prices, quantities, item IDs, loop controls, window arguments,
return values, call order and referents remain unchanged. The separate accepted
catalogue result `3` in the save panel remains explicitly unresolved; the write
path's same-valued no-space result does not establish its catalogue meaning.

## Source result

All six planned functions have the same final verdict: source propagation
reviewed, post-edit compiler and match verification deferred. The four typed
interfaces, byte reader, two outgoing widget arguments, three incoming detail
branches, stock index and four price-column origins now carry explicit domain
boundaries. Four curated signatures follow the source types without an evidence
tier change. The reviewed diff preserves the values, widths and control flow.

The detail-price ledger expression was reconciled. Source accounting still
covers all 111 C files and 6,002 retained numeric/character occurrences, each
with a reason. This is documentation coverage, not binary verification. No
builds, compiler checks, tests, post-edit matches or banking ran. Remaining
field identities and semantic/type review keep the full naming goal open.
