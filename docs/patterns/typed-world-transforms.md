# Complete world transforms and angle interfaces

## Function Match Plan at `36b85b3`

Continue the [typed-storage campaign](typed-storage-cast-campaign.md) through
collision capture, map-object/event placement and rendering, and the actor
rotation consumers. One complete object must replace each flat vector or
incompatible angle-prefix view. Retail hashes were initialized before this
campaign; the six image-qualified semantic views were collected for every
function below. Shared constructor, renderer and matrix-helper controls,
constraining call sites, source history and existing field/layout dossiers
were inspected. Both image identities remain explicit throughout comparison.
All selected units retain their current compiler profiles.

The common source hypothesis follows complete transfers, not address
proximity. `collision_query_world` copies four aligned words from map object
+8 and event +0x24, including their fourth lanes, then eight rotation bytes
from actor +0x2c, object +0x18 and event +0x34. The rotation copies use paired
unaligned word loads/stores. Model the map object's position as `VECTOR` and
its rotation, together with the actor's, using the same SDK/Euler union as
the effect record. Move that union into the shared math header as
`KfRotation`; all owners retain their current sizes and alignments.

The event's +0x24 region is the complete `reference_position` vector:
placement writes its X/Z from the authored position, wander updates those
lanes and its visibility cells, and collision, sound and rendering consume
that current position. Its Y lane is the existing floor-height coordinate.
The separate +0x14/+0x18 coordinates retain their existing identities.
The event's +0x34 rotation becomes authentic `SVECTOR`: loader writes zero
to X/Z, signed Y feeds angle arithmetic, and `RotMatrix` consumes the signed
SDK components. The former unsigned X/Z declarations have no independent
unsigned arithmetic consumers. The copied final lane is SDK pad, ending
before the animation-cache slot at +0x3c.

The three GAME callers of `pitch_yaw_to_forward_vector` supply Euler-angle
objects. The helper reads signed X and Y at +0/+2 and ignores roll. Share
`const KfEulerAngles *` with these actual callers and update the otherwise
identical OPEN helper and curated signatures. OPEN has no known incoming
references; no new caller is inferred. Remove the unused, incompatible
`KfPitchYaw` prefix type. Keep the genuine six-byte Euler extent for the
magic-cast local and other six-byte owners.

The actor-effect and player-update callers also pass their angle locals as
optional rotations to `effect_pool_construct`, whose selected branches copy
all eight SDK bytes. Those locals, including the actor's burst rotation,
need the eight-byte `KfRotation` owner: use its Euler member for the game
helpers and its SDK member for construction. Do not initialize pad or alter
call order merely to influence code generation. Preserve all scalar
expressions and enum domains; remove the directly redundant matrix casts,
magic camera-rotation casts and already-signed scalar conversions in these
edited functions.

The map-object loader's two-word `effect_output` local is also the fifth
constructor argument, a complete input SVECTOR. Give it that authentic type
and the input name `effect_direction`; its components remain uninitialized,
as observed in the retail caller. No synthetic initialization is justified.

## Per-function evidence snapshots

The CFG column is decoded blocks / direct calls / branches / returns.
Final verdicts were added after the verification below.
Branches include internal absolute jumps. Every return owns its final delay
slot. No strings occur in the changed functions. All outgoing references are
proven control or validated targets; incoming candidate table rows remain
hypotheses. Dispatch indirect jumps retain their existing table evidence.

