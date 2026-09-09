# Remaining data-owner campaign

## Function Match Plan

Starting point: `55c7215d`, in the isolated `codex/data-modules-20260909`
worktree. The user requests review and reduction of incorrectness in **every**
remaining failing data owner, continuing until the available evidence and
controlled source hypotheses are exhausted. A passing subset does not finish
this campaign. Keep all banked functions and the strict data/placement gate.

The fresh strict baseline has GAME 30/43 and OPEN 11/20 matching owners,
with 22 failures and no missing artifacts. Retain complete starting objects,
function scores and image-qualified semantic dossiers for every failed owner.
For each affected function inspect address/extent, retail disassembly/CFG,
callers/callees, strings, data/relocations, adjacency, history and prior verdict
before editing. Library candidates stay excluded from game progress.

Review initialized object identity and native section selection; private
versus exported uninitialized state; compiler reservation versus C extent;
complete-object ownership; original COMMON allocation; and the remaining
switch-table instruction destinations. Prior failed probes constrain the next
experiment but do not excuse an unexamined owner. Module changes require
independent shared-state/source evidence and must preserve all function words,
calls and physical referents unless a separately supported function correction
is being tested. Never introduce padding, fake objects, source permutations,
address scattering, relocation masking or a relaxed gate.

Publish a final evidence-based verdict for each starting owner, including any
partial improvement that does not close its gate and the specific remaining
unknown. Verify native compiler/assembler behavior independently when it
determines storage. Rebuild affected units, run full `kf build`, lint, repository
tests, type checks and diff checks; tooling changes also require Nix checks.
Bank only selected verified 100% functions and commit focused retained changes.

## Starting owners

| Image | Owner | Initial strict failure |
| --- | --- | --- |
| GAME | `game` | BSS size |
| GAME | `equipment` | BSS placement |
| GAME | `player_death` | BSS layout/placement |
| GAME | `player_core` | BSS layout/placement |
| GAME | `cd_file` | BSS layout/placement |
| GAME | `resources` | BSS layout/placement |
| GAME | `render` | BSS size/placement |
| GAME | `render_frame` | initialized data placement |
| GAME | `save_system` | BSS size |
| GAME | `map_object` | switch-table addend |
| GAME | `audio` | BSS placement |
| GAME | `map_scripts` | switch-table addend |
| GAME | `pad` | SDK BSS size |
| OPEN | `opening_helpers` | BSS size |
| OPEN | `camera_path` | BSS size |
| OPEN | `opening_scenes` | initialized data placement |
| OPEN | `resources` | BSS layout/placement |
| OPEN | `render_init` | BSS size |
| OPEN | `entity_render` | initialized data placement |
| OPEN | `audio` | BSS placement |
| OPEN | `format` | private buffer extent/base |
| OPEN | `pad` | SDK BSS size |

The fresh object audit supersedes the older campaign's switch-table list:
GAME `render` now has eighteen exact functions and matching RODATA. Only
`map_object` and `map_scripts` retain code-derived table addend failures.

## OPEN resource storage

The combined resource owner already declares the CD location and two saved
arena cursors privately. Their references are confined to the same loader
run; CD bytes are written before `CdControl`, and the controller reaches the
restore scenes only after the saving scenes. The three C objects are four
bytes each at `800375d8`, `800375e0`, `800375e8`, in the established tail-BSS
region. Their native G8 `.lcomm` reservations are eight bytes each, at offsets
0/8/16 in `.sbss`. The source expressions and declaration order are unchanged.

All ten complete function bodies, calls and physical referents survive G8
unchanged and remain raw retail-exact. Original ASPSX confirms three local
reservations, twenty initialized `.sdata` bytes with path offsets 0/8/12, and
seven independent XBSS requests for the two CD work objects and five map
grids. Keep the small-section correction; those exported allocations remain
a separate unresolved whole-link COMMON contract.

## Initialized renderer ownership hypotheses

OPEN's immutable floor-item light preset belongs to renderer setup rather
than the sprite-definition table: `render_initialize` establishes the shared
floor-item material and light state, while the traversal installs the preset
before its floor-item pass. Its single `SetLightMatrix` use does not prove that
the definition lived with the traversal. The five colour presets at `80035944`
and this light preset at `800359e4` form six adjacent complete SDK matrices;
the GAME homolog independently groups colour/light setup at the renderer API.
Probe one initialized preset contribution in `render_init`, retaining the
separate light identity/API use and the seven sprite records in `entity_render`.
This is a WIP ownership hypothesis, not a claim that colour and light matrices
are one indexed array or that original filenames have been recovered. Retain
only if all seven functions and all initialized bytes/referents remain exact.

