# Typed player and dialogue packed state

## Function Match Plan

Baseline is `6312950`, GAME.EXE only. The six semantic views for each selected
function, complete pre-edit objects and strict report are saved under
`build/cast-model/packed-state/`. The hash-verified retail bodies, callers,
callees, CFG and delay slots, data and relocation annotations, source history,
adjacent claimed functions and existing player/dialogue dossiers constrain
this model. No signature or profile change is planned.

The shared player object is the existing 0xe0-byte saved owner. Its cell at
0xca and previous cell at 0xcc each have unsigned z/x bytes and an unsigned
halfword comparison in `game_main_loop`. Model `KfMapCell` as a coordinates /
word union. Five aligned warp loads at player+0xc8 include pitch in the low
halfword and the current cell in the high halfword; the existing 0xffff0000
mask and authored coordinate keys must stay unchanged. Extend the motion
view through its associated current cell: `KfPlayerMotionFields` contains
the five existing halfwords followed by the cell, while `KfPlayerMotionState`
provides the complete fields and three aligned words. This is a representation
view, not a claim that the original source declared that group. The clear
routine still writes only the five motion halfwords individually, in retail
order. It must not clear the current cell or merge the stores.

The live event's four bytes at 8..11 are the stage cap, stage, page and page
delay. Three script predicates load them as a word and mask out the cap with
0xffffff00. Introduce `KfDialogueFields` and `KfDialogueState`, retaining the
byte producers, signed promoted comparisons, word predicates and constants.
The authored `KfMapEventDefinition` has a distinct layout and retains its
own stage-cap field. Use type-resolved member edits for every consumer.

Game-specific player input, tile occupancy, floor transitions, authored
scripts and TALK filename policy are negative controls for vendored ownership.
SDK/libc calls remain vendor callees. Candidate switch-pointer rows stay
candidates; unresolved indirect transfers are not resolved by this type edit.
The MAP path at 0x800122e4 and floor-five TALK paths at 0x80012a54/68 remain
literal strings in their existing RODATA claims.

Each row retains its complete ordered calls/referents, immediate values,
return and delay-slot instructions. The entry below records blocks / direct
JAL sites / conditional branches / return sites; indirect switch topology is
also retained in the saved disassembly.

| GAME function | VA / bytes | Strict before | CFG | First source hypothesis | Final verdict |
| --- | --- | ---: | --- | --- | --- |
| `game_main_loop` | `800146b8 / 2e4` | 100 | 11/43/5/1 | Unsigned cell halfword comparison; byte grid/update accesses | 100%; raw unchanged |
| `player_death_restart` | `800154b0 / 19c` | 100 | 7/6/3/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `player_clear_motion` | `80016eb8 / 30` | 100 | 1/0/0/1 | Five scalar halfword clears; preserve order and cell | 100%; raw unchanged |
| `player_sync_position_to_map` | `80016ee8 / 158` | 100 | 11/2/10/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `player_move_horizontal` | `800171fc / 828` | 96.569 | 100/7/74/1 | Shared motion/cell members; retain signedness and stores | Partial unchanged; raw unchanged |
| `player_update_view_bob` | `80017a24 / 5c` | 100 | 3/1/1/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `player_update_vertical_motion` | `80017a80 / 278` | 100 | 26/1/15/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `player_warp_to_floor_entry` | `80017cf8 / 144` | 100 | 6/6/3/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `player_update` | `80018880 / 1a1c` | 96.9096 | 325/66/213/1 | Shared motion/cell members; retain signedness and stores | Partial unchanged; raw unchanged |
| `menu_map_viewer` | `80022d7c / 400` | 100 | 15/17/9/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `map_event_refresh_dialogue_stage` | `800337ac / 74` | 100 | 8/0/4/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_event_pool_load` | `800338b8 / 22c` | 100 | 8/1/3/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_ambient_script_floor1` | `80033f64 / 288` | 100 | 35/5/24/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_ambient_script_floor2` | `800341ec / 70` | 100 | 5/2/3/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_ambient_script_floor3` | `8003425c / 88` | 100 | 6/2/4/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_ambient_script_floor5` | `800342ec / f4` | 100 | 6/5/4/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_action_script_floor2` | `800345bc / 54` | 100 | 4/1/2/1 | Word dialogue predicate and 0xffffff00 mask | 100%; raw unchanged |
| `map_action_script_floor3` | `80034610 / 90` | 100 | 8/2/4/1 | Word dialogue predicate and 0xffffff00 mask | 100%; raw unchanged |
| `map_floor5_transition_cutscene` | `800346a8 / 38c` | 100 | 32/12/19/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `map_action_script_floor5` | `80034a34 / 4c` | 100 | 3/1/1/1 | Word dialogue predicate and 0xffffff00 mask | 100%; raw unchanged |
| `map_event_interact` | `80034a80 / 2d4` | 100 | 26/7/17/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_event_pool_update` | `8003596c / 1f0` | 100 | 24/8/11/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `map_world_state_persist` | `80035b5c / 2b8` | 94.8218 | 28/0/17/1 | Shared dialogue/cell members; retain byte widths and ownership | Partial unchanged; raw unchanged |
| `map_restore_floor_state` | `80035e44 / 69c` | 100 | 50/18/27/1 | Shared dialogue/cell members; retain byte widths and ownership | 100%; raw unchanged |
| `player_warp_same_floor` | `800369ac / 144` | 100 | 5/7/3/1 | Shared motion/cell members; retain signedness and stores | 100%; raw unchanged |
| `player_warp_trigger_update` | `80036af0 / 24c` | 100 | 41/6/23/1 | Aligned pitch/cell word and 0xffff0000 mask | 100%; raw unchanged |

Fresh builds must preserve every banked function. Compare all functions in
the affected units against saved linked instructions, ordered calls and
physical references, then retail. Check the parent sizes and nested physical
offsets with the pinned compiler; update the shared curated type inventory
and existing inventory assertions. Run full build, repository tests, Ruff and
diff checks before banking only the directly selected exact functions.

## Result

All 71 functions in the ten affected units preserve their linked instruction
words and ordered calls/references. The 65 exact functions remain exact.
The 26 direct consumers comprise 23 exact functions and the unchanged player
horizontal-movement, player-update and world-persistence partials. The three
other partial neighbors also keep their prior bytes. Overall strict status is
unchanged at 439/471 (GAME 332/362, OPEN 106/108, PSX 1/1).

The typed member audit rewrote 312 occurrences across ten C files. Ten pointer
reinterpretations disappear: two cell halfword reads, five aligned warp word
reads and three dialogue word reads. Complete owner sizes and all newly nested
physical offsets compile unchanged. Five motion clears remain five halfword
stores, and the event definition retains its separate source layout.

All 112 image variants parse without errors. The written-cast census is 756,
including 43 header checks/conversions; 495 C pointer casts remain, down 311
from the initial 806. The added layout-check macro accounts for two header
casts; it does not hide an executable cast. The inventory has 112 types and
822 fields, of which 735 have supported names.

All 713 repository tests pass with nine skips. Existing inventory assertions
were updated for the union and nested field owners. Ruff and diff checks pass.
The final full build follows the source/configuration changes and retains the
existing data/reference/placement gate failures without artifact failures.
Only the 23 directly selected exact functions are eligible for banking.
