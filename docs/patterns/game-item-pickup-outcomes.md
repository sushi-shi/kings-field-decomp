# GAME item-pickup outcome types

## Function Match Plan

Starting from `fd3ecb9`, freshly hash-verify GAME retail and inspect the pickup
confirmation at 80021ffc, its sole direct caller `menu_enter_mode`, all three
pickup call sites in `map_interaction_dispatch`, and the root-menu/preview
controls. Capture complete item and adjacent menu functions for context.
Six-view dossiers and source histories are under
`build/constant-names/item-pickup-outcomes/`; the full object/score baseline
is `build/constant-names/post-master-fd3ecb9/`.

Correct `item_use_confirm` to `item_pickup_confirm`: retail loads the player's
stock byte, tests against 99, and increments it on acceptance. Give its s32
result a stored enum with pending -99, acquired 0, not acquired 1 and stack
full 2. The pending value is internal loop state and never a completed
return. Keep cancellation and model-load failure together, as retail does.
Propagate the result type through the function declaration/definition, local,
and all three pickup consumers. The shared variadic mode dispatcher returns
different domains, so explicitly encode the pickup result there and decode
only at known pickup-mode calls. Do not assign pickup meanings to root-menu
item IDs or exit codes.

Name the shared player stack limit in pickup and both buy-list scans. Preserve
the pickup equality test versus the buy-list strict upper bound. Replace the
pickup's two raw halfword label arrays with the existing positioned glyph
type consumed by `menu_draw_two_option`; use its field names and terminator.
Account for every retained literal in pickup and the complete mode dispatcher.

These functions are game-owned inventory/UI orchestration, absent from the
vendored inventory. SDK pad/GPU/render calls remain API boundaries. The
adjacent buy/sell/menu bodies, existing candidate signatures and incomplete
matches do not become new exact claims. Preserve widths, byte stock access,
call sets, CFG, delay slots, ordered relocations and all banked functions.
Force compilation, compare normalized symbols plus unchanged object bytes and
strict reports, check exact bodies against retail, and run modern Clang,
repository tests/lint and full `kf build` before commit. No new size assertions.

## Direct evidence

At GAME 8002203c, `lbu` reads bank-zero stock into s6 before model loading.
The model-load failure branch supplies result 1. At 80022200..08, stock equal
to 99 selects result 2. Otherwise 80022218..38 reloads the stock byte,
increments and stores it, then sets result 0 in the jump delay slot. No/cancel
sets result 1 at 80022258. The loop starts with -99 in s4 and exits only after
it changes, releasing the model before returning s4. This directly contradicts
the prior inventory identity's description of consuming normal items and
sparing key items.

The three map calls at 800352bc, 80035348 and 80035520 all use mode 1, remove
the item only on result 0, and enqueue notification 16 only on result 2. Other
outcomes retain the item. The mode wrapper at 80036ec0 forwards the pickup
result unchanged; its root path forwards an item ID or a negative menu result,
while its shop path returns zero. Its original O32 home-slot varargs access
remains separate portability debt.

The two buy scans use `sltiu ...,99` at 800215dc and 8002168c. Thus 99 is a
player stack limit, unrelated to equal-valued glyph or actor-drop codes. The
pickup comparison intentionally rejects exactly 99, not every value above
99; do not silently repair out-of-domain save data during a naming change.

## Typed labels and retained literals

The two stack labels have the shared `MenuGlyphString` layout: X/Y at bytes
0/2, then signed glyph codes at byte 4. The first label begins at (60,26),
the second at (60,46), twenty pixels lower. The renderer receives each complete
object and uses the selected index for its cursor/highlight. The acceptance
branch is index zero, so `accept_label` and `decline_label` describe the
observed actions without guessing glyph translations. Typed fields remove
the array-to-structure casts; unconsumed tail glyphs remain uninitialized.

The following ledger covers all 50 ordinary literals in the pickup function
and complete `menu_enter_mode.c`, down from 66 in the captured source.
Function claims are addresses/extents, and
named enum values are definitions rather than unexplained uses. Line numbers
refer to this final source; occurrence counts include both tokens on one line.

| Source / lines | Literals | Occurrences | Reason retained inline |
| --- | --- | ---: | --- |
| `item.c:490` | `0` | 1 | Initial selection is the first label; two-option index, distinct from the result code. |
| `item.c:491,560` | `0`, `1` | 2 | Confirmation highlight starts clear and is set on confirm; shared renderer's Boolean presentation input. |
| `item.c:492` | `0` | 1 | No prior pressed buttons before the first edge comparison. |
| `item.c:497,566` | `0` | 2 | Player stock bank; banks one and two belong to shops. |
| `item.c:498` | `0` | 1 | Model loader's zero-success/nonzero-failure boundary; preserve its complete nonzero predicate. |
| `item.c:501,506` | `0x3c` | 2 | Authored X coordinate 60 pixels for both option labels. |
| `item.c:502,507` | `0x1a`, `0x2e` | 2 | Authored Y coordinates 26/46 pixels, giving twenty-pixel row spacing; original placement rationale unknown. |
| `item.c:503,504,508,509,510` | `0x53`, `0x6a`, `0x63`, `0x61`, `0x6a` | 5 | Authored atlas glyph IDs: とる (take) and やめる (cancel), decoded in the [later font audit](game-menu-confirmation-protocol.md). Glyph 99 is unrelated to the player stack limit. |
| `item.c:503,504,505,508,509,510,511` | `0,1,2` and `0,1,2,3` | 7 | Consecutive glyph positions, including the terminator after two/three glyphs. |
| `item.c:517,523,529` | `0,0` | 6 | Initial frames select the first option and leave its confirmation highlight clear. |
| `item.c:531,543,550` | `1` | 3 | Retail PadRead call-site argument. The linked SDK routine reads global PadIdentifier and ignores this argument; it is not a controller-port selector. |
| `item.c:531,543` | `0` | 2 | Wait until no buttons remain pressed before entering or leaving the modal loop. |
| `item.c:551,552,558,570` | `0,0` | 8 | Current button bit set and previous bit clear identify an input edge. |
| `item.c:554,555,557,561` | `0,0,1,0` | 4 | Toggle the two positional choices and test whether the second, declining option is selected. These are renderer indices, not item IDs or pickup outcomes. |
| `menu_enter_mode.c:24` | `0` | 1 | SDK DrawSync synchronization selector before resetting the rendering heap. |
| `menu_enter_mode.c:32,35` | `4` | 2 | O32 word stride from the named mode argument to the first optional argument's home slot; retail reads that argument's low byte. Preserve the known nonportable ABI expression. |
| `menu_enter_mode.c:36` | `0` | 1 | Shop mode returns zero after its void panel; its caller ignores the result. This is not a pickup-success result. |

