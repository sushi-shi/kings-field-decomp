# Complete Sony helper candidate campaign

All **147 original sites** now use supplied SDK helpers: **75 vector triplets,
26 XY quads, 26 UV quads and 20 RGB triplets**, replacing 701 explicit component
assignments in 55 claimed function contexts across 30 source units. The initial
15-site pilot was integrated as `ad214ccb`; this follow-up tested the other
132 sites individually. All were retained. Twenty additional rectangle-form
trials also retained identical emitted objects.

One trial closes **GAME `display_initialize`, `8001bb94`, 332 retail bytes**:
**98.421684% → 100%**, reducing the candidate from 336 to 332 bytes. Its 29
following switch-table addends now match retail too. Three projectile updates
also use the existing signed `SVECTOR` direction view instead of casts from
unsigned component fields, with identical instructions.

The [initial SDK investigation](sony-header-helpers.md) records header hashes,
revision identifiers, vendored negative controls and the original census.
Compatible emitted code supports these source forms; it does not establish
that the original author wrote each macro or prove a historical compiler.

## Function Match Plan and scope

The follow-up started from integrated master `ad214ccb`, retaining the original
`48132a4e` census as the immutable checklist. The announced plan was to inspect
field order, widths, ownership and retail references, compile one substitution
at a time, compare complete objects, and stop at any changed bytes for review.
The campaign includes non-exact functions, rather than filtering to banked code.
The table below supplies a separate evidence snapshot and verdict for every
claimed function; the final section accounts for every original site.

Before the trials, image-qualified `addr`, `disasm --blocks`, both `xref` forms,
`strings` and `match` queries captured all 55 contexts and their immediate
neighbors. The caller pass captured 271 proven call sites in 71 callers. Source
history and shared declarations were inspected. These are game-owned functions
calling the independently identified SDK bodies: adopting an SDK macro does
not reclassify the enclosing function as vendored.

The candidate screen's pointer-returning constructor belongs to
`effect_pool_construct`; the two unclaimed inline backdrop sites belong to
`menu_status_panel`. Neither an address without its image nor the lexical
scanner's guessed function name serves as identity.

Every trial uses its unit's pinned GCC 2.5.7 PSX rebuild, `-O2 -G0 -mcpu=r2000`
and maspsx ASPSX 1.07 profile. Types, call sets, function/data claims, constants,
SDK boundaries and ownership inventories are preserved. No compiler switches,
assembly or artificial storage were added.

## Source choices and controls

- `setVector` writes exactly vx/vy/vz in order, including the existing fixed-point
  expressions and their original destination widths. `copyVector` preserves
  component conversion, including VECTOR-to-SVECTOR narrowing; it is never
  substituted for a complete object copy that includes padding.
- `addVector` preserves the three compound additions. Sites S009–S011 select
  `effect->direction.vector` from the already declared union. Retail uses signed
  `lh` at record offsets `2c/2e/30`, word position loads/stores at `0c/10/14`,
  and additions. Examples are GAME `80038cec..80038d18` and
  `800392a8..800392d8`. The signed view removes nine redundant casts and does
  not alter the union's layout or unsigned state interpretations elsewhere.
- The rectangle pass uses `setXYWH`/`setUVWH` when shared origins and spans
  explain all corners, including zero origins and commuted sums. These are
  twenty separately compiled refinements of already tested explicit-corner
  forms, not a parameter permutation search. The screen's named XY bounds
  remain `setXY4`; its horizontal UV span is 255 while its XY span is 256.
- Seven independently reflected UV quads retain `setUV4`, including the inline
  status backdrop's conditional corners. No flipped texture was converted to
  a positive-width rectangle. Color helpers retain the code byte; vector
  helpers retain padding. All repeated pointer/origin expressions are free of
  side effects. The `rsin`/`rcos` calls in S046 each remain evaluated once, in
  their original assignment order.
- Capitalized `SetPoly*`, `SetSemiTrans`, `GetClut`, `GetTPage` and `AddPrim`
  remain the retail calls. No direct COP2 wrapper, packed color/code write,
  reordered triple or unrelated SDK-name resemblance is counted in the 147.

