# Player combat and status state

This GAME.EXE campaign follows the player-facing half of the damage system from
the component formula through direct and radial damage. It is a semantic WIP:
the executable proves the component lanes and status bits. Retail menu assets
now identify status bit 2 as poison; other unresolved labels remain numbered.

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
| `0x800a07bc` | `player_state.damage_defense_component0` | `u16` | First defense supplied to the component formula. |
| `0x800a07be` | `player_state.damage_defense_component1` | `u16` | Second defense supplied to the component formula. |
| `0x800a07c0` | `player_state.damage_defense_component2` | `u16` | Third defense supplied to the component formula. |
| `0x800a07c2` | `player_state.poison_resistance` | `u16` | Random threshold checked before poison mask 4 is accepted; UI label is `毒`. |
| `0x800a07c4` | `player_state.damage_defense_component3` | `u16` | Fourth defense supplied to the component formula. |
| `0x800a07c6` | `player_state.damage_defense_component4` | `u16` | Fifth defense supplied to the component formula. |
| `0x800a07c8` | `player_state.status_effect0_timer` | `s16` | Initialized to 600 and decremented for bit 0. |
| `0x800a07ca` | `player_state.status_effect1_timer` | `s16` | Uses `-1` as inactive and is extended toward 970/1000 for bit 1. |
| `0x800a07cc` | `player_state.poison_timer` | `s16` | Initialized to 600 after the resistance check; one HP lost every 20 status updates. |
| `0x800a07ce` | `player_state.status_effect3_timer` | `s16` | Initialized to 300 and decremented for bit 3. |

The equipment/stat recalculation function at `0x80015714` clears and rebuilds
all six resistance/defense halfwords from equipment records, which corroborates
their family. The [poison evidence](patterns/game-player-poison.md) joins the
decoded menu glyph to the resistance check, equipment contribution and timer
lifecycle. Other unresolved component and status labels still need their own
direct witnesses; the poison identification does not establish their names.

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