GAME needs an independent review of the palette pair: both authored `RECT`
records have x=0, width=16, height=1 and consecutive palette rows 500/499;
only `render_initialize` reads them, through its two `GetClut` calls. Probe a
private two-entry palette table owned by that initializer, rather than two
exported globals defined in the unrelated frame caller. This sixteen-byte
table belongs with the adjacent colour-preset table used by the same renderer
API. The later error-screen filename then has the independently calibrated G8
small-data class. Reject this hypothesis if it changes any banked instructions.
Separately test the contiguous HUD/effect/notification helper family, whose
three sprite tables feed the shared frame path; keep each table's distinct
type, identity and complete extent. Original source boundaries remain WIP.

## Private audio workspace hypothesis

Both `audio_sequence_table` definitions have exactly one game-source use:
`audio_initialize` passes the complete workspace to the SDK's `SsSetTableSize`.
The SDK keeps the pointer; no other source module names the array. The shared
audio runtime is independently consumed by spatial/voice/sound helpers and
must remain exported. The sequence workspaces lie in each image's low fixed
BSS region, while the runtimes lie in the later allocation region. Probe
private workspace linkage in both overlays, removing only the unused externs,
and retain it only if all 21 functions preserve their complete words, calls
and physical referents. This corrects allocation class without claiming that
the current flattened COMMON placement contract is solved.

## Private OPEN CD workspace hypothesis

The two loaders are the only OPEN consumers of `cd_search_file` and
`cd_path_buffer`; GAME's error-screen path also consumes its corresponding
objects. Keep the shared loader/API types but move those GAME externs to the
GAME-specific header, and probe private OPEN definitions. Their low-BSS
addresses are contiguous (24-byte CdlFILE then 80-byte path), immediately
before the private audio sequence workspace. Check native fixed reservations
and all ten loader/resource functions before retaining the linkage change.

GAME CD's two initialized path fragments occupy the same late region as the
already calibrated resource filename and renderer error path. Probe G8 for
that existing owner: the exported CdlLOC must stay XBSS, while only the two
initialized fragments should move to `.sdata`. Preserve all three functions
and the two shared SDK work objects; their cross-module consumers prohibit
private linkage in GAME's present contiguous module model.


## Remaining small initializer controls

GAME's frame-pacer counters (two four-byte objects at `80057b0c/10`) and
voice-slot index (four bytes at `80057b84`) lie in the same established late
initialized region as the retained paths and sound selector. Probe each
owner's G8 profile separately. Native ASPSX must change only initialized
section ownership: exported game/audio state stays XBSS, the private sequence
workspace stays fixed BSS, and all nineteen complete functions must retain
their exact instructions and referents.

## Retained results and stopping evidence

Native GCC 2.5.7 / ASPSX 1.07 and the analysis adapter agree on all ten
retained initialized contributions, including every interior zero/padding
byte: GAME counters 8, CD paths 11, palettes/colour matrices 240, error path
7, HUD/notification/effect tables 336, frame light matrices 192, voice index
4; OPEN paths 20, renderer matrices 192, floor-item sprites 84 bytes. All
physical relocation targets remain unchanged. The four palette relocation
rows change only identity spelling, now with offsets 0/2/8/10 into the same
sixteen-byte table.

The GAME helper run is `8001f798..8001fde4`: three geometry/HUD functions
followed by four notification/buffer helpers, sharing the frame's sprite and
runtime state. It owns three distinct typed tables, not a fused byte array.
The frame owns its six light matrices; the renderer owns its private palette
pair and seven colour presets. OPEN keeps its colour and light identities
separate within renderer setup. These remain WIP module boundaries.

Native audio objects now reserve a fixed 344-byte private workspace and emit
an independent 144-byte XBSS runtime request in each overlay. OPEN CD now
reserves a fixed 104-byte contribution containing CdlFILE at +0 and the path
at +24, separately from its three eight-byte `.sbss` reservations and five
10,000-byte XBSS requests. C widths remain 24/80/4/4/4. The corresponding
GAME work objects remain exported because `display_show_error_screen`
names them outside the CD module.

The unchanged owners below also received native allocation audits. XBSS sizes
are allocation requests, never permission to enlarge C objects or insert
padding. All are still rejected by the existing data/placement checks where
the contract is unresolved.

