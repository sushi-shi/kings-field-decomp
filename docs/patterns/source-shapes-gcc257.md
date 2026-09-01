# Source shapes that decided exact matches under the 2.5.7 probe

Each row names the retail signature, the shape that reproduced it, and a
witness unit. These are observations about GCC 2.5.7 `-O2`, not rules for
other compilers.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `addiu c,c,-1; move ...; sll/sra 16; bne c,-1` | `s16 count = 8; do { ... } while (--count != -1);` | `matrix_interpolate` `0x800202fc` |
| `addiu c,c,-1; bnez c` | `s32 count = N; do { ... } while (--count != 0);` | `actor_definitions_load` `0x80030a6c` |
| `move v1,a1; ...; addiu a1,-1; bnez v1` | `while (count-- != 0)` with `s32 count` | `player_weapon_load_records_and_mirror_angles` `0x800150a8` (2.6.0 only) |
| `lhu v1,field; addu v1,v1,a0` (load lands in the result register) | `s32 value = field;` then `value += delta;` as two statements | `player_adjust_mp` `0x800156bc` |
| `lhu v0,field; addu v1,v0,a0` | single expression `field + delta` | rejected form of the same unit |
| `beqz a2,<second test>` | two sequential `if` blocks, not nested | `audio_set_listener_transform` `0x8003303c` |
| `addiu index,1` before the pointer bump inside a byte-clearing loop | `for (index = 0; index < N; index++, pointer++)` | `actor_pool_clear` `0x8002ce88` |
| `lw/sw x4` for a 16-byte copy; `lwl/lwr/swl/swr` for an 8-byte copy | plain struct assignment; the 2-aligned `KfVec4s` forces unaligned moves | `audio_set_listener_transform` |
| `lhu` then separate `sll/sra` on both operands of a difference | `u16` element loads with explicit `(s16)` casts on the difference; a direct `s16` load folds into `lh` | `matrix_interpolate` |
| `jal f; li a3,imm` | argument constant filled by the compiler's own delay-slot pass; no source action | `audio_play_spatial_default_range` `0x80032fb8` |
| `sh v0,off(s1)` runs after both `rsin`/`rcos` calls with `move s0,v0` in the second call's slot | compute both trig values into locals first, then assign the nine cells in row order | `matrix_set_rotation_x/y/z` `0x80014b7c` |

Open residues recorded during the same campaign (not steered):

- `actor_pool_find_free` `0x8002ca78`: retail joins the found and not-found
  paths at one `jr $ra` with `move v0,v1` in the compare's delay slot; every
  tried return shape emits a `j` to the epilogue instead.
- `matrix_set_rotation_yxz` `0x80014ccc`: retail keeps `move s1,a1` before
  the first `lh a0,4(s0)`; the probe schedules the load first.
- `save_workspace_allocate` `0x8002c27c`: retail sets `a1` to zero before
  reloading the payload pointer for the second `memset`; the probe reloads
  first. The call also needs a delinker naming decision because `GAME.EXE`
  links two vendored `memset` bodies.

## game-math-death

Shapes that decided exact matches in the game entry, main loop, math helper
and player-death band:

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `jal __main` as the first call, SNMAIN `start` tail-calls the body | the function is `main`; GCC inserts the hook only for that name | `main` `0x8001428c` |
| `lui/ori` address formation without relocations | numeric address constants in source (BSS start, heap base) | `main` |
| store inside a loop that never advances the pointer | plain `int *`; a `volatile` view hoists nothing but changes the loop form | `func_80014268` `0x80014268` |
| `sll/sra` of a divided value before `*100`, `andi 0xffff` at each grid access | `s32 cell = x / 2000 + (s16)(z / 2000) * 100` indexed as `(u16)cell` | `collision_query_world` `0x8001a5ac` |
| a `bne` to a trailing block for the uncommon case, `j loop` at its end | nested `if`/`else` blocks, not `continue` | `game_main_loop` `0x800146b8` |
| `beq` on two `lhu` values, copies done as two `lbu`/`sb` pairs | `*(u16 *)&a != *(u16 *)&b` compare with byte-wise member copies | `game_main_loop` |
| four-way branch on 12-bit angle distance, wrapped results masked with `0xfff` | the `angle_approach` body in `matrix_rotation.c` | `angle_approach` `0x80014a64` |
| `li v0,-1` shared between `sh` stores of `u16` and `s16` objects | the halfwords are `s16` (assigning `0xffff` to a `u16` emits `li 0xffff`) | `game_state_initialize` `0x800151cc` |
| `lbu/addiu/sb` through one address register for a flag that is tested, decremented and stored | an element of one byte array (`DAT_800652a8[0x2f]`), not a separate global | `player_death_restart` `0x800154b0` |
| `s0` loaded by `lbu` and compared without `andi 0xff` | an `s32` local holding a byte field | `player_death_restart` |
| a loaded operand kept in `a0`/`a1` and the same address used by `lh`/`sh` | a pointer to the halfword taken once (`s16 *bob = &...`) | `player_death_update_reverse_fade` `0x800186c4` |
| `addiu a1,a0,-128` / `-32` between matrix arguments | one `KfMatrix[7]` table indexed `[4]`, `[3]`, `[0]` | `player_death_apply_visual_fade` `0x800184b0` |

Open residues recorded during the same campaign (not steered):

- `game_main_loop`: retail reaches five player-block bytes through one base
  register holding `player_experience + 0xd`; GCC forms such a base only for
  offsets of one symbol. The block is one aggregate in the original source
  (`KfPlayerState`); a direct-member view through `player_experience`
  reaches 91% and a byte-pointer view 95%, neither exact.
- `collision_query_world`: retail reads `actor_definitions[id].collision_radius`
  and `map_object_definitions[id].collision_radius` relative to the pool base
  register (`lhu -1702(a0)`), so each definition table and its pool are one
  aggregate too. The rest of the residue is callee-saved register assignment
  (`x` in `s2`, `flags` in `s0` in retail).
- Scheduling class, no probe reproduces it: retail keeps loads in source
  order instead of hoisting them across earlier stores or `mult`/`mflo`
  (`vector2s_scale_shift11`, `game_state_initialize` growth-table loads,
  `player_death_restart`), keeps `sw ra` at the top of the prologue
  (`player_death_restart`), keeps parameter copies adjacent
  (`matrix_set_rotation_yxz`, `pitch_yaw_to_forward_vector`), and forms an
  argument address before the first call's other operands
  (`player_death_update_reverse_fade`, `player_death_apply_visual_fade`).
- `player_death_update`: retail has an 8-byte larger frame with no stack
  traffic and loads `camera_rotation.x` with `lhu` before subtracting, so the
  original field is unsigned or accessed through a different type.
- `angle_within_tolerance`: retail materialises the result through a branch
  (`li v1,1` on the true path); every expression form tried folds to `xori`.
