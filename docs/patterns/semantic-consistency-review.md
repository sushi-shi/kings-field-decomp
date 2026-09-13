# Shared-domain and helper semantic consistency

This follow-up checks the domains consolidated in PR #3 and the helpers
retained in PR #4 against their current consumers at `4d3dc5b7`. The
[enum ledger](../enum-reuse-review.tsv) retains the original member names and
maps them to the current declarations; the
[helper ledger](../common-code-candidates.tsv) retains the extraction decisions.
The tables below review semantic scope, independently of binary equivalence.
Shared policy names are reconstruction hypotheses, not recovered original
header boundaries. Different names are appropriate when they identify different
roles, subsets, encodings, or operations on a shared type.

## Corrections

OPEN `0x80014608 / 0x1fc` is now `opening_cylinder_transition`, with
`KfOpeningCylinderTransitionMode transition_mode` and
`KF_OPENING_CYLINDER_TRANSITION_*` lifecycle constants. The model remains
`KF_OPENING_TRANSITION_CYLINDER`: it names a resource selected from
`KfOpeningModelId`, whereas the mode names select an operation on that resource.
The [model inspection](open-model-domain.md) identifies OPEN model 19 as a
yellow cylinder. Retail loads 19 at `0x80014680` and copies it into four entity
records. The complete signed-halfword dispatch and its four proven call sites
in scene 3 and the ending give these operations:

| Mode | Operation |
| --- | --- |
| 0 / GROW | Create zero-height cylinders, animate growth, retain them. |
| 1 / REMOVE | Deactivate the four reserved records immediately. |
| 2 / SHRINK | Create tall cylinders, animate shrinkage, then deactivate. |
| 3 / CREATE | Create tall cylinders and return without animation. |

GAME's `KfWarpShimmerMode` remains distinct: its grow/remove, shrink/remove,
and grow/keep codes are 0, 1, and 2. Its effect pool, horizontal scale and
sound timing differ from OPEN's entity pool. Shared scale/yaw/stagger constants
name only the repeated animation quantities. The reserved OPEN slot constant
also gains the cylinder qualifier. Curated function identities, relocation
target spellings and the existing exact-baseline name follow the function
rename; no address, provenance or score is changed.

`opening_run` and OPEN `display_initialize` consistently call their transported
`KfOverlayMode` argument `overlay_mode`. OPEN `0x80015718` passes the same
selection to display initialization that the controller uses to dispatch intro
or ending scenes. It is broader than display configuration. GAME's zero-argument
`display_initialize` is a different image-qualified function.

GAME's four-byte word at `0x800958f8` is now `game_next_overlay_mode`.
`game_main_loop` initializes NONE and reads it as the loop-exit gate;
`player_update` selects INTRO when the menu requests return to the opening;
the ending warp selects ENDING. GAME `main` copies the word into
`KfOverlayArguments.result`, and PSX forwards it as the next OPEN request.
The name preserves GAME's ownership and states what the value selects.
Curated BSS identity and all five validated address references retain their
original image, address and width.

The system-message renderer is now
`display_show_system_screen(KfSystemScreen screen)`, with
`system_screen_path` and `SYSTEM_SCREEN_READ_ATTEMPTS`. Its seven proven
incoming call sites cover CD search/read errors, missing-card notification,
and pause. Thus the earlier error/stage names described only a subset of the
operation. The path bytes remain `\\E0.;1`, its selected digit is still copied
into the working CD path, and the read-attempt count remains 50.

The graphics parameter pass also makes the independently nullable view
arguments consistently `position_or_null` and `rotation_or_null` through
`render_set_view_transform`, `render_frame`, `opening_render_frame` and
`opening_scene0_render_frame`. Each NULL preserves that part of the stored
view; neither pointer's validity depends on the other. F4/FT4/G4 builders call
the complete X/Y/width/height argument `rectangle`. Both `render_map_cell`
implementations call their near/distant class `visibility`, distinguishing
it from the row/column coordinate and the cell's geometry attributes.

PR #4's terrain predicate is now `MAP_CELL_HAS_FULL_FLOOR`. It recognizes only
`KF_MAP_CELL_FLOOR` and `KF_MAP_CELL_STEP`. The neighboring diagonal cases in
`effect_collision_in_cell` need coordinate tests and can also contain floor;
the old `HAS_FLOOR` name overstated the predicate. The five uses retain the
same equality tests and short-circuit order.

## Shared domains and constants