| Original failing owner | Final verdict and evidence limit |
| --- | --- |
| GAME `game` | Corrected both counters to `.sdata`; exported four-byte `game_exit_code` requests eight bytes. Main and player update share it; original COMMON placement remains unproved. |
| GAME `equipment` | Native independent XBSS requests are 704-byte weapon records and 1176-byte armour records. Retail leaves 120 bytes between them. Shared item/menu consumers require external definitions; no evidence permits filling the gap or joining unlike tables. |
| GAME `player_death` | Sound bytes/RODATA remain exact. Native requests are fog 8, colour MATRIX 32, item stock 240. Fade helpers outside this run consume saved state; item/menu/resource code shares stock. Their low-small/low-fixed/later retail regions cannot be one flat contribution. |
| GAME `player_core` | Rotation 8, position 16 and growth table 480 are separate native requests; runtime 224 is distant in retail. Initializers, resources and player helpers share them. Their adjacency alone does not establish an enclosing object or private ownership. |
| GAME `cd_file` | Corrected eleven initialized path bytes to `.sdata`. Native requests remain location 8, search result 24, path 80, file table 1600. Error-screen code needs the first three; no supported contiguous private owner exists here. |
| GAME `resources` | Initialized sections remain exact. Six independent native requests: cell windows 3264 and five grids of 10000. Renderer/map/collision consumers share them, and retail includes gaps and two allocation regions. Existing array extents are proved; flat ordering is not. |
| GAME `render` | Corrected palette/colour ownership and error-path `.sdata`; all initialized bytes exact. Native runtime request is 149968 for a 149964-byte C object, plus an independent eight-byte request for four-byte `DAT_800a0768`. Main and rendering consumers share both. |
| GAME `render_frame` | **Strict data closure.** Its complete 192-byte light-matrix contribution now has one physical base; the sprite/palette definitions have evidence-backed owners. |
| GAME `save_system` | Initialized sections and four private event reservations remain exact. Each exported save-buffer pointer is four C bytes but requests eight; menu and save code share them. Twelve-byte target versus sixteen-byte candidate is still rejected. |
| GAME `map_object` | Initialized sound table exact. RODATA relocation +0 targets `.text+78c` in retail versus +788 in source because the effect constructor remains four bytes short. No source table constant can repair that code-derived destination. |
| GAME `audio` | Corrected private sequence workspace and initialized voice-index `.sdata`; shared runtime remains a separate XBSS request. Flat BSS placement remains unresolved. |
| GAME `map_scripts` | All 120 initialized bytes exact. RODATA relocation +28 targets `.text+1524` versus +1528 because the preceding pathname function is four bytes long. The later interaction function has its own twelve-byte-short residue. |
| GAME `pad` | SDK control: both private eight-byte `.sbss` reservations exact; `PadIdentifier` remains an exported four-byte int with an eight-byte native XBSS request. Changing the SDK ABI or claiming the provider revision as retail is unsupported. |
| OPEN `opening_helpers` | `opening_input_action` is a four-byte C value with an eight-byte XBSS request; controller/scenes consume it across modules. Original allocation placement is unresolved. |
| OPEN `camera_path` | Complete 100-byte shared camera state requests 104 native bytes. Scene functions name it externally; the next arena address does not establish a larger C structure. |
| OPEN `opening_scenes` | Early/late initialized boundary remains unresolved; the two native G8 controls below fail to explain it. |
| OPEN `resources` | Corrected path `.sdata`, three private `.sbss` reservations and private CD workspaces. Five exported grid requests still cannot be placed as one flat BSS contribution. |
| OPEN `render_init` | Complete 192-byte preset contribution exact. Runtime 149384 and a four-byte primitive counter (request eight) remain separate XBSS objects, shared with renderer traversal. |
| OPEN `entity_render` | **Strict data closure.** Seven sprite records occupy one complete 84-byte contribution; the light preset is owned by setup. RODATA remains exact. |
| OPEN `audio` | Corrected private 344-byte workspace; shared 144-byte runtime remains XBSS. Their widely separated retail addresses still reject flat BSS placement. |
| OPEN `format` | Native private reservation is 24 bytes for the minimum accessed 19-byte C span. The claim starts at odd `80037971`, seven bytes before digit anchor `80037978`; original outer bounds remain unknown. No aligned base/extent can be inferred merely from allocation rounding. |
| OPEN `pad` | Same independently checked SDK storage classes and unresolved exported identifier as GAME; complete text and all non-debug relocation records are unchanged. |

OPEN scene G8 moves the early three-byte `opening_scene0_sound` into small
data, contradicting its position immediately after the early camera paths
at `80035874`. Leaving the late sixteen-byte background RECT array intact
also keeps it in ordinary data. Splitting that pair into two real RECT
objects moves it to small data, but still wrongly moves the early sound.
Native section sizes are `.data/.sdata` 1000/48 for G8 and 984/64 for split
RECTs; neither reproduces the 988-byte early and 60-byte late contributions.
All seven functions preserve their baseline instructions (six exact, scroll
still partial). Reject both controls. Enlarging the three-byte SoundRef
would overlap the next rectangle at `80035878`; section forcing or an invented
wrapper/TU would have no independent support.

The formatter homolog has a useful but insufficient invariant: each image's
digit anchor lies 24 bytes after its sequence workspace ends. Both helpers
write backwards for padding. Neither clear/sizeof nor a complete enclosing
object boundary establishes the allocation start; the GAME anchor's own
24-byte inventory row also records unresolved outer bounds. Keep that limit
explicit rather than converting the common distance into a fabricated array.

