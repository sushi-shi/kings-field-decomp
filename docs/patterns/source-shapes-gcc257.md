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
| `li v0,0xfffe; addu a0,a0,v0`, followed by `andi` only at the bounds test | `u16 cell_x; cell_x -= 2;` where every later use needs only the low 16 bits | `collision_adjust_cell_occupancy` `0x8001a4e8` |
| persistent pointer in `t1`, `move a0,t1` at the outer-loop head, and `addiu t1,a0,100` in the bounds-branch delay slot | keep `next_row` across iterations, set a block-local `cell = next_row`, then advance `next_row = cell + 100` at the top | `collision_adjust_cell_occupancy` `0x8001a4e8` |

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

## save

Campaign over the contiguous save/memory-card/menu band `0x8002b078..0x8002ca78`
(module `save_system`, 24 functions). Compiler-level findings first:

| Retail signature | Source shape or profile | Witness |
| --- | --- | --- |
| `move a1,zero` (argument constant) scheduled before the `lui/lw` of another argument; loads not hoisted over independent moves | `-mcpu=r2000` on the GCC 2.5.7 probe; the r3000 schedule hoists the load. Adding the flag to `probe-gcc257-o2-g0` took the whole corpus from 63 to 70 exact with no regression | `save_file_read_header` `0x8002bd08`, `save_workspace_allocate` `0x8002c27c`, plus nine previously non-exact units |
| `mult s6,s3; mflo` for `0x2580 * entry` and `li s5,0x280`/`li s6,0x2580` materialized once at the top and reused for `write`/`read` lengths and comparisons | the sizes are plain `s32` locals initialised at the top (`payload_size = sizeof(KfSavePayload)`), not literal constants; a constant multiplicand is strength-reduced to shifts instead | `save_file_write_slot` `0x8002b73c`, `save_file_read_slot` `0x8002beb0` |
| `li s3,0x280` in a call delay slot after a `memset` that used the immediate, then `move a2,s3` for the `read` length | the length local is assigned after the `memset`; the `memset` uses `sizeof` directly | `save_file_read_header` `0x8002bd08` |
| one callee-saved register serves as scan index and retry counter (`s0`) | a single `index` local drives every `for`/retry loop in the function | `save_file_write_slot` |
| aligned 16-byte `lw/sw` copy loop, aligned loop plus `lw/sw` word remainder, alignment-checked `lwl/lwr` loop | `memcpy` builtin expansion with constant sizes; both operand types word-aligned gives the unchecked loop, a byte-typed operand gives the `or/andi/beqz` runtime check; retail's ranges fix the payload member types | `save_file_write_slot`, `save_file_read_slot` |
| `la a3,literal` before the header pointer load, aligned 48-byte loop plus `lw/lb` remainder for 53 bytes | `memcpy(dst, "literal", sizeof("literal"))` from a string literal; a `const char[]` extern gives the checked loop instead | `save_file_initialize_buffers` `0x8002c304` |
| 10-byte `lwl/lwr` copy from `.rdata` into the frame followed by `memset(.., 0, 6)` | `char path[16] = "TIM\\M000.";` local initializer | `menu_load_message_image` `0x8002c5e0` |
| `lui at; addiu at,sym; addu at,at,reg; lbu` with a second induction register stepping by 20 | an explicit byte-offset local (`record += 20`) that dies after the loop; `array[index * 20]` folds the symbol into a pointer induction variable instead | `save_file_write_slot` magic-record sampling |
| `move a0,s0` then `switch` on `a0` with result assignments in `a0`, `move v0,a0` at the join | `result = status; switch (result) { ... result = k; }` on a second local | `memory_card_check_or_format` `0x8002b4d8` and the three `save_system_*` wrappers |
| `bne file,-1,<epilogue>` with `li v0,5` in the slot then `j`+`li v0,3` | `if (file == -1) return 3; return 5;` (the reversed test emits `beq`) | `save_file_write_slot` |
| case bodies emitted in source order (`-1`, `0x65`, `0x6d`, `0x71`, ...) | list the `case` labels in retail body order, joining shared bodies (`case 0: case 4:`) at their position | `memory_card_show_status_message` `0x8002c510` |
| `beq id,0xff,<return 0>` sharing the final `return 0` | `if (message_id != 0xff) { ... } return 0;` rather than an early `return 0` | `menu_load_message_image` |

Recorded residues (not steered):

