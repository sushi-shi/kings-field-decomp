# GAME item-preview rotation and constants

## Function Match Plan

Start at `e745fb7`, with freshly hash-verified GAME retail, six-view dossiers,
source histories and adjacent/caller controls under
`build/constant-names/menu-preview-transform/`. All selected units use
`probe-gcc257-o2-g0`. Review all eleven loader calls and all three preview
consumers. The game functions implement menu/TMD policy; RotMatrix at
8004e9b8 is independently attributed to Psy-Q Release 2.5 LIBGTE/GEO and
remains a provider boundary, as do other GTE/GPU/library calls.

Replace the address-only yaw-halfword alias with the complete SDK SVECTOR
`menu_item_preview_rotation` at GAME 80057b70. Three callers load its +2
halfword, add 16/16/8, mask to 0xfff, and pass the address minus two to
RotMatrix. That exact SDK body reads signed halfwords at offsets 0,2,4.
Retail 80057b70..80057b78 contains eight zero bytes; the next object is the
memory-card path at 80057b78. The SDK's complete vector includes its final
alignment halfword. Merge the two surrounding coverage gaps and interior
halfword inventory into this one owner; keep every numeric relocation target
at 80057b72, represented as owner+2. Do not keep an overlapping yaw global.

Define the shared initialized vector in the model-preview module and declare
it in game_menu.h. This is a logical source owner; the original TU boundary
is still WIP. Propagate SDK field/pointer types through all three readers and
the loader reset, eliminating backwards casts. Name the angle wrapping and
Q12 light coefficients using existing mathematical domains, and share the
16-unit list-preview rate while distinguishing the 8-unit pickup-preview rate.
Explain all retained literals in the three preview functions, model renderer,
and model load/release pair. Preserve the separate price, UI and loader-result
protocols; their caller families need their own complete type audit.

Rebuild the affected units, compare ordered numeric references and all current
scores, and require every banked function to remain exact. Any partial-function
code changes must be checked from their first real divergence. Check the new
initializer against retail and inspect the full build's ownership result.
Run modern compiler checks, lint, repository tests and whitespace checks before
commit. No size assertions or compiler steering are part of this change.

## Per-function starting snapshots

| GAME address / bytes | Function | Strict % | Relevant constraint |
| --- | --- | --- | --- |
| `0x800291ec / 268` | `menu_draw_two_option` | 100.0 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x800292f8 / 1976` | `menu_draw_item_name_frame` | 94.52227 | Signed item ID, shared yaw halfword, SDK vector base two bytes earlier; Q12 light rows and fixed translation. |
| `0x80029de0 / 1328` | `menu_draw_string` | 99.98795 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x8002adf8 / 172` | `menu_format_number` | 100.0 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x8002aea4 / 104` | `menu_load_item_model` | 100.0 | Conditional load and release policy; reset only yaw on successful/empty model selection, preserving other components and failure return. |
| `0x8002af0c / 60` | `menu_release_item_model` | 100.0 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x80027b7c / 732` | `menu_draw_item_detail` | 91.28416 | Signed item ID, shared yaw halfword, SDK vector base two bytes earlier; Q12 light rows and fixed translation. |
| `0x800279c4 / 440` | `menu_item_model_preview` | 98.181816 | Signed item ID, shared yaw halfword, SDK vector base two bytes earlier; Q12 light rows and fixed translation. |
| `0x8001ed38 / 88` | `menu_render_item_model` | 100.0 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x80028380 / 852` | `menu_list_interact` | 87.24413 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x80021538 / 1476` | `item_menu_buy` | 97.20054 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x80021afc / 1280` | `item_menu_sell` | 96.77187 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x80021ffc / 696` | `item_pickup_confirm` | 100.0 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x800238d8 / 1476` | `menu_equip_select` | 97.89973 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x80022608 / 1908` | `menu_use_item_panel` | 92.8218 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |
| `0x800249a8 / 1212` | `menu_drop_item` | 98.85478 | Reviewed caller/callee control; call sites, data referents and delay slots retained in its six-view dossier. |

## Numeric meanings

A full angle turn is 4096 units. List/shop previews advance 16 per draw
(1.40625 degrees, 256 draws per revolution); pickup preview advances 8
(0.703125 degrees, 512 draws per revolution). These are draw-call rates, not
an assumed real-time frequency: confirmation paths can draw a preview twice.
The original choice of these rates is unproven. Resetting yaw to zero starts
each successfully selected model at the same orientation.

