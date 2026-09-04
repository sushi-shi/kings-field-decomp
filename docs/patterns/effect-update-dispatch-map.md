# effect_update_dispatch (0x80038a38) reconstruction map

Evidence map for the first semantic reconstruction. This was the last
not-started GAME function (0x180c bytes). It is the per-kind effect
behavior dispatcher and belongs to the `game.effect_update` module
(`src/game/effect_update.c`) as its next contiguous claim after 0x800388b4.
The first pass now covers all 16 distinct handler bodies and owns the exact
49-entry jump table; instruction matching remains open.

Current first-pass verdict: the function is structurally started, not matched
or bankable (37.918777%). The first instruction divergence is the prologue:
retail allocates 0xa8 bytes and saves `s8` through `s0`, while the current probe
allocates 0xc8 bytes and saves `s5` through `s0`. The reconstructed body is
0x16f0 bytes versus retail's 0x180c. This is an open source-shape/codegen
residue, not an attributed compiler wall.

## Signature / register model
- `s4` = current effect record = `KfEffectRecord *` from `current_effect`.
- `s3`/`s1` = `current_effect_magic_record` (`KfMagicRecord *`).
- `s6` = `s4->kind` (`KfEffectRecord` +1). `s2` = `s4->unknown_07` (+7).
- Prologue: `sp,-168`; saves ra,s8,s7,s6,s5,s4,s3,s2,s1,s0.

## Dispatch
`switch (kind)` on `kind-4`, bounds `(unsigned)(kind-4) <= 0x30` else default.
Dense jump table `RODATA(0x80012cf8, 0xc4)` = 49 words (kinds 4..52). Our
gcc-2.5.7 should emit an equivalent table if the switch spans cases 4..52 with
`default` = return. Table (kind -> handler VA):

| kind | handler | | kind | handler |
|---|---|---|---|---|
| 4,5,7,8,10,11,12,14,22 | 0x80038ab4 (COMMON) | | 32 | 0x80039974 |
| 13 | 0x80038ab0 (common, s5=0x96 first) | | 33 | 0x80039a30 |
| 6 | 0x80039b58 | | 34 | 0x80039df0 |
| 9 | 0x80039e5c | | 36 | 0x80039250 |
| 15 | 0x8003a054 | | 52 | 0x8003a084 |
| 16 | 0x8003a06c | | 18 | 0x800395b0 |
| 17 | 0x8003a208 | | 19 | 0x800393b0 |
| 20 | 0x800396a4 | | 21,23-31,35,37-51 | 0x8003a214 (default/return) |

16 unique bodies. `s5` is preset before some cases (0x96 for kind 13; 0x64 at
entry; 0x96 at 0x80038ab0).

## Callee set (all named game funcs + libc)
effect_map_collision, effect_magic_power, effect_pool_construct,
effect_spawn_trail_kind13, effect_spawn_ground_kind6, effect_scatter_triple,
effect_projectile_update_3d/2d, effect_floor_deform_line, vector_xz_to_angle,
angle_approach, rsin, rcos, matrix_set_rotation_x/y, player_apply_damage,
player_apply_radial_damage, actor_apply_damage, actor_pool_apply_radial_damage,
actor_pool_spawn, actor_pool_find_target_in_cone, collision_query_world,
audio_play_spatial_range, audio_play_spatial_default_range, rand.

## Approach for the dedicated session
Add `ADDRESS(0x80038a38, 0x180c)` + `RODATA(0x80012cf8, 0xc4)` to
`effect_update.c`. Declare the ~25 callees (K&R where already used that way).
Reconstruct the COMMON body first (kinds 4/5/7/8/10-14/22 at 0x80038ab4), then
each distinct case in table order. Expect a scheduler residue per case.
Epilogue at 0x8003a214 restores s0..s8,ra and `addiu sp,sp,168` in the `jr ra`
delay slot.