- Jump tables and string literals of this unit live in retail `.rdata` in
  function order (`0x8001235c..0x800124d2`: four switch tables, the title,
  three icon paths, the status-message table, the message template). The
  compiled objects reference them as `.rodata`-local symbols while the target
  names them `DAT_8001235c`, `save_title_text`, `memory_card_message_path_template`;
  every other instruction matches. Closing them needs module-owned data
  sections in the delinker (the DATA-claim work), not source changes:
  `memory_card_check_or_format`, `save_system_write_slot`,
  `save_system_read_header`, `save_system_read_slot`,
  `save_file_initialize_buffers`, `menu_load_message_image`,
  `memory_card_show_status_message`.
- `save_file_read_slot` `0x8002beb0`: retail forms the 0xe0-byte restore
  destination as `&player_weapon_asset_buffer - 108` after loading that
  pointer, which only a struct spanning `0x800a0780..` (player state) can
  produce; the identities model separate globals.
- `screen_show_image_until_input` `0x8002c794`: retail forms the ordering
  table base as `&asset_load_buffer + 0x20` and keeps `&asset_load_buffer`
  and later `&ordering_table` in `s2` across calls; the first needs a display
  state struct spanning `0x80070e9c..0x80090ebc`.
- `memory_card_show_status_message`: the probe copies the call result to
  `a0`/`v1` before the `!= 1` test; retail keeps it in `v0`.
- `talk_show_indexed_image` `0x8002c9d4`: retail materializes
  `&talk_image_path_template[6]` and derives the call argument as `-6`; the
  probe anchors on the first store (`[0xa]`); chained assignments did not
  change it.
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
## player

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `la reg,sym+off` once, then `lhu/sh 0(reg)` for one halfword while a sibling halfword keeps `lui/lhu sym+off` | the halfwords are members of one object (`KfPlayerState`); GCC 2.5.7 registers the address of the first member it touches repeatedly and folds the others. Separate globals never produce the registered form, an array or struct member at any offset does. | `player_increment_physical_power_training` `0x80015f28`; probe `build/probe/train.c` |
| `lw/sw` x4 loop over 0xe0 bytes from one base | whole-object struct copy; the loop size is the object size and fixes the object extent | `save_file_write_slot` `0x8002b73c` (defines `KfPlayerState`) |
| working values kept in `$a0`/`$a1` after the parameters die | the parameters are reused as the working variables, no extra locals | `player_calculate_damage_component` `0x8001627c` |
| independent stores emitted in a non-ascending field order | GCC keeps source order for independent stores; write the assignments in retail order (`z, y, x` for the camera rotation, `yaw, pitch, speed, forward, strafe` for the motion clear) | `game_initialize_session` `0x80016e24`, `player_clear_motion` `0x80016eb8` |
| a computed value stored to a global and passed to a call from the same register | compute into a local, store it, pass the local; re-reading the global instead reloads it into the argument register | `player_update_view_bob` `0x80017a24` |
| `lhu v0 = field; lhu v1 = field` twice before a store and compare | `committed = current; if (current == N) ...` with plain member reads; the compiler does not CSE across the intervening store | `player_begin_weapon_attack` `0x80016b24` |
| `beq falloff,0x1000,<else>` with the computed path falling through | `if (x != CONST) { compute } else { simple }`: the compiler places the first branch of the source first | `player_apply_radial_damage` `0x800166b4` |
| an ALU instruction from before a branch sitting in that branch's delay slot | put the statement before the `if` in source (`dx >>= 3;` ahead of the y-test); reorg only takes slot fills from the preceding block or the target | `player_distance_to_point` `0x80017108` |
| the same compare-and-branch duplicated in each arm of an `if`, both arms falling into one block, with the block's first store copied into the joining `j`'s delay slot | `if (cond ? A : B) { ... }`: GCC's do_jump expands a conditional expression by testing each arm directly | `player_update_weapon_attack` `0x80016bc0` |
| `lh v1,field; beq v1,-1; move a0,v1; addiu a0,a0,300; sh a0,field`, later `subu; andi; sltiu` on the stored register with no constant folding | `field += 300;` on the halfword member in place, then `u16 window = field;` and `(u16)(window - N) < M`; an `s16` local produces a second `lhu` load and an `s32` local folds the constants into the later subtraction | same |
| the reset stores run whether or not the probe found an actor | the `if (actor != -1)` guards only the damage call | same |
| `lbu v1 = grid[...]` then the `*100` chain in `v0`, `lh` of a second field, `negu`, store, `addiu -1500` on the second field | distinct variables for the grid byte and the negated height, and `view_offset = view_bob_offset - 1500` as its own local; reusing one variable or writing the sum inline lets the compiler fold the constant onto the other operand | `player_sync_position_to_map` `0x80016ee8` |
| `la reg,table-2; sll; addu; lbu 0(reg); lbu 1(reg)` | `entry = &table[floor - 1]; a = entry->x; b = entry->z;` (the -1 folds into the symbol; separate `table[floor - 1].x` expressions keep an explicit `addiu -1`) | `player_warp_to_floor_entry` `0x80017cf8` |
| `switch` on a byte with cases 0, 0x10, 0x20 compiled as `beq 0x10; slti 17; beqz 0; beq 0x20`, and `j` into the middle of another case | a `switch` whose case bodies appear in the retail address order (0x10, 0x20, then 0) with labels inside the cases as `goto` targets; a shared `player_death_begin()` at the end of two branches is cross-jumped into one call | `player_update_vertical_motion` `0x80017a80` |
| `div` for `/ 10` but `divu` for `% 10` on the same byte | plain `value / 10` and `value % 10` on a `u8` member; the compiler picks the unsigned remainder itself | `actor_show_info_image` `0x80017edc` |