| Final SDK helper | Sites |
| --- | ---: |
| `addVector` | 14 |
| `copyVector` | 7 |
| `setRGB0` | 17 |
| `setRGB1` | 1 |
| `setRGB2` | 1 |
| `setRGB3` | 1 |
| `setUV4` | 7 |
| `setUVWH` | 19 |
| `setVector` | 54 |
| `setXY4` | 2 |
| `setXYWH` | 24 |

## New exact GAME initializer

S101 replaces only the second DRAWENV's adjacent r0/g0/b0 zero stores with
`setRGB0(&game_graphics_runtime.display_draw_environments[1], 0, 0, 0)`.
The first changed instruction is function `+a4`: the graphics DTD address is
retained in `s0`, matching retail, instead of `a0`. Consequently the
`PutDispEnv` delay slot at `+f4` uses `addiu a0,s0,162`; the fog-distance store
at `+124` is `sw v0,18538(s0)` and no longer needs an extra address reload.
S100, the first DRAWENV helper, then preserves this exact result.

The raw physical chain is:

- `s0 = 80090ed6 = DRAWENV[0] + 16` (all offsets here hexadecimal).
- `80090ed6 + a2 = 80090f78`, the first DISPENV.
- `80090ed6 + 486a = 80095740`, the fog-distance field.

The function keeps its 32-byte frame, 13 proven calls, constants and final
return delay slot. Its complete instruction/ordered-relocation listing and
strict objdiff score match retail. All 18 functions in `game.render` are now
100%. The other 17 instruction/relocation listings are unchanged.

Shrinking this earlier body by four bytes also subtracts four from all 29
`.rodata` R_MIPS_32 switch-label addends. The final 116-byte table payload and
ordered `.text` relocations equal the delinked target. This corrects the
addend residue without editing any table, label identity or relocation input.
The `.rodata` placement and BSS extent issues remain visible to the full gate.

Three existing test assertions required updating: the old partial initializer
now must equal retail, the switch table now must have matching payload and
relocations while still failing placement, and the screen brightness negative
control moves the SDK color helper inside the wrong conditional. These retain
full retail-byte and shifted-owner negative controls.

## Integration with concurrent master work

During the pass, master gained `bec4cb62` and `40cf67ca`. The former independently
closed OPEN display initialization using both RGB helpers and removal of
unnecessary DRAWENV pointer locals; see the
[OPEN source experiment](open-era-source-experiment.md). Our original S138/S139
trials were byte-neutral at 96.652540%, so that independent closure is not
credited to this campaign. Both helpers were retained when composing with
master, and OPEN stays 100% after adding S137's vector helper. The GAME goto
simplifications from `40cf67ca` were also preserved.

Thus 132 sites were newly trialled after the initial pilot; two reached master
through the independent OPEN work, leaving 130 additional uses in this commit.
All 147 are present in the combined source. Against current master, only GAME
display initialization changes emitted code. The combined census is PSX 1/1,
GAME 342/362, OPEN 107/108: **450/471 exact**, one new exact from this campaign.

## Per-function evidence snapshots and final verdicts

Addresses and retail body sizes are hexadecimal. Initial scores are from the
`ad214ccb` evidence pass; final scores include the master composition above.
Every percentage is strict objdiff. A repeated non-100 score means the helper
trials leave that body byte-identical, not that it is banked or closed.