The remaining instruction-related data failures reproduce earlier controlled
residues. `map_object_spawn_effect` is 400/404 source/retail bytes, first
differing at +4 (`sw ra,32(sp)` versus `sw ra,36(sp)`), with five identical
calls and the same sequence-object referents. Acquisition/publication helpers,
post-sequence initialization scopes and SDK-vector controls already failed
in [game-map-object-sequence-inline.md](game-map-object-sequence-inline.md)
and [game-era-source-experiment.md](game-era-source-experiment.md).
`map_show_screen_image` is 148/144 bytes, first differing at +30 in the
remainder destination (`a2` versus `v1`); the address/floor-load schedule
leaves an extra load-delay nop. Direct paths, cursor/helper boundaries and
decimal-expression controls are recorded in
[game-image-path-components.md](game-image-path-components.md).
`map_interaction_dispatch` stays 2296/2308 bytes with matching call/referent
sets and unexplained reload/branch joins; its prior bounded controls are in
[game-low-trial-campaign.md](game-low-trial-campaign.md). These are
unattributed code-generation residues, not proved compiler mechanisms.

## Per-function final ledger

Every original failed owner's function has a six-view evidence snapshot. All
178 retain their initial strict scores and emitted sizes. This table includes
the SDK PAD controls; they do not count as game reconstruction progress.
`100` means strict objdiff exact. A lower number is a retained partial result.
The ten functions in the additionally affected GAME geometry/notification
and lighting owners also preserve their complete raw retail bodies.

