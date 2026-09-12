# Shared constant review

This records the first pass. The [complete follow-up](enum-equality-review.md)
reviews every remaining group and adds explicit value aliases where consumer
evidence supports them, while retaining the typed domains discussed below.

## Method and scope

Baseline: `283b3d58`. The [pylibclang comparison](../enum-comparison.md) collected
1,965 enum declarations across all 101 manifest C variants: 341 distinct
values, with 181 values having multiple names. All repository C sources are
represented by the manifest. Included headers are deduplicated by declaration
site while preserving image/define contexts. The generated before/after tables
and retail dossiers are local `build/` evidence, not curated identity inputs.

The comparison supplies names with equal evaluated values. The decisions below
come from their surrounding arithmetic, storage, calls, and consumer families.
They are source-model deductions, not proof of the original spelling or header
boundary. This review does not merge distinct typed domains just because their
encodings coincide, or turn every numerical match into a shared tuning knob.

## The value-10 example

There are 42 declarations at value 10 before this change, including 26 in
headers. The source-local comparison leads to one shared quantity:

| Existing name(s) | Decision and consumer evidence |
| --- | --- |
| `ACTOR_DAMAGE_SUBUNITS_PER_HP`, `PLAYER_DAMAGE_SUBUNITS_PER_HP` | Replace both with `KF_DAMAGE_SUBUNITS_PER_HP` in `combat.h`. Both damage paths multiply five sets of component inputs by ten, accumulate the component results, add five for rounding, and divide by ten before their separate final scaling steps. |
| `FIRE_DEFENSE_STATUS_BONUS` | Keep. Adds ten defense points when the fire-defense status is active in `player_recalculate_combat_stats`; this is a stat increment, not a subunit conversion. |
| `ANTIDOTE_HERB_HP_RECOVERY` | Keep. Adds ten whole HP in the antidote-herb item branch. |
| `EFFECT_SWING_ANGULAR_ACCEL` | Keep. Changes the hazard's angular velocity each update. |
| `PLAYER_PITCH_STEP_LIMIT` | Keep. Clamps normal camera pitch velocity. A velocity limit and an acceleration are different quantities. |
| `PLAYER_DEATH_INITIAL_PITCH_STEP` | Keep. Supplies the initial death-camera pitch step and the initial value of `death_camera_pitch_step`. |
| `PLAYER_DEATH_REST_PITCH_ACCELERATION` | Keep. Increments that step during the resting phase. Sharing the state variable does not make its initial velocity and acceleration identical concepts. |
| `PLAYER_WEAPON_CHARGE_DELAY_UPDATES` | Keep. Reloads the weapon-charge delay in equipment/attack setup. |
| `MAP_AMBIENT_COUNTDOWN_RELOAD` | Keep. Reloads the global ambient-script timer. |
| `MAP_EMITTER_COUNTDOWN_BASE` | Keep. Adds a base interval to a random emitter countdown; it does not operate on the ambient-script timer. |
| `MAP_EMITTER_VELOCITY_SHIFT` | Keep. Shifts a trigonometric product to form emitter velocity. Ten is a bit count, not a timer value. |
| `MENU_MAP_IMAGE_TOP_Y` | Keep. Positions the map image in screen pixels. |
| `STATUS_ATTACK_SCALE_NUMERATOR`, `STATUS_DEFENSE_SCALE_NUMERATOR` | Keep separately. `menu_draw_status_details` forms different displayed ratings: attack uses 10/8, defense 10/7. Neither path accumulates damage subunits or rounds them back to HP. |
| `WARP_SHIMMER_OWNER_ID` | Keep separate from the damage constants. The constructor stores this byte in `KfEffectRecord.id`; the warp-shimmer branch initializes a visual model and has no damage-dispatch consumer. Other effect kinds use the byte as a damage multiplier, but that does not establish the warp value's role. The existing owner interpretation remains WIP. |

The header candidates also retain their identities:

