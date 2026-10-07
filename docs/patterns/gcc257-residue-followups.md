# GCC 2.5.7 residue follow-ups

A short pass over the GAME/OPEN functions that were still non-exact after the
first residue campaign. GAME and OPEN `main` were out of scope. Allocator
numbers come from the instrumented 2.5.7 `cc1`, which prints local-alloc and
global-alloc priorities.

## Closed

- **`effect_map_collision`: a missing return in the outermost function.**
  Retail's final target-class switch has no default code. Class 0 returns the
  class-1 compare constant that is still in `$v0` (1). Class >3 returns 3.
  An explicit `default: result = 1` emits one extra `li v0,1`, which reorg
  places in the class-3 compare's delay slot. Falling off the end of a
  `static inline` helper is not the same as falling off the end of the real
  function: the inline's result becomes an uninitialized pseudo, which is live
  from entry and lands in `t8`. Folding the helper into `effect_map_collision`
  and returning directly from each case matches retail.
- **`menu_draw_item_detail`: contour weight on a loop index.** The glyph-row
  loop index `i` (refs 7, live 16, priority 0.875) lost to the reduced
  destination giv (refs 7, live 14, priority 1.0), so the two swapped
  `a0`/`a1`. Moving only `i = 0` into a `do { } while (0)` contour raises
  `i` to 8 weighted refs: floor_log2 goes from 2 to 3 and the priority to
  1.5. The allocation order becomes i, destination, glyph, as in retail. A
  hand-advanced destination pointer also gives the retail registers, but it
  loses the `4(a1)` field offset of the indexed giv.
- **`item_load_database`: a dividend with two deaths.** Retail ties the
  `n / 100` quotient to the divisor constant (`li v1,100` / `mflo v1`), not
  to the dividend `n = i + 1`. local-alloc only gives quantities to pseudos
  that are local to one block and die exactly once (`reg_n_deaths == 1`), and
  `combine_regs` will not tie to a dividend that has no quantity. Reusing `n`
  for the last digit character (`n = rem % 10 + '0'; name[13] = n;`) gives it
  a second death. Its second life overlaps the `rem / 10` quotient in `a3`,
  so global allocation also keeps it out of the `a3` preference that the
  remainder would otherwise give it, and `n` gets `v0`. A plain copy
  (`n = rem % 10; name[13] = n + '0';`) is CSE'd away and changes nothing.
  Reusing one function-scope `n` for the `cd_file_load_allocated` result fixes
  the tie but leaves `n` in `a3`. Statement order and declaration order do not
  change the quantity sort.

## Mechanisms found, function still open

- **`effect_update_dispatch`: three global priorities.** Retail allocates
  `magic` (s3), `radius` (s5) and then `kind` (s6). The probe has kind
  (0.1289) > radius (0.0659) > magic (0.1000 with 14 refs). Contour weighting
  on `magic`'s load (three nested contours, 17 refs, 0.162) and on the
  `effect_map_collision` call (three nested, radius 9 refs) reproduces every
  s-register, but the source is not humane enough to keep. A separate
  pre-existing local residue remains in the radial-blast block: the
  `phase & 0xff` quantity (refs 4, live 8, priority 1.0) loses to the
  `x37` multiply chain (refs 9, live 22, priority 1.227). Retail allocates the
  phase quantity first. Contours scale both quantities and never reverse
  them, because floor_log2 grows the chain's term as fast.
- **`map_object_spawn_drop`.** The natural lazy `object_id < 65` compare lets
  `sequence` and `object` share `s0` (4 s-registers). Retail keeps them in
  `s0`/`s1`. The eager `within_drop_range` local restores 5 s-registers, but
  it evaluates early and swaps `object_id`/`y_offset` (`s3`/`s4`).
  None of these change it: declaration order, one to three unused locals
  (to shift the pseudo count), splitting the post-increment, assigning the
  flag inside the `else if` condition, or contours on the sequence and
  `object_id` stores.
- **`opening_ending_scroll_run` and `player_move_horizontal`.** Not retried.
  The constant-one `move_movables` threshold
  ([open-ending-scroll.md](open-ending-scroll.md)) still needs
  savings x lifetime >= ~9.5 against 228 loop insns. The player function is
  a broad allocation residue with no single structural lead.