| Image / original owner | Address | Function | Retail/source bytes | Strict % |
| --- | --- | --- | ---: | ---: |
| game.game | `800146b8` | `game_main_loop` | 740/740 | 100.000000 |
| game.game | `8001499c` | `game_shutdown` | 56/56 | 100.000000 |
| game.game | `800149d4` | `frame_pacer_vsync_callback` | 32/32 | 100.000000 |
| game.game | `800149f4` | `frame_pacer_wait` | 112/112 | 100.000000 |
| game.equipment | `800150a8` | `weapon_records_load_and_mirror_angles` | 84/84 | 100.000000 |
| game.equipment | `800150fc` | `armor_records_load` | 44/44 | 100.000000 |
| game.equipment | `80015128` | `fixed6_ratio_step` | 60/60 | 100.000000 |
| game.player_death | `80015164` | `player_death_begin` | 104/104 | 100.000000 |
| game.player_death | `800151cc` | `game_state_initialize` | 740/740 | 100.000000 |
| game.player_death | `800154b0` | `player_death_restart` | 412/412 | 100.000000 |
| game.player_death | `8001564c` | `player_adjust_hp` | 112/112 | 100.000000 |
| game.player_death | `800156bc` | `player_adjust_mp` | 88/88 | 100.000000 |
| game.player_death | `80015714` | `player_recalculate_combat_stats` | 2068/2068 | 100.000000 |
| game.player_death | `80015f28` | `player_increment_physical_power_training` | 152/152 | 100.000000 |
| game.player_death | `80015fc0` | `player_increment_magic_training` | 152/152 | 100.000000 |
| game.player_death | `80016058` | `player_add_experience` | 548/548 | 100.000000 |
| game.player_death | `8001627c` | `player_calculate_damage_component` | 168/168 | 100.000000 |
| game.player_death | `80016324` | `player_apply_damage` | 912/912 | 100.000000 |
| game.player_death | `800166b4` | `player_apply_radial_damage` | 304/304 | 100.000000 |
| game.player_death | `800167e4` | `player_select_magic` | 100/100 | 100.000000 |
| game.player_core | `80016848` | `player_set_equipment_slot` | 488/488 | 100.000000 |
| game.player_core | `80016a30` | `player_equip_weapon` | 244/244 | 100.000000 |
| game.player_core | `80016b24` | `player_begin_weapon_attack` | 156/156 | 100.000000 |
| game.player_core | `80016bc0` | `player_update_weapon_attack` | 612/612 | 100.000000 |
| game.player_core | `80016e24` | `game_initialize_session` | 148/148 | 100.000000 |
| game.player_core | `80016eb8` | `player_clear_motion` | 48/48 | 100.000000 |
| game.player_core | `80016ee8` | `player_sync_position_to_map` | 344/344 | 100.000000 |
| game.player_core | `80017040` | `player_distance_to_point_in_cone` | 200/200 | 100.000000 |
| game.player_core | `80017108` | `player_distance_to_point` | 244/244 | 100.000000 |
| game.player_core | `800171fc` | `player_move_horizontal` | 2088/2128 | 96.568960 |
| game.player_core | `80017a24` | `player_update_view_bob` | 92/92 | 100.000000 |
| game.player_core | `80017a80` | `player_update_vertical_motion` | 632/632 | 100.000000 |
| game.player_core | `80017cf8` | `player_warp_to_floor_entry` | 324/324 | 100.000000 |
| game.player_core | `80017e3c` | `player_update_transform_snapshot` | 160/160 | 100.000000 |
| game.cd_file | `8001acf0` | `cd_file_load_allocated` | 368/368 | 100.000000 |
| game.cd_file | `8001ae60` | `cd_file_load_table_entry` | 316/316 | 100.000000 |
| game.cd_file | `8001af9c` | `cd_file_load_into` | 356/356 | 100.000000 |
| game.resources | `8001b100` | `tim_upload_images` | 128/128 | 100.000000 |
| game.resources | `8001b180` | `common_resources_load` | 528/528 | 100.000000 |
| game.resources | `8001b390` | `map_resource_path_set_floor` | 20/20 | 100.000000 |
| game.resources | `8001b3a4` | `map_resource_load_file` | 64/64 | 100.000000 |
| game.resources | `8001b3e4` | `map_resource_copy_words` | 48/48 | 100.000000 |
| game.resources | `8001b414` | `map_variant_assets_load` | 136/136 | 100.000000 |
| game.resources | `8001b49c` | `audio_play_current_map_sequence` | 188/188 | 100.000000 |
| game.resources | `8001b558` | `map_resources_load` | 600/600 | 100.000000 |
| game.render | `8001b7b0` | `display_show_error_screen` | 776/776 | 100.000000 |
| game.render | `8001bab8` | `lighting_set_active_color_matrix` | 44/44 | 100.000000 |
| game.render | `8001bae4` | `effect5_texture_cache_prepare` | 176/176 | 100.000000 |
| game.render | `8001bb94` | `display_initialize` | 332/332 | 100.000000 |
| game.render | `8001bce0` | `render_initialize` | 728/728 | 100.000000 |
| game.render | `8001bfb8` | `display_begin_frame` | 152/152 | 100.000000 |
| game.render | `8001c050` | `display_present_frame` | 152/152 | 100.000000 |
| game.render | `8001c0e8` | `tmd_select` | 44/44 | 100.000000 |
| game.render | `8001c114` | `tmd_get_object` | 36/36 | 100.000000 |
| game.render | `8001c138` | `tmd_set_current_vertices` | 16/16 | 100.000000 |
| game.render | `8001c148` | `tmd_select_object_vertices` | 60/60 | 100.000000 |
| game.render | `8001c184` | `render_set_view_transform` | 300/300 | 100.000000 |
| game.render | `8001c2b0` | `tmd_prepare_primitive_indices` | 768/768 | 100.000000 |
| game.render | `8001c5b0` | `tmd_register` | 60/60 | 100.000000 |
| game.render | `8001c5ec` | `tmd_release_last_allocation` | 32/32 | 100.000000 |
| game.render | `8001c60c` | `tmd_project_vertices` | 156/156 | 100.000000 |
| game.render | `8001c6a8` | `tmd_project_vertices_shift` | 172/172 | 100.000000 |
| game.render | `8001c754` | `tmd_transform_vertices` | 164/164 | 100.000000 |
| game.render_frame | `8001fde4` | `render_frame` | 1304/1304 | 100.000000 |
| game.save_system | `8002b078` | `save_system_read_catalog` | 216/216 | 100.000000 |
| game.save_system | `8002b150` | `menu_play_input_sound` | 132/132 | 100.000000 |
| game.save_system | `8002b1d4` | `memory_card_initialize` | 256/256 | 100.000000 |
| game.save_system | `8002b2d4` | `memory_card_shutdown_events` | 96/96 | 100.000000 |
| game.save_system | `8002b334` | `memory_card_begin_status_check` | 56/56 | 100.000000 |
| game.save_system | `8002b36c` | `memory_card_clear_events` | 88/88 | 100.000000 |
| game.save_system | `8002b3c4` | `memory_card_wait_event` | 188/188 | 100.000000 |
| game.save_system | `8002b480` | `memory_card_undeliver_events` | 88/88 | 100.000000 |
| game.save_system | `8002b4d8` | `memory_card_check_or_format` | 248/248 | 100.000000 |
| game.save_system | `8002b5d0` | `memory_card_format` | 120/120 | 100.000000 |
| game.save_system | `8002b648` | `save_system_write_slot` | 244/244 | 100.000000 |
| game.save_system | `8002b73c` | `save_file_write_slot` | 1268/1268 | 100.000000 |
| game.save_system | `8002bc30` | `save_system_read_header` | 216/216 | 100.000000 |
| game.save_system | `8002bd08` | `save_file_read_header` | 220/220 | 100.000000 |
| game.save_system | `8002bde4` | `save_system_read_slot` | 204/204 | 100.000000 |
| game.save_system | `8002beb0` | `save_file_read_slot` | 972/972 | 100.000000 |
| game.save_system | `8002c27c` | `save_workspace_allocate` | 104/104 | 100.000000 |
| game.save_system | `8002c2e4` | `save_workspace_release` | 32/32 | 100.000000 |
| game.save_system | `8002c304` | `save_file_initialize_buffers` | 524/524 | 100.000000 |
| game.save_system | `8002c510` | `memory_card_show_status_message` | 208/208 | 100.000000 |
| game.save_system | `8002c5e0` | `menu_load_message_image` | 300/300 | 100.000000 |
| game.save_system | `8002c70c` | `save_file_cleanup_temporary` | 136/136 | 100.000000 |
| game.save_system | `8002c794` | `screen_show_image_until_input` | 576/576 | 100.000000 |
| game.save_system | `8002c9d4` | `talk_show_dialogue_page` | 164/164 | 98.780490 |
| game.map_object | `800315c4` | `map_object_pool_find_interaction_from` | 448/448 | 100.000000 |
| game.map_object | `80031784` | `map_object_start_action_if_idle` | 32/32 | 100.000000 |
| game.map_object | `800317a4` | `map_object_effect_pool_acquire` | 144/144 | 100.000000 |
| game.map_object | `80031834` | `map_object_spawn_effect` | 404/400 | 94.504950 |
| game.map_object | `800319c8` | `map_object_spawn_actor_debris` | 396/396 | 100.000000 |
| game.map_object | `80031b54` | `map_object_pool_trigger_link` | 240/240 | 100.000000 |
| game.map_object | `80031c44` | `map_object_pool_clear_link` | 132/132 | 100.000000 |
| game.map_object | `80031cc8` | `map_object_pool_update` | 3096/3096 | 100.000000 |
| game.audio | `800328e0` | `audio_initialize` | 164/164 | 100.000000 |
| game.audio | `80032984` | `audio_load_vab` | 200/200 | 100.000000 |
| game.audio | `80032a4c` | `audio_play_map_sequence` | 272/272 | 100.000000 |
| game.audio | `80032b5c` | `audio_stop_sequence_fade` | 128/128 | 100.000000 |
| game.audio | `80032bdc` | `audio_stop_sequence_master_fade` | 156/156 | 100.000000 |
| game.audio | `80032c78` | `audio_shutdown` | 56/56 | 100.000000 |
| game.audio | `80032cb0` | `audio_close_vab` | 64/64 | 100.000000 |
| game.audio | `80032cf0` | `audio_play_spatial` | 712/712 | 100.000000 |
| game.audio | `80032fb8` | `audio_play_spatial_default_range` | 48/48 | 100.000000 |
| game.audio | `80032fe8` | `audio_play_spatial_range` | 44/44 | 100.000000 |
| game.audio | `80033014` | `sound_ref_key_off_bank0` | 40/40 | 100.000000 |
| game.audio | `8003303c` | `audio_set_listener_transform` | 112/112 | 100.000000 |
| game.audio | `800330ac` | `sound_ref_play` | 72/72 | 100.000000 |
| game.audio | `800330f4` | `audio_play_voice` | 424/424 | 100.000000 |
| game.audio | `8003329c` | `angle_shortest_delta` | 72/72 | 100.000000 |
| game.map_scripts | `80033ee4` | `actor_pool_find_at_tile` | 128/128 | 100.000000 |
| game.map_scripts | `80033f64` | `map_ambient_script_floor1` | 648/648 | 100.000000 |
| game.map_scripts | `800341ec` | `map_ambient_script_floor2` | 112/112 | 100.000000 |
| game.map_scripts | `8003425c` | `map_ambient_script_floor3` | 136/136 | 100.000000 |
| game.map_scripts | `800342e4` | `map_ambient_script_floor4` | 8/8 | 100.000000 |
| game.map_scripts | `800342ec` | `map_ambient_script_floor5` | 244/244 | 100.000000 |
| game.map_scripts | `800343e0` | `map_action_script_floor1` | 88/88 | 100.000000 |
| game.map_scripts | `80034438` | `map_reveal_fade` | 388/388 | 100.000000 |
| game.map_scripts | `800345bc` | `map_action_script_floor2` | 84/84 | 100.000000 |
| game.map_scripts | `80034610` | `map_action_script_floor3` | 144/144 | 100.000000 |
| game.map_scripts | `800346a0` | `map_action_script_floor4` | 8/8 | 100.000000 |
| game.map_scripts | `800346a8` | `map_floor5_transition_cutscene` | 908/908 | 100.000000 |
| game.map_scripts | `80034a34` | `map_action_script_floor5` | 76/76 | 100.000000 |
| game.map_scripts | `80034a80` | `map_event_interact` | 724/724 | 100.000000 |
| game.map_scripts | `80034d54` | `map_show_screen_image` | 144/148 | 88.888885 |
| game.map_scripts | `80034de4` | `map_interaction_dispatch` | 2308/2296 | 99.202774 |
| game.pad | `800500b8` | `PadInit` | 116/116 | 100.000000 |
| game.pad | `8005012c` | `PadRead` | 68/68 | 100.000000 |
| game.pad | `80050170` | `PadStop` | 60/60 | 100.000000 |
| game.pad | `800501ac` | `pad_init_bad_identifier` | 48/48 | 100.000000 |
| game.pad | `800501dc` | `pad_read_bad_identifier` | 48/48 | 100.000000 |
| game.pad | `8005020c` | `pad_stop_bad_identifier` | 48/48 | 100.000000 |
| open.opening_helpers | `80013c70` | `opening_poll_input` | 60/60 | 100.000000 |
| open.opening_helpers | `80013cac` | `angle_shortest_delta` | 72/72 | 100.000000 |
| open.camera_path | `80013cf4` | `opening_camera_path_compute_segment` | 784/784 | 100.000000 |
| open.camera_path | `80014004` | `opening_camera_path_begin` | 252/252 | 100.000000 |
| open.camera_path | `80014100` | `opening_camera_path_step` | 360/360 | 100.000000 |
| open.opening_scenes | `80014268` | `opening_scene0_run` | 372/372 | 100.000000 |
| open.opening_scenes | `800143dc` | `opening_scene1_draw_fade` | 384/384 | 100.000000 |
| open.opening_scenes | `8001455c` | `opening_scene1_run` | 172/172 | 100.000000 |
| open.opening_scenes | `80014608` | `opening_entity_transition` | 508/508 | 100.000000 |
| open.opening_scenes | `80014804` | `opening_scene3_run` | 816/816 | 100.000000 |
| open.opening_scenes | `80014b34` | `opening_ending_scene_run` | 756/756 | 100.000000 |
| open.opening_scenes | `80014e28` | `opening_ending_scroll_run` | 1944/1944 | 99.917694 |
| open.resources | `80016014` | `cd_file_load_allocated` | 328/328 | 100.000000 |
| open.resources | `8001615c` | `cd_file_load_into` | 316/316 | 100.000000 |
| open.resources | `80016298` | `tim_upload_images` | 128/128 | 100.000000 |
| open.resources | `80016318` | `resource_stream_copy_words` | 48/48 | 100.000000 |
| open.resources | `80016348` | `opening_resources_load_scene0` | 456/456 | 100.000000 |
| open.resources | `80016510` | `opening_resources_load_scene1` | 180/180 | 100.000000 |
| open.resources | `800165c4` | `opening_resources_load_scene3` | 240/240 | 100.000000 |
| open.resources | `800166b4` | `opening_resources_load_ending` | 308/308 | 100.000000 |
| open.resources | `800167e8` | `opening_resources_load_ending_entities` | 88/88 | 100.000000 |
| open.resources | `80016840` | `opening_resources_load_ending_sequence` | 156/156 | 100.000000 |
| open.render_init | `800168dc` | `lighting_set_active_color_matrix` | 44/44 | 100.000000 |
| open.render_init | `80016908` | `render_initialize` | 468/468 | 100.000000 |
| open.render_init | `80016adc` | `display_initialize` | 472/472 | 100.000000 |
| open.render_init | `80016cb4` | `primitive_buffer_allocate` | 132/132 | 100.000000 |
| open.entity_render | `80018ecc` | `opening_entity_render` | 552/552 | 100.000000 |
| open.entity_render | `800190f4` | `render_floor_item` | 332/332 | 100.000000 |
| open.entity_render | `80019240` | `opening_render_entities_and_items` | 664/664 | 100.000000 |
| open.audio | `80019ba4` | `audio_initialize` | 184/184 | 100.000000 |
| open.audio | `80019c5c` | `audio_load_vab` | 244/244 | 100.000000 |
| open.audio | `80019d50` | `audio_play_sequence_file` | 212/212 | 100.000000 |
| open.audio | `80019e24` | `audio_stop_sequence` | 156/156 | 100.000000 |
| open.audio | `80019ec0` | `audio_shutdown` | 56/56 | 100.000000 |
| open.audio | `80019ef8` | `audio_close_vab` | 76/76 | 100.000000 |
| open.format | `8001a3fc` | `format_int_dec` | 224/224 | 100.000000 |
| open.format | `8001a4dc` | `format_int_hex` | 140/140 | 100.000000 |
| open.format | `8001a568` | `format_pad_left` | 108/108 | 100.000000 |
| open.format | `8001a5d4` | `format_vsprintf` | 576/576 | 100.000000 |
| open.pad | `8002fe8c` | `PadInit` | 116/116 | 100.000000 |
| open.pad | `8002ff00` | `PadRead` | 68/68 | 100.000000 |
| open.pad | `8002ff44` | `PadStop` | 60/60 | 100.000000 |
| open.pad | `8002ff80` | `pad_init_bad_identifier` | 48/48 | 100.000000 |
| open.pad | `8002ffb0` | `pad_read_bad_identifier` | 48/48 | 100.000000 |
| open.pad | `8002ffe0` | `pad_stop_bad_identifier` | 48/48 | 100.000000 |

