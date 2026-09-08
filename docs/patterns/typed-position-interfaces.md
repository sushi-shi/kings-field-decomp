# Shared position interfaces

## Function Match Plan

Start at `e620b4c` in the isolated worktree. All six GAME semantic views were
collected for every function below. Additional controls cover the floor-height
helper, effect constructor, actor placement and pool update. The previous
world-transform and effect campaigns contain the corresponding full owner,
caller, adjacent-function and SDK evidence. Source history and the current
signature/field inventories were reviewed before editing.

The actor, player, effect and map-object position owners now contain authentic
`VECTOR` objects. The three-word `KfVec3i` type has no embedded field or array
owner: all remaining uses are pointer interfaces and local temporaries. Their
coordinate offsets are 0/4/8 and every game helper consumes only its stated
coordinates. Callers already supply SDK vectors through casts. The effect
constructor, in contrast, copies all 16 bytes from its position argument and
all eight bytes from its direction argument, including each SDK pad member.

First hypothesis: use `VECTOR` consistently for these shared position APIs
and their temporaries; retain each function's scalar coordinate operations,
not a new aggregate copy. Propagate the real type through every caller and
callee and delete the obsolete duplicate type. SDK pad members are untouched
when the source does not assign them. This models the existing SDK boundary
and avoids undersized objects in map-emitter constructor calls.

The map-emitter's six-byte local `KfVec3s direction` is passed to the eight-byte
constructor copy, so model that local as authentic `SVECTOR`. Its existing
three assignments remain unchanged. Replace `&object->position.vx` with the
actual complete position address. Other six-byte KfVec3s objects, including
actor attachment rows and scalar-only spawn rotations, keep their real type.

Types must not invent a frame or overread. Compare each changed object from
its first raw divergence, including stack allocation and local-address changes.
If a consistent type changes a banked result, record that concrete difference
and investigate its actual source constraint; do not add fake locals, volatile
carriers, padding or assembly. Keep every call, branch, immediate, reference,
load delay and transfer delay instruction. All profiles remain GCC 2.5.7 O2/G0.

Vendor negative control: the selected functions operate on game actor/player/
effect pools and damage, target, movement or emission policies. The arithmetic
leaf is constrained by its two game actor callers and is absent from the
vendored census; no SDK or runtime implementation is added. Incoming dispatch
candidates remain hypotheses, and no outgoing candidate is curated here.
All selected string queries are empty.

| GAME function | VA / bytes | Strict before | Blocks/JAL/branches/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| `vector3i_add_xz` | `80014f40 / 2c` | 100 | 1/0/0/1 | 100%; raw unchanged |
| `player_apply_radial_damage` | `800166b4 / 130` | 100 | 8/2/4/1 | 100%; raw unchanged |
| `player_distance_to_point_in_cone` | `80017040 / c8` | 100 | 6/2/3/1 | 100%; raw unchanged |
| `player_use_item` | `80018054 / 45c` | 100 | 43/20/31/1 | 100%; raw unchanged |
| `player_update` | `80018880 / 1a1c` | 96.9096 | 325/66/242/1 | Partial unchanged; raw unchanged |
| `actor_set_position` | `8002cbb8 / 9c` | 100 | 11/0/6/1 | 100%; raw unchanged |
| `actor_initialize_current` | `8002cd28 / a4` | 100 | 1/4/0/1 | 100%; raw unchanged |
| `actor_initialize_slot` | `8002cdcc / bc` | 100 | 1/4/0/1 | 100%; raw unchanged |
| `actor_pool_spawn` | `8002ced4 / b0` | 100 | 6/4/3/1 | 100%; raw unchanged |
| `actor_pool_apply_radial_damage` | `8002d4a8 / 1f8` | 100 | 14/2/8/1 | 100%; raw unchanged |
| `actor_pool_find_target_in_cone` | `8002d7f8 / 184` | 100 | 12/2/8/1 | 100%; raw unchanged |
| `actor_move_xz_with_collision` | `8002e954 / 3ac` | 100 | 59/6/41/1 | 100%; raw unchanged |
| `actor_spawn_action_effect` | `8002edd4 / 454` | 100 | 51/11/34/1 | 100%; raw unchanged |
| `actor_apply_horizontal_movement` | `8002f31c / 14c` | 100 | 16/1/10/1 | 100%; raw unchanged |
| `actor_update_current_action` | `8002fa88 / d90` | 100 | 166/69/129/1 | 100%; raw unchanged |
| `map_object_spawn_effect` | `80031834 / 194` | 94.5049 | 21/5/13/1 | Partial unchanged; raw unchanged |
| `map_object_spawn_actor_debris` | `800319c8 / 18c` | 100 | 11/6/6/1 | 100%; raw unchanged |
| `map_object_pool_update` | `80031cc8 / c18` | 98.9664 | 118/35/94/1 | Partial unchanged; raw unchanged |
| `map_event_pool_find_target_in_cone` | `80033b8c / 144` | 100 | 10/2/6/1 | 100%; raw unchanged |
| `effect_update_dispatch` | `80038a38 / 180c` | 96.9396 | 257/69/204/1 | Partial unchanged; raw unchanged |
| `magic_cast` | `8003a2a0 / 4c0` | 98.8915 | 37/13/24/1 | Partial unchanged; raw unchanged |

Fresh focused builds must preserve every banked exact function. Compare all
functions in the selected units against saved linked words, calls and ordered
references, then run the full build, existing tests, Ruff and diff checks.
Record partials separately and bank only the directly selected exact rows.
Full image data/reference/placement closure remains independently incomplete.

## Source and raw-object result

All 107 functions in the eleven affected units retain their complete linked
instruction words, calls and ordered references. This includes 100 exact
functions and seven unchanged partial neighbors. Among the 21 direct
consumers, sixteen are exact and five partial. No stack frame or local address
changes, including the vector temporaries and map-emitter direction. The
full SDK object declarations therefore replace the undersized views without
an instruction residue. Overall strict status remains 439/471.

The stage removes eighteen C pointer casts. No KfVec3i use remains in source,
headers or current curated signatures. The C pointer count is 512 (294 fewer
than the original 806). All 112 image variants parse without errors; the
written-cast census is 771 including 41 header casts. A separate diagnostic
review leaves only one incompatible-pointer warning, at a generic resource
chunk passed into a word-copy loader, for the resource-boundary campaign.

All 713 existing repository tests pass with nine skips, including the actor
and damage controls. Ruff and `git diff --check` pass. Fresh focused builds
and the full `kf build` have completed; the existing data/reference/placement
gates still fail in all three images, with no new artifact failure. Bank only
the sixteen directly selected exact functions.