### Scheduling model: `-mcpu=r3000` (profile change, 2026-09-02)

The rebuilt `cc1psx-257` banner reports `Cpu = 3000`, but the instruction
scheduler used the generic latency model unless `-mcpu=r3000` is passed. With
the flag, argument setup and load placement follow retail (`li a1,0xffff`
before the two `lw` loads in `player_distance_to_point_in_cone`; `move v0,a0`
before the stack-argument loads in `player_apply_radial_damage`). Applied to
every enrolled unit it produced no regressions and made ten more units exact:
`matrix_set_rotation_yxz`, the four vector scale helpers,
`primitive_buffer_commit_poly_ft4`, `game_state_acknowledge_pending`
(previously exact only under 2.6.0 without the second scheduling pass),
`save_workspace_allocate`, `audio_play_spatial_range`, `sound_ref_play`.
`probe-gcc257-o2-g0` now carries `cc1_flags = ["-mcpu=r3000"]`.

Open residues recorded in the player campaign (not steered):

- `player_add_experience` `0x80016058`: inside the level-up `while` loop the
  probe hoists `la s2,player_level_growth_table+0x1d4` and schedules the
  extrapolation loads early; retail keeps every growth-table access as a
  direct `lui/lhu` pair in statement order. Both 2.5.7 and 2.6.0 hoist the
  anchor for any source-level loop containing two related constant
  addresses (`build/probe/hoist.c`), and a goto-formed loop loses the
  retail `s0`/`s1` player anchors, so the shape is unexplained.
- `player_distance_to_point` `0x80017108`: the last range check keeps an
  inline `j` to the epilogue instead of the shared `bnez` form, and the
  final distance lands in `a0` instead of `v1`.
- `player_distance_to_point_in_cone` `0x80017040`: the early-return branch
  slot holds `nop` instead of `move v0,s1`.
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

## leaf frames

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `addiu sp,sp,-8` ... `addiu sp,sp,8` around a leaf with no stack traffic (`.frame $sp,8`, `vars= 8`) | a loop whose condition post-decrements a variable (`while (count--)` or `while (count-- != 0)`); `for (; count != 0; count--)` reserves nothing | `func_80014268` `0x80014268` |
| the same frame in a leaf without a loop | a load and a store that address the same global object (`tmd_state.current_asset = tmd_state.slots[index]`); GCC 2.5.8 sources: CSE relates the two addresses, combine folds the array address pseudo into the load, its stale `reg_n_refs` keeps it alive for reload, and `alter_reg` gives the dead pseudo a stack slot that nothing uses. Storing the same load into another object, or `-fforce-addr`, removes the frame | `tmd_select` `0x8001c0e8` |
| the same frame in `tmd_prepare_primitive_indices` `0x8001c2b0` | not reproduced; the function stores nothing to a global, so the folded pseudo must come from another expression; open | residue |


## render