| Image / VA / retail size | Function | Original sites | Initial → final strict % |
| --- | --- | --- | --- |
| GAME / `8002edd4` / `454` | `actor_spawn_action_effect` | S001, S002 | 100 → 100 |
| GAME / `800332e4` / `2dc` | `camera_path_compute_segment` | S003, S004 | 100 → 100 |
| GAME / `800335c0` / `c0` | `camera_path_begin` | S005, S006 | 100 → 100 |
| GAME / `80033680` / `11c` | `camera_path_step` | S007, S008 | 100 → 100 |
| GAME / `80038a38` / `180c` | `effect_update_dispatch` | S009, S010, S011, S012, S013, S014, S015 | 99.827810 → 99.827810 |
| GAME / `80036f44` / `82c` | `effect_pool_construct` | S016, S017, S018, S019, S020, S021, S022, S023, S024, S025, S026, S027, S028, S029 | 100 → 100 |
| GAME / `80037770` / `ac` | `effect_pool_spawn_typed` | S030 | 100 → 100 |
| GAME / `80037fe0` / `2b8` | `effect_projectile_update_3d` | S031 | 100 → 100 |
| GAME / `8003872c` / `90` | `effect_rotate_scale_offset_y` | S032, S033 | 100 → 100 |
| GAME / `8001e9a4` / `214` | `render_actor` | S034 | 100 → 100 |
| GAME / `8001ebb8` / `180` | `render_map_object` | S035 | 100 → 100 |
| GAME / `8001ed90` / `14c` | `render_floor_item` | S036 | 100 → 100 |
| GAME / `8001eedc` / `1e8` | `render_actor_sprite` | S037, S038 | 100 → 100 |
| GAME / `8003a2a0` / `4c0` | `magic_cast` | S039, S040, S041 | 100 → 100 |
| GAME / `8001f0c4` / `154` | `render_map_event` | S042 | 100 → 100 |
| GAME / `800315c4` / `1c0` | `map_object_pool_find_interaction_from` | S043, S044 | 100 → 100 |
| GAME / `80031834` / `194` | `map_object_spawn_effect` | S045 | 94.504950 → 94.504950 |
| GAME / `800319c8` / `18c` | `map_object_spawn_actor_debris` | S046 | 100 → 100 |
| GAME / `80014d34` / `d4` | `pitch_yaw_to_forward_vector` | S047, S048, S049 | 100 → 100 |
| GAME / `80014e48` / `5c` | `vector3s_scale_shift12` | S050 | 100 → 100 |
| GAME / `80028a70` / `77c` | `menu_list_render` | S051, S052, S053, S054, S055, S056 | 100 → 100 |
| GAME / `80022d7c` / `400` | `menu_map_viewer` | S057, S058, S059, S060, S061, S062 | 100 → 100 |
| GAME / `800292f8` / `7b8` | `menu_draw_item_name_frame` | S064, S065, S066, S067, S076, S077, S078, S079 | 99.570850 → 99.570850 |
| GAME / `80029ab0` / `1a0` | `menu_blit_sprite_translucent` | S080 | 100 → 100 |
| GAME / `80029c50` / `190` | `menu_blit_sprite` | S081 | 100 → 100 |
| GAME / `80029de0` / `530` | `menu_draw_string` | S068, S069, S070, S082, S083, S084 | 100 → 100 |
| GAME / `8002a310` / `200` | `menu_draw_number` | S071, S085 | 100 → 100 |
| GAME / `8002a510` / `6a4` | `menu_draw_window_backdrop` | S072, S073, S074, S075, S086, S087, S088, S089 | 99.971760 → 99.971760 |
| GAME / `8002accc` / `50` | `primitive_buffer_begin_poly_ft4` | S063 | 100 → 100 |
| GAME / `8002430c` / `69c` | `menu_status_panel` | S090, S091 | 99.962170 → 99.962170 |
| GAME / `80016bc0` / `264` | `player_update_weapon_attack` | S092, S093, S094 | 100 → 100 |
| GAME / `80016e24` / `94` | `game_initialize_session` | S095 | 100 → 100 |
| GAME / `80017e3c` / `a0` | `player_update_transform_snapshot` | S096 | 100 → 100 |
| GAME / `80018880` / `1a1c` | `player_update` | S097, S098 | 99.476960 → 99.476960 |
| GAME / `8001b7b0` / `308` | `display_show_error_screen` | S102, S103 | 100 → 100 |
| GAME / `8001bb94` / `14c` | `display_initialize` | S100, S101 | 98.421684 → 100 |
| GAME / `8001bce0` / `2d8` | `render_initialize` | S099 | 100 → 100 |
| GAME / `8001e480` / `16c` | `render_screen_sprite` | S104 | 100 → 100 |
| GAME / `8002c794` / `240` | `screen_show_image_until_input` | S105, S106, S107 | 100 → 100 |
| GAME / `80014314` / `1c0` | `sprite_add_ft4` | S108, S109, S110 | 100 → 100 |
| OPEN / `80013cf4` / `310` | `opening_camera_path_compute_segment` | S111, S112 | 100 → 100 |
| OPEN / `80014004` / `fc` | `opening_camera_path_begin` | S113, S114 | 100 → 100 |
| OPEN / `80014100` / `168` | `opening_camera_path_step` | S115, S116, S117, S118 | 100 → 100 |
| OPEN / `80018ecc` / `228` | `opening_entity_render` | S119, S120 | 100 → 100 |
| OPEN / `800190f4` / `14c` | `render_floor_item` | S121 | 100 → 100 |
| OPEN / `80015b0c` / `d4` | `pitch_yaw_to_forward_vector` | S122, S123, S124 | 100 → 100 |
| OPEN / `80013804` / `198` | `sprite_add_g4` | S125, S126, S127, S128, S129 | 100 → 100 |
| OPEN / `800143dc` / `180` | `opening_scene1_draw_fade` | S131, S132, S133, S134, S135, S136 | 100 → 100 |
| OPEN / `80014804` / `330` | `opening_scene3_run` | S130 | 100 → 100 |
| OPEN / `80016908` / `1d4` | `render_initialize` | S137 | 100 → 100 |
| OPEN / `80016adc` / `1d8` | `display_initialize` | S138, S139 | 96.652540 → 100 |
| OPEN / `80018344` / `2a4` | `render_enqueue_unlit_triangles` | S140, S141 | 100 → 100 |
| OPEN / `8001399c` / `1c0` | `sprite_add_ft4` | S142, S144, S146 | 100 → 100 |
| OPEN / `80013b5c` / `114` | `sprite_add_f4` | S143, S145 | 100 → 100 |
| OPEN / `80015be0` / `5c` | `vector3s_scale_shift12` | S147 | 100 → 100 |

