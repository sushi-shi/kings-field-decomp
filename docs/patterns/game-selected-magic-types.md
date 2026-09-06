# Selected-spell enum storage and propagation

## Function Match Plan

Start at `a199baf` on master with freshly hash-verified Japanese retail.
The project matcher workflow captures six GAME semantic views, source histories
and adjacent inventory under `build/constant-names/selected-magic-types/`.
All selected units use `probe-gcc257-o2-g0`; the profile remains a probe.

Introduce `KfSelectedMagicId` with byte storage for the five ranged spells
and none 255. The four instant spells are applied directly by another menu
and do not belong to this field domain. Propagate the selected-spell type
through the player field, selector parameter, update local and menu code
array. Keep encoded indices, generic texture/widget arguments and the broader
effect-kind boundary explicit. Preserve all legacy widths, promotions, data
offsets, call order, none handling and serialized player copies.

The functions implement game selection/state, menus, saving and effect policy;
they are absent from the vendored inventory. SDK memcpy, matrix, audio and
input services remain external. No library reconstruction is included.

Force affected compilation and require unchanged object sections, strict
scores, complete objdiff report and ordered referents. Check all exact controls
against raw retail words, run modern compilation and temporary compiler
accept/reject probes for the actual field/API, inventory validation, ruff,
repository tests, whitespace and full kf build. Add no size assertions or
per-field repository tests. Refresh affected literal ledgers; no new banking.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666 | Control: STAT.DAT spell rows immediately follow eighty 20-byte item rows; nine rows at offset 0x1318, names and runtime records share menu indices. |
| `0x8002317c / 1328` | `menu_magic_panel` | 95.89759 | Instant IDs 0..3; learned byte must equal 1. Preserve MP debit, HP halfword stores/cap and exact keep-masks 3/12; panel result also represents cancellation. |
| `0x80023e9c / 1136` | `menu_spell_select` | 98.06338 | Twenty byte-sized code entries contain only ranged IDs4..8 or none255. Type the array; decode its integer range iterator at insertion, encode only at texture/widget boundaries. Keep signed row selection separate. |
| `0x8002718c / 2104` | `menu_draw_name_list` | 100.0 | Read the typed selected spell, exclude none, then encode its index into the nine-row name table. |
| `0x800151cc / 740` | `game_state_initialize` | 100.0 | Starting selected spell ID8 is Light Needle; keep all stock and state initialization order. |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.0 | Dispoison learns at base magic37 with Healing known; Fire Wall at70, Lightning Bolt at75. Keep learned-byte conditions and notification calls. |
| `0x800167e4 / 100` | `player_select_magic` | 100.0 | Byte O32 input writes player+5c, masks to255 and resolves a 20-byte record pointer or null. Type field/parameter together; encode the record index explicitly. |
| `0x80018880 / 6684` | `player_update` | 96.94554 | Type the local spell copy; preserve typed calls/comparisons and encode only magic-record indices. Existing effect-kind IDs remain their broader domain. |
| `0x8003425c / 136` | `map_ambient_script_floor3` | 100.0 | Restoration region grants Resist Fire and Bless together; keep X15..17/Z64 and existing learned-byte checks. |
| `0x80034610 / 144` | `map_action_script_floor3` | 100.0 | Bracelet possession grants Wind Cutter; stage3 dialogue grants Fire Ball; preserve notification and progress referents. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Mirror of Truth exchange grants Healing; unrelated character, item, page, stage and link identities unchanged. |
| `0x8003a274 / 44` | `magic_load_records` | 100.0 | Control: copies all24 records, distinct from the nine displayed player spell names. |
| `0x8003a2a0 / 1216` | `magic_cast` | 98.89145 | Switch/comparisons retain the selected-spell type. Explicit encoding at the wider effect-kind constructor boundary preserves the five ranged IDs. |
| `0x80036f44 / 2092` | `effect_pool_construct` | 100.0 | Record indices now encode the named ranged-spell constants; effect kind23 normalization and broader effect-kind switches remain separate. |
| `0x8003781c / 52` | `effect_pool_set_current` | 100.0 | Control: runtime effect kind indexes the broader record table; do not narrow this to nine spells. |
| `0x80038a38 / 6156` | `effect_update_dispatch` | 96.93957 | Encode only named Lightning Bolt record indices; preserve all effect-kind comparisons and damage calls. |
| `0x8002b73c / 1268` | `save_file_write_slot` | 100.0 | Control: memcpy copies the serialized player prefix including byte+5c; learned flags are a separate24-byte table, not selected spell IDs. |
| `0x8002beb0 / 972` | `save_file_read_slot` | 100.0 | Control: raw player-prefix restore includes byte+5c and reconstructs selected-record pointer later through player_update. No new validation or per-field rewrite. |