Witnesses come from `src/game/render.c` (`game.render`, one interleaved
band: render, display, tmd, render, tmd). The GCC 2.5.8 sources
(`ftp.gnu.org/old-gnu/gcc/gcc-2.5.8.tar.gz`; 2.5.7 is not archived and the
two generate identical code) explain several of the rows.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `lui v1,&s.b; addiu v0,v1,-32; ... sw a1,0(a0); jal; sw a1,0(v1)` (second store's address materialised first, first store's base derived from it) | chained assignment `s.b = s.array[i] = value;` the destination of the outer assignment is expanded first, CSE's related-value pass derives the array base from it | `tmd_register` `0x8001c5b0` |
| `move a0,s0; lui a2,&other; addiu a2; jal MulMatrix0; addiu a1,s0,128` (one argument folded from a base register, the next absolute) | the folded operand lives in the same object as the base register; the absolute one is a separate global. CSE folds `sym+k` only against registers holding the same symbol, so an absolute address next to a folded one is boundary evidence | `render_initialize` `0x8001bce0` (`light_quadrant_matrices` split from `render_state`) |
| `lw v0,0(a2); srl v0,v0,0x18; andi v0,0xfd` beside `lbu v1,1(a2)` | `word = *(u32 *)packet;` as its own statement, then `switch ((word >> 24) & 0xfd)`; writing `mode = *(u32 *)packet >> 24` lets combine turn the load and shift into `lbu 3(a2)` | `tmd_prepare_primitive_indices` `0x8001c2b0` |
| `lhu v0,n; beqz v0,exit; addiu t0,v0,-1` then a bottom test `move v0,t0; andi; bnez; addu t0,t0,t1` (`t1 = 0xffff`) | `count = field; if (count == 0) return; left = count - 1; do { ... } while (left-- != 0);` with `u16` locals. `while (count--)` on a promoted `u16` copies before the entry test (`move; andi`), and `for (i = 0; i < n; i++)` is never reversed because 2.5.8's `check_dbra_loop` needs a constant bound | same |
| inner `lhu a0,4(a3); beqz a0,skip; addu a1,a0,t1` | `count = object->field; if (count != 0) { left = count; left--; do {...} while (left-- != 0); }`; `left = count - 1` computes in `int` and prints `addiu -1` | same |
| giv base on the earlier field (`addiu a3,v1,28`, `lw 0(a3)`, `lhu 4(a3)`) | read the later field first in source; loop.c makes the last-referenced field the combined giv base, and the scheduler then orders the loads by path length (`(offset + 12) + base` gives the offset load the longer path) | same |

Residues left in the same module (not steered):

- `render_set_view_transform` `0x8001c184`: retail copies the rotation
  through `a1` (a block-move scratch holding the constant address) and
  re-materialises `a0 = &render_state.view_rotation` for `RotMatrix`; every
  copy spelling and every flag sweep ties the copy address to the argument
  register instead. In the 2.5.8 sources `update_equiv_regs` replaces a
  constant-equivalent pseudo used exactly once in another basic block, so the
  original probably placed the copy and the call in different blocks in a way
  not yet found.
- `tmd_prepare_primitive_indices` `0x8001c2b0`: the unused 8-byte frame and a
  second `tmd_state.current_asset` load that sits before the guard branch
  without being merged by CSE (`cse_end_of_basic_block` follows a conditional
  jump only when its label is used once and preceded by a barrier).

## memory

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `move a2,a0` at entry, `&cursor` in `a0`, the stack entry stored from `a2` on both paths | reuse the `size` parameter as the recorded entry (`size = (s32)block;` / `size = (size + 3) & ~3;`); a separate `entry` local keeps `size` in `a0` and moves the cursor address to `a3` | `memory_allocate` `0x8001ac0c` (exact in both images) |
| `addu v0,a2,a1; sw v0,0(a0)` (size before block) | `*cursor += size;`; `*cursor = block + size` and `size + block` both emit `addu v0,a1,a2` | same |