The six remaining non-exact candidate functions preserve these first observed
residues; the helper trials do not establish compiler causes:

| Function | Retained residue |
| --- | --- |
| GAME `effect_update_dispatch` | First difference at `+38`: s3/s6 roles for the current magic record and effect kind are exchanged; the seven helper sites leave the whole object unchanged. |
| GAME `map_object_spawn_effect` | Saved-register placement differs from `+4`; the body remains 400 versus 404 retail bytes. S045 preserves its call/reference/control-flow model. |
| GAME `menu_draw_item_name_frame` | 160-byte frame versus retail 224, plus the existing item-name index instruction order; all eight rectangle sites remain neutral. |
| GAME `menu_draw_window_backdrop` | 40-byte frame versus retail 104; all eight rectangle sites preserve the complete body. |
| GAME `menu_status_panel` | 48-byte frame versus retail 112; both inline XY/UV sites preserve the body. |
| GAME `player_update` | 216-byte frame versus retail 224. Also retain the known representation/ownership issue: source references `.data+0x60` for `player_previous_input`, target names the independently claimed `80057b30` datum. Neither vector site repairs that data-section model. |

The three menu frames retain the same 64-byte difference after authentic
helpers; no dummy rectangle/packet local or padding is introduced to fill it.
The player relocation issue must be resolved as ownership/referent work before
attributing all of its score residue to code generation.

## Verification and reproduction

The initial per-site pass compiled 132 complete alternate units, stopping for
review at S101, the only changed object. The extra 20 rectangle trials were
all byte-identical. A separate audit replayed every approved substitution from
the saved starting sources and compared it to the final source tokens.

After composing master, all **170 functions in the 30 involved units** were
strict-scored again: **160 exact, ten unchanged partial siblings/targets**.
Among the 55 candidate function contexts, 49 are exact and six remain partial.
For 29 units, a fresh compile of master and the final built object agree in all
loadable bytes, sizes, alignments, symbol extents and ordered named relocations.
GAME render is the reviewed exception described above. Debug source paths and
line tables are excluded from the raw comparison.

`ruff check scripts tests` passes. All 112 source/image variants pass
`kf check-types`; the complete Python run passes 739 tests, and the focused
display/ownership run passes 22 tests. Offline Rust tests pass 96 tests with
five ignored corpus tests. `git diff --check` passes. Only C, documentation,
existing test controls and the selected match ledger change; no toolchain or
flake changes were made.

