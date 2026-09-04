# Source shapes that decided exact matches under the 2.5.7 probe

Each row names the retail signature, the shape that reproduced it, and a
witness unit. These are observations about GCC 2.5.7 `-O2`, not rules for
other compilers.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `addiu c,c,-1; move ...; sll/sra 16; bne c,-1` | `s16 count = 8; do { ... } while (--count != -1);` | `matrix_interpolate` `0x800202fc` |
| `addiu c,c,-1; bnez c` | `s32 count = N; do { ... } while (--count != 0);` | `actor_definitions_load` `0x80030a6c` |
| `move v1,a1; ...; addiu a1,-1; bnez v1` | `while (count-- != 0)` with an `s32` count | `weapon_records_load_and_mirror_angles` `0x800150a8` |
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
- `func_80029ab0`/`func_80029c50` `0x80029ab0` (menu sprite blitters): the two
  quad corner-inset constants (`-4`/`-3`, `-18`/`-2`), each used four times,
  are kept as `addiu` immediates in retail; the probe materializes each into a
  callee register (`li 0xfffc; addu`) and then schedules that `li` into the
  following load-delay slot that retail leaves as `nop`. Fields, values, load
  widths, associativity, and the per-corner `current_poly_ft4` reloads all
  match; only the constant materialization and the freed delay slot differ.
  The same u16-store-context constant handling recurs across the menu POLY_FT4
  assembly helpers.

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
| store inside a loop that never advances the pointer | plain `int *`; a `volatile` view hoists nothing but changes the loop form | `repeat_store_word` `0x80014268` |
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
`primitive_buffer_commit_poly_ft4`, `menu_release_item_model`
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
| `addiu sp,sp,-8` ... `addiu sp,sp,8` around a leaf with no stack traffic (`.frame $sp,8`, `vars= 8`) | a loop whose condition post-decrements a variable (`while (count--)` or `while (count-- != 0)`); `for (; count != 0; count--)` reserves nothing | `repeat_store_word` `0x80014268` |
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

## tmd_project

Witnesses come from `src/game/tmd_project.c` (`game.tmd_project`,
`0x8001c60c..0x8001c7f4`): three vertex-projection primitives that walk
`current_tmd_vertices` through the GTE and write eight-byte screen entries into
the `DAT_800911b0` scratch buffer. `func_8001c60c` is the shared helper called
by nine sites in the surrounding polygon emitters.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `beqz a0,exit; addiu s0,a0,-1` guard then an up-walking pointer pair with a `bottom` down-counter `addiu s0,s0,-1; li v0,-1; bne s0,v0` | `for (i = count - 1; i != -1; i--) { ...; out++; vtx++; }` -- the `!= -1` exit prints the `li -1`/`bne` down-counter and keeps the pointers forward; `i >= 0` prints `bgez` instead and `for (i=0;i<count;i++)` keeps `count` live in an extra callee-saved register | all three |
| unused 8-byte frame tail (`vars=24`, slots 32/36 never referenced) beside four live output slots at 16/20/24/28 | plain scalar `long p, flag, sz0, sz1;` under the `!= -1` for-loop; the reversed loop reserves the extra doubleword that the equivalent `if (count) { do {} while (--i != -1); }` does not | all three |
| `lhu v0,16(sp); sll v0,v0,1; sh` for the interpolation term, `lhu`/`lw` for the SZ FIFO word | `out->p2 = (u16)p << 1;` (unsigned halfword read) and `out->sz = (u16)sz0;` (perspective) or `out->sz = sz0 >> shift;` (arithmetic `lw`+`srav` when the shift is a `u8` argument) | `tmd_project_vertices`, `tmd_project_vertices_shift` |

`tmd_transform_vertices` `0x8001c754` is exact: its `&out->sz` induction
pointer carries three stores (`sxy.vy`, `sz`, `p2`) and so unambiguously
outranks the loop counter, matching retail's register assignment.

Residues left in the same module (not steered):