Additional affected owners, independently recompiled and raw-retail exact:

| Final owner | Address | Function | Retail/source bytes | Strict % |
| --- | --- | --- | ---: | ---: |
| game.geometry_render | `8001f798` | `render_weapon` | 280/280 | 100.000000 |
| game.geometry_render | `8001f8b0` | `render_effect_sprites` | 292/292 | 100.000000 |
| game.geometry_render | `8001f9d4` | `render_hud_gauges` | 112/112 | 100.000000 |
| game.geometry_render | `8001fa44` | `notify_enqueue` | 160/160 | 100.000000 |
| game.geometry_render | `8001fae4` | `notification_digit_set_v` | 24/24 | 100.000000 |
| game.geometry_render | `8001fafc` | `notify_effect_update` | 716/716 | 100.000000 |
| game.geometry_render | `8001fdc8` | `display_flip_buffer_index` | 28/28 | 100.000000 |
| game.lighting | `80033d80` | `lighting_transition_color_matrix` | 104/104 | 100.000000 |
| game.lighting | `80033de8` | `color_matrix_set_rgb` | 40/40 | 100.000000 |
| game.lighting | `80033e10` | `player_restore_vitals_with_color_cycle` | 212/212 | 100.000000 |

## Verification and closure accounting

The final full build retains GAME 345/362, OPEN 107/108 and PSX 1/1 exact game
functions, plus all thirteen exact vendored-source controls. Every one of the
484 report rows retains its original strict score and size. Independent final
compiles of the eleven changed source/profile families preserve all 74
complete function bodies, calls and physical referents at raw retail exact.
Only those 74 existing exact ledger entries are refreshed with `kf bank`;
no partial, new or vendored function is banked.

