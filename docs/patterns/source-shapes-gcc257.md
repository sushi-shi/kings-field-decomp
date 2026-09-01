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
