# Progress-controlled map links

## Function Match Plan

Name the three map-link IDs selected by weapon and boss progress, using shipped
placements and the actual clear/trigger consumers. Replace five raw calls in
GAME `map_restore_floor_state` (`0x80035e44 / 0x69c`) and the boss-death call in
`actor_update_boss_death_sequence` (`0x8002f8cc / 0x1bc`). Shared constants belong
with the map-link definitions. Keep the u8 APIs, all predicates, encoded values,
ordered calls, delay slots and resource contents.

The current source, history, cached retail/CFG/caller/callee/string dossiers and
the [script-state evidence](game-map-script-state.md) were reviewed. The restore
function has a 40-byte frame and nineteen proven calls. Calls at `0x8003633c`,
`0x80036360`, `0x800363a4` pass link 51; `0x8003647c` passes 52; `0x80036498`
passes 13, each in its delay slot. Preserve the cached switch-edge evidence
statuses rather than interpreting old unreachable annotations as proof against
the decoded calls. The caller is `map_load_floor`.

The boss-death function has a 64-byte frame, thirteen proven calls and seven
validated data pairs. It sets the saved completion flag, calls the link trigger
at `0x8002f94c` with 13 in the delay slot, then starts four actor-definition
death sequences. Its caller is `actor_update_current_action`. The continuing
effect/sound cadence and unrelated numeric arguments remain unchanged. These
are game-specific progress policies around separately attributed SDK services.

## Shipped groups and consuming behavior

The configured retail directory was validated. Decode the eight length-prefixed
chunks of every `KF/B1..B5/MIXA.DAT`; read 20-byte live object placements from
chunk 4 through the free/end marker. Match their IDs to the eight-byte definitions
in `KF/COM/COM.DAT` chunk 5. The ignored placement report records every matching
slot, cell, local position, yaw, link bytes and per-file SHA-256.

| Link | Shipped objects | Name and role |
| --- | --- | --- |
| 51 (`0x33`) | Tall hinged-door models 122/121, behaviors 1/0. B1 slots 15/16 at cells (66,4)/(68,4); B2 slots 16/17 at (46,4)/(48,4); B3 slots 17/18 at (37,4)/(39,4). | `KF_MAP_LINK_WEAPON_TRANSFORM_DOORS`: cleared on each of floors 1–3 when the weapon-transformed flag is set. |
| 52 (`0x34`) | B5 tall hinged-door models 122/121, behaviors 1/0, slots 12/13 at cells (78,45)/(78,47). | `KF_MAP_LINK_FLOOR5_SWORD_DOOR`: cleared when the player has Dragon Sword or Moonlight Sword, or the weapon-transformed flag is set. |
| 13 (`0x0d`) | B5 slots 4–9: six Fire Ball emitters, model 124. Slots 16–19: four boss projectile emitters, model 115. Their definition behavior is 255. | `KF_MAP_LINK_BOSS_EMITTERS`: cleared by the boss-death trigger and reapplied from the saved completion flag, disabling subsequent emission. |

No other live placements carry these three links. Coordinates above are X,Z.
No room names or original authoring labels are inferred. The B5 resource hash is
`75bf2657ae9a1aada8a679134b7c446087acad9c81156f8a3479614a27d5d023`.

The [link-clear review](floor3-dialogue-door-link.md) establishes that clearing
accepts definition behaviors zero through eight and stores the absent-link byte
255. The hinged-door interaction checks that byte before starting either leaf,
so clearing links 51/52 unlocks the authored pairs.

The map-object placement loader starts models 115 and 124 in projectile-emitter
action 80. GAME `map_object_pool_trigger_link` (`0x80031b54 / 0xf0`) tests runtime
action, not definition behavior, for this family: actions 11 and 80–82 take the
link-clearing branch at `0x80031ba8..bc`. The emitter update stops at its initial
absent-link check. Thus link 13 disables these emitters despite their definition
behavior being 255; it does not open a door or start another projectile cycle.

The shared names describe these observed policies. They do not introduce link
validation or alter the general encoded link field, which also carries authored
switch and object references. Builds, compiler checks, tests, post-edit matches
and banking remain deferred until the naming pass finishes.

## Source review result

All six raw link arguments now use the shared names. Reversing the substitutions
and removing their definitions recovers both prior C files and the shared header
exactly. These encodings are authored identifiers, not magnitudes with units or
a derived formula; their observed consumers establish the useful names.

The current ledgers match all 111 C files and 6,081 retained occurrences. Floor
restoration and boss-death update remain unverified after this naming edit.
No runtime or binary-match claim follows from the source comparison. Unknown
field identities and other semantic work remain open.