Full builds exit 1 on the existing data/ownership gates. The same 54 units
remain divergent: data matching is PSX 0/1, GAME 8/44, OPEN 3/20; target relink
is PSX 1/1, GAME 63/77, OPEN 32/38. Within GAME render, `.rodata` advances from
an addend mismatch to the still-unresolved placement diagnostic. The payload
improvement does not satisfy the whole-section gate, and no data closure is
claimed. All previously exact functions remain exact.

After staging only the campaign, `kf bank` selects 155 verified exact functions
from affected sources. It refreshes 154 existing exact records and promotes
GAME display initialization to 100%; the other ledger rows are unchanged.
Partial functions are not banked.

Local trial sources, objects, per-site decisions and disassembly dossiers are
under `build/sony-helpers/all-candidates/`; they are not committed.

Reproduce in `nix develop`, with validated retail inputs:

```sh
kf try --unit game.render
kf try --unit open.render_init
kf sema --image game match display_initialize
kf sema --image open match display_initialize
kf check-types
ruff check scripts tests
python -m unittest discover -s tests -v
cargo test --offline --manifest-path tools/Cargo.toml
kf build
```

For the decisive single-site control, start from `ad214ccb:src/game/render.c`,
replace only DRAWENV[1]'s r0/g0/b0 zero triplet with the S101 helper above, and
compile that alternate source with `kf try --unit game.render --source <path>`.
Compare strict objdiff, raw constants and relocation targets as well as the
first changed instruction. No inventory or profile edit is needed.

## Complete 147-site ledger

Locations are **original source line numbers at `48132a4e`**, recoverable with
`git show 48132a4e:src/<location>`. They do not move when earlier sites shrink.
Each row identifies the original destination object and final macro. “Same”
means the single-site trial preserved the previously accepted whole object;
“pilot” refers to the earlier verified 15-site commit. S138/S139 also arrived
on master independently, as explained above. Every row is retained.

