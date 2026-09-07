# GAME menu-window layout selectors

## Function Match Plan

Starting from `5659505`, give the loaded window selector a shared signed-word
enum, propagated through `menu_draw_window`, `menu_two_option_prompt`,
`menu_list_init`, every reconstructed caller, and direct shop-label indexing.
Keep row indices and counts distinct from window kinds. Rename the misleading
list initializer's `row, column` parameters to `kind, row`.

Before editing, capture all six image-qualified semantic views for each
affected GAME function, all 112 source objects and all 484 strict scores.
Review caller argument setup and delay slots, the adjacent window/list/text
helpers, source history, loaded resources and vendor inventory. The decoded
game menu policy is absent from the vendored inventory; SDK input, GPU and
file-loading routines remain provider boundaries.

The 31 proven window-draw calls pass kinds 0/1/2/4/5/6/7 or forward the
prompt's kind. Its three proven callers pass 2/4/5. The seven list-init calls
pass (kind,row) = (7,0), (7,1), (0,0), (0,1), (1,category), (1,1), (0,4).
All selectors are full-width register arguments: a0 for window/prompt and a1
for list initialization. There is no new enum-typed stored field in this
campaign. The modern view preserves the enum domain; the retail view keeps
the existing signed 32-bit ABI.

Retail window drawing multiplies the selector by 264 and rows by 24. It
omits the shared backdrop only for kind 6. The prompt's unsigned
`(kind - 4) < 2` gate forwards save summaries only for kinds 4/5; retain this
set and its emitted instructions. List initialization copies ten halfwords
from layout +28 plus row*24; its return delay slot stores glyph width eight.
Keep all input waits, row ordering, numeric values, resource bytes, calls,
delay slots and ordered relocation targets unchanged.

Force all affected compiles and compare every non-debug object section,
all strict scores and each affected function's linked words and referents.
Require every existing exact to remain exact. Run modern type checking,
inventory validation, Ruff, repository tests, whitespace checks and full
`kf build`. This is a naming/type propagation batch, with no new banking or
compiler attribution claim.

## Loaded records and names

`item_load_database` copies 2376 bytes from `COM/STAT.DAT` offset `0x390`:
nine 264-byte records, each containing a title and ten positioned row labels.
The next item-name bank begins at `0xcd8`. The file is 5708 bytes; SHA-256:
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.

The glyphs were rendered from the shipped `COM/MIX.TIM` font, palette at
231572 and texture at 232096, with the established 14-by-12 grid and voiced
mark flags. MIX.TIM SHA-256:
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.

| Index | Name suffix | Decoded resource and consumer evidence |
| --- | --- | --- |
| 0 | ROOT | No title; use item, use magic, equipment, status, discard, system, options, return. `menu_root` dispatch agrees. |
| 1 | EQUIPMENT | 装備 title; weapon, magic, shield, head, torso, arm, leg, other, return. Equipment and spell lists copy their respective row labels. |
| 2 | SYSTEM | システム title; load, quit game, return. `menu_save_load_hub` uses this record despite its older function name. |
| 3 | SAVE_LOAD | セーブ・ロード title; save, load, quit game, return. Resource identity is decoded, but no confirmed selector call uses this record. |
| 4 | SAVE | セーブ title; three area slots, card initialization, return. The save panel writes slots or formats the card. |
| 5 | LOAD | ロード title; three area slots and return. The load panel reads a selected slot. |
| 6 | CONFIG | オプション title; effects volume, music volume, status display, direction display, return. The configuration panel draws its own accompanying controls. |
| 7 | SHOP | 買う・売る title; buy, sell, return, plus a separate gold label. Its existing shop constant acquires the shared enum type. |

Record 8 is 264 zero bytes. Retain it in the named nine-record bank extent,
without inventing a selectable window identity. Zero glyph words are the
font's first glyph, not text terminators; the all-zero record is therefore
not evidence for a valid empty rendered menu. Resource presence alone also
does not prove that record 3 is reachable.

## Verification and function verdicts

