# GAME saved floor-script states

## Function Match Plan

Starting at `89a9e9f`, recover the persistent script prefix inside the existing
`KfMapRuntimeState` owner. GAME retail hashes were verified with `kf init`.
Fresh image-qualified address, CFG/disassembly, incoming/outgoing references,
strings and match snapshots are under `build/constant-names/map-script-state/`.
Source history and the save/load inverse constrain the shared layout.

The hypothesis is five 1700-byte floor slots, each with a ten-byte script
prefix and a variable-length record stream. Give the supported prefix fields
their own storage types; retain the serialized word view used by save I/O.
The floor determines which prefix variant applies. Uninterpreted bytes remain
serialized storage, without claims about their gameplay meaning. This is a
view of one complete owner, not new globals at interior addresses.

`lbu`/`sb` prove byte storage for flags and entry/exit stages. The floor-3
value is incremented and saturated at four, so it is a count, not an enum.
Two floor-1 scripts share states zero (await entry), one (await exit), and two
(complete); each writes complete before attempting its actor/object change.
Binary flags distinguish unset and set, retaining each consumer's original
equality or nonzero predicate. Local pointers must carry the field type.

The affected functions implement game-specific map, actor and save policy,
not SDK algorithms. Their retail calls to Psy-Q rendering/audio routines
remain library boundaries. No vendored bodies are reconstructed. Preserve
all calls, branches, delay slots, signedness and ordered address addends.
Force-recompile affected units, compare every object against the frozen
baseline, compare strict scores and linked retail words for exact functions,
then run Clang, repository tests/lint and the full build before commit.

## Persistent byte meanings

Offsets are relative to the saved world block at GAME `8009ddb4`.

| Offset | Field | Direct evidence |
| --- | --- | --- |
| 0 | floor-1 object removal stage | `33f64`: enter X `[2,5)`, Z `[27,30)`, then leave X `[2,28)`, Z `[11,41)`; remove the object near `(9000,57000)` with radius padding 3000. |
| 1 | floor-1 actor activation stage | `33f64`: enter X `[7,12)`, Z `[31,41)`, then leave X `[2,14)`, Z `[25,46)`; initialize the actor at tile `(7,40)` and replace a nearby object's ID with 92. `35e44` disables that actor until stage 2. |
| 2 | floor-1 passage opened | `343e0`: inventory item 56 opens copy region 1 once. `35e44` reapplies that region on reload. |
| 3 | floor-1 revival enabled | `31cc8` sets this after a reveal-map-piece action. `154b0` consumes item 47 to revive with restored vitals instead of resetting game state. `33f64` enables spatial sound 5 at the fixed floor-1 position. |
| 3400 | floor-3 revealed-piece count | `31cc8` increments after a reveal-map-piece action, clamps at four, and applies copy regions 2/3. `35e44` reapplies both when the count is four. |
| 6800 | floor-5 character arrived | `34438` disables floor-2 event 3 after its rising fade and sets this byte; `35e44` activates floor-5 event 1. Both authored event definitions use character 11. |
| 6801 | floor-5 weapon transformed | `34a34` sets this after `346a8` removes weapon/item 10 and changes the displayed object from 10 to 11. Reload clears link 51 on floors 1–3 and link 52 on floor 5. |
| 6802 | floor-5 boss encounter started | `342ec` sets this before the two character-17 images, restores five attack animation slots of actor definition 7 and applies copy region 4. `2d120` ignores damage to that actor until set; `31cc8` gates emitter object 115; `35e44` restores the matching setup. |
| 6803 | floor-5 boss defeat complete | `2f8cc` sets this at the completed boss death animation. `35e44` reapplies link 13 and related actor deaths; `36af0` allows the ending exits. |

The state codes encode transitions, not durations. Four is the required
number of completed reveals; it is unrelated to floor ID 4 or copy region 4.
Floor numbers select saved slots using zero-based indices. The ten-byte
prefix, 1700-byte stride and five slots come from the serializer and the
8500-byte save copy, not alignment padding. No new size assertions are added.

The two additional format names are `KF_MAP_SAVED_RECORD_BYTES = 1690`
(1700 minus the ten-byte prefix) and `KF_MAP_SAVED_WORLD_WORDS = 2125`
(five 1700-byte slots divided into four-byte words). Their literal enum
definitions are accepted by the repository's deliberately restricted layout
parser. The saved-world union is shared by the runtime owner and
`KfSavePayload`; save I/O copies its word representation, while gameplay
uses actual typed members. The variable record parsers still use a byte
cursor and retain their independently reviewed arithmetic and stream format.