| Function | Image VA / bytes | Initial strict % | CFG counts | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME `pitch_yaw_to_forward_vector` | `80014d34 / d4` | 100.000000 | 1/4/0/1 | 100%, unchanged and exact |
| GAME `player_use_item` | `80018054 / 45c` | 100.000000 | 43/20/31/1 | 100%, unchanged and exact |
| GAME `player_update` | `80018880 / 1a1c` | 96.909640 | 325/66/242/1 | 96.909640%, unchanged partial |
| GAME `collision_query_world` | `8001a5ac / 504` | 97.943924 | 49/5/36/1 | 97.943924%, unchanged partial |
| GAME `render_actor` | `8001e9a4 / 214` | 100.000000 | 7/20/4/1 | 100%, unchanged and exact |
| GAME `render_map_object` | `8001ebb8 / 180` | 100.000000 | 8/15/5/1 | 100%, unchanged and exact |
| GAME `render_map_event` | `8001f0c4 / 154` | 100.000000 | 4/15/2/1 | 100%, unchanged and exact |
| GAME `actor_set_rotation` | `8002cc54 / 10` | 100.000000 | 1/0/0/1 | 100%, unchanged and exact |
| GAME `actor_initialize` | `8002cc64 / c4` | 100.000000 | 5/2/3/1 | 100%, unchanged and exact |
| GAME `actor_try_attack_player` | `8002d6a0 / 158` | 100.000000 | 8/5/5/1 | 100%, unchanged and exact |
| GAME `actor_try_select_action_distance_facing` | `8002dd94 / 120` | 100.000000 | 15/4/11/1 | 100%, unchanged and exact |
| GAME `actor_try_select_ground_action` | `8002deb4 / 164` | 100.000000 | 14/4/11/1 | 100%, unchanged and exact |
| GAME `actor_try_select_facing_action` | `8002e018 / d8` | 100.000000 | 10/3/7/1 | 100%, unchanged and exact |
| GAME `actor_try_select_profiled_action` | `8002e0f0 / 1f8` | 100.000000 | 23/4/16/1 | 100%, unchanged and exact |
| GAME `actor_move_xz_with_collision` | `8002e954 / 3ac` | 100.000000 | 59/6/41/1 | 100%, unchanged and exact |
| GAME `actor_move_along_heading` | `8002ed00 / d4` | 100.000000 | 6/5/3/1 | 100%, unchanged and exact |
| GAME `actor_spawn_action_effect` | `8002edd4 / 454` | 100.000000 | 51/11/34/1 | 100%, unchanged and exact |
| GAME `actor_prepare_charge_toward_player` | `8002f228 / f4` | 100.000000 | 7/5/3/1 | 100%, unchanged and exact |
| GAME `actor_update_current_action` | `8002fa88 / d90` | 100.000000 | 166/69/129/1 | 100%, unchanged and exact |
| GAME `map_object_probe_forward` | `80030eb8 / c4` | 93.755104 | 13/1/12/1 | 93.755104%, unchanged partial |
| GAME `map_object_pool_load` | `80031008 / 448` | 100.000000 | 43/11/27/1 | 100%, unchanged and exact |
| GAME `map_object_distance_to_point` | `80031450 / a8` | 100.000000 | 7/1/5/1 | 100%, unchanged and exact |
| GAME `map_object_pool_find_interaction_from` | `800315c4 / 1c0` | 100.000000 | 14/5/9/1 | 100%, unchanged and exact |
| GAME `map_object_spawn_effect` | `80031834 / 194` | 94.504950 | 21/5/13/1 | 94.504950%, unchanged partial |
| GAME `map_object_spawn_actor_debris` | `800319c8 / 18c` | 100.000000 | 11/6/6/1 | 100%, unchanged and exact |
| GAME `map_object_pool_update` | `80031cc8 / c18` | 98.966410 | 118/35/94/1 | 98.966410%, unchanged partial |
| GAME `map_event_pool_load` | `800338b8 / 22c` | 100.000000 | 8/1/5/1 | 100%, unchanged and exact |
| GAME `map_event_distance_to_point` | `80033ae4 / a8` | 100.000000 | 7/1/5/1 | 100%, unchanged and exact |
| GAME `map_event_pool_find_target_in_cone` | `80033b8c / 144` | 100.000000 | 10/2/6/1 | 100%, unchanged and exact |
| GAME `map_ambient_script_floor2` | `800341ec / 70` | 100.000000 | 5/2/3/1 | 100%, unchanged and exact |
| GAME `map_reveal_fade` | `80034438 / 184` | 100.000000 | 8/8/4/1 | 100%, unchanged and exact |
| GAME `map_floor5_transition_cutscene` | `800346a8 / 38c` | 100.000000 | 32/12/23/1 | 100%, unchanged and exact |
| GAME `map_interaction_dispatch` | `80034de4 / 904` | 96.490470 | 119/54/92/1 | 96.490470%, unchanged partial |
| GAME `map_event_update_wander` | `80035708 / 1d8` | 100.000000 | 19/9/12/1 | 100%, unchanged and exact |
| GAME `map_event_update_animation_loop` | `800358e0 / 8c` | 100.000000 | 5/1/3/1 | 100%, unchanged and exact |
| GAME `map_world_state_persist` | `80035b5c / 2b8` | 94.821840 | 28/0/18/1 | 94.821840%, unchanged partial |
| GAME `map_restore_floor_state` | `80035e44 / 69c` | 100.000000 | 50/18/32/1 | 100%, unchanged and exact |
| GAME `actor_transform_definition5_to6` | `80036d3c / f4` | 100.000000 | 5/8/2/1 | 100%, unchanged and exact |
| GAME `magic_cast` | `8003a2a0 / 4c0` | 98.891450 | 37/13/24/1 | 98.891450%, unchanged partial |
| OPEN `pitch_yaw_to_forward_vector` | `80015b0c / d4` | 100.000000 | 1/4/0/1 | 100%, unchanged and exact |