| Family | Consumer check and verdict |
| --- | --- |
| Combat component indices and HP subunits | Actor/weapon arrays use the same five-component order; physical-only arrays stop after three. Actor and player formulas convert tenths of HP, but their base-power formulas and final scaling remain different. Retain common indices and unit conversion. |
| Overlay mode | PSX copies `entry_args.result` directly into `request`; OPEN passes the request into its controller and display setup. Retain one mode domain with signed request and unsigned stored result. Rename the two parameter spellings as above. |
| Map operation | `map_object_pool_trigger_link` forwards `definition.behavior_type` to `map_object_start_action_if_idle`. Retain one operation domain; `behavior_type` is authored policy and `action` is current dispatch state, so their field names remain different. |
| Object/item IDs and equipment bounds | Map pickup passes an object ID into the item menu; equipment paths index the item records using the same values. Retain `KfObjectId`, inventory-specific `item_id` roles and scenery-specific names. A shared type does not make scenery IDs valid item-table subscripts. |
| Spell/effect IDs | `magic_cast` passes spell identities to effect construction, and construction reads the common magic records. Retain the common kind domain; spell, effect-model and billboard tables remain distinct roles/resources. Utility and player-selection APIs accept subsets. |
| Packed actor effect code | Actor dispatch masks the five-bit payload before `actor_effect_kind_from_payload`; the paired flag and sentinel belong to the packed representation. Retain that boundary and separate code/kind names. |
| Stock banks | Shop arguments select rows of `item_stock`; bank zero is the player's stock. The reviewed parameter name `shop_bank` identifies the shop subset of `KfItemStockBank`; it does not imply a second encoding. |
| Trade modes and menu rows | Buy/sell dispatch forwards the same transaction direction to pricing. Integer row indices encode the selected action; return/gold rows are layout-only values. Retain the distinction between action, price direction and row roles. |
| Menu results | Confirm/pickup/panel callers test their own valid subsets of `KfMenuResult`. The confirm choice is negated: decline-choice 1 becomes cancelled-result -1, not declined-result 1. Nonnegative selector payloads remain IDs/rows rather than universal result codes. Retain the bridge and subset contracts. |
| Sprite state | HUD traversal skips HIDDEN and stops at END; effect traversal continues only through VISIBLE rows; notifications draw eligible members of a fixed six-record table. VISIBLE means draw eligibility in each consumer, not one shared traversal algorithm. HUD row IDs and sprite state have different roles. The notification field `active` is a visibility latch; it does not imply HUD identity or a separate state encoding. |
| Floor-item appearance | Facing and frame-count masks describe the same packed byte in loaders and renderers. The biased facing selector remains a separate representation; animation frame index is not the packed frame count. |
| Save completion | Card completion and successful file operations share OK. Menu-facing failure/format/no-space results remap the status channel, and cleanup errors differ again. Retain those distinct result types and conversion sites. |
| Main RAM capacity | Allocator bounds and overlay stack derive from the same fixed RAM capacity. Initial arena extent and stack reservation remain separate policies; no individual-global placement follows from the shared capacity. |
| Player camera pitch and restored fog | Container look and player motion use the same pitch limit on the player camera; darkness restores the renderer's normal fog distance. Retain common policy quantities, separate motion rates and darkness interpolation. |
| Cylinder animation | Matching scale/yaw/stagger loops support a shared animation policy, with the separate lifecycle contracts above. This is weaker evidence than direct value transport and does not prove one historical configuration owner. |
| Projected-vertex capacity | GAME and OPEN each allocate their own projected-vertex storage with the same WIP capacity. Retain one capacity constant, preserve separate storage and extra morph scratch extent. |
| Extended sound range | Boss/effect calls pass the same maximum/attenuation pair to `audio_play_spatial_range`. Retain the named API policy; equal actor-target range is unrelated. |
| Menu depths and item-name column | Enqueue sites share OT layers and must preserve insertion order within each layer. Item-preview and equipment labels use the same X column, with their own Y/pitch. Retain shared layout policy; no shared primitive owner is inferred. |
| Effect animation tuning | Projectile roll, uniform dissipation, branch/child yaw and orbit audible/reset range retain the same units and operations in their consumers. Sharing these quantities does not generalize growth, gravity or lifetime policy across effects. |
| Projectile launch and target acquisition | Actor/player launch paths scale the same direction and pass it to effect construction; launch/homing queries target actors through the same API. Retain default, lightning and wind-cutter speeds and common acquisition limits; equal lightning/wind speed alone does not merge spell policies. |
| Angle conversion and ranking | `rand() >> KF_RANDOM_ANGLE_SHIFT` extracts a full angle; cone searches seed a signed ranking error. Angle randomness, probability shifts and world distance remain distinct quantities. |
| Floor brightness, formatting and scene plane | Paired GAME/OPEN floor renderers use the same RGB initialization; formatters reserve the same digit prefix; OPEN scene loaders and transitions use the same base Y. Retain those common quantities while each image keeps its own storage. |