| Site | Original location | Destination | Final helper | Trial |
| --- | --- | --- | --- | --- |
| S001 | `game/actor_behavior.c:525` | `offset` | `setVector` | Same |
| S002 | `game/actor_behavior.c:540` | `position` | `addVector` | Same |
| S003 | `game/camera_path.c:33` | `path->position_delta` | `setVector` | Same |
| S004 | `game/camera_path.c:40` | `path->rotation_delta` | `setVector` | Same |
| S005 | `game/camera_path.c:52` | `path->position_fixed` | `setVector` | Same |
| S006 | `game/camera_path.c:55` | `path->rotation_fixed` | `setVector` | Same |
| S007 | `game/camera_path.c:74` | `path->position_fixed` | `addVector` | Pilot |
| S008 | `game/camera_path.c:77` | `path->rotation_fixed` | `addVector` | Pilot |
| S009 | `game/effect_dispatch.c:226` | `effect->position` | `addVector` | Same |
| S010 | `game/effect_dispatch.c:378` | `effect->position` | `addVector` | Same |
| S011 | `game/effect_dispatch.c:424` | `effect->position` | `addVector` | Same |
| S012 | `game/effect_dispatch.c:542` | `local_motion` | `copyVector` | Same |
| S013 | `game/effect_dispatch.c:547` | `effect->position` | `addVector` | Same |
| S014 | `game/effect_dispatch.c:605` | `position` | `setVector` | Same |
| S015 | `game/effect_dispatch.c:749` | `position` | `setVector` | Same |
| S016 | `game/effect_pool.c:78` | `record->rotation.vector` | `setVector` | Same |
| S017 | `game/effect_pool.c:88` | `record->rotation.vector` | `setVector` | Same |
| S018 | `game/effect_pool.c:106` | `record->rotation.vector` | `setVector` | Same |
| S019 | `game/effect_pool.c:128` | `record->rotation.vector` | `setVector` | Same |
| S020 | `game/effect_pool.c:142` | `record->rotation.vector` | `setVector` | Same |
| S021 | `game/effect_pool.c:150` | `record->rotation.vector` | `setVector` | Same |
| S022 | `game/effect_pool.c:158` | `record->rotation.vector` | `setVector` | Same |
| S023 | `game/effect_pool.c:175` | `record->rotation.vector` | `setVector` | Same |
| S024 | `game/effect_pool.c:184` | `record->rotation.vector` | `setVector` | Same |
| S025 | `game/effect_pool.c:198` | `record->rotation.vector` | `setVector` | Same |
| S026 | `game/effect_pool.c:212` | `record->rotation.vector` | `setVector` | Same |
| S027 | `game/effect_pool.c:222` | `record->rotation.vector` | `setVector` | Same |
| S028 | `game/effect_pool.c:232` | `record->rotation.vector` | `setVector` | Same |
| S029 | `game/effect_pool.c:404` | `record->rotation.vector` | `setVector` | Same |
| S030 | `game/effect_pool.c:424` | `record->rotation.vector` | `setVector` | Same |
| S031 | `game/effect_update.c:67` | `world` | `addVector` | Same |
| S032 | `game/effect_update.c:243` | `scaled` | `setVector` | Same |
| S033 | `game/effect_update.c:246` | `rotation` | `setVector` | Same |
| S034 | `game/entity_model_render.c:35` | `screen` | `setVector` | Same |
| S035 | `game/entity_model_render.c:84` | `screen` | `setVector` | Same |
| S036 | `game/entity_render.c:75` | `screen` | `setVector` | Same |
| S037 | `game/entity_render.c:125` | `screen` | `setVector` | Same |
| S038 | `game/entity_render.c:130` | `scale` | `setVector` | Same |
| S039 | `game/magic.c:70` | `offset` | `setVector` | Same |
| S040 | `game/magic.c:78` | `world_pos` | `addVector` | Same |
| S041 | `game/magic.c:123` | `rotation` | `copyVector` | Same |
| S042 | `game/map_event_render.c:31` | `screen` | `setVector` | Same |
| S043 | `game/map_object.c:109` | `offset` | `setVector` | Same |
| S044 | `game/map_object.c:122` | `offset` | `setVector` | Same |
| S045 | `game/map_object.c:198` | `object->position` | `setVector` | Same |
| S046 | `game/map_object.c:232` | `object->position` | `setVector` | Same |
| S047 | `game/matrix_rotation.c:143` | `source` | `setVector` | Same |
| S048 | `game/matrix_rotation.c:148` | `source` | `copyVector` | Same |
| S049 | `game/matrix_rotation.c:153` | `direction` | `copyVector` | Same |
| S050 | `game/matrix_rotation.c:176` | `vector` | `setVector` | Same |
| S051 | `game/menu_list_render.c:79` | `current_poly_ft4` | `setXYWH` | Same |
| S052 | `game/menu_list_render.c:110` | `current_poly_ft4` | `setXYWH` | Same |
| S053 | `game/menu_list_render.c:136` | `current_poly_ft4` | `setXYWH` | Same |
| S054 | `game/menu_list_render.c:87` | `current_poly_ft4` | `setUVWH` | Same |
| S055 | `game/menu_list_render.c:118` | `current_poly_ft4` | `setUVWH` | Same |
| S056 | `game/menu_list_render.c:146` | `current_poly_ft4` | `setUVWH` | Same |
| S057 | `game/menu_map_viewer.c:51` | `poly_bg[0]` | `setRGB0` | Same |
| S058 | `game/menu_map_viewer.c:75` | `poly_marker[0]` | `setRGB0` | Same |
| S059 | `game/menu_map_viewer.c:64` | `poly_bg[0]` | `setXYWH` | Same |
| S060 | `game/menu_map_viewer.c:88` | `poly_marker[0]` | `setXYWH` | Same |
| S061 | `game/menu_map_viewer.c:56` | `poly_bg[0]` | `setUVWH` | Same |
| S062 | `game/menu_map_viewer.c:80` | `poly_marker[0]` | `setUVWH` | Same |
| S063 | `game/menu_runtime.c:556` | `current_poly_ft4` | `setRGB0` | Same |
| S064 | `game/menu_runtime.c:138` | `current_poly_ft4` | `setXYWH` | Same |
| S065 | `game/menu_runtime.c:160` | `current_poly_ft4` | `setXYWH` | Same |
| S066 | `game/menu_runtime.c:182` | `current_poly_ft4` | `setXYWH` | Same |
| S067 | `game/menu_runtime.c:204` | `current_poly_ft4` | `setXYWH` | Same |
| S068 | `game/menu_runtime.c:304` | `current_poly_ft4` | `setXYWH` | Same |
| S069 | `game/menu_runtime.c:327` | `current_poly_ft4` | `setXYWH` | Same |
| S070 | `game/menu_runtime.c:350` | `current_poly_ft4` | `setXYWH` | Same |
| S071 | `game/menu_runtime.c:390` | `current_poly_ft4` | `setXYWH` | Same |
| S072 | `game/menu_runtime.c:425` | `current_poly_ft4` | `setXYWH` | Same |
| S073 | `game/menu_runtime.c:447` | `current_poly_ft4` | `setXYWH` | Same |
| S074 | `game/menu_runtime.c:469` | `current_poly_ft4` | `setXYWH` | Same |
| S075 | `game/menu_runtime.c:491` | `current_poly_ft4` | `setXYWH` | Same |
| S076 | `game/menu_runtime.c:146` | `current_poly_ft4` | `setUVWH` | Same |
| S077 | `game/menu_runtime.c:168` | `current_poly_ft4` | `setUV4` | Same |
| S078 | `game/menu_runtime.c:190` | `current_poly_ft4` | `setUV4` | Same |
| S079 | `game/menu_runtime.c:212` | `current_poly_ft4` | `setUV4` | Same |
| S080 | `game/menu_runtime.c:249` | `current_poly_ft4` | `setUVWH` | Same |
| S081 | `game/menu_runtime.c:274` | `current_poly_ft4` | `setUVWH` | Same |
| S082 | `game/menu_runtime.c:313` | `current_poly_ft4` | `setUVWH` | Same |
| S083 | `game/menu_runtime.c:335` | `current_poly_ft4` | `setUVWH` | Same |
| S084 | `game/menu_runtime.c:358` | `current_poly_ft4` | `setUVWH` | Same |
| S085 | `game/menu_runtime.c:398` | `current_poly_ft4` | `setUVWH` | Same |
| S086 | `game/menu_runtime.c:433` | `current_poly_ft4` | `setUVWH` | Same |
| S087 | `game/menu_runtime.c:455` | `current_poly_ft4` | `setUV4` | Same |
| S088 | `game/menu_runtime.c:477` | `current_poly_ft4` | `setUV4` | Same |
| S089 | `game/menu_runtime.c:499` | `current_poly_ft4` | `setUV4` | Same |
| S090 | `game/menu_status_panel.c:18` | `current_poly_ft4` | `setXYWH` | Same |
| S091 | `game/menu_status_panel.c:26` | `current_poly_ft4` | `setUV4` | Same |
| S092 | `game/player_core.c:181` | `offset` | `setVector` | Same |
| S093 | `game/player_core.c:184` | `rotation` | `setVector` | Same |
| S094 | `game/player_core.c:189` | `result` | `addVector` | Same |
| S095 | `game/player_core.c:232` | `player_state.camera_position` | `setVector` | Same |
| S096 | `game/player_core.c:622` | `rotation_out` | `addVector` | Same |
| S097 | `game/player_update.c:430` | `spawn_offset` | `setVector` | Same |
| S098 | `game/player_update.c:438` | `position` | `addVector` | Same |
| S099 | `game/render.c:208` | `angles` | `setVector` | Same |
| S100 | `game/render.c:175` | `game_graphics_runtime.display_draw_environments[0]` | `setRGB0` | Same |
| S101 | `game/render.c:178` | `game_graphics_runtime.display_draw_environments[1]` | `setRGB0` | New exact |
| S102 | `game/render.c:62` | `prim` | `setXY4` | Same |
| S103 | `game/render.c:70` | `prim` | `setUVWH` | Same |
| S104 | `game/render_sprite.c:31` | `prim` | `setRGB0` | Same |
| S105 | `game/save_system.c:855` | `polygon` | `setRGB0` | Same |
| S106 | `game/save_system.c:822` | `polygon` | `setXY4` | Same |
| S107 | `game/save_system.c:830` | `polygon` | `setUVWH` | Same |
| S108 | `game/sprite_add_ft4.c:31` | `prim` | `setRGB0` | Pilot |
| S109 | `game/sprite_add_ft4.c:15` | `prim` | `setXYWH` | Pilot |
| S110 | `game/sprite_add_ft4.c:23` | `prim` | `setUVWH` | Pilot |
| S111 | `open/camera_path.c:33` | `opening_camera_path_state.position_delta` | `setVector` | Same |
| S112 | `open/camera_path.c:46` | `opening_camera_path_state.rotation_delta` | `setVector` | Same |
| S113 | `open/camera_path.c:61` | `opening_camera_path_state.position_fixed` | `setVector` | Same |
| S114 | `open/camera_path.c:67` | `opening_camera_path_state.rotation_fixed` | `setVector` | Same |
| S115 | `open/camera_path.c:89` | `opening_camera_path_state.position_fixed` | `addVector` | Same |
| S116 | `open/camera_path.c:95` | `opening_camera_path_state.rotation_fixed` | `addVector` | Same |
| S117 | `open/camera_path.c:101` | `opening_camera_path_state.position` | `setVector` | Same |
| S118 | `open/camera_path.c:107` | `opening_camera_path_state.rotation` | `setVector` | Same |
| S119 | `open/entity_render.c:50` | `screen` | `setVector` | Same |
| S120 | `open/entity_render.c:56` | `scale` | `copyVector` | Same |
| S121 | `open/entity_render.c:123` | `screen` | `setVector` | Same |
| S122 | `open/matrix_rotation.c:94` | `source` | `setVector` | Same |
| S123 | `open/matrix_rotation.c:99` | `source` | `copyVector` | Same |
| S124 | `open/matrix_rotation.c:104` | `direction` | `copyVector` | Same |
| S125 | `open/opening_render.c:35` | `prim` | `setRGB0` | Pilot |
| S126 | `open/opening_render.c:38` | `prim` | `setRGB1` | Pilot |
| S127 | `open/opening_render.c:41` | `prim` | `setRGB2` | Pilot |
| S128 | `open/opening_render.c:44` | `prim` | `setRGB3` | Pilot |
| S129 | `open/opening_render.c:27` | `prim` | `setXYWH` | Pilot |
| S130 | `open/opening_scenes.c:560` | `transition_position` | `setVector` | Same |
| S131 | `open/opening_scenes.c:328` | `left` | `setRGB0` | Same |
| S132 | `open/opening_scenes.c:331` | `right` | `setRGB0` | Same |
| S133 | `open/opening_scenes.c:292` | `left` | `setXYWH` | Same |
| S134 | `open/opening_scenes.c:301` | `right` | `setXYWH` | Same |
| S135 | `open/opening_scenes.c:310` | `left` | `setUVWH` | Same |
| S136 | `open/opening_scenes.c:319` | `right` | `setUVWH` | Same |
| S137 | `open/render_init.c:65` | `angles` | `setVector` | Same |
| S138 | `open/render_init.c:143` | `open_graphics_runtime.display_draw_environments[0]` | `setRGB0` | Same; upstream |
| S139 | `open/render_init.c:146` | `open_graphics_runtime.display_draw_environments[1]` | `setRGB0` | Same; upstream |
| S140 | `open/render_unlit.c:55` | `prim->sdk` | `setRGB0` | Same |
| S141 | `open/render_unlit.c:77` | `prim->sdk` | `setRGB0` | Same |
| S142 | `open/sprite_add_ft4.c:30` | `prim` | `setRGB0` | Pilot |
| S143 | `open/sprite_add_ft4.c:54` | `prim` | `setRGB0` | Pilot |
| S144 | `open/sprite_add_ft4.c:14` | `prim` | `setXYWH` | Pilot |
| S145 | `open/sprite_add_ft4.c:46` | `prim` | `setXYWH` | Pilot |
| S146 | `open/sprite_add_ft4.c:22` | `prim` | `setUVWH` | Pilot |
| S147 | `open/vector_math.c:11` | `vector` | `setVector` | Same |
