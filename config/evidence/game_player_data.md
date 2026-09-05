# GAME player paths and floor-entry data

## Function Match Plan

Baseline: `199860e`, GAME.EXE only, pinned `probe-gcc260-o2-g0` probe.
The campaign adds four initialized DATA owners to two existing modules. It
does not change function bodies, invent a TU boundary, merge unrelated data,
reclassify any game function as SDK code, or change any relocation row.

The complete bytes, callers, callees, strings, raw disassembly/CFG, adjacent
functions, history and current scores were inspected before the first edit.
The earlier semantic dossiers are `game_semantic_player_stats.tsv` and
`game_semantic_player_interactions.tsv` (history `d82bc11`). Header-only
centralization did not supply definitions or prove the old string extents.

| GAME function | VA / body | Baseline | Evidence snapshot |
| --- | --- | --- | --- |
| `player_equip_weapon(u8)` | `80016a30 / f4` | 100% | Masks a0 to 8 bits, stores weapon ID, handles ff sentinel, writes decimal digits at path +9/+10; calls CD load, exit on failure, asset registration and combat-stat recalculation. Return owns sp+24 delay slot. |
| `player_warp_to_floor_entry(void)` | `80017cf8 / 144` | 100% | Six CFG blocks; no incoming arguments/result; loads current floor byte, forms table-2 plus floor*2, lbu offsets 0/1, writes x/z cells, scales each by 2000 and adds 1000. Calls shimmer twice, conditional map/audio/pool reload and map synchronization; return owns sp+40 delay slot. |
| `actor_show_info_image(const KfActor *)` | `80017edc / c8` | 100% | a0 is actor pointer; lbu actor+1, floor byte, decimal division, path writes +3/+7/+8. Two render calls then image display; display call delay slot recovers path base from +3; return owns sp+24. |
| `map_event_show_person_image(const KfMapEvent *)` | `80017fa4 / b0` | 100% | a0 is event pointer; lbu event+1, decimal division, writes path +8/+9. Two render calls then image display; display call delay slot recovers path base from +8; return owns sp+24. |

Weapon callers are game initialization, player update, equipment menu and the
floor-five cutscene. Their calls pass a byte ID (menu explicitly masks it),
zero or ff; the callee independently proves u8. The other three callers are
item-use branches: no argument for floor entry, actor/event query result in
a0 in each image-helper call's delay slot. The preceding transform-snapshot
return owns its halfword-store delay slot at 80017ed8; item-use's final return
ends at 800184b0. These boundaries and all ADDRESS claims remain unchanged.

None of the four functions appears in `functions_vendored.tsv`. Their
player/item/map policy, resource names and object-field access are game
evidence, not libc/SDK algorithms. The SDK `exit` callee remains separately
vendored; no library body is reconstructed here.

## Complete owners and source hypothesis

| Owner | GAME VA | Size | Source definition / payload |
| --- | --- | --- | --- |
| `weapon_image_path_template` | `8005581c` | 16 | player_core; `WEPON\WEP00.MIM` plus NUL |
| `floor_entry_cells` | `8005582c` | 10 | player_core; `(15,2), (29,56), (28,18), (7,22), (39,69)` as five x-then-z byte records |
| `enemy_info_image_path_template` | `80055838` | 14 | player_use_item; `ENE0\EI00.TIM` plus NUL |
| `person_image_path_template` | `80055848` | 15 | player_use_item; `PRSN\PER00.TIM` plus NUL |

The previous external character-array declarations were 15/13/14 bytes,
excluding each actual terminator. The three strings were also absent from
the canonical data identity inventory. DATA now owns the complete minimum
array, including NUL, with readable C initializers. No consumer outside the
owning source uses the path names, so their catch-all header declarations
are removed. Existing global linkage is retained as a source-model choice;
the linked executable does not prove historical global versus static linkage.

The floor table uses the established KfFloorEntryCell type, not KfMapCell:
the latter's field order is reversed. Its old extern const qualifier was a
first-pass assumption. The actual owner is in the initialized-data band after
the text contribution, between writable paths, far from this module's
switch-table RODATA at 80012030. Define the table in DATA without const; the
function's read-only pointer remains const and its body remains verbatim.
This models storage, not a claim that gameplay mutates the entry cells.

The exact eight reviewed HI/LO references are retained: weapon sites 80016aac
and 80016ac8 with addends +9/+a; floor 80017d40 with -2; actor 80017f0c,
80017f58, 80017f84 with +3/+7/+8; person 80018000, 80018034 with +8/+9.
There are no data pointer relocations in these four payloads. Their complete
read/write consumers support module ownership independently of adjacency.

