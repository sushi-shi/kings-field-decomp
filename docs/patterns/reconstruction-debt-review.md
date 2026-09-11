# Reconstruction debt review: owners, varargs, storage and reuse

Reviewed at master `3c416e40` with the pinned `probe-gcc257-o2-g0` profile.
Every retained change keeps its functions strict objdiff 100%; the live
exact counts stay PSX 1/1, GAME 350/362 and OPEN 106/108, which are the
counts the tree already had at `3c416e40` (the README block was stale by
one GAME and one OPEN function that commit had already made non-exact).

## Owner recovery from member pointers: 26 sites to 0

The GAME TMD emitter and both OPEN TMD emitters recovered
`KfGraphicsRuntime*` from the projected-vertex address with the
`(u32)&((T *)NULL)->member` idiom so that each `AddPrim` could read the
ordering-table pointer as `lw v0,-756(s6)` / `lw v0,-276(s5)`.

The relative load is what GCC 2.5.x produces on its own when the constant
address is present in the same extended basic block:

- `explow.c:memory_address` forces a constant address into a register when
  CSE is expected, so `&game_graphics_runtime.tmd_projected_vertices + off`
  registerises the base in every switch arm that uses it.
- `cse.c:use_related_value` then rewrites any later
  `symbol + other_offset` in that block as `reg + delta` when the delta is a
  16-bit displacement; farther fields (the primitive buffer at +0x20) keep
  absolute addressing because their delta does not fit.
- `loop.c:combine_movables` merges the per-arm constant sets into one hoisted
  register, which is why retail derives `s6 = a1 + 488` from the
  `current_asset` address in the preheader.

So the natural source has **no `vertices` local at all**: the emitters spell
`(KfScreenVertex *)((u8 *)runtime.tmd_projected_vertices + offset)` and
`&runtime.display_state.ordering_table[...]` directly. GAME
`render_enqueue_tmd` compiles instruction-identical this way. OPEN
`render_enqueue_unlit_triangles` additionally needs its depth test and
`AddPrim` inside each arm (as GAME and the OPEN TMD emitter already have);
the pinned jump optimiser cross-jumps the two identical tails into the
retail shared tail, while a source-level shared tail is a join block whose
CSE table is empty. Keeping a pointer local instead swaps `s6`/`s7`.

OPEN `opening_run` reads `memory_arena.start` as `lw v0,-8(s0)` relative to
the saved `&memory_arena.allocation` inside the reload block. That block is
entered by fall-through and by one jump, and `cse_end_of_basic_block` only
follows a conditional jump whose target label is preceded by a barrier, so a
pointer local set outside the block never reaches it (controls: direct
field through the pointer, pointer taken in the block, pointer taken at the
loop top, duplicated reload blocks). The relation is still natural: with no
pointer local and `memory_arena.allocation.cursor = memory_arena.start;`
written before the stack-depth reset, `expand_assignment` registerises the
destination address (+8) first, the source and the depth slot become
`-8(reg)` and `4(reg)`, the loop pass hoists the constant into `s0`, and the
second scheduler moves the depth store ahead of the load, exactly as retail.

## Artificial address arithmetic: 2 cases to 0

Both OPEN FT3 arms still compute the third vertex as
`vertex1 + (v2_offset - v1_offset)`. With the direct lookup the functions
compile identical except for an eight-byte smaller frame: retail keeps one
unallocated four-byte stack slot below its spills (`sp+32` in the TMD
emitter, `sp+16` in the unlit emitter).

The slot is a pseudo with stale `reg_n_refs`: `combine.c:try_combine` only
zeroes the reference count of an eliminated I2 destination when
`newi2pat == 0`, and a three-insn combination that has to re-emit I1
(`added_sets_1`) leaves the eliminated subtraction result referenced but
never live, so `global.c` builds an allocno with no conflicts, assigns no
register, and `reload1.c:alter_reg` gives it a stack home. Any unused
aggregate local (a `CVECTOR`, a two-element array) leaves the same slot at
the same position, below the reload spills. Neither the subtraction nor an
unused local is evidenced by retail bytes and the choice is not decidable
from the image; both arms now use the direct third-vertex lookup and a
never-read `u16 unattributed_stack_slot[2]` carries the word. The local is
never read, so it introduces no undefined behaviour.