The field-only functions retain their exact producer/consumer operations:
scalar rotation reads/writes keep their signed widths; position reads keep
32-bit storage and explicit low-halfword projection; persistence keeps its
serialized widths; collision keeps complete aggregate copies. Only the
angle helper interfaces and the two constructor callers need call-argument
member changes. No SDK bodies or declarations are reconstructed. The pool,
collision, AI, script and render policies are game-owned; the four-call
pitch/yaw helpers compose game matrix helpers with separately vendored
`ApplyMatrix` providers. Existing SDK/FID attribution is unchanged.

Build from the first focused renderer and actor-effect comparisons, then
rebuild all header consumers. Resolve and compare complete linked words,
ordered call/data targets and aggregate layout against the saved objects.
Require every banked exact function to remain 100%; inspect each first
actual divergence in referent, call, CFG and type order. Finish with the
repository tests, Ruff, `git diff --check`, a full build and focused banking.
The existing full-image data/ownership failures remain separate evidence.


## Verification

All 129 functions in the affected units preserve every reference-resolved
instruction word, ordered call target and data referent. This includes 120
retail-exact functions and nine existing partials. All 40 directly edited
functions retain their initial strict scores; 32 are exact and eight remain
partial, as listed individually above. The actor-effect caller's complete
rotation locals and the loader's authentic direction local preserve their
frames, offsets and instruction streams. There is no new code-generation
residue. Overall strict counts stay 439/471: GAME 332/362, OPEN 106/108 and
PSX 1/1.

Raw comparison resolves each section-relative data reference to its unique
containing claimed datum. It does not impose a common section base where
existing claims disagree: the player-update unit's data is one such existing
placement failure. Both sides retain the same datum offsets and numeric
referents. No relocation target bits are masked, and no whole-image link
success is inferred from these function comparisons.

The stage removes 26 C-file pointer casts and three redundant scalar casts.
The C-file pointer count is now 557, down 249 from the original 806. The full
AST census records 813 written casts: 574 pointer targets and 239 scalar
targets, including the new constant layout assertions. The inventory has
105 types and 805 fields, 717 named. Assertions cover the complete actor,
map-object and map-event extents, vector/rotation pads and animation slots.

The 713-test run had one stale signature-ledger assertion; both image ledger
entries were corrected to the Euler input/SDK output interface, then all 113
inventory tests passed. The other repository tests passed, with nine skips.
Ruff and `git diff --check` pass. Fresh focused compiles and the final full
build preserve all scores. The build still fails the existing data ownership,
reference-closure and section-placement gates with no artifact failures.
Only the 32 directly edited strict-exact functions are selected for banking.