| Header name(s) at value 10 | Why they remain separate |
| --- | --- |
| `KF_PLAYER_DAMAGE_MULTIPLIER_ONE` | Dimensionless tenths multiplier applied **after** the damage calculation; distinct from tenths of HP used inside the component sum. |
| `KF_FORMAT_DECIMAL_DIGITS` | Number of digits needed for the integer formatter. |
| `KF_RGB555_BLUE_SHIFT` | Bit position of the blue channel. |
| `KF_AUDIO_VOICE_SLOTS` | Audio slot capacity. |
| `KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY` | Effect-object group capacity. |
| `MENU_GLYPHS_PER_ROW`, `MENU_WINDOW_ROW_CAPACITY` | Different array dimensions: glyphs within a row versus rows within a window. |
| `KF_MAP_SAVED_RECORDS_OFFSET` | Byte offset into serialized map state. |
| `MENU_NUMBER_BLANK` | Blank glyph code in the number atlas. |
| `KF_ASSET_MAP_EVENT_FIRST` | First registry index for map-event assets. |
| `KF_ACTOR_ANIM_SLOT_EFFECT2` | Animation slot in an actor definition. |
| `KF_ACTOR_EFFECT_CODE_SCATTER_PROJECTILE`, `KF_EFFECT_KIND_SCATTER_PROJECTILE` | Related serialized effect codes, already represented by different shared typed domains. Explicit encoding/decoding connects actor definitions to effect dispatch; collapsing the domains would lose that boundary. |
| `KF_EFFECT_BILLBOARD_CURSE_PROJECTILE`, `KF_EFFECT_MODEL_MOONLIGHT_PROJECTILE` | Indices in different visual-resource families. |
| `KF_EFFECT_HAZARD_RISE_FIRST`, `KF_EFFECT_MOONLIGHT_TRAVEL_LAST`, `KF_EFFECT_PROJECTILE_IMPACT_END` | Distinct effect-kind phase boundaries, including different first/last/end semantics. |
| `KF_HUD_ATTACK_PANEL` | HUD record selector. |
| `KF_ITEM_DRAGON_SWORD`, `KF_MAP_OBJECT_DRAGON_SWORD` | Inventory item and map-object domains. The object-to-item mapping is real, but the APIs and serialized fields remain distinct. |
| `KF_MAP_ATTRIBUTE_0A` | Unresolved map-attribute encoding; numerical equality adds no meaning. |
| `KF_MAP_OBJECT_ACTION_COPY_REGION`, `KF_MAP_OBJECT_BEHAVIOR_COPY_REGION` | Definition behavior selects an action in `map_object_pool_update`. They belong to separate state/definition domains, already shared through `game_map.h`. |
| `KF_NOTIFICATION_SCHOLAR_BONES` | Notification/message selector. |
| `SAVE_STATUS_10` | Save-status encoding; no connection to the quantities above. |

Retail GAME `actor_apply_damage` at `0x8002d120` and `player_apply_damage` at
`0x80016324` confirm the shared unit conversion. The actor call at
`0x8002d244` and player call at `0x800164bc` each complete a multiplication by
ten in the delay slot. Their rounding adds are at `0x8002d2fc` and
`0x800165b0`. Keep the distinct component algorithms, final scale denominators,
argument widths, load delays, and call ordering unchanged.

## Actor angle thresholds

`ACTOR_SELECTION_ANGLE_TOLERANCE = 0x18e` (398) and
`ACTOR_MULTI_HIT_SELECTION_ANGLE_TOLERANCE = 0x1c7` (455) each have one enum
declaration in the full comparison. Searches of their numeric spellings and
source consumers reveal no second semantic owner. Keep both local to `actor.c`.

The first serves `actor_try_select_action_distance_facing` and
`actor_try_select_ground_action`; the latter serves
`actor_try_select_facing_action`, whose action caller selects multi-hit.
Retail loads the tolerances in the `angle_within_tolerance` call delay slots
at GAME `0x8002de8c`, `0x8002dff0`, and `0x8002e0c8`. Nearby aim and weapon
cones with values 341 and 1365 have different roles and are not replacements.

## Other shared definitions