## Recovered domain and storage

An intervening matching commit, `a665d54`, made GAME `0x80028380`
`menu_list_interact` exact (852 bytes, 100%). Its generic `s32 item_id`
argument has three magic-preview comparisons against none255. Fresh six-view
evidence, its caller/adjacent dossier and an exact pre-edit object are captured
as `widget-before*`. Encode none to `s32` at those comparisons; the generic
widget still accepts item IDs and instant-spell texture IDs. Require every
section of that exact object to remain unchanged and compare all213 retail
words/call referents again. This is an additional nineteenth reviewed function.

`KfSelectedMagicId` contains Lightning Bolt 4, Fire Ball 5, Fire Wall 6,
Wind Cutter 7, Light Needle 8 and none 255. Their names come from the
[retail spell labels](game-spell-identities.md). The ranged menu is the
only selection producer besides initialization and restoration: it builds
IDs 4..8 and appends none 255. Healing, Dispoison, Resist Fire and Bless are
applied immediately by `menu_magic_panel` and never assigned as the selected
spell by the observed callers.

Modern C++20 sees a scoped enum with `u8` underlying storage. The pinned
legacy compiler sees the existing `u8` typedef and integer enumerators.
The typed chain includes `KfPlayerState.selected_magic_id`,
`player_select_magic`'s parameter, the `player_update` local, and the ranged
menu's twenty-entry `codes` array. Direct field assignments, calls, enum
comparisons and switch cases preserve the domain throughout that chain.

GAME `0x800167ec/0x800167f0` stores the incoming byte at player+0x5c;
`0x800167f4` masks the input to 255. The none branch clears the pointer at
player+0x60; other values use the original 20-byte runtime-record stride.
The menu's final path at `0x800242cc..0x800242e4` loads the chosen byte,
stores it at+0x5c and calls the setter with a second byte load. Both stores
and their ordering remain in source. No initialization or new range check
has been inserted.

Clang's MIPS record-layout dump places the enum at decimal 92 (0x5c), the
next byte array at 93, and the selected-record pointer at 96 (0x60). The
compiled retail objects independently retain all storage accesses. No size
assertions were added to source, headers or compiler probes.

## Explicit boundaries

| Boundary | Representation | Reason |
| --- | --- | --- |
| Menu range iterator into typed code array | `KF_ENUM_DECODE(KfSelectedMagicId, code)` | The integer iterator visits the evidenced contiguous IDs4..8 before storing a selected-spell value. |
| Runtime record/name lookup | `KF_ENUM_ENCODE(u8, spell)` | A table index is an integer; scoped enums do not implicitly index arrays. |
| Texture loader | `KF_ENUM_ENCODE(u8, codes[index])` | This shared resource API consumes an encoded byte texture ID, including255 for no texture. |
| Confirmation widget | `KF_ENUM_ENCODE(s32, codes[index])` | Its generic detail argument is shared across item/spell modes; retain the established width. |
| Effect constructor | `KF_ENUM_ENCODE(u8, selected_magic_id)` | Its broader kind domain includes many effects beyond player spells. |
| Corresponding effect-kind constants | Explicit byte encoding of the spell constant | The matching numeric identities are established by the cast-to-constructor call chain; the two domains remain distinct. |
| Save/load | Existing player-prefix byte copy | Serialization already copies the field's storage; this change adds no per-field conversion or validation. |

