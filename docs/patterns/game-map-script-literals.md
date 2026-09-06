# GAME floor-script literal audit

## Function Match Plan

Starting at `a1f9e05`, audit the complete `map_scripts.c` module, including
its initialized camera/light data and path-format macros. GAME retail hashes
are freshly verified. Six-view snapshots, source histories, the frozen
object/report baseline and supporting callee dossiers are under
`build/constant-names/map-script-literals/`.

Reuse existing Q12, angle, audio-volume, camera-end and actor-capacity names.
Name the two weapon-transform phases with an s32 stored enum, and the modal
menu's root/pickup/shop modes with an s32 stored enum propagated through its
parameter and all five direct callers. Name the container item terminator,
four-slot walk, shared interaction padding, door-facing tolerance, and
transform hold/swap countdowns. Preserve authored IDs, coordinates and tuning
rates with individual reasons. Replace the first magic record's byte-view
write with its existing `learned` field, supported by the retail store.

Each function's calls, CFG, delay slots, reference addends and signed/unsigned
operations must remain unchanged. These are game-policy routines, outside
the attributed vendored extents; SDK trigonometry, random generation and GTE
operations remain library calls. Compare complete source objects and strict
scores after forced compilation, independently link exact functions to
retail, then run modern checking, repository tests/lint and full `kf build`.

## Meaning and units

The transform's first phase adds the current yaw step, increases it by one
until 240, and holds at that speed. Loading countdown 40 and then counting
down to one gives 40 iterations at peak speed before phase two. The object
changes from resource ID 10 to 11 and emits a radial blast when the countdown
reaches 20. Phase two adds the current step and decrements it to zero, then
starts the fall-and-tip action. One step is 1/4096 turn; peak speed is
21.09375 degrees per loop. The authored magnitude and acceleration choice
have no established historical rationale. Iterations are rendered updates,
not a promised elapsed-time rate.

The camera destination is `(173000,-11500,85000)`, with yaw half a turn and
path speed 100 world units per update. `camera_path_compute_segment` divides
the reduced-distance estimate by speed and constructs Q4 deltas. The terminal
record is recognized solely by X = -1; its other authored -1 lanes are
preserved without inventing independent sentinels. The transient object's
tile is `(85,40)`, centered in the cell, at floor Y minus 1300; the blast is
another 600 units above it. These are distinct positions and offsets.

The character-transfer fade samples Q12 blend 0..4096 in steps of 128:
33 renders, with light-matrix interpolation at four times the blend for
the first nine samples through 1024. The remaining 24 samples raise event 3
by 130 units and rotate yaw by 128 each, totaling 3120 world units upward
and 3072 angle units (270 degrees). The return fade uses 4096..0 in steps
of 256, giving 17 renders. The negative unit Y rows of the light matrix
are Q12 directions; its translation lanes are zero and the matrix interpolator
only touches the nine rotation/light elements. No tuning rationale is claimed.

Interaction probes are 1500 units forward for cell attributes and 1000 for
events/objects, using Q12 sine/cosine. Both overlap queries receive 800 units
of radius padding. Containers accept an inclusive eighth-turn facing error
(45 degrees); doors accept 341/4096 turn (about 29.9707 degrees) from either
side. The nearby door-leaf search adds 6000 units of radius padding. These
values describe separate queries, not one universal interaction distance.

From closed pitch zero, the hinged container subtracts 32 angle units on
each of 32 renders to reach -1024 (minus 90 degrees). Its camera adjustment
adds 16 only while unsigned `(pitch - 191)` is at least 1858: outside the
inclusive signed-angle interval 191..2048. Starting from zero it reaches
192 after twelve increments; the saved camera pitch is restored afterward.
The asymmetric limits remain explicit because their original rationale is
unproven. The four item bytes are interpreted from the existing link storage;
the source's repeated first-parameter precheck and byte walk are preserved.