## cd_file

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `lwl/lwr` + `lb` from a `.data` array, `swl/swr` + `sb` into the path buffer | `memcpy(path, cd_path_prefix, sizeof cd_path_prefix)` with `char cd_path_prefix[5] = "\\KF\\";` (a 5-byte constant-length block move; `strcpy` of a global array cannot fold) | `cd_file_load_allocated` `0x8001acf0` |
| `addiu s0,s0,1` in the branch delay slot, then `li s0,0x64; li s1,1; addiu s0,s0,1` | `for (attempt = 0; attempt < 3; attempt++) { ... if (result == 0) { attempt = 100; loaded = 1; } }`; the loop increment is copied into the delay slot because the fall-through overwrites it | same |
| `srl v0,v1,0xb; addiu s1,v0,1; sll v0,s1,0xb` with `s1` cleared to zero right after | the rounded sector count shares the `loaded` variable (`loaded = (size >> 11) + 1; size = loaded << 11;`); a separate local keeps the count in `v0` and swaps `s0`/`s1` in the sibling loader | `cd_file_load_into` `0x8001af9c` |
| `sll s1,a1,2; addu s1,s1,a1; sll s0,s1,2` then `sll v0,s1,2` again before the second table load | index the record array at every use (`cd_file_table[index].size`); a pointer local computes the address once | `func_8001ae60` `0x8001ae60` |

## resources

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `lw v0,0(s2); addiu v0,v0,4; addu s2,s2,v0; jal f; addiu a0,s2,4` | an assigning macro used inside the argument: `f(STREAM_NEXT(stream) + 4)` with `#define STREAM_NEXT(p) ((p) += *(u32 *)(p) + 4)`; `stream = stream + len + 4` as a statement adds the base first (`addu`, then `addiu 4`) | `map_resources_load` `0x8001b558`, `common_resources_load` `0x8001b180` |
| `move s0,s2` before the advance, the advance in the next call's delay slot | `block = stream; STREAM_NEXT(stream); call();` | `map_resources_load` |
| runtime `(src\|dst) & 3` test around a 16-byte copy loop versus a plain `lw/sw` loop | `memcpy` with byte pointers (alignment 1) versus `memcpy(table, (KfPlayerLevelGrowth *)(p + 4), sizeof table)` where the cast target is word-aligned | `common_resources_load` |
| `move a0,s0` without `andi` before a call taking a small argument | the callee's parameter is `s32`, not `u8`; a `u8` prototype makes the caller mask | `map_resource_path_set_floor` `0x8001b390` |

## actor pool and asset registry

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `move s0,a1; lhu s1,0(s0); addiu s0,s0,4` and the loop stores `s0` into the table | advance the pointer parameter itself (`archive += 4; ... archive += *(u32 *)archive;`); a separate cursor local loads through `a1` and swaps `s0`/`s1` | `asset_registry_load_tmd_archive` `0x800204c0` |
| a free path that jumps over the placement increment to the actor increment, and a terminator branch that is only `li s5,1; j <free path>` | `if (finished == 1) { mark_free: free...; continue; } ... else { finished = 1; goto mark_free; } placements++; } while (actor++, count-- != 0);` — the shared actor increment sits in the loop condition, so `continue` reaches it without a second increment and the placement pointer is the first induction variable incremented (its giv is initialised last) | `actor_pool_load_placements` `0x800308c0` |
| duplicated free statements in the terminator branch | not merged by cross-jumping: the second store re-materialises `0xff` in `v0` instead of the hoisted `s6`; `goto` is the shape | same |
| `move s5,zero` before `lui s2` (the pool base) in the preheader | declare and initialise `finished` before `actor` | same |
| `li s4,0x7f` ... `do { ... } while (count-- != 0)` over 128 slots | `u16 count = 0x7f;` with the post-decrement test | `actor_pool_update` `0x80030818` |

## actor actions

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `addu a2,v0,v0; addu a2,a2,v0; srl a2,a2,1` | `u32 rate = definition->turn_rate; (rate + rate + rate) >> 1`; `rate * 3` prints `sll; addu` and a signed sum prints `sra` | `actor_move_along_heading` `0x8002ed00` |
| two locals kept in stack slots (`sw v0,24(sp)`, `sw v0,32(sp)`) around a call instead of `s1`/`s2` | a `struct KfVec3i` local whose `x` and `z` are the values (aggregates stay in memory); scalar locals live in saved registers | `actor_apply_horizontal_movement` `0x8002f31c` |
| `sra v1,v1,0x10` on a collision result | keep the result in an `s32` even though `collision_query_world` returns `u32` | same |
| the second modulo test skipped when the first fails | nest the loop-sound test inside the effect branch | `actor_update_boss_death_sequence` `0x8002f8cc` |
| `addiu s0,s3,8` reused for a byte table and, shifted, for two halfword tables | one `index = action + 8` local indexing the definition's per-action arrays | `actor_update_effect_action` `0x8002f468` |