The wider comparison identifies these additional connections. Constants move
to their existing shared owner headers; the damage unit gets a small shared
combat header. No runtime globals or per-object linker placements are added.

| Old definitions | Shared definition and semantic connection |
| --- | --- |
| `ACTOR_EFFECT_DEFAULT_SPEED`, `MAGIC_DEFAULT_SPEED` (600) | `KF_EFFECT_PROJECTILE_DEFAULT_SPEED` in `game_effect.h`: both launch paths scale the same forward direction vector before passing it to `effect_pool_construct`. |
| `ACTOR_LIGHTNING_VARIANT_SPEED`, `LIGHTNING_SPEED` (800) | `KF_EFFECT_LIGHTNING_SPEED`: lightning and its alternate visual variant use the same speed to scale velocity and convert target distance to travel updates. |
| `ACTOR_WIND_CUTTER_SPEED`, `WIND_CUTTER_SPEED` (800) | `KF_EFFECT_WIND_CUTTER_SPEED`: the wind-cutter case overrides the default in both launch paths. Keep it distinct from lightning despite equal speed. |
| `MAGIC_TARGET_MAX_DISTANCE`, `PLAYER_WEAPON_MAGIC_TARGET_RANGE`, `HOMING_TARGET_MAX_DISTANCE` (20000) | `KF_EFFECT_ACTOR_TARGET_MAX_DISTANCE`: all bound actor acquisition through `actor_pool_find_target_in_cone`; weapon launch stores the selected actor index in the homing control domain, and homing tracking repeats the query. |
| `PLAYER_WEAPON_MAGIC_TARGET_CONE`, `HOMING_TARGET_CONE_ANGLE` (1365) | `KF_EFFECT_ACTOR_TARGET_WIDE_CONE`: the wide actor acquisition cone at weapon launch and during homing tracking. Burst/ordinary spell aim cones remain separate. |
| `ACTOR_CONE_INITIAL_BEST_ERROR`, `MAP_EVENT_CONE_INITIAL_BEST_ERROR` (30000) | `KF_CONE_SEARCH_INITIAL_ANGLE_ERROR` in `game_math.h`: both cone searches seed a signed-halfword angular-error ranking before choosing the best candidate. This is not a world-distance limit. |
| `KF_ACTOR_RANDOM_YAW_SHIFT`, `MAP_DROP_RANDOM_YAW_SHIFT`, `MAP_EVENT_RANDOM_YAW_SHIFT`, `GROUND_VISUAL_ANGLE_RANDOM_SHIFT` (3) | `KF_RANDOM_ANGLE_SHIFT` in `game_math.h`: each extracts a complete 12-bit angle from the SDK's 15-bit `rand()` result. Actor initialization/wandering, map-object drops/debris, map-event wandering, and radial ground visuals use the same representation conversion. |
| `FLOOR_ITEM_RENDER_BRIGHTNESS` in GAME and OPEN (180) | `KF_FLOOR_ITEM_RENDER_BRIGHTNESS` in `item.h`: both floor-item render sweeps initialize all three material color channels before rendering the shared floor-item family. |
| `FORMAT_LEADING_PAD_BYTES` in GAME and OPEN (8) | `KF_FORMAT_LEADING_PAD_BYTES` in `debug.h`: the same decimal/hex formatter buffer convention reserves eight bytes before the digit anchor. Keep separate storage objects for the two images. |
| `OPENING_ENTITY_SCENE_BASE_Y`, `TRANSITION_BASE_Y` (-10000) | `KF_OPENING_SCENE_BASE_Y` in `open_resources.h`: the loaders position scene-3/ending entities at the same scene plane used by their transition placement in the scene runners. |

The projectile speed loads are explicit in GAME at `0x8002ef98`,
`0x8002efc4`, `0x8003a3cc`, and `0x8003a3d0`. The common query arguments are
visible at `0x80019754`/`0x80019760` and `0x80039788`/`0x80039790`, including
the cone argument in each call's delay slot. Both ranking sentinels are loaded
at `0x8002d838` and `0x80033bcc`. The two brightness stores begin with 180 at
GAME `0x8001f434` and OPEN `0x8001935c`. Formatter references resolve to each
image's own `format_number_storage + 8`; no address is shared across images.

