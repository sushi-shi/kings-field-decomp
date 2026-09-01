# Player update, lighting presets, and status timer 4

The GAME.EXE band at `0x800187a4..0x8001a29c` contains three small lighting
wrappers, one player-status leaf, and the large `player_update` routine. The
address band is an investigation boundary, not proof that these functions
formed one original translation unit.

## Adjacent exact helpers

| Address | Identity | Effect | Confirmed caller |
| --- | --- | --- | --- |
| `0x800187a4` | `lighting_apply_weapon9_environment` | Blends the current color matrix toward `color_matrix_table[4]` by `0x9c4` and installs half the current fog-near distance. | `player_update`, guarded by equipped weapon ID 9 |
| `0x800187f0` | `lighting_apply_timed_player_effect` | Blends toward `color_matrix_table[5]` by `0xc00`. | `player_update`, while `player_light_effect_timer` is active |
| `0x80018824` | `lighting_apply_color_preset6` | Blends toward `color_matrix_table[6]` by `0xc00`. | None decoded |
| `0x80018858` | `player_status_apply_effect4` | Sets status bit 4 and installs a 500-frame timer. | The effect dispatcher at `0x8002317c` |

All four C reconstructions are strict 100% object matches under the current
`probe-gcc257-o2-g0` probe. This is a matching result, not proof that GCC 2.5.7
or this optimization profile was the historical toolchain.

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
| `0x50` | `player_status_effect4_timer` | `s16` | Initialized to `-1`, set to 500 with status bit 4, decremented by `player_update`, and clears the bit at zero. |
| `0x52` | `player_light_effect_timer` | `s16` | Initialized to `-1`, set to 1000 by item use, decremented by `player_update`, and sampled by effect-model rendering. |

The complete structure and every field extent are checked in
`include/kf/semantic_types.h`, `config/retail/structures.tsv`, and
`config/retail/structure_fields.tsv`. Several reconstructed files still spell
interior members as separate extern symbols because the current relocation
inventory carries member-level identities. That is a WIP linkage model, not a
claim that the original program declared unrelated globals. The intended final
source model is one typed aggregate wherever the executable proves a common
object owner; aggregate-global delinking must preserve the same interior
HI16/LO16 addends before those declarations are migrated safely.

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
