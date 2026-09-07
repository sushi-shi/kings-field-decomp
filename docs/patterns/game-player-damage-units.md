# Player damage units and death-module literals

## Function Match Plan

Starting at `8bb4c8b`, name the player damage helper's intermediate tenths of
an HP, half-unit rounding, the independent dimensionless tenths multiplier,
and the poison random bucket conversion. Reuse Q12 unity/shift constants in
radial damage and the two map-hazard calls. Propagate multiplier unity through
the direct actor-attack, actor-bounce and map-hazard callers. Reuse full audio
volume for the death cue. Name the restart-local 255 value by its purpose of
forcing the floor-load path, without treating it as an actual floor identity.

Before editing, `kf init` validates the retail inputs, and all six GAME semantic
views, neighbors, source history, 112 objects and 484 scores are captured under
`build/constant-names/player-damage-units/`. The prior player-status, poison,
damage-label and actor-constant reviews provide shared-field and call-site
evidence. These bodies implement game combat, state and resource policy;
`rand`, GTE state reads and audio services remain external library boundaries.
The pinned SDK's `RAND.H` defines `RAND_MAX` as 32767; its BIOS implementation
and the probability distribution are not inferred from that contract.

| GAME address | Function | Bytes | Strict match (before = after) | Evidence and proposed change |
| --- | --- | --- | --- | --- |
| `80015164` | `player_death_begin` | 104 | 100% | Clears death counters and saves render state; full-volume argument 127 occupies the sound-call delay slot. |
| `800154b0` | `player_death_restart` | 412 | 100% | Fruit revival preserves world state; fresh startup puts 255 in the saved local floor at 8001556c so the later comparison with floor one reloads resources. |
| `8001564c` | `player_adjust_hp` | 112 | 100% | Ledger control: signed delta, zero/death branch, maximum-HP cap and final halfword store. |
| `800156bc` | `player_adjust_mp` | 88 | 100% | Ledger control: signed delta, zero floor, maximum-MP cap and final halfword store. |
| `8001627c` | `player_calculate_damage_component` | 168 | 100% | Ledger control: adds one-fifth base power to defense, clamps excess, protects a zero denominator, adds the squared-attack term; no calls. |
| `80016324` | `player_apply_damage` | 912 | 100% | Five calls consume tenfold powers/defenses/components; add five, divide ten, multiply Q12, arithmetic-shift twelve, then multiply/divide the independent tenths argument. |
| `800166b4` | `player_apply_radial_damage` | 304 | 100% | Unsigned radius division, halfword attenuation boundaries, logical product shifts; unity falloff bypasses division; status argument zero is the final call delay slot. |
| `800167e4` | `player_select_magic` | 100 | 100% | Ledger control: clear charge, store/reload selected byte, null record for the named no-magic sentinel. |
| `80018880` | `player_update` | 6684 | 96.945540% | Map attributes 0x3a/0x3f call damage with Q12 unity and tenths unity; preserve distinct normal-state guard, damage channels and poison flag. |
| `8002d6a0` | `actor_try_attack_player` | 344 | 100% | Attack records supply three channels and optional status, Q12 unity and final argument ten. |
| `8002fa88` | `actor_update_current_action` | 3472 | 100% | Player-collision bounce supplies striking input 15 with Q12 unity and final argument ten; preserve collision/vertical-state flow. |

Keep all widths, intermediate truncation, comparison operators, status order,
raw calls and relocation targets. The radial helper's unused base-power
argument and its existing zero-radius behavior stay intact. Effect payload IDs
remain their existing mixed-use fields rather than being renamed en masse from
their downstream multiplier role. Keep the actor helper's independent units
and different base-power placement separate from this player-specific rule.

Force the four affected units and compare from the first real divergence.
Require all focused instructions/calls/referents and scores to remain unchanged;
verify exact bodies against retail and isolate any concurrent actor edit.
Run inventory, modern checking, Ruff, repository tests, whitespace and full
`kf build`. Complete a ledger for the eight remaining player-module functions
and sound initializers, refresh earlier ledgers' locations, and retain explicit
scope for the large callers. No tests, size assertions or new banks are proposed.

## Unit and boundary evidence

`PLAYER_DAMAGE_SUBUNITS_PER_HP` gives the tenfold working precision of the five
player damage components. Each call receives physical power, its corresponding
defense, and the incoming attack channel multiplied by ten. The helper adds
one-fifth of player power to defense before its excess/squared-attack formula.
The actor-target helper instead adds its base-power contribution to attack;
its existing private scale remains independent.