- `tmd_project_vertices` `0x8001c60c` (84%) and `tmd_project_vertices_shift`
  `0x8001c6a8` (86%): structurally exact -- frame, stack layout, control flow,
  the RotTransPers/ReadSZ2 call set and every instruction match -- but the loop
  counter and the `&out->sz` induction pointer trade `$s0`/`$s1`. The `-dg`
  dump settles the attribution: the greg pass orders the allocnos
  `88 86 78 72 73`, i.e. the two-store `&out->sz` giv (`sz` and `p2` writes plus
  its own bump, four in-loop operand refs) outranks the loop counter (three), so
  the probe gives the giv `$s0` and the counter `$s1`. Retail assigns the
  opposite -- counter `$s0`, giv `$s1` -- despite routing the identical two
  stores through the giv, so the tie is decided by a priority weighting the
  2.5.7 build does not reproduce, not by any source shape. Dropping a store from
  the giv would tie the two allocnos (the counter's lower number then wins) but
  changes the emitted bytes, so it is not steerable from C.
  `tmd_transform_vertices`, whose giv carries three stores and so unambiguously
  outranks the counter, matches exactly; the residue is the borderline
  two-store giv alone. Same giv-base / callee-saved-register-choice
  compiler-build question as the `func_8002317c` and `func_800238d8` classes.

## render enqueue

Witnesses come from `src/game/render_enqueuers.c` (`game.render_enqueue`,
`0x8001c7f8..0x8001e480`). The four consumers are one evidenced TU: their text
order (TMD, model, map, sprite) is mirrored exactly by the gapless load-data
run at `0x80057b58..0x80057b6c` (three neutral `CVECTOR`s followed by the
sprite `SVECTOR` light normal). The projected textured-sprite enqueuer
`func_8001e230` and lit map-geometry emitter `func_8001de18` are structurally
complete but remain below exact for two distinct walls.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `jal RotTransPers4; move s1,v0` (the anchor depth saved in the call's delay slot, one beat before the corner batch clobbers `$v0`) | `otz = RotTransPers(&anchor, ...); RotTransPers4(corners...);` -- the sprite sorts by the projected origin, not the corner batch; the anchor return lives in `$v0` across the `RotTransPers4` argument set-up | `func_8001e230` `0x8001e230` |
| four `long sxy0..sxy3;` scalars packed above `p`/`flag`, not an array below them | declare the four screen-XY outputs as separate `long`s, addressed only at the `RotTransPers4` call, so their slots follow `p`/`flag` (whose addresses are taken at the earlier `RotTransPers`) | same |

Residues, both blocked (not steerable from C under this probe):

- `func_8001e230` `0x8001e230` (88%): the same post-reload-scheduler wall as
  its banked sibling `func_8001e480` (`render_sprite.c`). gcc-2.5.7 fills the
  screen-XY load-delay slots with the clut/tpage global loads (hoisting them
  ahead of their stores) and schedules the anchor-depth save so that the OT
  index needs one extra callee-saved register than retail's five. Neither `-O2`
  nor `-O2 -fno-schedule-insns2` reproduces both the hoisted prologue and the
  un-hoisted body at once -- the open scheduler-attribution residue.

- `func_8001de18` `0x8001de18` (13%): a base-sharing divergence that turned out
  **not** to be the dominant one. Retail holds `&tmd_state.current_asset`
  (`tmd_state+0x20`) in one register and forms the projected-vertex buffer as
  `base+488` and the ordering-table pointer as `(buffer-756)`, materialised
  from a single relocation. Because gcc shares a base register only across
  accesses the source proves are one object (the `render_initialize` "boundary
  evidence" row above), that arithmetic proves `tmd_state`, the vertex buffer
  `DAT_800911b0`, and `display_state.ordering_table` were **one
  graphics-context aggregate** the curation split into separate identities --
  the same aggregate carries the sprite material `DAT_80095058` at
  `buffer+16040`, reached that way by the larger emitters `func_8001d730` and
  `func_8001c7f8`. The aggregate was modelled and probed (see the
  graphics-context section below); it reproduces the base register exactly, but
  it lifts `func_8001de18` only `12.85% -> 13.7%`. The dominant divergence is
  **not** the split identity -- it is the same unattributed gcc-2.5.7
  register-allocation/frame wall as its siblings: retail uses `$s6`/`$s7` for
  the buffer/counter where the probe swaps them, and retail reserves an extra
  8-byte frame slot (`-80` vs `-72`) to spill the packet `header` across the
  loop-body calls where the probe keeps it live. The `render_initialize`
  boundary evidence for base-sharing remains sound; what is wrong is the earlier
  claim that the split identity was the reason for the 13% score. The larger
  dispatchers `func_8001c7f8` (0xf38, an eleven-way TMD primitive switch whose
  jump table lives far away at `0x8001222c`) and `func_8001d730` (0x6e8, the
  actor-model GT/FT path) share the same aggregate and the same regalloc wall.

## graphics-context aggregate (`0x80090ebc..0x800957e0`)

The base-register arithmetic in `func_8001de18` (`&tmd_state.current_asset`
reaching `display_state.ordering_table` at `-756` off the `+488` buffer
pointer) and in the display initializer `func_8001bb94` (`&draw_env[0].dtd`
reaching `disp_env[0]` at `+162` and `render_state.fog_near_distance` at
`+18538`, all with the linked bytes carrying no relocation at the offset
sites) prove that a contiguous BSS run the curation split into many identities
was **one declared object** in the source. gcc shares a base only across
members of one symbol, and a separate global can never sit inside another
global's extent, so every identity between the proven endpoints is a member.
The proven extent is `0x80090ebc..0x800957e0` (`0x4924` bytes); the OPEN.EXE
layout already models the leading ordering-table pointer as a global separate
from `display_state` (size `0x20024`), so GAME's `display_state` shrinks to
`0x20024` and the pointer becomes the aggregate's first member.

| aggregate offset | address | member | current identity |
| --- | --- | --- | --- |
| `+0x0000` | `0x80090ebc` | `ordering_table` (current-frame OT pointer) | tail of `display_state` |
| `+0x0004` | `0x80090ec0` | `draw_environments[2]` (`DRAWENV`) | `display_draw_environments` |
| `+0x00bc` | `0x80090f78` | `disp_environments[2]` (`DISPENV`) | `display_disp_environments` |
| `+0x00e4` | `0x80090fa0` | 8-byte gap | (unmodelled) |
| `+0x00ec` | `0x80090fa8` | `tmd_state` (`current_asset` at `+0x10c`) | `tmd_state` |
| `+0x0110` | `0x80090fcc` | `asset_registry_entries[60]` | `asset_registry_entries` |
| `+0x0200` | `0x800910bc` | `current_tmd_vertices` | `current_tmd_vertices` |
| `+0x0204` | `0x800910c0` | `pool_records[12]` | `pool_records` |
| `+0x02f4` | `0x800911b0` | `projected_vertices` (8-byte screen entries) | `DAT_800911b0` |
| `+0x419c` | `0x80095058` | sprite/floor scratch, floor items, frame counters | `DAT_80095058`, `floor_items`, ... |
| `+0x47e4` | `0x800956a0` | `render_state` (`fog_near_distance` at `+0xa0`) | `render_state` |
| end `+0x4924` | `0x800957e0` | -- | `light_quadrant_matrices` (separate, per `render_initialize`) |

Probe result (aggregate modelled as `KfGraphicsContext graphics_context` at
`0x80090ebc`, its span retargeted in `relocs.tsv`, its consumers migrated):

- The base register is reproduced exactly on both witnesses: the delinked
  target and the compiled object both anchor `graphics_context+0x10c`, form the
  buffer as `+488`, the ordering table as `-756`, `disp_env` as `+162` and the
  fog word as `+18538` -- one relocation for the whole span.
- `func_8001bb94` (display init) improves `94.25% -> 98.42%`: here the
  base-sharing *was* the dominant residue. The remainder is a callee-saved
  register / CSE decision the 2.5.7 build does not make -- retail keeps
  `$s0 = &draw_env[0].dtd` live across the four trailing calls and stores the
  far `fog_near_distance` as `sw v0,18538(s0)`, where the probe drops the base
  after `PutDispEnv` and re-materialises the fog store with a fresh `lui`.
- `func_8001de18` (map enqueuer) improves only `12.85% -> 13.7%`: base-sharing
  is a small fraction of its body, and the register-swap + extra-frame-slot
  wall above dominates the score.

Verdict: the aggregate is the correct structural model and is byte-clean where
tested (`func_8001e230` stayed `88.2%` under the migration, confirming
single-object member migrations do not shift bytes), but it closes neither
enqueuer nor the initializer to exact -- both residues are the unattributed
  gcc-2.5.7 register-allocation/scheduling wall class (compare the
  `tmd_project_vertices` two-store-giv residue and the `func_8001e230`
  post-reload-scheduler residue). Landing the model is high-ripple (it renames
`display_state`/`tmd_state`/`render_state`/`display_draw_environments` and the
buffers across ~28 units and retargets ~230 relocation sites) and banks no new
  function, so it is deferred until the compiler/regalloc attribution is settled;
  until then the individual identities stay.

The later TU/data-ownership pass recovered `map_textured_primitive_color` at
`0x80057b60` and its interior `cd` referent at `+3`, plus
`render_sprite_light_normal` at `0x80057b64`. This keeps all other consumer
scores unchanged, makes the combined four-object data contribution exact, and
raises `render_enqueue_map` from `12.854961%` to `18.305344%`. Its first
remaining divergence is still the aggregate base formation described above;
the later frame/register-allocation residue remains unattributed.

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

## map objects

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `beq a,11; slti a,11; slti a,83; slti a,80` ladder | `switch (action) { case 11: case 80: case 81: case 82: ... default: ... }`; the `&&` spelling folds `>= 11 && < 83` into one unsigned range test | `map_object_pool_trigger_link` `0x80031b54` |
| `beq id,0xff` whose delay slot holds the index increment from the loop tail | `for (; index < N; index++, object++) { if (id == 0xff) continue; ... }`; a `while` with the body under `if (id != 0xff)` fills the slot from the fall-through instead | `map_object_pool_find_interaction_from` `0x800315c4` |
| `-1` hoisted into `s7` for the distance compare | compare the call result inline in each branch (`if (f(...) != -1) return index;`); one compare after a `distance` join loads `-1` per iteration | same |
| `sw x; ...; lw x` reload of a field just stored | store x, y, z first and compute the cells afterwards: any later store through the object pointer invalidates the CSE entry, so the cell reads reload | `map_object_spawn_effect` `0x80031834`, `map_object_spawn_actor_debris` `0x800319c8` |
| `lui s0,&counter` kept in a saved register across the acquire call, `lhu; addiu; sh` then the old value stored | `u16 *sequence = &counter; ... object->spawn_sequence = (*sequence)++;` | same |
| `srl s1,v0,3; andi s1,0xffff` with the parameter's register reused | a separate short-lived `u16 angle = (u32)rand() >> 3;` (the parameter is dead, so the angle inherits its register); reusing the parameter itself keeps it live and swaps the argument registers | `map_object_spawn_actor_debris` |
| `addiu t3,a2,-1280` (definitions from the pool base) in a leaf loop | a local `definitions = map_object_state.definitions;` pointer hoisted by loop.c and folded by the second CSE pass; indexing the array directly keeps the absolute form | `map_object_pool_clear_link` `0x80031c44` |

Residues left in the module:

- `map_object_spawn_effect`: retail keeps the sequence pointer in `s0` and the
  acquired object in `s1` with the return copy scheduled before the counter
  store; every tried spelling reuses `s0` for the object.
- `map_object_pool_clear_link`: retail's `kind < 8` branch targets the
  following `kind != 8` test with an empty delay slot; ours jumps to the loop
  tail and fills the slot with the pointer increment.

## actor AI

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| a chain of `j <choose>` exits with the chosen id loaded in delay slots | an `if`/`else if` chain assigning `chosen`, with `goto choose` only where a branch must skip later tests after side effects (`rand()`) | `actor_select_next_action` `0x8002e2e8` |
| `beqz action → case0; li 1; bne action,1 → after; case1 body; case0 body` | `switch (action) { case 1: ...; break; case 0: ...; break; }` with case 1 written first | same |
| `move s1,zero` in the delay slot of a `rand()` roll branch | assign `chosen = 0` inside the taken branch (`if (rand() < 1092) { chosen = 0; goto choose; }`), not before the roll | same |
| `lbu s0,7(a); jal rand; sll s0,s0,7; slt v0,v0,s0` | write the threshold on the left: `(actor->unknown_07 << 7) > rand()` so it is evaluated before the call | `actor_update_awareness` `0x8002e6a8` |
| `andi s2,s0,0xff` once for several compares after a call | copy the promoted `u8` into an `s32 kind` local after the distance query and compare `kind` | same |
| `lw ×4; sw ×4` copy of the actor position before `vector3i_add_xz` | `KfActor.position` is a `KfVec4i` (16 bytes); `target = actor->position;` | `actor_move_xz_with_collision` `0x8002e954` |
| blocked handling laid out first, free path after, with `bnez v0 → blocked` from the drop checks | `if (result != -1 && (result != 0x1fff0 \|\| vertical_state == 3)) { blocked: if (stop == 0) { if/else chain } return 1; }` then the free path with `goto blocked` | same |
| `bgez x → L; li v0,0xc00; li v0,0x400; L: sh` (one store) | `if (delta->x < 0) yaw = 0x400; else yaw = 0xc00;` — the ternary sets the temporary before the branch and takes another register | same |
| `slti drop,400; bnez → -120 block` with the -300 block inline | `if (drop >= 400) { -300 } else { -120 }` | same |

Residue: `actor_update_awareness` keeps the constant 1 in `s5` for both the
lifecycle switch and the later `kind == 1` compare; ours re-materialises it.

| `sll v0,limit,16` shared in the branch delay slot by both clamp branches | write both clamps with the field first (`movement > limit`, `movement < -limit`) so the extension pseudo takes the same register in both arms and reorg can hoist the common first instruction | `actor_apply_random_movement` `0x8002f558` |

### Effect spawner (`actor_spawn_action_effect`)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `move s2,a0` then `andi s2,s2,0x1f` in place | the code parameter is an `int`; a `u8` parameter that is modified is spilled to a stack slot and reloaded (`sb`/`lbu`) | `actor_spawn_action_effect` `0x8002edd4` |
| `addiu s7,v1,-28` emitted after the loop-invariant stack stores, not at entry | use `actor_state.player_position.x` directly inside the loop; the loop optimizer hoists the field address into the preheader. A local pointer is computed at entry and allocated earlier | same |
| `subu a1,a1,v0; andi a1,a1,0xfff; jal; move s5,a1` | `s16 facing = (0x800 - actor->rotation.y) & 0xfff;` then pass `facing`: the halfword arithmetic loads the field with `lhu`, computes the value in the argument register, and the promoted local receives a copy in the delay slot. A `u16` or `s32` local computes into the callee-saved register and copies into `a1` | same |
| `addiu a0,a0,1000; subu a0,a0,v0` | `position.y - (player.y - 1000)`: fold's `split_tree` rewrites `ARG0 - (VAR - CON)` into `(ARG0 + CON) - VAR`, while `(position.y + 1000) - player.y` becomes `position.y - (player.y - 1000)` in RTL (`addiu -1000; subu`) | same |
| one clamp `slt; bnez; li 1; j; div; mflo` after the code-10 body, and the code-9 body ends with `j` back into it | a `goto` from the code-9 branch into a label inside the code-10 branch. Duplicated clamp source is cross-jumped the other way: `do_cross_jump` deletes the copy before the jump it is processing, and the pass walks forward, so the first block always loses its copy. The block swap in jump.c needs label-free ranges and cannot move a clamp with its own branch | same |
| `beqz s8` loop entry test, `li s8,1` in the delay slot, `li s8,2` conditional | `repeat = 1; if (code & 0x20) repeat = 2; for (i = 0; i < repeat; i++)` | same |
| jump table at `0x800124d4` for codes 5..24 (20 entries) | a `switch (effect_code)` listing cases 5, 7..13, 22..24 with an empty default; the table is claimed with `RODATA(0x800124d4, 0x50)` | same |
| `bnez s6 -> L; addiu v0,a0,-1500 (delay); addiu v0,a0,1500; L: sh` | `if (i == 0) offset.x += 1500; else offset.x -= 1500;` reorg hoists the else arm's instruction into the delay slot because the fallthrough overwrites it | same |

### Action dispatcher (`actor_update_current_action`, 90%)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `lbu; sh zero,18; sb v0,10` init idiom | `timer = 1; animation_id = a[k]; animation_phase = 0;` in that order: the scheduler fills the load delay with the independent halfword store. Phase first leaves `sh; lbu; nop; sb`, because a load cannot move above a store through a different base pointer (alias analysis assumes a conflict) | `actor_update_current_action` `0x8002fa88` |
| `lbu v0,21(s1)` hoisted above seven zero stores, `sb v0,10` last | the animation assignment is the second statement; the dependent `sb` becomes ready last and trails the independent stores | same |
| case 2: `beqz -> L0; beq 1 -> L1; j default` with bodies after | a nested `switch (actor->action_timer)` with `case 0`, `case 1`, `default`; the single `actor_advance_animation_wrapped` call sits after the inner switch and the blocked path leaves with `goto` to the vertical section. Duplicating the call per case merges the copies first and the compiler-made label then hides the shared aim call from cross-jumping (`jump_chain` only covers original labels) | same |
| case 33's timer==0 path jumps into the far branch's aim call | one `move(1, 0); wrapped(steps[1])` tail after the if/else chain; the near-home branch leaves with `break` | same |
| vertical switch through a 5-entry jump table | `case 0: break;` is listed explicitly: four cases stay below `CASE_VALUES_THRESHOLD` (5 without `casesi`) and compile to compares | same |
| cases 2..4 jump back into case 1's land/fall blocks | `goto land` / `goto fall` into labels inside case 1. Duplicated blocks always lose the earlier copy. Case 2/3 is `if (next_y >= floor_height) goto land; goto fall;`: the jump-around-a-jump inversion yields `beqz -> land; j fall`, and with `<` the inverted `bnez -> fall; j land` cross-jumps into case 1's branch | same |
| `beq -> A; beq -> B; j end; A; B` on `result & 0xffff` | a nested `switch (result & 0xffff)` with cases 0xfff0 and 0xfff1; the two-case dispatch is emitted ahead of the bodies | same |
| `beq v1,0x10 -> stagger` into the damage block | `goto stagger` to a label after the `player_apply_damage` call inside the 0x80 branch | same |
| `lh a1,54(s1)` / `lhu a1,60(s1)` passed without extension | `actor_try_attack_player`, `actor_advance_animation_clamped` and `actor_advance_animation_wrapped` are declared without prototypes in this unit, so halfword fields are promoted as `int` | same |
| `move a0,v0; sra v1,a0,16` for the vertical collision result | a separate local for the vertical query. Sharing one `result` across three sites gives it nine references and a higher priority than the shifted copy, which swaps `a0` and `v1` | same |
| `lui/addiu DAT_80055880+0x36` | `map_cell_attribute_height_table[attribute - 1]`; the reviewed relocation row names the table in `target_name` so the delinker measures the addend from it | same |
| residue: `lbu v0,5; ... sll v1,v0,5` (tile load in `v0`, product chain in `v1`) | ours loads the tile into `v1` and accumulates in `v0`, costing two `nop`s on the following `lh a0,14`. Every multiply spelling, named-local variant and the 2.6.0 probe allocate the same way: local-alloc ranks the in-place accumulator (4 refs in 2 insns) above the load. Unattributed | same |

### Map object placement loader (`map_object_pool_load`)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `sh zero,40(sp)` before `move s7,a0` | `u16 ended = 0;` initialised before a local copy of the parameter (`placement = placements`); the parameter's own pseudo stays in `a0` and the copy lands after the flag store | `map_object_pool_load` `0x80031008` |
| spill slots `remaining` 32, `ended` 40, `definition` 48 above the 8-byte output buffer at 24 | spilled locals take stack slots in declaration order (upward); declare `remaining`, then `ended`, then the definition pointer | same |
| `beq id,0xff -> tail; ... tail: j fill; sh ended` and the fill block `li 0xff; j advance; sb` | `if (ended == 1) { fill: object_id = 0xff; } else if (id != 0xff) { body } else { ended = 1; goto fill; }`; `continue` jumps to the loop head instead and duplicating the store loses the earlier copy | same |
| `lw 0(s2); lw 4(s2); sw 6(s0); sw 10(s0)` with `s2 = placement + 12` | `memcpy((u32 *)&object->link, (const u32 *)&placement->link, 8)` with the builtin declaration: word pointers give the MIPS aligned block move and the link address seeds the placement base register. A struct copy of the halfword-aligned link type emits `lwl/lwr`, and two separate word assignments cannot interleave their loads past the first store | same |
| `sb 0xff,14(s0)` between the height lookup and the height subtraction | `object->action = 0xff` written after the `position_y` statement; the scheduler hoists it over the final subtract and store | same |
| `if (remaining-- == 0) break;` at the bottom, entry count 189 | a `for (;;)` with the post-decrement test, giving 190 iterations | same |

### Map object action update (`map_object_pool_update`, 99.1%)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `addiu v0,s7,-1; move s7,v0; sll v0,v0,16; bnez` loop test | `s16 count` in `for (count = 190; count != 0; object++, count--)`: the signed halfword test folds to one shift, a `u16` counter emits `andi` | `map_object_pool_update` `0x80031cc8` |
| `lhu v0; addiu v1,v0,1; andi s1,v0,0xffff; sh v1` | `elapsed = object->action_timer++;` into a halfword local of its own; case 0 instead reads the timer into `timer` first and increments later after the pair lookup | same |
| `lbu a1` (case 96), `lbu a0` (case 97) for the floor grid byte | one block-scoped `s32 attribute` per case, read before the position update. A single function-scope local shares one register; an inline expression after the stores is not hoisted above them | same |
| `lh a0,-2(s0)` once, `addiu v0,a0,160` / `-160`, shared `andi; sh` | `tilt = object->rotation.x;` then each arm assigns `object->rotation.x = (tilt +/- 160) & 0xfff`; the earlier arm's tail cross-jumps into the later one | same |
| case 80: `bnez timer -> tail` with the decrement after the inner switch | `if (timer == 0) { ... } else { timer--; }` rather than an early `timer--; break;` | same |
| `lui s1; addiu s1` address kept across three calls for the stair counter | `counter = &DAT_8009eafc;` and `*counter` accesses; a plain global is re-addressed at every use | same |
| `lui/addiu` for both door sounds, never folded from one another | the sound refs at `0x80056188` are thirteen separate `SoundRef` objects (`gameplay_sound_ref_N`); with one array CSE derives the second address from the first | same |
| residue: `lbu; li; la B; bne; la A` for the door sound | ours schedules `la B` above the compare operands with every spelling (if/else, ternary, default-then-override); only `-fno-schedule-insns` keeps retail's order, which breaks the rest. Unattributed | same |

### Player combat stats and equipment

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `id*28 + 0x800a00dc` for every armor slot | `&armor_records[id - 13]`: the 1176-byte block at `0x800a0248` is 42 records of 28 bytes for item ids 13..54, and the relocation rows name `armor_records` so the addend is measured from it | `player_recalculate_combat_stats` `0x80015714`, `player_set_equipment_slot` `0x80016848` |
| `lbu 0(rec+2)` added twice into `damage_defense_component0` | two `+=` statements of the same field, as retail does for every armor slot | `player_recalculate_combat_stats` |
| `la v1,field; lhu v0,0(v1); addiu; sh v0,0(v1)` shared store for the accessory cases | a `switch` with cases 48..52 and 42 each doing one `+=`; the stores cross-jump into a single `sh v0,0(v1)` | same |
| `lbu -12704(v0)` / `la v1,-12584; lbu 0(v1)` for the milestone flags | bytes 0, 20, 80 and 120 of the 20-byte magic record table `DAT_8009ce60[]`; the relocation rows name that base | same |
| `bne timer,-1 -> clamp; ...; la 1000` placed after the clamp | `if (timer != -1) { if (timer < 970) timer = 970; } else { timer = 1000; }`; the `== -1` first form lays the constant inline | `player_apply_damage` `0x80016324` |
| `addiu s1,s1,5; div; mult; sra s1; mult; div; mflo v1` | `damage += 5; damage = (scale * (damage / 10)) >> 12; loss = (multiplier * damage) / 10;` with `loss` a separate local used for the compare and the subtraction | same |
| `(rand() * 100) >> 15` | the multiply is by 100, spelled `sll 1, addu, sll 3, addu, sll 2` | same |

### Item use (`player_use_item`)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| key search falls into the lever search when exhausted | `case 53..74` runs its `for (;;)` search and falls through into `case 56..69`, which restarts from the last index (-1) | `player_use_item` `0x80018054` |
| `bne id,89 -> link test; angle test; beqz -> next` with `li s2,1` in the branch delay slot | `else if (object_id != 89 || angle_within_tolerance(...)) { used = 1; if (link == item) ... else func(4); }` | same |
| `move s1,zero` in the first `jal rsin` delay slot | `index = 0;` assigned after the reach computations; the scheduler hoists the constant move above the calls into the load delay | same |
| saved registers from `sp+32` with `addiu s1,sp,24` | the cone searches take an `s32 *distance` out-parameter: one word local rounded to 8, not a vector | same |
| the two jump tables with a zero word between them | one `RODATA(0x80012048, 0x130)` claim spanning both tables and the padding | same |

### Horizontal movement (`player_move_horizontal`, residue)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `move s7,a0` and `move a0,s7` for every use of the heading | the parameter is `s32`; an `s16` parameter is re-extended and spilled | `player_move_horizontal` `0x800171fc` |
| `sw t4,32(sp)` / `sw t4,40(sp)` word spills of the two map cell bytes | `s32` locals loaded from the `u8` fields | same |
| `move s1,v0` after `vector_xz_to_angle` | the yaw returns `s32` in this unit; a `u16` return truncates with `andi` | same |
| `la t4,collision_target; addiu s8,t4,24`, deltas as absolute `lhu`, re-target as `lw -16(s8)`/`lw -24(s8)` | plain global accesses reproduce this: the hoisted invariant is the radius address and CSE folds the position loads onto it | same |
| residue: `la s0,player_state+202; lbu 0(s0)`, then `addiu s5,s0,-22` as the base for the camera position, floor height and map cell through the loop and after it | every probe (global accesses, a `KfPlayerState *` pointer, a map-cell pointer, a floor-height pointer, the pointer assigned before or inside the loop) either folds the pointer into absolute addresses or anchors the base at another offset; the register permutation and the two halfword delta spills follow from that. Best result 54% with direct accesses. Unattributed | same |

### Player frame update (`player_update`)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `sw s5,player_previous_input; j` past a second `sw s5` that precedes `jal player_update_vertical_motion` | the item-use branch and the movement branch each store the held input; the movement branch also calls `player_update_vertical_motion()`, so the tails differ and cross-jumping cannot merge the stores | `player_update` `0x80018880` |
| `slt v1,forward,-limit; bnez` with the clamp store as the branch target | `if (forward >= -limit) field = forward; else field = -limit;` for the backward/left cases; the forward/right cases keep `if (forward > limit) field = limit; else field = forward;` | same |
| `lh s0,0(s3); mult s0,s0; mflo s0` | `strafe_sq = player_state.motion_state.strafe_velocity; strafe_sq *= strafe_sq;` (load into the accumulator, square in place); direct member access, no pointer local | same |
| `mflo a0; lh; bgez; move s2,a0; negu s2,a0` | `strafe = strafe_sq / magnitude; if (strafe_velocity < 0) strafe = -(strafe_sq / magnitude);` (the second division is CSE'd; a separate `if/else` keeps two divisions) | same |
| `beq id,0xff` jumping to a second `lbu id; beq id,0xff` instead of past it | `magic_id = player_state.selected_magic_id; if (magic_id != 0xff) { charge }` for the guard on the button path: jump threading refuses when either compared register is a user variable (`REG_USERVAR_P` in `rtx_equal_for_thread_p`); a field re-read is threaded through | same |
| `li s2,1` in the mp-check delay slot, `bne v0,s2` / `beq v0,s2` on `unknown_78`, and the current-mp anchor in `s1` | no `attachment = 1` statement: the constant 1 for the two compares lives in a temporary that shares `s2` with the later `attachment`; an explicit initialisation extends `attachment` over the anchor's range and swaps `s1`/`s2` | same |
| `addiu a0,s1,142` in the `beq effect,5` delay slot | `origin = &player_state.camera_position;` computed before the effect test and passed only to the 0x155 cone search; the 0x555 search spells the address itself (`addiu a0,v0,-22` from the yaw anchor) | same |
| `lhu v1,56(sp); addiu v1,v1,-4; addu v1,v1,v0` after `rand`; `sra v0,v0,9` | `angles.x -= 4 - (rand() >> 9);` (`angles.x - 4 + (rand() >> 9)` and `+= (rand() >> 9) - 4` both fold the constant onto the random term) | same |
| `beq timer,-1; move v1,v0; ... li v0,-1; j clear; sh v0,0(a0)` then `sh; sll; sra; bne` | `if (!(flags & bit)) { timer = -1; flags &= ~bit; } else { timer--; if (timer == -1) flags &= ~bit; }`; the duplicated clear is cross-jumped and the copy comes from the post-decrement | same |
| `sh zero; j clear; ... bnez timer,check600; clear: ...; j recalc` | `if (!(flags & 1)) { timer = 0; clear; recalc } else if (timer == 0) { clear; recalc } else if (timer == 600) recalc; timer--;` | same |
| retail frame 224 with `direction` at sp+40 (ours 216, sp+32) | retail declares one more memory-resident local of at most 8 bytes before `direction`; nothing references it. Not reconstructed (an unreferenced placeholder would be artificial padding) | same |
| residue: `lhu v0,42(v0)` and `la v1,player_state+42` for the status flags next to a `la a0,player_state+78` timer anchor | ours relates the flags address to the timer anchor (`lhu v1,-36(a1)`). cse's `use_related_value` links every `player_state+K` constant in the block; GCC 2.5.7 and 2.6.0 both relate here. Unattributed | same |
| residue: `bgez; nop; li v0,0x20` and `bltz; nop; ...; sll s0,v0,7` | ours fills both delay slots from the fallthrough. Same RTL shape; unattributed reorg difference | same |
| residue: `lw a0,142(s1)` for `camera_position.x` and pitch/roll loads after the position stores | ours loads `camera_position.x` into `v1`, which frees `a0` for the pitch load and lets sched2 hoist the angle stores above the position stores. Register choice, unattributed | same |

### Floor entry warp and cone distance (`player_warp_to_floor_entry`, `player_distance_to_point_in_cone`)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `bne mv,3 -> L; nop; jal audio_play_current_map_sequence; L: jal func_80020a2c` | only the sequence call is conditional on `map_variant == 3`; `func_80020a2c()` runs on both paths. The earlier reconstruction nested both calls, which also let reorg copy the target's `li v0,1` into the branch slot | `player_warp_to_floor_entry` `0x80017cf8` |
| `beq distance,-1 -> ret; move v0,s1` and the epilogue starting with `move v0,s1` | one `return distance;` at the end with the cone test nested under `if (distance != -1)`; reorg copies the epilogue's move into both branch slots. An early `return distance;` yields a separate return block and an empty slot | `player_distance_to_point_in_cone` `0x80017108` |
| residue: `li v1,1; bne v0,v1; nop; li v0,-1` with the call result never copied out of `v0` | every spelling tried (`if (r != 1) return r; return -1;`, the flipped test, `if (r == 1) r = -1;`, `?:`, `goto`) copies the result to another register first because the constant 1 takes `v0` in local-alloc. Unattributed | `memory_card_show_status_message` `0x8002c510` |

### Display initialization (`func_8001bb94`)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| `env[1].dtd` stored before `env[0].dtd` | `display_draw_environments[0].dtd = display_draw_environments[1].dtd = 1;` — the chained assignment evaluates the constant, stores the right operand (env[1]) first, then env[0] | `func_8001bb94` `0x8001bb94` |
| residue (94.25%): retail keeps `s0 = &display_draw_environments[0].dtd` callee-saved and derives `display_disp_environments[0]` as `s0+162` (PutDispEnv arg) and `render_state.fog_near_distance` as `s0+18538` | the retail object carries no relocation at the PutDispEnv-arg or fog-store sites — the linked bytes are `addiu a0,s0,162` / `sw v0,18538(s0)` — so the three were one combined declaration. Merging them into the graphics-context aggregate (see that section) and migrating this unit reproduces the base and lifts `94.25% -> 98.42%`; the last residue is that the probe drops `$s0` after `PutDispEnv` and re-materialises the far fog store with a fresh `lui` instead of keeping the base callee-saved across the four trailing calls. Unattributed regalloc/CSE residue | same |

### DMA/IRQ handler registration (`system_callbacks` TU)

| Retail evidence | Source shape | Function |
| --- | --- | --- |
| ten `func_8004f*` bodies each `move a1,a0; jal <dispatcher>; li a0,<n>` | thin wrappers `void f(u32 h){ dispatcher(<n>, h); }` over two dispatchers (`func_8004fc2c` for DMA channels 0-5 via DPCR at `*DAT_80057d1c`; `func_8004fcac` for IRQ events via IMASK at `*DAT_80057d14`) | `func_8004fac4`..`func_8004fc08` |
| enable path loads `*ptr`, then `\| 0x800000`, then `\| (0x101 << (ch+16))`, one store | compound `*DAT_80057d1c \|= 0x800000 \| (0x101 << (channel + 16));` — the compound form loads once and ORs in source order; the expanded `*p = *p \| A \| B` reschedules the shift ahead of the load | `func_8004fc2c` `0x8004fc2c` |
| `slot = &table[ch]; *slot = 0;` cleared in the branch delay slot, set to the handler at the end of the enable path | a pointer local to the table element written 0 before the `if`, then `= handler` inside the enable arm | both dispatchers |
| residue: retail computes the table base (`lui/addiu`) before the index `sll`, loads `*ptr` before materializing `0x800000`, and lays the enable arm as the fall-through | ours schedules the index shift first and orders the IMASK/DPCR arms differently. Semantic order, call set, referents and CFG agree; the residue is R3000 instruction scheduling and basic-block layout. Unattributed | `func_8004fc2c` (69%), `func_8004fcac` (33%) |

### Recurring residue: callee-saved register permutation

Several structurally-exact functions differ from retail only in *which*
callee-saved register (`s2`..`s5`) each call-surviving local/parameter receives
— the instruction stream, order, referents, and call set all match. Observed on:

| Function | Detail |
| --- | --- |
| `func_80014314` (POLY_FT4 sprite builder, 82.2%) | retail `s2=texcoords, s3=clut, s4=ot_index, s5=tpage`; ours permutes them. `s1=position`/`s6=color` agree. |
| `func_8003a8fc` / `func_8003a81c` (hex/decimal formatters) | retail `a1=buffer, a2=index`; ours swaps them. Three declaration orderings gave the same result. |

The permutation is stable across source reorderings and declaration-order
changes, so it is not steerable from C. Because `local-alloc` assigns
callee-saved registers by an internal priority/qty ordering, a systematic
permutation across many functions is a candidate signal that the probe
compiler build or its register-allocation behaviour differs slightly from the
original toolchain (see the open compiler-attribution question). Worth a
controlled probe before treating these as closed. Unattributed for now.
## map interaction

Witnesses come from `src/game/map_interaction.c` (`game.map_interaction`, the
contiguous band `0x800346a8..0x800356e8` bracketed by the `func_800346a0` and
`func_800356e8` stubs). This is the per-frame nearby-event/object interaction
dispatcher `func_80034de4` called by `player_update`, plus its scripted
transition cutscene (`map_floor5_transition_cutscene`), a trigger latch
(`func_80034a34`), a talk/progress-image dispatcher (`func_80034a80`), and a
floor-image loader (`func_80034d54`).

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `lw v0,8; lw v1,12; lw a0,16; lw a1,20; sw x4` then `lw v0,172(sp); addiu -600; sw` (the field reloads after the block store) | `struct KfVec4i spawn = *(struct KfVec4i *)&effect->position_x; spawn.y -= 600;` — the aligned 16-byte struct copy leaves the members in memory, so the later `spawn.y` read reloads; four separate `words[k] = field` assignments keep the value in a register and subtract in place | `map_floor5_transition_cutscene` `0x800346a8` |
| `beqz stage,A; beq stage,s4,B; j C` three-way dispatch with `A`,`B` laid out after the test and `C` the common tail | `switch (stage) { case 0: A; break; case 1: B; break; }` then the shared tail `C`; an `if (stage==0)…else if (stage==1)` inverts the first test (`bnez`) and inlines `A` | same |
| `lbu v1,grid; sll v0,v1,1; …` with no `andi 0xff` before the `*100` chain | read a `u8` grid byte into an `s32` local; a `u8` local re-masks with `andi` before the multiply | same |
| `lw v1,map_event_pool+0x4c; li a0,-256; and; lui/ori 0x28010500; bne` (one masked word compare of four adjacent bytes) | `(*(u32 *)&map_event_pool[1].image_limit & 0xffffff00) == 0x28010500` — a word pun of the image_limit/index/dirty/delay bytes; three byte compares never fold to one `lw` | `func_80034a34` `0x80034a34` |
| `lbu v0,flag; addiu v0,v0,-1; sb v0,flag` (a byte decrement as `addiu -1`) | `DAT_800652a8[k]--;` (post-decrement); `DAT_800652a8[k] -= 1;` emits `addiu +255` on the promoted unsigned byte | `func_80034a80` `0x80034a80` |
| cases whose successful branch ends `refresh(); j epilogue` while the failing/other cases fall into a shared tail | `case N: if (cond) { …; refresh(ev); return; } break;` with the tail after the `switch`; a `break` on the success path routes through the tail instead of the epilogue | same |
| `addu v0,event,image_index; lbu v0,2(v0)` guard, then `talk(floor, image_index, kind, image_dirty)` with `a1` reused from the guard load | `if (event->tag.bytes[event->image_index - 1] != 0) talk_show_indexed_image(floor, event->image_index, event->kind, event->image_dirty);` — the guard's `image_index` load stays live in `a1` for the call | same |

Residues recorded in the module (not steered):

- `func_80034d54` `0x80034d54` (73.7%): retail forms the screen-image path base
  `DAT_80056238` by registering `&DAT_80056238[5]` (the floor-digit store, whose
  value needs a global load) and deriving the `screen_show_image_until_input`
  argument as that pointer `- 5`, leaving `[8]`,`[9]`,`[10]` as absolute
  `lui/sb`. The probe instead registers `&DAT_80056238[8]` (the first store) and
  derives the base as `- 8`, which also pushes the `mfhi` remainder from `v1`
  to `a2`. Every store order and buffer spelling anchors on the first store;
  this is the same unattributed anchor residue recorded for
  `talk_show_indexed_image` `0x8002c9d4`.
- `func_80034de4` `0x80034de4` (partial): the 0x904-byte per-frame interaction
  dispatcher is not yet fully reconstructed. It computes a spatial-audio source
  point twice (`rsin/rcos` with the 1500 and 1000 multipliers), switches the
  `map_cell_attribute_grid` cell attribute, then either processes an overlapping
  `map_event_pool` region (a three-way state machine on the event's
  `unknown_0e`, one arm of which indexes an unidentified pointer table at
  `0x80090fe4` addressed relative to `DAT_80095088`) or walks the map-object
  pool with `map_object_pool_find_interaction_from`, dispatching each object on
  an 84-entry `switch (map_object_state.definitions[object_id].behavior_type)`
  jump table (`RODATA 0x80012a7c`), and finishes with a five-way
  `switch (current_floor)`. Retail's 72-byte frame saves `s0..s8` and spills the
  sound point to the stack; the frame and register allocation cannot be matched
  until every case body is reconstructed, so exact remains open.
## item / shop menu

Witnesses come from `src/game/item.c` (`game.item`, the contiguous
`0x80020b4c..0x800222b4` run: the floor-item loader, the item-database loader,
and the shop buy/sell/use menus).

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `sll x,5; subu; sll 2; addu; sll 4` for `tile*2000`, `sll 1; addu; sll 3; addu; sll 2` for `height*100` | `u8 tile` and `u8 grid[z][x]` multiplied by the plain constants; GCC strength-reduces `*2000` and `*100` itself | `func_80020b4c` `0x80020b4c` |
| a windowed grid loads the later field of the record last (`addiu s0,ptr,10`) and the store cursor last field (`addiu s1,dst,20`) as the derived giv bases | reference the highest-offset field last in the loop body, and advance `dst`/`src` (the giv whose init fills the guard branch delay slot) before the lower-offset one | `func_80020b4c` |
| `li s4,1` (confirm flag) hoisted into the `slti cursor,2; beqz` clamp branch delay slot | set the flag unconditionally at the head of the confirm branch (`confirm = 1;`) before the `if (cursor < N)`; a flag set only inside the taken arm leaves the slot a `nop` | `func_800212d8` `0x800212d8` (exact) |
| four-way `if ((pad & BIT) && !(prev & BIT))` edge tests that fall through to the next direction when a button is held | one `else if` chain with the full `&&` edge condition per direction; a nested `if (pad & BIT) { if (!(prev & BIT)) ... }` skips the remaining directions on a held button | `func_800212d8` (exact) |
| a yes/no prompt frame 96 with the two menu strings at `sp+16` and `sp+40` | `s16 prompt[12]; s16 options[12];` fixed-size menu-string buffers (only the leading cells and the `-1` terminator are written) | `func_80021ffc` `0x80021ffc` (exact) |
| the can-use guard returns `1` through the shared epilogue with `li v0,1` in the branch delay slot, not the `-99` result | `if (!usable) return 1;` as a distinct constant, while the accept/decline paths `return result` | `func_80021ffc` (exact) |

Residues recorded in the same module (not steered):

- `func_80020b4c` `0x80020b4c` (89.5%): retail copies the `placements`
  parameter to `s2`, saves it in `a1`, and re-materialises `&floor_item_count`
  through `a0` then `v1` inside the count loop (two extra coalescing moves); the
  probe keeps `placements` in `a0` and holds the address in `a1`, a strictly
  cheaper allocation. Referents, relocations, call set, CFG and the whole second
  loop match.
- `func_80020cfc` `0x80020cfc` (96.1%): the three-digit filename division
  allocates `i / 30` to `v1` and reuses the `i + 1` register for `(i+1) / 100`;
  retail puts `i / 30` in `a2` and takes a fresh register for the second
  quotient. Every division, dividend order and named-local variant tried keeps
  the same class assignment. Everything else (the `memcpy` stat-bank loader, the
  sector rounding) matches.
- `func_80021538` `0x80021538` (62.3%) and `func_80021afc` `0x80021afc`
  (67.5%): the buy/sell panels reconstruct with correct referents, call sets and
  control flow, but two residues remain across the ~0x5c0 bodies. (1) The
  callee-saved assignment of the menu variables permutes against retail
  (`selection`/`confirm`/`inventory_base` land in different `sN` than retail's
  `s1`/`s5`/`s6`). (2) `loop.c` relates the two item icon tables
  (`DAT_80059108 == DAT_80058dc0 + 840`) into one base register that reaches the
  second with a `-840` addend; the probe materialises each table symbol
  separately. Both are allocation/loop-reduction classes, not structural errors.

## map events

Witnesses come from `src/game/map_events.c` (`game.map_events`, the contiguous
band `0x80035708..0x80035e14`): the per-frame `map_event_pool` driver
`func_8003596c` called by `game_main_loop`, its two event updaters
`func_80035708` (grid-wander AI) and `func_800358e0` (rotating sound emitter),
and the per-floor state serialiser `func_80035b5c`. `func_8003596c`'s
current-floor dispatch jump table lives out of line at `RODATA(0x80012be4, 0x14)`
(five entries); the census carried it as `fragments = 2` because the indirect
`jr` hid the case bodies, corrected to one 0x1f0 body.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `sw v0,32(sp)`/`sw v0,40(sp)` for the query point then `lw` reloads at every use, only `s0` saved | the collision point is a `struct KfVec4i point` local (`point.x`, `point.z`); aggregates stay in memory, so the members spill and reload, where scalar `s32` locals would occupy `s1`/`s2` | `func_80035708` `0x80035708` |
| `sh v0,54(s0); sll/sra v0; jal angle_to_forward_xz` (the stored angle reused, not reloaded) | `s16 heading = angle_approach(...); event->rotation = heading; angle_to_forward_xz(heading, &forward);` — a local carries the result to both the store and the sign-extended argument; `rotation`/`rotation_target` are `s16` so the field loads are `lh` | same |
| `beq unknown_0e,s1 -> body1; li 2; beq unknown_0e,2 -> body2; j cont; body1; body2; cont:` | `if (event->unknown_0e == state) goto call1; if (event->unknown_0e == 2) goto call2; goto cont; call1: ...; call2: ...; cont:` — explicit `goto`s to out-of-line bodies reproduce the tests-first layout and reuse `state` (`s1`); an `if`/`else if` chain emits the inline `bne`-skip form | `func_8003596c` `0x8003596c` |
| `slt v0,image_dirty,limit` (signed) on two `lbu` byte values | `s32 limit = event->tag.bytes[event->image_index - 1];` then `event->image_dirty >= limit`; a `u8 limit` makes GCC 2.5.7 compare `u8 >= u8` unsigned (`sltu`) | same |
| `lui/addiu a0,&DAT; lhu v1,0(a0); addiu v0,v1,-1; bnez v1; sh v0,0(a0); li 3; sh v0,0(a0)` (one address register, original value tested) | `u16 *gate = &DAT_8009ddb0; u16 current = *gate; *gate = current - 1; if (current == 0) *gate = 3;` — the pointer local keeps `&DAT` in one register across the three accesses; `DAT_8009ddb0-- == 0` on the global re-materialises the address each time, and the same idiom through the pointer masks the decremented value (`andi`) rather than testing the original | same |
| `u16 count = 7; do { ... } while (index-- != 0)` promoted with `move; andi 0xffff` before the test | the eight-slot pool walk; the direct-global sibling `DAT_8009ddb2-- == 0` keeps the clean `bnez original` form (no pointer, no mask) | same |

Residues recorded in the module (not steered):

- The map-event globals are one unmodelled BSS aggregate: `map_event_pool`
  (`0x8009db88`), `current_map_event` (`0x8009dda8`), `DAT_8009ddb0`/`b2`/`b4`,
  and the per-floor save records at `0x8009d71a` share one base register in
  retail. `func_800358e0` `0x800358e0` (73%) reaches `&map_event_pool[0]` and
  `&map_event_pool[0].reference_x` as `&current_map_event - 544` / `- 508`;
  separate globals emit their own `lui/addiu` pairs. Modelling it needs the
  aggregate struct that the data inventory still leaves unresolved, so the unit
  keeps clean symbols and takes the residue. `func_80035b5c` `0x80035b5c` (84%)
  is addressed through the `DAT_8009ddb4` base (matching retail's single
  register for the pool, the event-field iterator, and the floor buffer), but
  its actor/map-object serialisation loops keep the `active` counter and the
  filter constants in permuted temporaries (`t0`/`t1`), derive
  `map_object_state.definitions` from a fresh symbol rather than the objects
  base register, and schedule the leading base `addiu`s differently — the
  register-permutation and IV-strength-reduction residue class over a large
  leaf.
## hub menu and item-use

Witnesses come from `src/game/menu.c` (`game.menu`, the contiguous
`0x800222b4..0x80022d7c` run: a save-buffer splash, the eight-row hub-menu
dispatcher, and the consumable-item panel). This band sits directly after
`item.c` and shares its list-widget API (`func_8002ad6c` init,
`func_80028a70` render, `func_80028380` query, `func_8002aea4` guard) and the
`KfItemMenu` context; the panel mirrors `func_80021afc`.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `addiu v0,sp,656; sw save_payload_buffer; addiu v0,sp,16; sw save_header_buffer` then a 3-iteration draw loop | point the shared workspace pointers at frame-local `KfSaveHeader`/`KfSavePayload` locals (0x280 at +16, 0x2580 at +656), then `i=0; do { i++; draw; } while (i < 3);` — the post-increment fills the first call's delay slot | `func_800222b4` `0x800222b4` (exact) |
| jump-table `switch (selection)` where case 0 `j`s into case 5's join tail | `case 0: result = f(); goto join; ... case 5: result = g(); join: if (result == -1) result = -99; break;` — cases 0 and 5 must share the `== -1` fold explicitly; per-case duplication defeats cross-jumping | `func_80022348` `0x80022348` |
| the out-of-range `sltu 6,selection; bnez` fills its delay slot with `li v0,-99` (the join compare constant), never storing `result` | omit the `default:` label entirely; the out-of-range path leaves `result` at its prior value and reaches the shared `if (result == -99)` test, whose `-99` is the hoisted `v0` | `func_80022348` |
| a running name pointer based at `DAT_80059108` advanced 20 bytes per code (even on skipped codes) | `s16 *name = DAT_80059108; for (code=0x2a; code<0x30; code++, name += 10) { ... labels[found][j] = name[j]; }` — the explicit giv keeps the table symbol as the base; `DAT_80059108[(code-0x2a)*10+j]` folds the offset into a `DAT_80059108-840` base instead (90.7% -> 92.8%) | `func_80022608` `0x80022608` |
| `sltu maximum,current; ... sh current` with `&current_hp` registered for the compare-load and the clamp store | write the clamp with the current value first (`if (vitals.current_hp > vitals.maximum_hp) vitals.current_hp = vitals.maximum_hp;`) so the twice-touched member (load + store) wins the registered base; `maximum < current` registers `&maximum_hp` instead (90.1% -> 90.7%) | `func_80022608` |
| restorative effects apply in place and clamp afterwards | `if ((u32)(code-0x2a) < 6) { inv[code]--; if/else on 0x2b..0x2f adds 25/10/80/150/300 to `vitals.current_hp` and masks `status_effect_flags` with 0xb/0x3/0; then two `current > maximum` clamps; }` — the `sltu` field compares come from the `lhu` zero-extended u16 members | `func_80022608` |

Residues recorded in the module (not steered):

- `func_80022348` `0x80022348` (96.6%): retail promotes the loop-invariant
  constant `-1` to a callee-saved register (`s6`, adding an eighth save slot so
  `ra` lands at 44) and compares `selection`/`result`/the case-1 result against
  it; the probe re-materialises `li v0,-1` (or reuses the `result` pseudo) at
  each site. The extra save shifts every branch offset and cascades into the
  case tails' delay-slot fills. Referents, relocations, call set and CFG match.
  Same callee-saved-constant class as `actor_update_awareness`.
- `func_80022608` `0x80022608` (92.8%): the consumable panel reconstructs with
  correct referents, calls, constants and control flow, but the caller-saved
  allocation permutes against retail — the two name givs land in `t2` where
  retail uses `t1`, the `-99` selection sentinel in `v0` where retail keeps it
  in `s2`, and `s3`/`s4` initialise in the other order. Retail also keeps a
  `player_state` base register live across the effect switch and the full-heal
  case (`&current_hp` reused for the `+= 300` load/store), while the probe
  re-forms `lui/addiu` per access. Both are the allocation / registered-base
  classes shared with `func_80021afc` and the documented `player_state`
  residues, not structural errors.

## map load and per-floor world-state restore

Witnesses from `src/game/map_load.c` (`game.map_load`, band
`0x80035e44..0x800365f8`). `func_80035e44` is the exact deserialise inverse of
`func_80035b5c` (`map_events.c`): it reads the same 1700-byte per-floor record
(`base - 1690 + 1700 * current_floor`, `base = &DAT_8009ddb4`) and rebuilds the
eight `map_event_pool` slots (reached as `base - 556`), the live-actor lifecycle
overrides, the 190 `map_object_state.objects` ids, the linked-object payloads,
and the two effect-object pools `objects[160..169]` / `objects[170..189]`; the
common tail dispatches a per-floor script through a five-entry jump table.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `sb v0,0(a1); sb v0,-5(a0); ...; lbu v0,-4(a0); addu v0,a1,v0; sb v1,2(v0)` (a store whose offset is a just-read field) | mirror the serialiser field-for-field: `event->image_index = *in++; event->tag.bytes[event->image_index - 1] = *in++;` — the `+2` is `tag` at struct offset 3 minus the `-1` index | `func_80035e44` `0x80035e44` |
| effect record position rebuilt as `cell*2000 + ((rand()*2000)>>15)` with a `-height*100` y | `object->position_x = object->cell_x * 2000 + ((rand() * 2000) >> 15);` etc., `object->position_y = -(map_floor_height_grid[cell_z][cell_x] * 100);` — the u16 cell members drive the `*125<<4` strength reduction | same |
| two `sh` to `&object->link` (a dead `lo` store then `lo|hi<<8`) | `*(u16 *)&object->link = *in++; *(u16 *)&object->link |= *in++ << 8;` — the reserialised low byte then the OR keeps `lo` in-register (no reload) | same |
| a five-case `switch (current_floor)` compiled through a `sltu 4; lw table; jr` casesi table at `0x80012bfc` | `switch (player_state.progress_state.current_floor) { case 1..3,5: ...; default: break; }` claimed with `RODATA(0x80012bfc, 0x14)`; floor 4 falls to the empty default (table slot 3 = end) | same |

Reconstruction notes:

- The whole function is symbolic, not numeric: every table base relocates to its
  owning global exactly as `func_80035b5c` does (`map_event_pool` as
  `&DAT_8009ddb4 - 556`, `actor_state`/`map_object_state`/`map_floor_height_grid`
  by their own symbols). The candidate census had not seeded these hi16/lo16
  pairs; they are curated `manual:game_map_load`.
- The delinker's `decoded-target-mismatch` withhold is the reliable check on a
  hand-computed reloc target: it caught four transcription slips in the floor-5
  script (`-1980`->`DAT_8009f844`, `-9268`->`map_event_pool[1].state`,
  `-1978`->`DAT_8009f846`, `21170/21171`->`DAT_800652a8[0xa]/[0xb]`), which also
  fixed the four source reads.

Residue recorded (not steered):

- `func_80035e44` `0x80035e44` (90.2%): referents, relocations, call set, CFG,
  types, constants and the `-40` frame all match; the divergence is the
  documented callee-saved permutation (retail carries the loop index in `s2`,
  the probe in `s3`, cascading through the six restore loops), the count-loop
  idiom (`n-1; do {} while (--n != -1)` vs the probe's `while (n-- != 0)`, which
  spills and grows the frame when forced), and the loop-optimiser's giv-base
  choice for the effect records (retail bases the induction register on the last
  field so the members sit at negative offsets). Same class as the
  `func_80035b5c` single-base-register residue in `map_events.c`. `func_800364e0`
  and `func_80036554` in the same TU are exact.

## menu sub-panels (map / magic / options)

Witnesses come from `src/game/menu_panels.c` (`game.menu_panels`, the contiguous
`0x80022d7c..0x800238d8` run directly after `menu.c`): the map-item viewer
`func_80022d7c` (drawn from the item panel), the magic-cast panel
`func_8002317c` (hub option 1), and the option sub-menu dispatcher
`func_800236ac` (hub option 2). The unit owns `RODATA(0x800122e4, 0x2c)`: the
`"MAP\M00."` path literal followed by `func_800236ac`'s eight-entry switch table.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `lui s3,&bank+120; addiu s5,s3,-120` hoisted into the preheader, each `AddPrim` target formed as `buffer*160 + (s3 - k*40)` | model the untyped disc-loaded sprite bank as a typed 2-D array `POLY_FT4 DAT_800580e8[2][4]` and index `&DAT_800580e8[buffer][k]`; loop.c hoists `&bank[0][3]` (the highest element, +120) as the giv base and derives the lower rows by subtraction. A `(POLY_FT4(*)[4])` pointer local keeps the +0 base and adds, and never hoists | `func_80022d7c` `0x80022d7c` (exact) |
| a shared `j loop; addiu frame,1` reached from the `frame<2` and `frame==2` arms, `frame>2` reaching the loop head without the increment | `if (frame < 2) { frame++; } else if (frame == 2) { wait; frame++; } else { if (poll()==0) continue; wait; return; }` — an explicit per-arm `frame++`; a single trailing `frame++` after the `if` chain leaves a dead cross-jumped `j loop; addiu` copy | same |
| `move s2,zero` (the frame counter) scheduled as the second instruction, ahead of the `char path` literal copy and its `memset` | declare `s32 frame = 0;` *before* the `char path[16]` string local: GCC emits the counter's `li 0` at function entry only when its initializer precedes the block move; declared after, the scheduler sinks it below `memset` | same |
| `char path[16] = "MAP\\M00.";` -> 9-byte `lwl/lwr`+`lb` copy then `memset(path+9, 0, 7)` | the 9-byte string literal is carved by the `RODATA` claim as a `.rodata`-local reference (addend 0), matching for a literal that begins the section | same |

Delinker/reloc curation learned here:

- `func_8002317c` reads `magic_records[selection].mp_cost` twice (the affordability
  test and the deduction). Both are raw `lui 0x800a; addiu -12688` in the linked
  image; the census carried no rows, so both need hand-added `mips_hi16_lo16`
  pairs to `magic_records` (`+16`). The delinker withholds a pair as
  `instruction-pair-mismatch:HI site is not LUI` when `site_va` is put on the
  preceding `lhu` instead of the `lui` — the HI site must be the exact `lui`.
- The map sprite bank base is likewise unrelocated in the census: `func_80022d7c`
  forms `&DAT_800580e8[0][3]` as `lui 0x8006; addiu -32416` with no row. Adding
  the `mips_hi16_lo16` pair to `DAT_800580e8` (`+120`) resolves it once the
  source hoists the same `+120` base.

Residues recorded in the module (not steered):

- Jump-table `.align 3` (toolchain, `func_800236ac`, blocks exact): `cc1psx-257`
  emits `.align 3` before the switch jump table, so with the `"MAP\M00."` string
  (9 bytes) leading the unit's one `.rodata` claim the table lands at offset 16.
  The retail object placed the table at offset 12 (its `.rodata` starts at the
  4-mod-8 address `0x800122e4`, so `.align 3` and `.align 2` coincide there, but
  its section alignment of 4 proves the original used `.align 2`). Referents,
  calls, CFG and register allocation otherwise match; the switch load's addend
  (`+16` vs `+12`) and the four `.rodata` padding bytes are the only residue.
  Unattributed — the same open compiler-build question as the register
  permutation class, not steerable from C.
- `func_800236ac` also leaves `selection = -1` in the `jal func_8002abb4` delay
  slot where retail fills the preceding `bne result,-99` delay slot with it; the
  reorg heuristic differs when a call immediately follows the reset store.
- `func_8002317c` `0x8002317c` (95.9%): the magic panel clones the `func_80022608`
  consumable panel and inherits its classes — the `selection = -99` default
  scheduled early instead of into the `func_80028380 != -1` test's delay slot,
  the spell-list build's two givs (`name` pointer, `magic_records[code]` offset)
  incremented in the opposite order, and the window-cursor `if (window != 0)`
  test inverted with its arms swapped. Referents, calls, constants, the MP
  affordability/deduction and the four spell effects all match.

## player warp / floor transition (0x80036618..0x80036e38)

`src/game/player_warp.c` reconstructs the warp band.
`actor_transform_definition5_to6` (the actor colour-fade sequence) is exact:
two `blend` loops over `lighting_set_color_matrix` with the map-event
state assignments and actor position/rotation ramps. `func_80036e38`
(`src/game/func_80036e38.c`) is exact once modelled as a K&R varargs function
`u32 f(s32 mode, ...)`: retail spills `a0`-`a3` to the incoming home slots and
reads `mode` from `[0]` and the flag byte from `[4]` (`lbu`), keeping only the
result in `s0`. Reading the flag as `*((u8 *)&mode + 4)` reproduces the home-slot
`lbu` and forces `mode` to memory; a named `u8 flag` parameter instead keeps the
value in a callee-saved register and grows the frame.

Residue recorded (not steered):

- `func_80036850` / `func_800369ac` (99.0% / 99.1%): referents, relocations,
  call set, CFG, types and constants all match; the only divergence is the
  prologue schedule of one argument-save move. Retail saves every incoming
  argument to its callee-saved register before the first body instruction
  (`move s1,a0; move s0,a1; move a0,zero`); the probe interleaves the first
  call-argument setup between two of the saves (`move s1,a0; move a0,zero;
  move s0,a1`). Same register assignment, same count, one instruction reordered.
- `func_80036618` (~64%): the warp-shimmer animator. Structurally faithful
  (four-effect spawn, 48-frame animation with the `frame==8` sound, mode-2 pool
  retention) but hits the same loop-optimiser residues documented for
  `func_80035e44`: retail strength-reduces the `effects[]` walks to moving
  pointers with a `s6=-1` down-counter idiom (`do {} while (--i != -1)`) and
  carries `mode` in an extra callee-saved `s7`, where the probe indexes
  `effects[i]` per iteration with an up-counter and one fewer saved register.
- `func_80036af0` (~62%): the per-frame scripted-trigger dispatcher. The switch
  on the current floor (jump table `0x80012c14`) and the per-floor cell-key
  comparisons match, but GCC's cross-jumping merges the many
  `func_80036850(n, 0)` / `func_800369ac(...)` call tails into shared sites in a
  different basic-block order than retail, and the probe hoists the current-floor
  load above the frame allocation where retail allocates first. Cross-jump /
  block-ordering residue; not steered.

Residues recorded in `game.menu_select` (`func_800238d8`, `func_80023e9c`):

- The two panels are the option menu's equipment-select (`func_800238d8`) and
  spell-select (`func_80023e9c`) handlers, and both reconstruct with the
  window-cursor test already flipped to `if (window == 0) scroll--; else
  window--;` and the confirmation join written as the `func_80028380 == -1`
  if/else — i.e. they do not carry the two residues catalogued above for
  `func_8002317c`. Referents, call set, constants, the two equipment jump
  tables, and the equip/select writes all match.
- `func_800238d8` (84.7%): the equip list-build reads the owned-item block
  `DAT_800652a8[i]` and the name table `DAT_80058dc0[i*10]` as two loop givs.
  Retail strength-reduces the stride-1 owned access to a walking pointer whose
  base `&DAT_800652a8[0]` is hoisted into the caller-saved `a0` at the top of
  the function (the list-build loop is call-free), then forms `owned = a0 + i`;
  the probe keeps `owned` as a preheader-initialised `&DAT_800652a8[start]`
  walking pointer with the base in a scratch temporary instead, so the top of
  the function is one instruction shorter and the switch-merge/jump-table
  offsets shift by that instruction. The name giv, its inner copy, the loop
  counter register, and everything after the loop are identical. Making the
  base a source variable forces a seventh callee-saved register (`s7`) rather
  than the retail `a0`, so it is not steerable from C — the same giv-base /
  register-choice compiler-build question as the `func_8002317c` class.
- `func_80023e9c` (90.7%): structurally identical to retail except for a
  callee-saved register permutation. The literal `1` (compared by both
  `confirm == 1` and `func_8002af48(...) == 1`) is hoisted into a callee-saved
  register; retail assigns `s2=selection`, `s3=1`, `s4=input`, whereas the probe
  assigns `s2=1`, `s3=input`, `s4=selection`, plus the same paired-giv
  increment-order swap seen in `func_8002317c`. Unattributed; not steered.

## effect-pool spawn (0x80036f44..0x8003784f)

`src/game/func_80036f44.c` reconstructs the effect-pool spawn band. The
KfEffectRecord layout (60-byte stride: header bytes, a `VECTOR position` at
0x0c, and rotation/scale/direction 16-bit triples at 0x1c/0x24/0x2c) is proven
from the two constructors and modelled in `kf/game_effect.h`; the default
scale is 0x1000 and the direction triple is copied from the SVECTOR argument.

`func_80037770` is exact: a specialised constructor (`player_use_item` caller)
that seeds a fixed type-0xf0/kind-0x34 record. Its `jal effect_pool_find_free`
needed the `0x800377a4` `mips26` reloc promoted to reviewed.

Residue recorded (not steered):

- `func_8003781c` (~63%): publishes the current effect record and its
  `magic_records` row through `current_effect` (0x8009db84) and
  `current_effect_magic_record` (0x8009db80). The retail unit reaches
  `magic_records` by `addiu a1,a1,-3364` off the `current_effect` base
  register: `magic_records`, `effect_pool_records`,
  `current_effect_magic_record`, and `current_effect` are one consecutive
  block (0x8009ce60..0x8009db88) in the
  original translation unit, so the linker-resolved delta is an assemble-time
  constant and one `lui`/`%hi` load serves two globals. A reconstruction that
  references `magic_records` as its own extern emits a separate `lui`+HI16 pair
  and cannot share the base register. Reproducing it would require owning that
  whole bss block (and thus migrating `magic_records`, used by many player/menu
  units) into this TU; keeping the honest `&magic_records[kind]` reference and
  the two real HI16/LO16 relocs (`current_effect`,
  `current_effect_magic_record`) instead leaves
  the shared-high-halfword divergence as a documented data-layout residue.
- `func_80036f44` (~46%): the general constructor and its ~45-case kind switch
  (jump table `0x80012c28`). Structurally faithful — the common record init,
  the `magic_records[kind]`-indexed spatial sounds, and every case's field
  writes are decoded and modelled with the real identities — but it stacks
  three GCC 2.5.7 residue classes at once: (1) switch cross-jumping / tail
  merging across the many audio-call and return tails, in a different
  basic-block order than retail; (2) K&R stack-vararg reads (the fifth argument
  `direction` is the base of the on-stack `arg6..arg8` slots), where the probe's
  register allocation keeps one fewer callee-saved register than retail's
  `s0`-`s5`, shifting the whole frame (-48 vs -56) and every subsequent offset;
  and (3) the retail unaligned 8-byte `lwl/lwr` copies of the direction/rotation
  triples, which a field-by-field struct copy renders as aligned `lh/sh`. The
  arithmetic, referents, call set and per-kind semantics all match; the residue
  is codegen shape, not source facts, and is not steered.

`effect_map_collision` (the 0x76c routine at `0x80037850`) was subsequently
reconstructed after the cell-attribute owners were reviewed. It is a
divide-by-2000 cell walker with its own six-case jump table (`0x80012ce0`) and
currently scores `71.711580%`; the remaining collision-math/cross-jump residue
is documented rather than treated as an unstarted function.

## pad

Campaign over the adjacent GAME vendored band `0x8005005c..0x8005023c` and
the homologous OPEN PAD band `0x8002fe8c..0x80030010`: the
`game.intr_tail` critical-section helper has INTR.OBJ lineage, while both sets
of six PAD functions have PAD.OBJ lineage. The three public PAD entry points
dispatch on the archive-named private `PadIdentifier`; `pad_buf` holds the word
returned by the low-level PAD API and `pad_status` is cleared during init. Each
invalid-identifier path reaches a private "Bad PadIdentifier" reporting stub.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `sw a0,PadIdentifier; sw zero,pad_status; li v0,-1; sw v0,pad_buf; bnez a0` then `PAD_init2`/stub both called `(0x20000001, &pad_buf)` with `move s0,v0` in the following `jal`'s slot | store the three private globals, select the call into `result`, then call `ResetCallback` and return `result` | `PadInit` in GAME and OPEN (EXACT) |
| `lw v0,PadIdentifier; addiu sp; bnez v0; sw ra(slot)` then `return ~pad_buf` | `if (PadIdentifier==0) PAD_dr(); else stub(); return ~pad_buf;` — the tested load feeds the branch, so the probe hoists it above the frame | `PadRead` in GAME and OPEN (EXACT) |
| bad-identifier stub `printf(fmt, PadIdentifier)` with no return statement, consumed in a value context by the caller | K&R `u32 stub() { printf(fmt, PadIdentifier); }` — declares no parameters, falls off the end; the caller passes extra args and reads the incidental `v0` | `PadInit` -> `pad_init_bad_identifier` in both images (EXACT) |

The original `probe-gcc257-o2-g0` campaign left a repeatable load-placement
residue: retail hoists the first data load above the frame allocation / register
saves whenever the loaded value feeds a call argument or a callee-saved
register. The later `probe-gcc257-o2-plain` profile reproduces that schedule;
all thirteen current LIBETC source-verification functions are now exact without
source steering: the GAME INTR tail, both six-function PAD copies, their
overlay-specific data, and both literal ranges. The old residue remains useful
evidence for these units' profile selection, not an open function mismatch.

One data-side detail matters when reconstructing PAD.OBJ. The pinned
GCC/maspsx path places separately defined file-static tentative BSS words at
eight-byte object offsets even though each object is four bytes. GAME keeps all
three PAD statics in BSS, while OPEN links `pad_buf` and `pad_status` as
zero-initialized load data and only `PadIdentifier` in BSS. The delinker now
uses an eight-byte candidate alignment only for static BSS claims whose retail
addresses support it; retaining generic four-byte packing made GAME's modeled
PAD `.bss` 16 bytes while the reconstruction emitted 32 bytes.

## debug/format

Campaign over the contiguous text-formatting band `0x8003a7dc..0x8003ac4b`
(module `debug_text`, six functions). From's own minimal `printf` family
(the Sony LIBAPI `printf` and LIBGPU `sprintf` are vendored elsewhere); the
band is dead debug scaffolding, so every signature was recovered from the
bodies and the internal call graph. All six reached exact; the levers that
decided the two harder ones are reusable:

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| loop-invariant constants materialised (`lui/ori` divisor) *before* the relocated buffer address (`lui/addiu`) | initialise the plain-constant local (`divisor`) *before* the address local (`out = buffer`); declaration order sets the emission order of hoisted setup, not the register numbers | `format_int_dec` `0x8003a81c`, `format_int_hex` `0x8003a8fc` |
| `subu w-len; addiu count,-1; ... sb; bnez; addiu count,-1` prepend loop reusing the width-argument register | reuse the parameter as the counter (`width -= len; while (width-- != 0) *--s = pad;`), not a fresh `count` local | `format_pad_left` `0x8003a988` |
| `move len,zero` hoisted above the load it guards, branch delay slot filled by the pointer bump | compute the loop pointer (`p = s + 1`) *before* the `if (*s)` guard so it, not the `len = 0`, is the delay-slot candidate | `format_pad_left` |
| switch case *bodies* laid out in a fixed order with shared emit/copy tails reached by back-`j` | order the `case` labels in the source to match the retail body order (the value-based dispatch tree is independent of source order); put the fall-through body first and reach it from later cases with `goto` | `format_vsprintf` `0x8003a9f4` |
| conversion flags (`width`,`zero_pad`) never initialised at entry | declare them uninitialised; they are only assigned inside the `%` case and read under the `in_format` guard, so an `= 0` emits two dead `move sN,zero` at entry and shifts every offset | `format_vsprintf` |
| copy loop keeps the character in one register across the store, char register shared with the format-dispatch char | reuse the dispatch char local and load-then-increment in the test (`while ((c = *s++) != 0) { *out++ = c; count++; }`); a separate `*s` test plus `*s++` body defeats the CSE and stalls the load | `format_vsprintf` |
| `args` in `s2`, `count` in `s3` (the shorter-lived pointer takes the lower callee-saved register) | give the argument pointer the shorter live range (use the `s32 *` parameter directly, no copy) and hold `count`'s reference count down by routing the width-digit literal through the shared literal emit (`goto literal`); one extra `count++` site raises its allocation priority and steals `s2` | `format_vsprintf` |

The last row is the decisive one: a structurally exact `format_vsprintf`
stalled at 99.4% with only `args`/`count` swapped between `s2` and `s3`.
Reference count, not declaration order, drives GCC 2.5.7's callee-saved
priority here — folding one redundant `count++` into the shared single-char
emit tail dropped `count`'s priority just enough for the shorter-lived `args`
pointer to win `s2`, matching retail exactly.

## GAME polygon-enqueuer / pool-emitter band (0x8001c7f8..0x8001ed38)

The internals the frame renderer's emitters call: the large TMD primitive
enqueuers (`func_8001c7f8` 0xf38, `func_8001d730` 0x6e8, still unreconstructed),
the screen-space sprite emitters, the visible-cell dispatcher, and the pooled
model emitters `render_actor`/`render_map_object`.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `bnez in_range,<table>` with the fallback branch as the fall-through | invert to `if (out_of_range) DAT = &fallback; else DAT = table[i];` so the fallback (not the branched-to table lookup) is the fall-through block | `render_map_cells` `0x8001e83c` (EXACT) |
| `sw window,DAT; ...; lw v0,DAT; lhu 6(v0)` (the just-stored global pointer is reloaded, not reused) | assign directly to the global pointer in each `if`/`else` branch and read `DAT->field` afterwards; a `KfCellWindow *window` local kept live across the store lets CSE reuse the value and drops the reload | same |
| `u8` do/while counters with `andi 0xff` at the bottom test, `int` cell deltas compared `sltiu ...,100` | `u8 rows = DAT->height; do { ...; } while (rows-- != 0);` with `int` row/col deltas and `if ((u32)row < 100)` range guards | same |
| `POLY_FT4 *p = display_state.primitive_buffer->cursor; ...->cursor += 40; if (->cursor > ->end) return;` | keep the cursor advance and the overflow guard as the buffer's own member accesses (re-read after the store); write the guard `cursor > end` so the cursor loads before the end | `func_8001e480` `0x8001e480` (structural; scheduling residue) |
| `p->x0 = p->x2 = v;` storing the higher-index cell first | chained assignment `p->x0 = p->x2 = value;` — the outer destination's store is emitted after the inner (`sh @24` before `sh @8`) | same |

Residues left in the band (not steered):

- `render_actor` `0x8001e9a4`: retail keeps a redundant `andi 0xff` on the
  lbu-loaded actor descriptor byte before the `>> 4` nibble shift (and again on
  `high - 1` before the depth-table index), and computes `high - 1` in its own
  register rather than folding the `-1` into the DAT_80095038/DAT_80095048
  addends. cc1psx-257's `nonzero_bits` provably drops the mask (verified in
  isolation for `u8`, `int`, `(unsigned char)`-cast, and `(x & 0xff) >> 4`
  spellings), which also swaps the descriptor/object callee-saved registers and
  trims the frame by 8. Same class as `func_8001ed90` above.
- `render_map_object` `0x8001ebb8`: retail lowers the four-way `behavior_type`
  dispatch as a sequential comparison tree (`bltz; slti 2; bnez; slti 4; beqz;`
  fall-through `li 0xb4`) with the object-id mask duplicated across the exit
  blocks. cc1psx-257 if-converts the `< 4 ? 180 : 0` tail to `negu; andi 0xb4`
  and cross-jumps the mask into one block for every `switch`, if-else-chain, and
  `goto`-shaped source tried. Retail optimises the tail less, not more.
- `func_8001e480` `0x8001e480`: the post-reload scheduler hoists the clut/tpage
  global loads into the load-delay slots after the screen X/Y reads; retail
  leaves those nops and keeps each load beside its store. `-O2` reproduces
  retail's prologue load-above-`subu` hoist but also this body hoist;
  `-fno-schedule-insns2` drops both. No single 2.5.7 flag matches retail's
  partial scheduling, so this is left as an open compiler-attribution residue.

All three are the recurring "2.5.7 optimises more aggressively than the retail
compiler" pattern (mask elision, if-conversion, delay-slot filling); the sources
are the honest shapes and are not distorted to re-introduce the retail idioms.

## GAME polygon-emitter / geometry-render band (0x8001ed90..0x8001f218)

The per-entity emitters that the frame renderer sweeps over the object, actor,
map-event, effect and weapon pools.

| Retail signature | Source shape | Witness |
| --- | --- | --- |
| `lhu` on both a pool field and a `render_state` view-position `long`, then `subu; sh` | `s16 screen.vx = pool->pos_x - (u16)render_state.view_position.vx;` — the `(u16)` cast of the long member folds the truncated subtract to `lhu` on both operands | `func_8001eedc` `0x8001eedc`, `func_8001f0c4` `0x8001f0c4` (EXACT) |
| `RotTrans(&screen, sp+t, &flag)` writing the view-space position straight into a `MATRIX`'s `t` column at `matrix+0x14` | pass `(VECTOR *)&model.t` as the RotTrans output; a later `RotMatrix`/`matrix_set_rotation_*` fills `model.m` and `MulMatrix0/2` leaves `t` untouched | `func_8001f0c4`, `func_8001eedc` (EXACT) |
| `negu a1,a1; jal; addiu a1,a1,50` (the add scheduled into the delay slot) | write `50 - y` as `-y + 50` so the probe forms `neg`+`addiu` instead of `li 0x32`+`subu` | `func_8001f798` `0x8001f798` (EXACT) |
| `bnez cond,<project>` with the fallback block inline | invert to `if (visible == 0) { select; project; } else { project; }` so the zero case is the fall-through | `func_8001f0c4`, `func_8001eedc` (EXACT) |
| a `render_state` member matrix reached as `addiu a0,base,-off` off a single view-position anchor register | keep the natural `render_state.view_matrix` / `render_state.pitch_matrix` member accesses; the probe anchors one base at `render_state+0xa4` and rematerialises the matrices as negative offsets | `func_8001ed90`, `func_8001eedc`, `func_8001f0c4` (EXACT for the latter two) |

Local declaration order is load-bearing for these frames: the probe lays scalars
out in declaration order above the outgoing-argument area, so the source lists
`screen`, then `scale` (when present), then the model `MATRIX`, then the RotTrans
`flag`, to reproduce `sp+24 / sp+32 / sp+48 / sp+80` (`func_8001eedc`).

Open residue (not steered): `func_8001ed90` extracts a facing nibble as
`sprite->orientation & 0xf0` and uses it in `beqz`/subtract. Retail zero-extends
the masked byte with a redundant `andi 0xff` after the `andi 0xf0`; the value
provably fits a byte, so cc1psx-257's `nonzero_bits` analysis elides the second
mask regardless of whether the local is `u8`, `u32`, `(u8)`-cast, or built with a
compound `&=` (the inverse of the byte-field cases above, where a wider local
suppresses an `andi`). Referents, call set, CFG and every other instruction match;
the single extra zero-extension is the only divergence, left as an unattributed
instruction-selection residue rather than steered with dead code.

## item / inventory menu panels

Campaign over the contiguous item/inventory/save-load menu-panel band
`0x800249a8..0x8002589c` (four functions, dispatched from the hub menu
`func_80022348` and the save-confirmation panel `func_800222b4`). All four are
list-widget panels of the same shape as the banked `func_800238d8`
(`menu_select`) and `func_80022608` (`menu`): a windowed cursor over rows drawn
by the shared `func_8002abb4`/`func_80028914`/`func_80027ee4`/`func_8002ac34`
frame helpers, an edge-triggered pad loop (`0x1000`/`0x4000`/`0x20`/`0x40`), and
a `PadRead(1)`-release wait. Roles: `func_800249a8` drops a held item
(filtering equipped copies via the seven equipment ids at `player_state+0x64`
and `+0x90..+0x95`, then decrementing `DAT_800652a8[code]`); `func_80024e64` is
the save/load hub; `func_800250c4` is the save panel (three slots, a
card-format row at cursor 3, exit at 4); `func_8002552c` is the load panel
(three `KfSaveSlotSummary` slots plus exit).

Source shapes that were load-bearing:

| Retail form | Source shape | Witness |
| --- | --- | --- |
| a single callee-saved base (`s5`) holds `DAT_800652a8` for the whole body; the build loop strength-reduces `inv[code]` into an advancing temp | index a stable base pointer (`inv = DAT_800652a8; inv[code]`, `inv[selection]--`) rather than an advancing `inv++`, so the exit decrement keeps the base alive across the calls | `func_800249a8` (5.7% -> 91.5%) |
| catalogue read filtered to the "no card" path via `beq result,1` sharing the fall-through into the menu | write `if (read_catalog(..) != 1) { nodata; return -1; }` (the negated test), not `if (== 1) { menu } else { nodata }` | `func_8002552c`, `func_800250c4` |
| `KfSaveSlotSummary summaries[3]` at `sp+16`; the load panel's confirm reads `summaries[cursor].fields[2]` (offset 8) for slot occupancy | the 24-byte `u32 fields[6]` summary, three of them (`0x48` bytes, matching the save panel's `memset(.., 0, sizeof)`) | both save/load panels |
| the seven equipment ids share one base register with byte offsets `0, 0x2c..0x31` | take `u8 *equip = &player_state.equipped_weapon_id;` and index `equip[0x2c]` etc., not the named `player_state.equipped_*` fields (which re-`lui` per field) | `func_800249a8` |

Residues (structurally faithful, not steerable -- the list-widget compiler wall;
do not chase):

- `func_800249a8` (91.5%): a three-way callee-saved permutation --- retail
  `selection=s1`, `codes`-base`=s2`, `input=s3`; cc1psx-257 rotates them to
  `s1/s2/s3 = codes-base/input/selection`. The banked twin `func_800238d8`
  lands `selection=s3`, so there is no fixed mapping; the extra build-loop
  pressure (the `counts[]` scratch and `equip` base) moves the allocno order.
  Also one loop delay-slot swap (retail advances the inventory temp `t0` in the
  branch slot and the name pointer `t4` in the body; ours reverses it).
  Declaration-order and induction-shape variants do not flip either; a `scan`
  helper local is a forbidden fake and did not help.
- `func_80024e64` (91.2%): the two-arm action dispatch (`if (action==0) load;
  else if (action==1) save;`) -- retail branches *out of line* to the load/save
  blocks and returns `move v0,s1` at the epilogue; cc1psx-257 inlines the load
  block (inverting the first test) and schedules `move v0,result` into the
  fall-through. A `switch (action)` did not change it. The `-1` constant does
  hoist into `s6` as retail has it.
- `func_8002552c` (58.0%): retail hoists the loop-invariant constants `1` and
  `3` into callee-saved `s5`/`s6` (frame `0x78`); cc1psx-257 rematerialises them
  with `li` at each compare (frame `0x70`, two fewer saved regs), which cascades
  every downstream offset. The instruction stream is otherwise identical. The
  nearly-identical save panel `func_800250c4` shows retail itself *not* hoisting
  here, so this is a cost-model coin-flip, not a source fact.
- `func_800250c4` (53.8%): retail keeps the reused `status` result in a
  dedicated `s4` (six saved regs, frame `0x78`); cc1psx-257 coalesces it with
  the `prev`/`i` temp in `s0` (five saved regs, frame `0x70`), again cascading
  offsets. Same callee-saved-count residue as the load panel, opposite direction.
