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

## actor

Shapes settled while reconstructing `src/game/actor.c` (band
0x8002ca78..0x8002e0f0):

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `addiu v0,s1,-1824` / `addiu s8,s1,9244` (one symbol, member offsets) | one aggregate `KfActorState` for definitions, actors, player snapshot, and current/target pointers; separate globals can never produce base-relative member addresses | `actor_pool_begin_death_by_definition`, `actor_pool_find_overlap` |
| `lbu v1,5; chain; lh v1,16` (second load after the chain that reads its register) | reuse one temporary for the tile index and then the local offset | `actor_initialize_current` `0x8002cd28` |
| `li v0,0xff` shared by `sb v0,8` and `sb v0,56` | both fields unsigned bytes assigned `0xff`; an `s8` field would materialize `-1` separately | `actor_initialize` |
| `sll v0,a1,0x10; bgez` then raw `a1` reused | `s16 delta` parameter tested for sign and added unextended | `actor_advance_animation_wrapped` |
| `lhu` of a definition byte compared with `0xff` | the byte is `u8`; an `s8` compare against `0xff` folds away entirely | `actor_pool_begin_death_by_definition` |
| `bnez ... -> exit` on every failing test with `li v0,-1` in the slot, no inverted `beqz; j` | a shared `goto out_of_range` exit label instead of repeated `return -1` | `actor_distance_to_point` |
| `sw a0,0(a2)` before the index division result is stored | assign the current pointer before computing the index | `actor_bind_current` |
| `andi v0,s2,0xff` on every return path | selector actions and results are `u8`, not `s8` | `actor_try_select_*` |

Open residues (not steered):

- `actor_apply_damage`, `actor_pool_apply_radial_damage`,
  `actor_pool_find_overlap`, `actor_try_attack_player`,
  `actor_play_sound_at_phase`: retail hoists argument-register copies
  (`move a1,s3`, `move s2,a0`) above independent loads; the 2.5.7 probe keeps
  them adjacent to their call or use.
- `actor_try_select_*`, `actor_pool_find_target_in_cone`: retail keeps every
  prologue register save together and loads the current actor into `s1`
  afterwards; the probe schedules that load right after the `s1` save.
- `actor_initialize_slot`: the `lifecycle = 1` store stays between the first
  load and the multiply chain in retail; the probe sinks it below the chain.
- `actor_animation_crossed_phase`: register choice only (`v1`/`a2` swapped).
- `actor_pool_find_free`: the found path joins the not-found path at one
  `jr $ra` with `move v0,v1` in the compare's delay slot; every tried return
  shape emits a `j` to the epilogue.