The zero bytes at 80055836..38, 80055846..48 and 80055857..58 are not enlarged
array extents. They remain explicit census gaps; inter-object packing and
whole-section tails must be checked separately. A score decrease caused by
exposing these objects is not permission to pad, crop or scatter contributions.

## Verification plan

Recompile both units, run focused matches, compare complete function reports
and raw text/relocation evidence with the saved baseline. Check all four
compiled object symbols, extents and initializers against retail, then inspect
the complete native/Python data comparison and independent target relinks.
Run all repository tests, Ruff, diff checks and a full default build before
commit. Record each function's final verdict and remaining whole-data failures;
an individually correct initializer is not whole-section closure.

## Focused verdicts

`player_equip_weapon`, `player_warp_to_floor_entry`, `actor_show_info_image`
and `map_event_show_person_image` each remain exactly 100%. The unchanged
item-use dispatcher remains 100% too. All 17 function report rows in the two
units are unchanged: 15 exact, with `player_distance_to_point` at 95.327866%
and `player_move_horizontal` at 93.538315%. Their pre-existing control-flow,
frame and instruction-order residues are not addressed or newly attributed.

Only the two base objects and two module targets change. Across all 112
source objects and 1719 target objects, raw text bytes, ordered text relocation
names/types/offsets and RODATA remain unchanged. All 484 function report rows
across PSX/GAME/OPEN are unchanged, preserving all 354 exact game functions.

The four complete compiler-emitted named allocations reproduce all **55
retail bytes**, including the three NULs. Tests check both compiled and
delinked object symbols (name, size, storage and linkage) and complete
initializers; these checks do not mistake named-object equality for whole-TU
data equality.

The strict default gate exposes two new whole-DATA mismatches:

| Unit | Target DATA | Compiler DATA | First divergence |
| --- | ---: | ---: | --- |
| `game.player_core` | 26 B | 32 B | extra section extent after the complete floor table |
| `game.player_use_item` | 31 B | 32 B | extra section extent after the complete person path |

The objects themselves and the inter-path packing agree. No padding is added
to either declared array, no emitted bytes are trimmed, and no toolchain
profile is changed. The core tail cannot be silently assigned to its owner:
the next module's path starts at 80055838, only two bytes after the table.
The complete-source data total therefore drops from 13/60 to 11/60 (GAME
9/39 to 7/39), while the existing two SDK contributions remain separately 2/2.

A raw check of all 32 emitted core DATA bytes at 8005581c differs at offsets
28..31: retail has `ENE0`, belonging to the next path, whereas the compiler
tail has zeros. The item-use object's extra byte happens to equal the zero
at 80055857, but equality of that byte does not prove array or section
ownership. These observations are not a reason to alter compiler output.

## Full verification and remaining closure

The affected units were freshly compiled (`kf try`, then `kf match`);
`kf build compare -j 4` and the full default `kf build -j 4` followed. The
comparison graph completes. The default build still fails explicitly on
data, the same six placement conflicts and incomplete reachable ownership.
No exact function was banked and no comparison threshold was relaxed.

GNU ld independently relinks each updated whole target: core reproduces 5830
initialized bytes (5780 text, 24 RODATA, 26 DATA), and item-use reproduces
1827 (1492 text, 304 RODATA, 31 DATA including two inter-path packing bytes).
Each section has one consistent base; there are no new placement conflicts.
All-image target relinking remains 108/114, with the six previously recorded
failures. These are target witnesses, not linked-source-executable equality.

Reachability now records GAME 69 source-owned and 383 config-owned ranges;
OPEN remains 57 and 279, PSX 2 and zero. Thus four game-reached ranges move
from config-only to source ownership. Unpaired config ranges decrease from
664 to **660** (GAME 382, OPEN 278); SDK 2/2 proofs remain current. Every
other uncertainty category remains unchanged. Complete reachable-byte
coverage remains unproven.

All 535 local tests, Ruff and diff checks pass. Clean flake checks pass with
59 local-retail/oracle skips. The inventory-count control now records 3010
data identities, accounting for the three newly canonical path identities;
the new focused controls separately verify their exact owners and extents.
All-function report-row SHA-256 remains
`2cf8ef7d82c49c41aa1db39b174b4e4bca42af2172a16eccfb12d2bc6e7035fb`.

Reproduce the focused evidence after hash-checked retail initialization:

```sh
kf try --unit game.player_core
kf try --unit game.player_use_item
kf build compare
python -m unittest tests.test_game_player_data -v
kf verify data --image game --detail
kf verify roundtrip --unit game.player_core
kf verify roundtrip --unit game.player_use_item
kf verify reachability --output build/player-data-reachability.json
kf verify roundtrip --output build/player-data-roundtrip.json
kf build
```

The DATA mismatch and reachability commands intentionally remain nonzero.