## User-review dispositions

| Review issue | Final decision and evidence |
| --- | --- |
| Cylinder tuning versus opening mode names | Rename the OPEN function, lifecycle enum/members and reserved-slot constant consistently. Keep the model-resource ID distinct from the lifecycle operation and keep GAME's different mode encoding. |
| `SetDispMask(1)` | Retain the authentic SDK integer argument: 1 enables display output; 0 inhibits it. Pinned Psy-Q Release 2.5 `demo/SOUND/SIMPLE/MAIN.C:90` explicitly documents those meanings, and `LIBGPU.H` declares `int SetDispMask(int mask)`. The argument does not select a framebuffer or a game mode. Correct the OPEN evidence description: non-ending display initialization leaves output disabled, and the intro controller subsequently enables it. |
| `KfOverlayMode display_mode` versus `mode` | Use `overlay_mode` consistently in OPEN controller and display setup. The transported selection controls the entire intro/ending sequence as well as display initialization. |
| HUD ID versus shared sprite state | Retain `KF_HUD_*` row identities and `KF_SPRITE_*` visibility values. Row selection and draw eligibility are different roles; HUD, effect and notification traversal contracts remain explicitly separate. |
| `s32 result = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING)` | Retain the encoded integer in `menu_root`: the use-item branch can return an object ID, the magic branch temporarily receives a spell ID before remapping it, and the system branch returns menu controls. Declaring the entire variable `KfMenuResult` would falsely put those payloads in the control domain. The typed selection wrappers remain appropriate at narrower panel boundaries. |
| Whether the original developers reused that variable | Retain the current mixed-channel reconstruction without claiming the original variable spelling or scope. The linked control/value flow proves the channel's contents, not the original source's local-variable organization. |
| Cleaner typed inventory subscripts | Retain explicit `KF_ENUM_ENCODE` at the raw `item_stock` array boundary. Bank and object selectors stay distinct domains before conversion; C++ scoped enums require that conversion for a built-in array. A typed `operator[]` wrapper could offer another checked interface, but its absence is not a semantic inconsistency and this review does not invent a new storage owner merely to remove casts. |
| `KF_MAGIC_*` in an effect-kind field | Retain the canonical spell names in `KfEffectKind`. `magic_cast` passes the selected spell value directly to effect construction and both access the common record namespace. The spell prefix identifies named spell members of that namespace; it does not introduce another encoding. Effect models, billboard sprites and packed actor-code flags still have separate domains. |
| `KF_MAP_OBJECT_OP_LIFT_DOOR` beside `KF_MAP_OBJECT_OP_03` | Retain the known operation-2 name and unresolved operation-3 identity. Shared collision/render treatment does not prove operation 3's lifecycle; serialized-resource evidence and its boundary limitations are recorded below. |
| GAME exit code versus overlay mode | Rename the GAME-owned word to `game_next_overlay_mode`. Its consumers establish a next-program mode and loop-exit gate, with no generic error-status consumer. Preserve `KfOverlayResultWord`'s unsigned storage. |
| Graphics rectangle, cell and optional-pose arguments | Apply the role-specific names above to all declarations, definitions and curated signatures. Keep `tmd_release_last_allocation`'s unused `slot` parameter as an ABI-domain name while documenting that both image bodies ignore it. |
| All enum/record arguments, including unrelated functions using one type | Apply the individual decisions in [parameter-naming-review.tsv](../parameter-naming-review.tsv). The pylibclang inventory covers declarations and definitions in both frontend modes; its `by_type` index compares names across functions as well as within signatures. Different names require an actual role distinction. The cross-function pass additionally aligns the current-effect setter with `effect` and retains the common `floor` spelling for the dialogue resource selector. |
| Reusable resource/serde evidence | Add `kf resources census` for independently specified fixed-record layouts. It reports signed/unsigned values, explicit zero counts, resource hashes, row indices and byte references. Apply it separately to the authored definition payload and the larger retail loader read; neither byte counts nor a renderer alone assign semantic names. |

