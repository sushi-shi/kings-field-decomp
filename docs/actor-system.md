# Actor core

The reviewed GAME actor core begins at `0x8002ca78`. This is a semantic WIP,
not a claim that `actor`, `action`, or any field name reproduces FromSoftware's
original spelling. The names describe effects visible in retail MIPS and may
be refined as more of the surrounding update/render code is inventoried.

## Storage layout

The array boundaries are supported by independent size and stride evidence:

| Address | Extent | Identity | Evidence |
| --- | ---: | --- | --- |
| `0x80056080` | `0x104` | `actor_action_profiles[26]` | the only user indexes 26 ten-byte records; each record supplies five signed distance/weight values |
| `0x8006bd98` | `0x720` | `actor_definitions[12]` | `func_80030a6c` copies `0x1c8` words; all consumers use a `0x98` stride |
| `0x8006c4b8` | `0x2400` | `actor_pool[128]` | allocation/clear/search loops use 128 records at a `0x48` stride |
| `0x8006e8b8` | `0x10` | `actor_player_position` | copied from the per-frame player snapshot and used by actor range/angle code |
| `0x8006e8c8` | `0x08` | `actor_player_rotation` | adjacent optional rotation copy in the same per-frame setter |
| `0x8006e8d0` | `0x04` | `current_actor_definition` | `actor_bind_current` derives it with `definition_id * 0x98` |
| `0x8006e8d4` | `0x04` | `current_actor` | bound actor pointer used throughout the action/update routines |
| `0x8006e8d8` | `0x02` | `current_actor_index` | `(actor - actor_pool) / 0x48` |
| `0x8006e8da` | `0x02` | `current_actor_definition_id` | zero-extended byte at actor offset `0x01` |
| `0x8006e8dc` | `0x04` | `player_target_actor` | written from player-facing cone searches and compared with `current_actor` during ground-action selection |

Startup clears `0x2b48` bytes beginning at `actor_definitions`, exactly through
the byte before `0x8006e8e0`. The definition array, actor pool, and context
globals therefore form one contiguous actor subsystem. The old Ghidra-derived
identities for addresses inside the two arrays were fields/elements, not
separate C globals; curation replaces them with the owning array extents.

`KfActorDefinition`, `KfActorActionProfile`, and `KfActor` are declared in
`include/kf/semantic_types.h` with exact `0x98`, `0x0a`, and `0x48` size checks. Unknown
bytes remain explicit padding. The currently supported actor fields include
the slot/free marker, definition ID, tile/local/world positions, lifecycle,
action and timer, animation phase and step, health, rotation, and movement
impulse. Definition fields used by the reviewed functions include collision
radius and height, awareness distance, initial health, hit/death actions,
death sound, three player-attack components, and five actor defenses.

The action-profile table is initialized data rather than BSS. Its 26 records
end exactly at `0x80056184`, and the selector indexes them at a `0x0a` stride.
The five signed halfwords are named by their observed distance-band and weight
roles. The table's source linkage is still `unknown`: a single executable user
does not prove whether its original declaration was file-local `static` or an
externally linked global.

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
components and status bytes to `func_80016324`; that callee calculates five
player damage components and subtracts the result from player health. The
callee remains address-named until the larger player-combat family is reviewed.

The action and lifecycle numeric values remain unnamed. The current function
names describe selection policy and state effects without inventing enum labels
or original FromSoftware spellings. Per-function CFG, xref, signature, and
vendor-negative evidence for this block is in
`config/evidence/game_semantic_actor_ai.tsv`.

`func_8002cab4` remains address-named. It has no admitted callers and only
calls `actor_pool_find_free` while discarding the returned pointer. Its twelve
FID hits are generic 0x20-byte collisions across unrelated SDK objects, so
neither a game purpose nor a library identity is justified.

Per-function CFG, call, signature, and vendor-negative evidence for the core is in
`config/evidence/game_semantic_actor_core.tsv`.

## Next questions

- Name the five defense/damage components from inventory, combat, and spell
  callers rather than their numeric position alone.
- Resolve lifecycle and action enum values across the remaining action handlers
  at `0x8002e954..0x80030818`.
- Determine which remaining `KfActor` bytes are collision, AI, and render state.
- Recover translation-unit boundaries before deciding whether actor globals
  had external or file-local linkage in the original C.