## Out-of-object pointer: 1 case to 0

GAME `format_pad_left` writes up to seven padding bytes below the digit
anchor at `0x800598a8`, but the GAME scratch claim started at the anchor.
Both images now use the same model: the digit anchor is eight bytes into an
aligned `char[24]` (`0x800598a0`, `0x80037970`), which is the smallest
eight-byte-rounded reservation covering the seven pad bytes, sign, ten
digits and NUL. Both units' `.bss` now match the native reservation; the
outer allocation bounds remain unresolved in both images: the preceding
344-byte `audio_sequence_table` ends 16 bytes below each base, and the next
identity 80 bytes above each base is an unreferenced byte of unknown owner,
so neither neighbour bounds the allocation.

## Manual varargs: 3 functions to 0

`vendor/include/stdarg.h` carries the classic Unix `va_start`/`va_arg`
macros for the pinned compiler (the Release 2.5 media ship no `STDARG.H`;
the GCC 2.5.8 `va-mips.h` variant adds alignment arithmetic that retail
lacks, and the generic GNU form advances the cursor before loading, which
`format_vsprintf` contradicts with `lw a0,0(s2); addiu s2,s2,4`). The
load-then-advance form `*((type *)(ap))++`, a GNU C lvalue cast the pinned
compiler accepts, reproduces all three readers: `menu_enter_mode` and
`notify_enqueue` read their optional argument from its home slot and both
`format_vsprintf` bodies walk a real `va_list`. Modern checkers take the
builtin branch; since the pinned preprocessor runs with `-undef`, the branch
keys on `__has_builtin`, which clang exposes regardless.

## Unrelated variable reuse: 4 functions to 0

`actor_initialize_current` and `actor_initialize_slot` now compute the
positions directly; the "shared temporary" the comment described was not
needed. `player_calculate_damage_component` and `player_distance_to_point`
had updated their parameters in place because retail holds the derived
values in the incoming argument registers. Locals assigned later move
(`v1`/`a0`, and the fifth-argument reload leaves `a3`), but a local
initialised from the parameter at its declaration takes over the
parameter's entry copy and live range, so `threshold`, `excess` and
`tolerance` inherit the registers and both functions stay exact.

## Stack aggregates and unused members

`notify_effect_update` no longer needs the `KfNotificationDigitBuffer`
union: a plain `s16` digit array read through `(u16)` casts compiles to the
same `lhu` loads. The remaining unread slot is the second pointer the
projection functions pass to `ReadSZ2` (`tmd_project_vertices`,
`tmd_project_vertices_shift` and the three OPEN siblings). `ReadSZ2` is the
exact Release 2.5 `LIBGTE.LIB` REG member and writes only through its first
argument, yet retail materialises both stack addresses, so the
two-argument compatibility declaration and its unread scalar reflect the
call sequence, not a modelling choice.

## Unresolved buffer bounds: 3 regions to 0

GAME's `unknown_projection_morph_20318[0x3e88]` is now
`tmd_projected_vertices[1000]` plus `morph_scratch[1001]`: the split at
`0x1f40` is the reviewed morph-scratch start, the projected capacity equals
OPEN's independently modelled runtime, and the morph remainder is exactly
1001 packed vectors, of which element 0 is the header-sized extra vector the
pool blends. The capacities are extents, recorded as `candidate` layout
rows, not proven original declarations. The two formatter scratch
allocations are claimed as the aligned 24-byte reservations their code
accesses; those claims match the data gate in both images, and the source
notes that the original declaration extents cannot be read from the image.

## Verification

Each retained change was first compared instruction-for-instruction against
the previous assembly with the pinned compiler, then scored strictly with
`kf match`; the tree-wide census after the review is 405 written casts
(322 pointer, 83 scalar) and 37 union definitions, and the cleanliness
floors were ratcheted down to the measured board.