The original failed-owner count falls **22 to 20**. GAME now has 29/41 and
OPEN 12/20 strict passing data owners. The pass count stays 41 while the live
owner denominator falls 63 to 61: merging the two passing geometry/notification
owners removes one row, and moving the lighting-only data to renderer setup
removes another. The two actual closures are GAME `render_frame` and OPEN
`entity_render`; neither the consolidation nor a partial initialized-section
repair is counted as a closure. PSX remains 1/1. All source data comparisons
have zero artifact failures.

Full `kf build` exits nonzero on the remaining data, known-reference and
placement gates. Target relink is GAME 57/64, OPEN 34/38 and PSX 1/1; failing
section-base contracts remain rejected. No section scattering, forced size,
COMMON acceptance, relocation masking or relaxed exact threshold is added.
The first rebuild required `--reconfigure` after removal of the consolidated
source, then rebuilt the affected production objects; a final full build
also completed after banking.

Verification passes: all 771 repository tests (nine optional skips), the 127
focused inventory/storage/layout tests, `ruff check scripts tests`, and
`git diff --check`. Original ASPSX full initialized payloads agree with the
retail bytes and adapter objects. Both PAD text payloads and all non-debug
relocation records reproduce their starting objects. Native objects, complete
six-view dossiers, temporary source controls and full function/data report
snapshots stay under ignored `build/data-exhaust/`.