## Verification

Forced compilation of the nine affected/owner units, followed by the complete
header-dependent comparison build, retains every section of all 112 source
objects byte for byte, including symbols, relocations and debug line data.
All 484 strict function scores and the complete objdiff report are identical
to the frozen `89a9e9f` baseline. Twenty exact functions in the 26-function
review reproduce all 2030 linked retail instruction words, including ordered
calls and data referents. The six partial functions retain their entire
emitted objects and scores. No functions were newly banked.

Modern checking retains 62 passing units and 50 failing units; all 323
pre-existing error diagnostics are identical after removing source line and
column numbers. No casts, suppressed diagnostics or size assertions were
added. Ruff and whitespace checks pass. The repository suite ran 656 tests
in 81.084 seconds: 655 pass, with the existing untracked save/load-hub word
mismatch as the sole failure. Only the existing inventory totals test changed
to account for six additional layouts and seventeen field/view entries.

The full build retains its prior closure gaps: source data 7/60, SDK data
4/4, target relink PSX 1/1, GAME 75/77 and OPEN 34/38. The new fields remain
inside the existing owner; no relocation target, data claim or evidence tier
was changed.

The preceding map-event ledger now covers 145 retained literal occurrences,
down from 157, and the map-object review covers 259, down from 267. Removed
occurrences became the named saved fields, flag/stage codes and reveal count.
The remaining map-script literals, including authored resource IDs, geometry
and timing, still need their own complete audit. The broader naming goal and
ten unresolved `unknown_` source lines remain open.

## Function snapshots and final verdicts

All rows select GAME.EXE and `probe-gcc257-o2-g0`. Signatures, load/store
widths and CFGs are unchanged; full six-view snapshots are in the evidence
directory above. Exact rows include independently linked retail comparison.
Partial rows retain complete emitted objects, without a new closure claim.

| GAME VA / bytes | Function | Initial = final strict % | Verdict / retail words |
| --- | --- | ---: | --- |
| `0x800151cc / 740` | `game_state_initialize` | 100 | Exact / 185 |
| `0x800154b0 / 412` | `player_death_restart` | 100 | Exact / 103 |
| `0x8002b73c / 1268` | `save_file_write_slot` | 100 | Exact / 317 |
| `0x8002beb0 / 972` | `save_file_read_slot` | 100 | Exact / 243 |
| `0x8002d120 / 904` | `actor_apply_damage` | 89.9867 | Partial, unchanged |
| `0x8002f8cc / 444` | `actor_update_boss_death_sequence` | 100 | Exact / 111 |
| `0x80031cc8 / 3096` | `map_object_pool_update` | 98.9664 | Partial, unchanged |
| `0x80033ee4 / 128` | `actor_pool_find_at_tile` | 100 | Exact / 32 |
| `0x80033f64 / 648` | `map_ambient_script_floor1` | 100 | Exact / 162 |
| `0x800341ec / 112` | `map_ambient_script_floor2` | 100 | Exact / 28 |
| `0x8003425c / 136` | `map_ambient_script_floor3` | 100 | Exact / 34 |
| `0x800342e4 / 8` | `map_ambient_script_floor4` | 100 | Exact / 2 |
| `0x800342ec / 244` | `map_ambient_script_floor5` | 100 | Exact / 61 |
| `0x800343e0 / 88` | `map_action_script_floor1` | 100 | Exact / 22 |
| `0x80034438 / 388` | `map_reveal_fade` | 100 | Exact / 97 |
| `0x800345bc / 84` | `map_action_script_floor2` | 100 | Exact / 21 |
| `0x80034610 / 144` | `map_action_script_floor3` | 100 | Exact / 36 |
| `0x800346a0 / 8` | `map_action_script_floor4` | 100 | Exact / 2 |
| `0x800346a8 / 908` | `map_floor5_transition_cutscene` | 100 | Exact / 227 |
| `0x80034a34 / 76` | `map_action_script_floor5` | 100 | Exact / 19 |
| `0x80034a80 / 724` | `map_event_interact` | 100 | Exact / 181 |
| `0x80034d54 / 144` | `map_show_screen_image` | 88.8889 | Partial, unchanged |
| `0x80034de4 / 2308` | `map_interaction_dispatch` | 83.0468 | Partial, unchanged |
| `0x80035b5c / 696` | `map_world_state_persist` | 94.8218 | Partial, unchanged |
| `0x80035e44 / 1692` | `map_restore_floor_state` | 99.9645 | Partial, unchanged |
| `0x80036af0 / 588` | `player_warp_trigger_update` | 100 | Exact / 147 |