## Retained helper boundaries

`KF_MAP_OBJECT_OP_03` remains unresolved. Its collision-edge marker writes two
cells like `KF_MAP_OBJECT_OP_LIFT_DOOR`, and the renderer gives both the same
depth bias. Those shared geometric treatments do not establish an independent
action or an object identity. The [map-operation evidence](game-map-object-constants.md)
supports the lift-door name for operation 2 only; operation 3 keeps its numeric
name until a consumer or resource supplies stronger evidence.

The reviewed retail `COM.DAT` (7,380 bytes,
SHA-256 `8fea86564bf5927f00156edf3163f3d008f86289103dd076746fad7a8922e329`)
adds a serialized-data control. Its definition chunk has a length word at
`0x160c`, payload at `0x1610`, and 1,128 payload bytes: 141 records of eight
bytes. None has behavior 3. Records 117 and 118 have behavior 2 and identical
bytes `02 00 00 00 e8 03 00 00`.

The authored payload is not the complete runtime-copy boundary. Retail copies
1,280 bytes (160 records) starting at the same `0x1610`, including the next
growth chunk's length and its first 148 payload bytes. The existing codec
models that distinct source extent in `game_data.rs` and `records.rs`.
Censusing that full source range also finds no behavior 3. Behavior 2 occurs
at indices 117, 118, 148, 154 and 157; the last three are bytes copied from the
growth chunk, not independently stored map-object definitions. Neither census
supplies an authored behavior-3 asset to inspect. This still does not prove
the code can never encounter 3 after runtime writes or with other resources.

For unresolved serialized values, first trace the field's width/encoding and
resource loader, census both the authored payload and the actual copy extent,
connect authored rows to their assets, then compare their runtime consumers.
Do not treat records reinterpreted across a chunk boundary as independently
authored rows. A matching render
or collision branch alone establishes only shared treatment. Empty census
results preserve uncertainty rather than assigning a meaning from an adjacent
enum value. Keep resource offsets and hashes with the result so the same
method can be applied to other enum/structure fields.

Every retained candidate is listed below, including the two names that refer
to the same terrain predicate and the two prepared-TMD traversal candidates.
The existing ledger records the byte comparison and rejected extraction forms.