Every original failed owner has a final verdict above. No remaining supported
source correction survived the reviewed controls; the remaining twenty need
new allocation/ownership/extent evidence or an explanation of the recorded
instruction differences. This exhausts the current evidence-backed repair
campaign, while keeping those failures visible.

## Integration with current master

The integration retains master's later menu UV helpers, typed glyph workspace,
explicit row-position accesses and source cleanup within the consolidated
menu/notification modules. Audio workspaces retain master's SDK-compatible
`char` element type and this campaign's private linkage. All 484 function
scores and sizes reproduce master `66af94a4`: GAME 348/362, OPEN 107/108 and
PSX 1/1 exact, plus thirteen vendor-source controls. Data remains GAME 29/41,
OPEN 12/20 and PSX 1/1; the original twenty remaining owner failures stay visible.

The executable-score README table now separately reports **Data modules exact**,
using the complete strict data gate. It refreshes during build/check/bank and
link updates. Freshness comes from a read-only Ninja dry run, which correctly
recognizes rebuilds whose identical output files retain old timestamps. Missing
artifacts and stale inputs show unavailable rather than an invented zero or pass.
The native EXE similarity columns remain independent of these source-object counts.

Integration checks pass: 775 repository tests (nine optional skips), twelve
focused README/OPEN-layout controls, Ruff, diff checks and `nix flake check -L`.
The isolated flake also runs all 775 tests, with 140 environment/retail skips.
The OPEN layout control now creates its own target-name registry, so that
negative fixture works without a pre-existing retail build. Full builds retain
zero comparison-artifact failures and the existing data/reference/placement
gates. Only the 108 strict-exact functions in the affected owner families are
selected for the integration baseline refresh.
