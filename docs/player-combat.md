# Player combat and status state

This GAME.EXE campaign follows the player-facing half of the damage system from
the component formula through direct and radial damage. It is a semantic WIP:
the executable proves the component lanes and status bits. Retail menu assets
identify poison, curse, darkness and the player attack/defense components.
The remaining status behaviors are slowed movement and a fire-defense boost;
their [timer evidence](patterns/game-player-statuses.md) preserves each distinct
countdown and tuning value.

## Reviewed functions

| VA | Identity | Evidence-backed role |
| --- | --- | --- |
| `0x8001627c` | `player_calculate_damage_component` | Computes one defended incoming component with a linear remainder and quadratic term. |
| `0x80016324` | `player_apply_damage` | Applies status bits, combines five components, applies Q12 and tenths scaling, clamps current HP, and selects the hit state. |
| `0x800166b4` | `player_apply_radial_damage` | Bounds the player against a cylinder, applies distance falloff, and forwards a zero-status damage payload. |
| `0x80017108` | `player_distance_to_point` | Returns horizontal distance within horizontal and optional vertical bounds, otherwise `-1`. |

The component helper is similar in extent and arithmetic shape to
`combat_calculate_damage_component`, but it is not the same function. The
actor helper takes `(base_power, attack, defense)` and tests `attack` first.
The player helper takes `(base_power, defense, attack)` and tests `attack` in
its third argument. Its observed arithmetic is:

```text
threshold = defense + base_power / 5
remainder = max(attack - threshold, 0)
damage = remainder + attack * attack / (2 * max(threshold, 1))
```

`player_apply_damage` multiplies all five incoming components and the shared
base-power stat by ten before the component calls. It rounds their sum by
adding five and dividing by ten, applies `scale_q12` with a 12-bit shift, then
applies `multiplier_tenths` by dividing by ten. A nonzero result is subtracted
from `player_vitals.current_hp`, clamped at zero, and changes the player action
state to one unless the death state `0xff` is already active.

## Globals and statics

These addresses are now members of the shared `KfPlayerState` object at
`0x800a0780`. Its 0xe0-byte save copy and registered-base member accesses
establish aggregate ownership; the former interior identities are no longer
separate globals. The original object's external/static linkage remains
unresolved in the curated inventory.

| VA | Identity | Type | Evidence |
| --- | --- | --- | --- |
| `0x800a07aa` | `player_state.status_effect_flags` | `u16` | Damage sets bits 0..3; the player update clears each bit when its timer expires. |
| `0x800a07bc` | `player_state.cutting_defense` | `u16` | Cutting defense, labelled 切る. |
| `0x800a07be` | `player_state.striking_defense` | `u16` | Striking defense, labelled 打つ. |
| `0x800a07c0` | `player_state.piercing_defense` | `u16` | Piercing defense, labelled 刺す. |
| `0x800a07c2` | `player_state.poison_resistance` | `u16` | Random threshold checked before poison mask 4 is accepted; UI label is `毒`. |
| `0x800a07c4` | `player_state.magic_defense` | `u16` | Magic defense, labelled 魔の魔法; distinct from the holy-attack label. |
| `0x800a07c6` | `player_state.fire_defense` | `u16` | Fire defense, labelled 炎の魔法. |
| `0x800a07c8` | `player_state.curse_timer` | `s16` | Curse countdown; stat recalculation applies a 20-point physical-power penalty. |
| `0x800a07ca` | `player_state.darkness_timer` | `s16` | Darkness countdown; 32-step fades at both ends and distinct 970/1000 refresh/initial values. |
| `0x800a07cc` | `player_state.poison_timer` | `s16` | Initialized to 600 after the resistance check; one HP lost every 20 status updates. |
| `0x800a07ce` | `player_state.slowed_timer` | `s16` | Slowed movement countdown; movement limit 36 instead of 180, turn limit 5 instead of 28. |

The equipment/stat recalculation function at `0x80015714` clears and rebuilds
all six resistance/defense halfwords from equipment records, which corroborates
their family. The [poison evidence](patterns/game-player-poison.md) joins the
decoded menu glyph to the resistance check, equipment contribution and timer
lifecycle. The [damage-label evidence](patterns/game-player-damage-labels.md) joins
cutting, striking, piercing and fire labels to their attack/defense fields.
The fourth attack is labelled holy magic, while its defense counterpart has
a different retail label and is named magic_defense. Generic damage payload
arguments retain their unresolved context; player status names follow the
separate timer and behavior evidence.

## Radial calls and unresolved ownership

Three decoded sites in `0x80038a38` call `actor_pool_apply_radial_damage` and
`player_apply_radial_damage` back-to-back with the same eleven arguments. The
player wrapper intentionally does not consume `base_power`, although preserving
that argument is required by every caller's shared payload. Its final value is
used as a tenths multiplier by `player_apply_damage`; this is distinct from the
actor core's high-nibble `hit_flags` interpretation.

`player_distance_to_point` uses `camera_position.x/z` as the player reference.
A `point_y` value of `0xffff` disables its vertical test. Otherwise it compares
the adjacent vertical player reference against a height-centered interval,
then returns the horizontal distance computed through Psy-Q `SquareRoot0`.
These call relationships do not establish an original translation unit, so all
four functions remain separate reconstruction candidates.