| Candidate | Semantic boundary reviewed |
| --- | --- |
| actor-facing | Bearing uses the actor's position and cached player position; caller narrowing and steering policy stay outside. |
| actor-home-init | Local actor initializer derives home X/Z, queries floor Y, publishes position/zero rotation and initializes the actor; slot lifecycle publication remains the caller. |
| actor-player-distance | Local distance helper bounds the cached actor/player separation; its maximum belongs to each behavior caller. |
| audio-sequence-close | Stop/close and publication share one ordered operation; active check and optional fade remain caller policy. |
| audio-voice-reset | Reset clears tracked voice IDs; it is not an SDK key-off operation or a full audio reset. |
| audio-voice-slot | Increment/wrap/evict/store/key-on is one tracked-slot operation; availability gates and spatial attenuation remain callers. |
| camera-fixed-state | Publishes position and rotation into Q4 accumulators; does not compute a segment. |
| camera-step | Advances Q4 pose and publishes wrapped angles; segment selection and the caller's Y offset stay explicit. |
| card-new-device | Local two-pass acknowledgment is shared only by callers with the same event sequence; stale-catalog read handling differs. |
| cd-location | Copies minute/second/sector, deliberately retaining track. It is not a full record copy. |
| cone-angle-error | Folds wrapped angle difference into a half-turn magnitude; caller cone thresholds and ranking remain outside. |
| current-color-blend | Local helper snapshots the current lighting matrix each call; persistent multi-frame snapshots cannot use it. |
| decimal-path | Writes exactly three decimal characters without a terminator; signed input arithmetic and caller bounds remain unchanged. |
| distance3 | Computes approximate vector length using the existing signed pre-square shifts; no position/reference frame is imposed. |
| equipped-item | Tests seven equipped inventory slots in order; magic selection is a separate domain. |
| fade-color | Steps byte RGB brightness using the common transition rate and limit; does not promise saturation before byte wrap or implement a whole fade. |
| floor-item-frame | Advances and wraps the animation byte against the packed frame count; timing and facing are caller concerns. |
| input-edge | Tests a single button's rising edge, with the previous sample short-circuited; does not poll input or mean held state. |
| interaction-probe | Publishes yaw-based X/Z probe coordinates; caller supplies reach and computes height separately. |
| map-cell-passable | Renamed predicate recognizes full floor/step shapes, not complete collision passability. |
| map-floor-predicate | Same full-floor predicate; diagonal geometry and occupancy checks stay in their owners. |
| menu-background | Local reflected-tile draw and shared reverse background enqueue are distinct operations despite one candidate family. |
| menu-glyph | Begins one text glyph's packet/page/CLUT/rectangle; glyph lookup, marks, UV and commit remain ordered in the text renderer. |
| menu-list-step | Previous/next maintain selection, cursor and scroll together; callers must handle empty lists first. |
| menu-preview | Local helper initializes light-source coefficients only; model rotation, allocation and scratch ownership remain callers. |
| menu-status-glyphs | Draws current/slash/maximum from caller-owned text workspace; does not unify signed live/save data storage. |
| menu-tile | Local list-tile draw retains SDK packet construction and publication; it is not a generic allocation API. |
| overlay-load | Local launch owns ordered driver/load/Exec setup; caller-owned header and request/result exchange survive returns. |
| placement-position | Converts one byte tile coordinate plus signed local offset to world units; Y and lifecycle policy remain callers. |
| player-floor-position | Snapshots camera X/Z plus floor Y, not the camera's full position. |
| radial-falloff | Computes the common distance weight and attenuated scale; the full-falloff bypass remains in both callers. |
| resource-stream-next | Advances an aligned length-prefixed byte cursor to the next header; does not load/copy a payload or check capacity. |
| save-summary | Copies/compares six semantic fields, ignoring padding; player-state extraction has a different source layout. |
| tmd-format | Names typed object-table and packet-body boundaries; format and lighting dispatch stay explicit. |
| tmd-packet-walk | Decodes packet body length and prepared vertex offsets without changing split header/body traversal. |
| tmd-vertex-access | Prepared indices are byte offsets, not vertex-array indices; preserves the pointer arithmetic and table ownership. |

## Verification

The final combined campaign preserves allocated section bytes, alignments,
BSS/COMMON sizes, unordered resolved symbol metadata, and ordered relocations
for all 101 rebuilt objects against the verified PR #4 baseline. Only four
reviewed identity renames are normalized: the cylinder transition, system-screen
function and path, and GAME next-overlay word. The undefined symbol-table rows
for the main-loop function and overlay word exchange order in GAME main; their
metadata and the ordered relocations resolved through those names agree. The
complete strict objdiff report is unchanged.

Full native `kf build` passes. PSX.EXE and OPEN.EXE are byte-identical. GAME.EXE
has 221 changed bytes caused solely by the renamed COMMON global's native
allocation: all 61 native GAME objects preserve sections and ordered referents,
and restoring only its old name in scratch assembly recreates the complete
baseline GAME.EXE. See the [native COMMON control](native-common-symbol-names.md)
for addresses, hashes, commands, and the explicit placement limitation.

The full repository suite passes 824 tests with nine skips, and all four flake
checks pass. The fresh parameter AST scan covers 700 selected enum/record parameter sites
out of 1,517 parameter sites across 101 manifest C variants and 212 type/image
groups. All 700 final-name dispositions reconcile: 573 retained and 127 renamed. The focused graphics controls also pass, including the negative
control for conditional view-matrix reconstruction. Enum values, ABI widths,
function extents, calls, and curated data addresses remain unchanged.

| Image / function | Final strict verdict |
| --- | --- |
| OPEN `opening_cylinder_transition` | 100%, preserved. |
| OPEN `opening_scene3_run` | 100%, preserved. |
| OPEN `opening_ending_scene_run` | 100%, preserved. |
| OPEN `opening_run` | 100%, preserved. |
| OPEN `display_initialize` | 100%, preserved. |
| GAME `collision_query_world` | 100%, preserved. |
| GAME `effect_map_collision` | 99.87369%, existing residue preserved. |

Strict objdiff remains 458 exact functions out of 471. Full `kf check` retains
the baseline's existing failures: 29 of 62 data-owning units diverge, and target
relink succeeds for 58/65 GAME, 35/39 OPEN, and 1/1 PSX units. The naming campaign
neither introduces nor resolves those ownership/relink limitations.

No new result is banked and no vendored body is reconstructed.