The four instant-spell constants and the 9/24 table counts remain ordinary
integer constants. Menu cursor positions, cancellation -1 and pending -99
also retain their integer representations. None 255 belongs to the selected
spell domain and is not a menu cancellation value.

`KF_ENUM_DECODE` is a representation conversion, not a validator. Corrupt
serialized bytes or an explicit conversion can still create an unnamed enum
value; the original runtime behavior is preserved. The compiler now prevents
accidental implicit assignments and calls from other domains. This does not
claim runtime exhaustiveness or complete enum typing of all effect/item APIs.

## Compiler evidence

Temporary probes use the actual project header and the same Clang MIPS
C++20 arguments as the editor, rather than a separately modeled field.
The positive probe passes every named ranged spell/none through an array,
field, local, parameter and typed return. Five independent negative probes
are rejected:

| Probe | Rejected operation |
| --- | --- |
| Raw field assignment | `selected_magic_id = 8` |
| Instant spell assignment | `selected_magic_id = KF_MAGIC_HEALING` |
| Unrelated enum assignment | `selected_magic_id = KF_EQUIPMENT_SLOT_HEAD` |
| Erased API argument | `player_select_magic(KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHT_NEEDLE))` |
| Raw return | Returning 8 from a `KfSelectedMagicId` function |

Probe sources, diagnostics and the layout dump remain under the ignored
campaign directory. No per-field repository test or permissive compiler
option was introduced.

The affected [equipment](game-equipment-literal-ledger.md),
[floor-script](game-map-script-literal-ledger.md) and
[spell/menu](game-spell-literal-ledger.md) ledgers now show the explicit
encodings and typed array. Their retained literal counts remain 143, 296 and
127 respectively; this batch changes type propagation, not authored values.

## Verification and final verdicts

The first verification against `a199baf` preserved all 112 object sections,
all 484 strict scores and the full report. Matching commits subsequently
landed independently in the shared master checkout. Fresh verification
therefore compares all objects again and identifies each allowed difference
by unit and address instead of attributing those improvements to this enum.
The five independently changed units are `game.menu_item_drop`,
`game.menu_status_panel`, `game.menu_list_interact`, `game.menu` and
`game.item`. Their nine changed scores are recorded in
`recheck/concurrent-differences.json`; all other scores and object sections
remain unchanged against the original baseline.

Every enum-edited object is unchanged. The newly exact confirmation widget
is additionally identical in every section to its captured pre-edit object.
All nineteen reviewed functions retain their starting scores: thirteen exact
and six partial, including both exact save serialization controls. Raw
verification covers 2824 complete retail words for the exact functions,
including delay slots and ordered call/address referents. No new match or
banking claim belongs to this type-only batch.

Modern compilation remains 64/112 passing and 48 failing. After encoding the
widget's generic sentinel comparisons, the multiset of 320 diagnostics is
identical to the pre-enum baseline. The positive actual-header compiler probe
passes and all five negative probes fail for their intended type errors.
Inventory validation, repository lint and whitespace checks pass. The full
repository tests and `kf build` were rerun after the intervening matching
changes. All **657 tests pass** in 80.483 seconds. The earlier save/load-hub
test failure was resolved by the independent matching work.

The full build still fails its existing data/ownership checks: source data
matches 8/61, independent config contributions 4/4, target relinks PSX1/1,
GAME75/77 and OPEN34/38. The two GAME/four OPEN section-base conflicts remain,
with zero artifact failures. At handoff the independent shop-panel changes
are committed as `f283e10`; the enum commit contains only its own source,
curated type evidence and documentation changes.
