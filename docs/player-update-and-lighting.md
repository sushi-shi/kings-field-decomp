# Player update, lighting presets, and fire-defense boost

The GAME.EXE band at `0x800187a4..0x8001a29c` contains three small lighting
wrappers, one player-status leaf, and the large `player_update` routine. The
three repeated wrappers form the `game.lighting_presets` reconstruction unit;
their shared call shape, adjacent color-table entries, and common profile
support that ownership model. The wider address band remains an investigation
boundary, not evidence that the status leaf or `player_update` shared the TU.

## Adjacent exact helpers

| Address | Identity | Effect | Confirmed caller |
| --- | --- | --- | --- |
| `0x800187a4` | `lighting_apply_weapon9_environment` | Blends the current color matrix toward `color_matrix_table[4]` by `0x9c4` and installs half the current fog-near distance. | `player_update`, guarded by equipped weapon ID 9 |
| `0x800187f0` | `lighting_apply_timed_player_effect` | Blends toward `color_matrix_table[5]` by `0xc00`. | `player_update`, while `player_state.light_effect_timer` is active |
| `0x80018824` | `lighting_apply_color_preset6` | Blends toward `color_matrix_table[6]` by `0xc00`. | None decoded |
| `0x80018858` | `player_apply_fire_defense_boost` | Sets the fire-defense boost flag and installs its 500-update timer. | menu_magic_panel at `0x8002317c` |

All four C reconstructions are strict 100% object matches under the current
`probe-gcc257-o2-g0` probe. This is a matching result, not proof that GCC 2.5.7
or this optimization profile was the historical toolchain.

The following `player_apply_fire_defense_boost` remains separate: it mutates
the fire-defense timer and flag; its only proven caller is the
menu magic panel. The preceding reverse-death fade is likewise retained in the
player-death unit because it owns a larger death-state transition sequence.

The third lighting wrapper has no decoded incoming reference. Its function
boundary is still supported by its complete 56-byte stack frame, two direct
calls, return sequence, and exact repetition of the preceding wrapper's source
shape. Its name describes only the observed table entry; no gameplay purpose
or caller is invented.

## Player-state fields

The two recovered timers are fields of the checked `0xe0`-byte
`KfPlayerState` layout:

| Offset | Field | Type | Observed lifecycle |
| ---: | --- | --- | --- |
| `0x50` | `player_state.fire_defense_timer` | `s16` | Initialized to `-1`, set to 500 with status bit 4, decremented by `player_update`, and clears the bit at zero. |
| `0x52` | `player_state.light_effect_timer` | `s16` | Initialized to `-1`, set to 1000 by item use, decremented by `player_update`, and sampled by effect-model rendering. |

The complete structure is declared in `include/kf/game_player.h`; every field
extent is checked against `config/retail/structures.tsv` and
`config/retail/structure_fields.tsv` by `kf inventory check`. Reconstructed
sources use the `player_state` aggregate. The delinker must preserve the
interior HI16/LO16 addends that identify its fields.

## Relocation findings

The initial comparisons for the three lighting functions had identical code
bytes but non-exact object scores. Two causes were isolated from the raw ELF
relocation tables:

- references to `color_matrix_table[4..6]` must use the base symbol with
  implicit addends `0x80`, `0xa0`, and `0xc0`, rather than synthetic symbols
  named after each interior address; and
- `lighting_apply_color_preset6` was found outside the original reachability
  traversal, so its relocation proposals retained the `instruction-word`
  discovery channel. Manual `status=reviewed` now admits those MIPS26 and
  HI16/LO16 rows after the same opcode, target, pair, and owner validation used
  for reachable code. Candidate instruction-word rows remain withheld.

Per-function evidence, signatures, and final verdicts are recorded in
`config/evidence/game_semantic_player_update.tsv`.

The [status naming evidence](patterns/game-player-statuses.md) identifies the
boost as +10 fire defense and explains the countdown/recalculation endpoints.
It also records the darkness matrix, 32-step fades, curse and slowed controls.
The original duration and strength tuning remain unknown.
