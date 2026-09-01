# Player vitals and death transition

This campaign follows the GAME.EXE player-vitals helpers at
`0x80015164..0x800156bc` into the three visual-transition routines they call at
`0x800184b0..0x800187a4`. The latter are a dependency expansion, not evidence
that the two address bands belonged to one original translation unit.

## Vitals and restart policy

`player_adjust_hp` adds a signed delta to `player_vitals.current_hp`, clamps it
to `maximum_hp`, and starts the death sequence after clamping a non-positive
result to zero. `player_adjust_mp` is the corresponding zero-to-maximum clamp
for MP. Its observed caller uses it for periodic positive regeneration.

`game_state_initialize` is intentionally broad: it initializes map progress,
all four player-vitals fields, settings and numerous still-unresolved world,
inventory, and equipment values. Both the normal gameplay startup path and the
full-reset death path call it, so neither `new_game` nor `player_initialize`
would describe all observed effects.

`player_death_restart` chooses between a special continuation path and that
full reset. The continuation path decrements an unresolved byte, restores
current HP and MP from their maxima, and places the camera at a distinct fixed
position. Both paths converge on subsystem cleanup, camera state preparation,
and player update state `0xfe`. The byte controlling the special branch remains
address-named: decrement-and-restart behavior alone does not prove a lore-level
meaning such as a life or continue counter.

## Two-phase visual state machine

The main player dispatcher gives the otherwise opaque state values direct
meaning:

| State | Handler | Observed effect |
| --- | --- | --- |
| `0xff` | `player_death_update` | Drops and pitches the camera, advances the fade, renders, then restarts at Q12 `0x1000`. |
| `0xfe` | `player_death_update_reverse_fade` | Reverses the matrix/fog/intensity transition and returns the state to zero. |

Values `1..7` participate in ordinary player action/reaction handling, so the
byte at `0x800a0822` is not modeled as a Boolean death flag. It is also an
interior byte of the unresolved four-byte datum at `0x800a0820`; this pass does
not create an overlapping scalar identity merely to attach a name.

`player_death_begin` writes state `0xff`, resets the two death accumulators,
plays a packed sound reference, snapshots the current color matrix and fog-near
distance, and is reached from the HP-zero path plus fatal map hazards.
`player_death_apply_visual_fade` is shared by both transition handlers and
applies one Q12 blend to the color matrix, light matrix, fog-near distance, and
scene intensity.

## Globals and statics

The executable proves identity and extent but generally cannot prove external
versus file-local linkage. These four objects therefore retain `scope=unknown`:

| Address | Identity | Type/extent | Evidence |
| --- | --- | --- | --- |
| `0x80057e78` | `player_death_saved_fog_near` | `s32`, `0x4` | Written from `fog_near_distance` on death entry and read by both fade directions. |
| `0x80058060` | `player_death_saved_color_matrix` | `KfMatrix`, `0x20` | Filled by `ReadColorMatrix` and supplied twice to the active fade; program entry also repeatedly clears its first word. |
| `0x800a0858` | `player_death_camera_pitch_step` | `u16`, `0x2` | Cleared only by death entry/restart and accumulated only by the state-`0xff` camera fall. |
| `0x800a085a` | `player_death_visual_blend` | `s16`, `0x2` | Advanced in 100-unit steps and sign-extended in the reverse phase before comparison with Q12 `0x1000`. |

The matrix is a newly admitted BSS extent. Its `ReadColorMatrix` destination
and two fade inputs use carry-adjusted `lui`/`addiu` sequences and are now
reviewed relocation rows. Program entry constructs the same address with
`lui 0x8005; ori 0x8060`. That fourth xref proves the identity but is not an
interchangeable ELF HI16/LO16 pair, so it remains address evidence rather than
an admitted relocation. The saved fog and accumulator relocations are likewise
promoted only where the low opcode and decoded target were checked directly.

Several nearby fields remain unresolved. `0x800a0854` is also used by general
camera bobbing, while `0x80095064` and `0x80095720` are shared render state with
broader callers. Address proximity to the death accumulators is not ownership
evidence.

Per-function evidence and final signature verdicts are recorded in
`config/evidence/game_semantic_player_death.tsv`.
