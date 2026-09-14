# Actor core

The reviewed GAME actor core begins at `0x8002ca78`. This is a semantic WIP,
not a claim that `actor`, `action`, or any field name reproduces FromSoftware's
original spelling. The names describe effects visible in retail MIPS and may
be refined as more of the surrounding update/render code is inventoried.

## Storage layout

The array boundaries are supported by independent size and stride evidence:

| Address | Extent | Identity | Evidence |
| --- | ---: | --- | --- |
| `0x80056080` | `0xfa` | `actor_action_profiles[25]` | 25 ten-byte records supply five signed distance/weight values to action selection |
| `0x8005617c` | `0x0c` | `boss_death_phase_sounds[4]` | packed three-byte `SoundRef` records; the death handler directly uses entries one through three |
| `0x80057b80` | `0x03` | `boss_death_loop_sound` | packed bank/program/note bytes passed to `sound_ref_play` during the death sequence |
| `0x8006bd98` | `0x720` | `actor_definitions[12]` | `func_80030a6c` copies `0x1c8` words; all consumers use a `0x98` stride |
| `0x8006c4b8` | `0x2400` | `actor_pool[128]` | allocation/clear/search loops use 128 records at a `0x48` stride |
| `0x8006e8b8` | `0x10` | `actor_player_position` | copied from the per-frame player snapshot and used by actor range/angle code |
| `0x8006e8c8` | `0x08` | `actor_player_rotation` | adjacent optional rotation copy in the same per-frame setter |
| `0x8006e8d0` | `0x04` | `current_actor_definition` | `actor_bind_current` derives it with `definition_id * 0x98` |
| `0x8006e8d4` | `0x04` | `current_actor` | bound actor pointer used throughout the action/update routines |
| `0x8006e8d8` | `0x02` | `current_actor_index` | `(actor - actor_pool) / 0x48` |
| `0x8006e8da` | `0x02` | `current_actor_definition_id` | zero-extended byte at actor offset `0x01` |
| `0x8006e8dc` | `0x04` | `player_target_actor` | written from player-facing cone searches and compared with `current_actor` during ground-action selection |
| `0x8009f847` | `0x01` | `boss_defeat_complete` | set by the floor-five/definition-seven death sequence and read by map-transition gating |

Startup clears `0x2b48` bytes beginning at `actor_definitions`, exactly through
the byte before `0x8006e8e0`. The definition array, actor pool, and context
globals therefore form one contiguous actor subsystem. The old Ghidra-derived
identities for addresses inside the two arrays were fields/elements, not
separate C globals; curation replaces them with the owning array extents.

`include/kf/game/actor.h` owns `KfActorDefinition`, `KfActorActionProfile`,
`KfActorPlacement`, `KfActor`, the complete `KfActorState` aggregate, and the
actor-owned globals and operations. The packed `SoundRef` embedded by actor
definitions remains owned by `include/kf/lib/audio.h`. The structure inventory
validates all of these layouts; unknown bytes remain explicitly opaque. The
currently supported actor fields include the slot/free
marker, definition ID, tile/local/world positions, lifecycle,
action and progress, animation phase and step, health, rotation, and movement
components. Definition fields used by the reviewed functions include collision
radius and height, awareness distance, initial health, hit/death actions,
death sound, three player-attack components, and five actor defenses.

The action-profile table is initialized data rather than BSS. Its 25 records
end exactly at `0x8005617a`, and the selector indexes them at a `0x0a` stride.
The five signed halfwords are named by their observed distance-band and weight
roles. The table's source linkage is still `unknown`: a single executable user
does not prove whether its original declaration was file-local `static` or an
externally linked global.

This boundary corrects the first pass's 26-record assumption. Raw bytes at the
would-be record 25 are two padding bytes followed by four packed `SoundRef`
records beginning at `0x8005617c`; direct references into entries one through
three corroborate the split. The old `XXXX` string at `0x80056184` was a false
ASCII pattern spanning the last sound reference and following binary data.