After summing the five results, retail adds five at `800165b0`, divides by ten
at `800165b8`, multiplies by the Q12 scale at `800165e8`, and arithmetic-shifts
twelve at `800165f8`. The new spelling derives five as half the named unit.
It then multiplies by the separate `multiplier_tenths` and divides by ten again
at `8001660c`. `KF_PLAYER_DAMAGE_MULTIPLIER_ONE` names that dimensionless unity,
shared by the ordinary actor attack, player bounce and two map-hazard calls.
Combining either division with a later stage would change truncation behavior.
All signed products, divisions, halfword inputs and final HP stores remain.

The hazard calls still supply channels 5/3/5 under attribute 0x3a's normal-state
guard, and only the poison flag under attribute 0x3f. Both now spell their
existing 4096 and ten arguments with Q12 and multiplier unity. The bounce's
striking input fifteen remains an authored attack component, not a promise
of fifteen HP damage. Other effect paths pass their existing mixed-use ID
through the multiplier argument. The radial effect calls' scale 5000 is a
Q12 value of 5000/4096; the actor API separately defines 5000 as unity.

The poison roll computes `(rand() * 100) >> 15`, giving buckets zero through
99 under the SDK's 0..32767 range. `PLAYER_POISON_ROLL_BUCKETS` and
`PLAYER_POISON_ROLL_SHIFT` express that conversion. The strict comparison
means resistance zero rejects bucket zero, and resistance 99 or higher rejects
every bucket. The RNG distribution remains unproved. Status changes precede
the numeric loss check, so a zero-loss call can still apply a status.

Radial damage rejects the signed distance result -1 first. Unity falloff
bypasses division; otherwise the unsigned radius forces unsigned division
of the shifted distance. The result narrows to a halfword, the attenuation
products retain unsigned-word truncation and logical Q12 shifts, and the
result narrows again before forwarding. Q12 names replace only the existing
4096 and twelve values. No zero-radius guard, falloff clamp or use of the
unused base-power argument is invented. The forwarded status remains zero
in the final call's delay slot.

The restart override is local control: `PLAYER_RESTART_FORCE_FLOOR_LOAD` retains
255 in `s0` at `8001556c`, after starting a fresh game, to force the following
floor-one comparison into the resource-load path. Fruit revival instead
preserves the previous floor value and persists the world before restoring
HP/MP. Neither branch assigns 255 to the player's actual floor field. Restart
placements, camera offsets, zero origins, null pointers and the sound bank's
program/tone/note selectors retain the individual meanings in the ledger.

## Verification and scope

Every function in the snapshot table has final verdict **unchanged**. Independent
resolution preserves 3162 candidate words, 157 ordered calls and 300 address
references. The ten exact bodies reproduce all 1504 retail words and agree with
independently resolved target objects. The larger `player_update` caller retains
96.945540% and its existing non-exact instructions; naming its two call sites
does not close that function.

The isolated `8bb4c8b` source/header build and a second build containing only
these five edited files agree in every section of all 112 objects except the
player module's debug line table. The live objects' allocated sections, runtime
symbols and ordered relocations agree with the isolated named result outside
the independently edited actor unit. Within that unit the reviewed attack
function's raw words/calls/referents remain unchanged; frozen actor source also
compiles identically with either header. The only change among the 484 captured
scores is the concurrent `actor_distance_to_point` reconstruction, 99.333336%
to 100%, which is separate from this campaign.

The four affected units were forcibly compiled, and the full build rebuilt
the final player source after line wrapping. Inventory validation, Ruff, all
678 existing tests (79.482 seconds) and whitespace checks pass. Modern checking
retains the same 320 error diagnostics and 64/112 passing source/image variants.
Full `kf build` retains the known data/placement failures: data PSX 0/1, GAME
9/42, OPEN 2/19; target relinks 1/1, 75/77 and 34/38; six conflicting section
bases and zero artifact failures. No new tests, size assertions or banks are
part of this naming change.

The new [remainder ledger](game-player-death-literal-ledger.md) accounts for
56 inline numeric occurrences in eight functions and all sound initializers.
Combined with startup's 98 occurrences in the
[database/startup ledger](game-item-database-literal-ledger.md) and the 45 in
the [progression ledger](game-player-progression-literal-ledger.md), it covers
all **199** remaining occurrences in `src/game/player_death.c`, down from 227.
A lexer census compares every function/expression/token multiplicity and source
location against those three ledgers; no function or initializer is skipped.
Retail claims and named constant definitions are excluded explicitly.
The four multiplier-unity arguments and two Q12-unity arguments in the large
callers remove six more inline occurrences, for a net reduction of 34.
The larger callers retain their separate audit scopes; this result completes
the player-module literal inventory, not the repository-wide naming objective.