The floor-5 entry trigger admits tile X 38..40, Z 7 and unsigned yaw
1808..2288 inclusive: half a turn plus/minus 240 (21.09375 degrees).
It displays authored character-17 pages and installs actor-definition-7
attack clips 2/3/3/3/1. These are resource IDs, not action-state enum codes.
The floor-2 ambient sound accepts 4000 of the SDK's 32768 random outputs
(12.20703125%) while event 1 is on stage 2, page below 3. This is an accepted
output fraction, not evidence of independent trials or a specific sound name.

Menu mode 0 opens the player root menu, mode 1 performs item pickup
confirmation, and mode 2 opens the character's shop. The mode-1 callee,
now corrected to `item_pickup_confirm`, adds inventory stock: zero
means acquired, one means cancelled/load failure, and two means stack full.
The modal wrapper's result remains an integer because root-menu returns and
pickup outcomes have different domains. Its existing O32 optional-argument
home-slot access is preserved; typing the selector does not repair that
separate modern varargs portability debt.

The [pickup follow-up](game-item-pickup-outcomes.md) gives the mode-1 result
its own enum and decodes it at all three pickup callers. The shared wrapper
continues to carry the selected mode's numeric result.

## Coverage and verification

The [literal ledger](game-map-script-literal-ledger.md) gives reasons for
all 319 ordinary numeric/character occurrences in `map_scripts.c`, down
from 363 after the pickup and equipment follow-ups. There are 178 groups; identical
expressions with different roles
(animation availability versus successful pickup) keep separate reasons.
Comments, strings and digits within identifiers are excluded. The 42 tokens
in sixteen function claims, four data claims and one rodata claim remain
literal retail ownership evidence.

Eight local enum values are accounted for separately: the phase codes 0/1,
hold/swap countdowns 40/20, overlap padding 800, door tolerance 341, container
capacity four and empty-item byte 255. The shared menu header adds the three
mode codes 0/1/2. Their operation is described above. Four macro-value tokens
are representation constants: `0xffffff00` selects the packed dialogue
stage/page/delay bytes; shifts 8, 16 and 24 place those bytes while excluding
the low stage-cap byte. The path buffer's indices address authored ASCII
digits; decimal division/remainder by ten and character `'0'` stay explicit.

All three edited units were forcibly compiled, followed by the complete
header-dependent comparison build. All 112 objects retain every non-debug
section, symbol and relocation. Only `map_scripts.c` changes debug line data.
All 484 strict scores and the full report equal the frozen `a1f9e05` baseline.
Fifteen exact reviewed functions reproduce all 974 linked retail words and
ordered calls/referents. The three partial functions retain their complete
emitted code; no new exact claims or banks are made.

Modern checking retains 62 passing and 50 failing units, with all 323 error
diagnostics unchanged after source-line/column normalization. Both
`game.map_scripts` and `game.menu_enter_mode` pass. The existing player-update
unit still has its unrelated SDK/input declaration errors. Ruff and
whitespace checks pass. The repository suite ran 656 tests in 90.090 seconds:
655 pass, with only the known untracked save/load-hub mismatch failing.
No new tests or size assertions were added.

Full `kf build` retains its existing data/ownership closure gaps: source
data 7/60, SDK data 4/4, and target relink PSX 1/1, GAME 75/77, OPEN 34/38.
The wider naming goal remains open. The
[equipment follow-up](game-equipment-domains.md) names the shared empty-item
sentinel, including this cutscene's weapon removal. The
[asset-header follow-up](game-asset-animation-layout.md) resolves the signed
animation-clip count. Ten unresolved `unknown_` source lines still require
more evidence.

## Function snapshots and final verdicts

All rows select GAME.EXE and `probe-gcc257-o2-g0`. Each captured dossier
contains the initial signature, width, call, CFG, delay-slot and reference
evidence. The menu parameter retains its s32 storage with its new enum domain.

| GAME VA / bytes | Function | Initial = final strict % | Verdict / retail words |
| --- | --- | ---: | --- |
| `0x80018880 / 6684` | `player_update` | 96.9455 | Partial, unchanged |
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
| `0x80036e38 / 200` | `menu_enter_mode` | 100 | Exact / 50 |
