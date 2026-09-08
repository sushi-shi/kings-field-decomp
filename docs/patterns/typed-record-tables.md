# Typed equipment and magic tables

## Function Match Plan

Baseline is `c209f4e`, GAME.EXE only. The loaders share the confirmed
`common_resources_load` caller and consecutive resource chunks. The retail
bodies, CFG/delay slots, callers/callees, strings, current matches, existing
equipment and effect-owner dossiers, source history and adjacent functions
are reviewed before editing. Complete pre-edit objects and the six semantic
views are saved under `build/cast-model/record-tables/`.

The weapon loader copies 176 words (16 records of 44 bytes), then mirrors the
sixteen SDK Y rotation halfwords in place. The armor loader copies 294 words
(42 records of 28 bytes). The magic loader copies 120 words (24 records of
20 bytes) into the existing effect-state owner. Each loop spans the complete
table, rather than one record. These explicit bounds and consumer strides
support `KfWeaponTable`, `KfArmorTable` and `KfMagicTable` unions with typed
entry arrays and full word arrays. No cross-record pointer walk or pointer
reinterpretation is needed to express the copy.

Keep the curated weapon/armor symbol identities and complete extents; their
BSS ownership remains the existing configuration model. Magic remains inside
`effect_state`. Use entry members in every consumer and complete table input
types in the three loader signatures. The generic resource payload converts
to the typed table at the call boundary. Preserve the weapon post-copy loop,
all unsigned stores and learned-state/save serialization behavior.

These are game-owned equipment, spells, player progression and resource
policies; the established vendored inventories and SDK boundaries exclude them
from library attribution. SDK audio/geometry and libc calls remain vendor
callees. No candidate relocation is promoted. The source-level table names
are a layout model, not proof of original typedef spellings.

| GAME function | VA / bytes | Strict before | Blocks/JAL/returns | Evidence / initial source hypothesis | Final verdict |
| --- | --- | ---: | --- | --- | --- |
| `weapon_records_load_and_mirror_angles` | `800150a8 / 54` | 100 | 5/0/1 | Complete table word copy; weapon mirror loop retained | 100%; raw unchanged |
| `armor_records_load` | `800150fc / 2c` | 100 | 3/0/1 | Complete table word copy and typed input | 100%; raw unchanged |
| `player_recalculate_combat_stats` | `80015714 / 814` | 100 | 44/3/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `player_select_magic` | `800167e4 / 64` | 100 | 4/0/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `player_set_equipment_slot` | `80016848 / 1e8` | 100 | 24/1/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `player_equip_weapon` | `80016a30 / f4` | 100 | 7/4/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `player_update` | `80018880 / 1a1c` | 96.9096 | 325/66/1 | Typed table entries; existing record members and widths retained | Partial unchanged; raw unchanged |
| `common_resources_load` | `8001b180 / 210` | 100 | 7/10/1 | Serialized chunk passed to complete-table input | 100%; raw unchanged |
| `menu_magic_panel` | `8002317c / 530` | 100 | 66/20/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `menu_spell_select` | `80023e9c / 470` | 100 | 57/20/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `save_file_write_slot` | `8002b73c / 4f4` | 100 | 42/24/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `save_file_read_slot` | `8002beb0 / 3cc` | 100 | 36/13/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `map_ambient_script_floor3` | `8003425c / 88` | 100 | 6/2/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `map_action_script_floor3` | `80034610 / 90` | 100 | 8/2/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `map_event_interact` | `80034a80 / 2d4` | 100 | 26/7/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `effect_pool_construct` | `80036f44 / 82c` | 100 | 52/7/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `effect_pool_set_current` | `8003781c / 34` | 100 | 1/0/1 | Typed table entries; existing record members and widths retained | 100%; raw unchanged |
| `effect_update_dispatch` | `80038a38 / 180c` | 96.9396 | 257/69/1 | Typed table entries; existing record members and widths retained | Partial unchanged; raw unchanged |
| `magic_load_records` | `8003a274 / 2c` | 100 | 3/0/1 | Complete table word copy and typed input | 100%; raw unchanged |

Rebuild every affected unit, check complete layouts with the pinned compiler,
and compare every linked instruction, ordered call and physical referent
against baseline and retail. Preserve every banked function. Update current
shared signatures, type/data inventories and existing assertions coherently.
Full build, repository tests, Ruff and diff checks precede banking only exact
selected consumers. Partial functions keep their existing bytes and verdicts.

## Result

All 92 functions in the twelve selected units retain their linked instruction
words and ordered calls/references. The 83 exact functions remain exact.
The nineteen selected consumers comprise seventeen exact functions and the
unchanged player-update and effect-dispatch partials. Seven other partial
neighbors also keep their bytes. Overall strict status remains 439/471.

The three table types preserve the full 0x2c0, 0x498 and 0x1e0-byte extents.
They expose entry arrays for record consumers and full aligned word arrays
for the existing copy loops. The weapon loop still mirrors sixteen halfwords
at record+0x26 after copying all 176 words. The magic loader's supported typed
input describes the complete 24-record payload supplied by its sole caller.
No global identity, field encoding, underlying record size or physical owner
is replaced. The current type/data inventories and loader signatures agree.

Five loader pointer casts and two now-unnecessary casts of generic resource
payloads are removed. All 112 retail-mode C image variants parse without errors or
incompatible-pointer diagnostics. The census is 736 written casts, including
43 header checks/conversions; C pointer casts total 475, down 331 from 806.
The inventory has 115 types and 828 fields, with 741 supported field names.

All 713 repository tests pass with nine skips; the existing inventory tests
also pass independently. Ruff and diff checks pass. The final full build
follows the source/configuration changes and retains the existing
data/reference/placement gate failures without artifact failures or exact
regressions. Only the seventeen directly selected exact consumers are banked.