The signed pending value -99 is outside the completed pickup result set; its
original numerical choice remains unproven. The three final outcomes are
named for the actual inventory effect. The two label terminators reuse
`MENU_TEXT_END`, and the stack limit is shared with both buy-list scans.
The refreshed map-script ledger has 320 retained literals in 179 groups,
down by six numeric pickup-result comparisons.

## Verification and remaining mismatch

Forced compilation passes for item, mode-entry, map-script and menu-runtime
units. All 112 objects retain identical data and relocation bytes and symbol
properties after normalizing the one function rename. Every function's code
is unchanged except the already partial `map_interaction_dispatch`: 80 words
differ only in reviewed GPR fields. Instruction count/order, opcodes, immediate
values, branch destinations and delay-slot positions are unchanged. The
other functions in that same object remain byte-identical.

The first changed source-object instruction is dispatcher +0x21c: the overlap
result is held in s3 instead of s2. Later object pointers move from s3 to s2,
container counters from s1 to s0, and the separate pickup outcomes from s0 to
v1. Each of the three retail pickup call sites also copies v0 to v1. These
are observed register changes following a semantically distinct local, not
historical compiler attribution. Source control flow still differs from
retail; the function is not exact and is not banked.

Its strict score moves from 83.04679% to 83.436745%. The other 483 scores are
unchanged, including every banked function. The normalized full report changes
only that function and its aggregate percentages. All five exact reviewed
controls reproduce 415 complete retail instruction words after independent
relocation application, with identical ordered calls and data referents.

Modern Clang retains 64 passing and 48 failing source/image variants; all
320 error diagnostics are unchanged after symbol/line normalization. The
typed pickup and mode results introduce no new errors. Ruff and whitespace
checks pass. The repository suite runs 656 tests in 84.197 seconds: 655 pass,
with only the known untracked save/load-hub mismatch failing. No new tests,
size assertions or SDK-body reconstruction are introduced.

Full `kf build` retains its existing closure failures: source data 7/60,
SDK data 4/4, target relink PSX 1/1, GAME 75/77 and OPEN 34/38. This campaign
does not resolve data placement or incomplete ownership. The overall naming
goal remains open, including the ten source `unknown_` occurrences.

## Function snapshots and final verdicts

All rows select GAME.EXE and `probe-gcc257-o2-g0`. Complete pre-edit dossiers
are retained for adjacent and caller/callee controls as well as changed bodies.

| GAME VA / bytes | Function | Initial → final strict % | Verdict |
| --- | --- | ---: | --- |
| `0x80018880 / 6684` | `player_update` | 96.94554 → same | Caller control; code unchanged |
| `0x80020b4c / 432` | `item_load_floor_placements` | 98.888885 → same | Adjacent control; code unchanged |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666 → same | Adjacent control; code unchanged |
| `0x800212d8 / 608` | `item_menu_root` | 100 → same | Exact / 152 retail words |
| `0x80021538 / 1476` | `item_menu_buy` | 97.20054 → same | Shared stack limit; code unchanged |
| `0x80021afc / 1280` | `item_menu_sell` | 96.77187 → same | Adjacent control; code unchanged |
| `0x80021ffc / 696` | `item_pickup_confirm` | 100 → same | Renamed/typed; exact / 174 retail words |
| `0x800222b4 / 148` | `menu_save_confirm` | 100 → same | Adjacent control; exact / 37 retail words |
| `0x80022348 / 704` | `menu_root` | 96.86364 → same | Other return-domain control; code unchanged |
| `0x80022608 / 1908` | `menu_use_item_panel` | 92.8218 → same | Other return-domain control; code unchanged |
| `0x800292f8 / 1976` | `menu_draw_item_name_frame` | 94.52227 → same | Preview control; code unchanged |
| `0x80034de4 / 2308` | `map_interaction_dispatch` | 83.04679 → 83.436745 | Typed pickup local; reviewed GPR changes |
| `0x80036e30 / 8` | `func_80036e30` | 100 → same | Unresolved adjacent stub; exact / 2 retail words |
| `0x80036e38 / 200` | `menu_enter_mode` | 100 → same | Encoded mode boundary; exact / 50 retail words |