## Other candidates left separate

These are the main tempting collisions from the wider table. Their names and
uses do not justify tying the values together in this source model.

| Candidate family | Decision |
| --- | --- |
| Actor/player component coefficients 5 and 2 | Keep separate. Actor base power augments attack, while player base power augments the defensive threshold; their denominators refer to different intermediate quantities. The shared HP subunit does not collapse the formulas. |
| Actor/player step-up velocities -300 and other fall/step constants | Keep independent movement tuning and distinct acceleration/velocity units. |
| Shared numerical RNG shifts in homing jitter, odds, and yaw generation | Only the complete-angle conversion is shared. Homing pitch/yaw perturbations subtract their own biases; selection shifts form probability ranges, not absolute headings. |
| GAME warp shimmer and OPEN entity transition: count 4, duration 48, scale step 256, tall scale 8192, yaw step 512 | Related loop shapes, but separate effect/model owners, mode protocols, frame pacing, and lifetime rules. Keep their animation tuning local; common values alone do not establish a shared configuration. |
| Actor boss and effect extended audio ranges 20000/60000 | Different sound events and call sites. They do not acquire the target-distance constant merely because their maximum distance is also 20000. |
| UI marker depths 500 and content depths 1000 | Map markers, shared menu quads, item previews, and text have separate rendering roles. No primitive-identity or common configuration evidence requires unification. |
| Camera/door angle limits and aim cones near 191/341 | A cone width is not a camera clamp. The follow-up connects the two 191 camera limits through their shared player-camera consumer; the 341 aim policies stay separate. |
| Spell/weapon offsets 200/400, collision radii/heights, and timing phases | Physical units and consumers differ; weapon and spell launches also have different X conventions. |
| Actor gravity 20, emerging fall acceleration 20, dropped-object gravity 20 | Independent motion models. Player fall acceleration is 40 and bouncing objects use 30, so there is no universal gravity constant supported by these consumers. |
| Capacities, indices, bit masks and named phase values | Preserve the shared domain names. For example, 4096 is an angle period, fixed-point unity, and several animation periods; 65535 is a collision mask and several unrelated sentinels. |
| Map-object IDs that equal item IDs; actor effect codes that equal effect kinds | Preserve the existing typed domains and explicit bridges. Equal serialized encodings do not make the caller-facing types interchangeable. |
| Memory constants at 1 MiB/2 MiB and buffer/sector sizes | Preserve memory-policy, object extent, and SDK meanings. The follow-up shares the fixed 2 MiB RAM capacity; no address identity or individual-global placement is inferred. |

## Verification

The affected-function list and final results follow below. The comparison
requires identical runtime sections, alignment, symbols (including COMMON
allocations), and ordered relocations for all reconstructed objects, then
unchanged strict objdiff results. The native executable build is a separate
check from the existing analysis data/ownership gates. No function is banked
as new progress by this naming review.