The new names replace 33 raw call arguments, the backdrop exclusion, two
range-test literals and the source bank extent: **37 source occurrences**.
The named bank extent is also shared by the header declaration. The prompt
now spells equality with SAVE or LOAD; its object is byte-identical to the
previous unsigned-range source, including its delay-slot subtraction by four.
Only conversion to a table index encodes a window enum as an integer.

All ten affected units were forced to compile. All **112 complete object
files are byte-identical** to the pre-edit snapshot, and all **484 strict
scores are unchanged**. Each row below also passed complete linked-word,
ordered call and data-address comparison. All prior 100% rows agree with
the raw retail instructions and the delinked target object. Other rows keep
their existing residues; no function is newly exact or banked.

| GAME VA / retail bytes | Function | Before = after % | Source words / calls / address pairs |
| --- | --- | --- | --- |
| `0x80020cfc / 1500` | `item_load_database` | 99.7467 | 375 / 5 / 10 |
| `0x800212d8 / 608` | `item_menu_root` | 100 | 152 / 25 / 0 |
| `0x80021538 / 1476` | `item_menu_buy` | 99.7696 | 369 / 21 / 11 |
| `0x80021afc / 1280` | `item_menu_sell` | 99.6094 | 320 / 20 / 5 |
| `0x800222b4 / 148` | `menu_save_confirm` | 100 | 37 / 7 / 2 |
| `0x80022348 / 704` | `menu_root` | 96.8636 | 175 / 28 / 1 |
| `0x80022608 / 1908` | `menu_use_item_panel` | 99.4822 | 477 / 24 / 27 |
| `0x8002317c / 1328` | `menu_magic_panel` | 100 | 332 / 20 / 20 |
| `0x800236ac / 556` | `menu_option_root` | 100 | 139 / 20 / 2 |
| `0x800238d8 / 1476` | `menu_equip_select` | 100 | 369 / 25 / 13 |
| `0x80023e9c / 1136` | `menu_spell_select` | 99.5422 | 284 / 20 / 3 |
| `0x800249a8 / 1212` | `menu_drop_item` | 99.5478 | 303 / 20 / 3 |
| `0x80024e64 / 608` | `menu_save_load_hub` | 100 | 152 / 20 / 2 |
| `0x800250c4 / 1128` | `menu_save_panel` | 100 | 282 / 58 / 0 |
| `0x8002552c / 880` | `menu_load_panel` | 100 | 220 / 42 / 0 |
| `0x8002589c / 1284` | `menu_config_panel` | 99.8598 | 321 / 19 / 9 |
| `0x80027b7c / 732` | `menu_draw_item_detail` | 91.2842 | 183 / 17 / 20 |
| `0x800286d4 / 576` | `menu_two_option_prompt` | 100 | 144 / 16 / 0 |
| `0x80028914 / 348` | `menu_draw_window` | 87.5402 | 88 / 6 / 8 |
| `0x8002ad6c / 140` | `menu_list_init` | 100 | 35 / 0 / 1 |

The modern compiler keeps the same 320 diagnostics, with 64/112 image/source
variants passing. Temporary compilations using the actual compilation
database accept correctly typed calls to all three APIs and reject both raw
integers and an unrelated confirmation enum at each selector argument. No
new test files, suppressions, size assertions or cast-based API bypasses were
added. Explicit enum-to-index conversion occurs at the array boundary.

The existing inventory test caught the old `menu_list_init` signature in its
semantic evidence row. Updating that row alongside the identity table restores
consistency; the test itself is unchanged. The final suite passes **678 tests
in 83.694 seconds**. Ruff and whitespace checks pass. The suite includes the
concurrent trial-flow work, which is outside this naming commit.

Full `kf build` retains the existing data ownership/placement failures:
PSX 0/1, GAME 9/42 and OPEN 2/19 source-data units exact; target relink is
PSX 1/1, GAME 75/77 and OPEN 34/38. The same two GAME/four OPEN conflicting
section bases remain, with zero artifact failures. No tooling or flake
implementation changed in this batch.

The [16-occurrence ledger](game-menu-window-literal-ledger.md) completes
window-renderer and list-initializer coverage. Existing equipment coverage
drops from 141 to 139 after naming the two list window selectors; preview
coverage remains 196 with its enum-to-index expressions refreshed. These
counts do not claim a complete literal audit of the remaining caller bodies.