## Reviewed functions

The core establishes these operations:

- pool search, clear, and spawn;
- per-frame player-transform copy and current-actor binding;
- position, cell, rotation, action, and initialization helpers;
- definition-wide death transition; and
- five-component damage calculation and health/action update.

`actor_apply_damage` accepts a base power, five still-unidentified component
values, a scale, and hit flags. The inventory deliberately calls them
`component0` through `component4`; naming physical or magical damage classes
would outrun the current evidence.

## Targeting, animation, and action selection

The next reviewed block, `0x8002d4a8..0x8002e954`, supplies the actor AI's
common mechanics:

- radial actor damage, point/volume distance, overlap search, and facing-cone
  target selection;
- wrapped and clamped 12-bit animation advancement, phase-crossing tests, and
  phase-triggered positional sound;
- four action-eligibility policies using distance, chance, facing, floor
  contact, the player-selected target, or a weighted action profile; and
- ordered next-action selection plus lifecycle/awareness transitions.

`actor_try_attack_player` is named from its complete effect chain rather than
proximity alone. Its success path passes the current definition's three attack
components and status bytes to `player_apply_damage`; that callee calculates
player damage and subtracts the result from player health.

The animation tables now use `KF_ACTOR_ANIM_SLOT_*` indices, independently
of action codes and resource animation IDs. The dispatcher mapping and its
exceptions are documented in
[actor animation-table slots](patterns/game-actor-animation-slots.md).
The [actor state review](patterns/game-actor-states.md) names all decoded
action, lifecycle, vertical, collision and slot-policy values. The byte at
+`0x38` is `action_progress`, because it contains phase markers and counters,
including a selection lock, rather than solely a timer.
Per-function CFG, xref, signature, and
vendor-negative evidence for this block is in
`config/evidence/game_semantic_actor_ai.tsv`.

## Movement and action dispatch

The reviewed block now continues through `actor_pool_update` at `0x80030818`.
It identifies collision-aware x/z movement, turn-and-move, charge preparation,
horizontal and random three-axis movement, definition-driven effect spawning,
the complete current-action dispatcher, and the 128-record per-frame pool
update.

`actor_update_current_action` has two indirect jumps. The first indexes 128
entries at `0x80012524` by the actor's action byte; the second indexes five
entries at `0x80012724` by vertical state. Both tables belong to the behavior
unit's `RODATA(0x800124d4, 0x264)` claim. Their topology supports the dispatcher
role; the transfers remain indirect control flow, and candidate pointer edges
are not promoted merely by naming their source cases.

The special death path is constrained more tightly than a generic animation:
action six invokes it only on floor five for definition seven. It plays three
phase sounds, emits periodic debris and a loop sound, transitions definition
groups 0, 2, 3, and 4 to death, and sets `boss_defeat_complete`. That flag is
then consumed by map-transition logic on floors one and five. `boss` is a
behavioral label; the character's proper name remains unknown.

Per-function evidence for these ten functions is in
`config/evidence/game_semantic_actor_actions.tsv`.

`func_8002cab4` remains address-named. It has no admitted callers and only
calls `actor_pool_find_free` while discarding the returned pointer. Its twelve
FID hits are generic 0x20-byte collisions across unrelated SDK objects, so
neither a game purpose nor a library identity is justified.

Per-function CFG, call, signature, and vendor-negative evidence for the core is
in `config/evidence/game_semantic_actor_core.tsv`.

## Next questions

- Name the five defense/damage components from inventory, combat, and spell
  callers rather than their numeric position alone.
- Explain the remaining attack, movement and effect tuning values without
  inventing original design rationale.
- Determine which remaining `KfActor` bytes are collision, AI, and render state.
- Recover translation-unit boundaries before deciding whether actor globals
  had external or file-local linkage in the original C.