The local light matrix has two rows (-1,-1,-1) in Q12 and a third all-zero
row. These are authored directional coefficients, not a normalized light
vector: each nonzero row has magnitude sqrt(3). Its translation fields are
not read by MulMatrix0/SetLightMatrix. The normal translation is (560,140,1500)
and pickup translation (220,140,1500), in the GTE transform's coordinate units.
The model renderer uses a projection-plane distance of 200. These facts
explain what the values do, without inventing the original placement rationale.

## Literal coverage

The [per-occurrence ledger](game-menu-preview-literal-ledger.md) accounts for
204 retained literals after the [shop price follow-up](game-shop-price-domains.md)
(initially 213) in the selected functions and edited-module initializers,
down from 236 in the captured source. It distinguishes matrix indices and Q12
coefficients, layout pixels and ordering-table positions, glyph data and field
widths, zero orientation and independent allocation/file-result protocols.
The four newly explicit initializer zeros are included. Other functions in
these shared modules remain part of the wider, unfinished constant audit.

## Verification

All 109 unaffected objects retain every non-debug section. Across the three
edited units, all 21 functions are compared after resolving relocations to
numeric addresses. Only `menu_draw_item_name_frame` changes linked instructions.
Its first change is at +0x34: twelve separate base/yaw-load/yaw-store setup
instructions become ten instructions using one owner-relative address. The
new +0x34..+0x5c sequence matches raw retail, including the yaw store in the
RotMatrix call delay slot. The prefix before +0x34 is unchanged, and every
word after old +0x64 equals the corresponding word after new +0x5c. The
ordered call list is unchanged. The three old initial address pairs (base,
yaw, yaw) become the single retail yaw pair; later data referents are unchanged.

Pickup preview improves 94.52227 → 95.37652%, with a compiled body of 1992
bytes against retail's 1976. The first remaining divergence is its 160-byte
frame versus retail's 224-byte frame and associated saved-register offsets.
This remains an unattributed reconstruction residue. All other 483 scores and
all prior exact results are preserved. No bank rows or compiler profiles change.

Source and delinked target each own the eight zero initializer bytes and match
raw retail. The full report changes only the new eight-byte data comparison,
its aggregate counters, and the pickup preview's fuzzy values. Full build
improves source-data matches from 7/60 to 8/61; SDK/config-data remains 4/4,
and target relink remains PSX 1/1, GAME 75/77 and OPEN 34/38. Existing two
GAME/four OPEN section-base conflicts remain, with no artifact failures.

The modern compiler has the identical multiset of 320 diagnostics: 64 passing
and 48 failing source/image variants. Ruff and whitespace checks pass. The full
656-test run takes 79.842 seconds and retains only the known failure in the
user's untracked save/load-hub test. No tests, source size assertions, tooling
implementation or flake are added or changed.

## Per-function final verdicts

| GAME address | Function | Final strict % | Verdict |
| --- | --- | --- | --- |
| `0x800291ec` | `menu_draw_two_option` | 100.0 | Exact; 67 complete retail words |
| `0x800292f8` | `menu_draw_item_name_frame` | 95.37652 | Reviewed rotation setup; partial |
| `0x80029de0` | `menu_draw_string` | 99.98795 | Unchanged linked code; partial |
| `0x8002adf8` | `menu_format_number` | 100.0 | Exact; 43 complete retail words |
| `0x8002aea4` | `menu_load_item_model` | 100.0 | Exact; 26 complete retail words |
| `0x8002af0c` | `menu_release_item_model` | 100.0 | Exact; 15 complete retail words |
| `0x80027b7c` | `menu_draw_item_detail` | 91.28416 | Unchanged linked code; partial |
| `0x800279c4` | `menu_item_model_preview` | 98.181816 | Unchanged linked code; partial |
| `0x8001ed38` | `menu_render_item_model` | 100.0 | Exact; 22 complete retail words |
| `0x80028380` | `menu_list_interact` | 87.24413 | Unchanged linked code; partial |
| `0x80021538` | `item_menu_buy` | 97.20054 | Unchanged linked code; partial |
| `0x80021afc` | `item_menu_sell` | 96.77187 | Unchanged linked code; partial |
| `0x80021ffc` | `item_pickup_confirm` | 100.0 | Exact; 174 complete retail words |
| `0x800238d8` | `menu_equip_select` | 97.89973 | Unchanged linked code; partial |
| `0x80022608` | `menu_use_item_panel` | 92.8218 | Unchanged linked code; partial |
| `0x800249a8` | `menu_drop_item` | 98.85478 | Unchanged linked code; partial |
