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

## player

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `la reg,sym+off` once, then `lhu/sh 0(reg)` for one halfword while a sibling halfword keeps `lui/lhu sym+off` | the halfwords are members of one object (`KfPlayerState`); GCC 2.5.7 registers the address of the first member it touches repeatedly and folds the others. Separate globals never produce the registered form, an array or struct member at any offset does. | `player_increment_physical_power_training` `0x80015f28`; probe `build/probe/train.c` |
| `lw/sw` x4 loop over 0xe0 bytes from one base | whole-object struct copy; the loop size is the object size and fixes the object extent | `save_file_write_slot` `0x8002b73c` (defines `KfPlayerState`) |
| working values kept in `$a0`/`$a1` after the parameters die | the parameters are reused as the working variables, no extra locals | `player_calculate_damage_component` `0x8001627c` |
| independent stores emitted in a non-ascending field order | GCC keeps source order for independent stores; write the assignments in retail order (`z, y, x` for the camera rotation, `yaw, pitch, speed, forward, strafe` for the motion clear) | `game_initialize_session` `0x80016e24`, `player_clear_motion` `0x80016eb8` |
| a computed value stored to a global and passed to a call from the same register | compute into a local, store it, pass the local; re-reading the global instead reloads it into the argument register | `player_update_view_bob` `0x80017a24` |
| `lhu v0 = field; lhu v1 = field` twice before a store and compare | `committed = current; if (current == N) ...` with plain member reads; the compiler does not CSE across the intervening store | `player_begin_weapon_attack` `0x80016b24` |