| Image | Function | VA / bytes | Final strict result |
| --- | --- | --- | --- |
| GAME.EXE | `player_apply_damage` | `0x80016324 / 0x390` | 100% exact, preserved |
| GAME.EXE | `player_update` | `0x80018880 / 0x1a1c` | 99.969480%, preserved residue |
| GAME.EXE | `render_entities` | `0x8001f218 / 0x580` | 100% exact, preserved |
| GAME.EXE | `actor_initialize` | `0x8002cc64 / 0xc4` | 100% exact, preserved |
| GAME.EXE | `actor_apply_damage` | `0x8002d120 / 0x388` | 100% exact, preserved |
| GAME.EXE | `actor_pool_find_target_in_cone` | `0x8002d7f8 / 0x184` | 100% exact, preserved |
| GAME.EXE | `actor_spawn_action_effect` | `0x8002edd4 / 0x454` | 100% exact, preserved |
| GAME.EXE | `actor_update_current_action` | `0x8002fa88 / 0xd90` | 100% exact, preserved |
| GAME.EXE | `map_object_spawn_effect` | `0x80031834 / 0x194` | 97.128716%, preserved residue |
| GAME.EXE | `map_object_spawn_actor_debris` | `0x800319c8 / 0x18c` | 100% exact, preserved |
| GAME.EXE | `map_event_pool_find_target_in_cone` | `0x80033b8c / 0x144` | 100% exact, preserved |
| GAME.EXE | `map_event_update_wander` | `0x80035708 / 0x1d8` | 100% exact, preserved |
| GAME.EXE | `effect_update_dispatch` | `0x80038a38 / 0x180c` | 99.827810%, preserved residue |
| GAME.EXE | `magic_cast` | `0x8003a2a0 / 0x4c0` | 100% exact, preserved |
| GAME.EXE | `format_int_dec` | `0x8003a81c / 0xe0` | 100% exact, preserved |
| GAME.EXE | `format_int_hex` | `0x8003a8fc / 0x8c` | 100% exact, preserved |
| OPEN.EXE | `opening_scene3_run` | `0x80014804 / 0x330` | 100% exact, preserved |
| OPEN.EXE | `opening_ending_scene_run` | `0x80014b34 / 0x2f4` | 100% exact, preserved |
| OPEN.EXE | `opening_ending_scroll_run` | `0x80014e28 / 0x798` | 99.917694%, preserved residue |
| OPEN.EXE | `opening_resources_load_scene3` | `0x800165c4 / 0xf0` | 100% exact, preserved |
| OPEN.EXE | `opening_resources_load_ending` | `0x800166b4 / 0x134` | 100% exact, preserved |
| OPEN.EXE | `opening_resources_load_ending_entities` | `0x800167e8 / 0x58` | 100% exact, preserved |
| OPEN.EXE | `opening_render_entities_and_items` | `0x80019240 / 0x298` | 100% exact, preserved |
| OPEN.EXE | `format_int_dec` | `0x8001a3fc / 0xe0` | 100% exact, preserved |
| OPEN.EXE | `format_int_hex` | `0x8001a4dc / 0x8c` | 100% exact, preserved |

All 101 reconstructed objects preserve their allocated sections, alignments,
symbol values/sizes (including COMMON), and ordered relocation records.
The complete objdiff report is identical to the baseline: all 484 reported
function scores are unchanged, including the 21 exact affected functions and
four affected partials above. The latter retain these first normalized
instruction/relocation differences:

| Function | Retail | Compiled |
| --- | --- | --- |
| GAME `player_update`, `0x80018880` | `addiu sp,sp,-224` | `addiu sp,sp,-216` |
| GAME `map_object_spawn_effect`, `0x80031854` | `move s4,a3` | `move s3,a3` |
| GAME `effect_update_dispatch`, `0x80038a70` | `lui s3,0` | `lui s6,0` |
| OPEN `opening_ending_scroll_run`, `0x800150c0` | HI16 names `opening_ending_scroll_camera_path` | HI16 names `.data` |

The opening row is a relocation-symbol representation difference; the paired
addend and complete objects are unchanged. These observations do not attribute
a compiler mechanism or claim closure of any partial function.

Full native `kf build` succeeds for PSX, GAME, and OPEN; each resulting EXE is
byte-identical to the captured baseline executable. Ruff and `git diff --check`
pass. The local repository suite runs 800 tests successfully (9 skipped).
`nix flake check -L` passes, including its isolated 800-test run (142 skipped
because local retail/build inputs are unavailable in that check).

`kf analyze` produces the fresh objects/report but retains the baseline's
existing data/ownership/placement gate failures. Both before and after have
source-data matches PSX 0/1, GAME 23/41, OPEN 10/20, and target relink checks
PSX 1/1, GAME 58/65, OPEN 35/39. This PR does not change those inventories or
claim those gates pass.

The final census has 1,951 declarations. Twenty-five prior declaration sites
become eleven shared definitions; every other enum name, owning file, and
evaluated value is unchanged. Each shared definition appears once after header
deduplication. Both reviewed actor angle thresholds remain in `actor.c`.
