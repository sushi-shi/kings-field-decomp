# GAME nullable view transform

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x8001c184 render_set_view_transform`, 300 bytes,
`src/game/render.c` in `game.render`; strict objdiff initially 96.800000%.
The unit starts this campaign at 14/18 exact, GAME at 267/362 exact.
Retail files were hash-validated with `kf init`. All six matcher semantic
views were inspected with `--image game`, including unfiltered references.

- The sole decoded caller is `render_frame` (`0x8001fde4`), whose call at
  `0x8001fdf8` forwards its incoming `$a0` and `$a1` unchanged and does not
  consume a return value. Its delay slot saves `$s0`. The supported signature
  remains `void(const VECTOR *, const SVECTOR *)`, each independently nullable.
- The position branch at `0x8001c190` targets `0x8001c23c`; its delay slot
  preserves the rotation pointer in `$t0`. The position path copies all four
  words, divides signed X and Z by 2000, and stores halfword cell coordinates.
- The rotation branch at `0x8001c23c` is `beqz t0,0x8001c26c`, with a `nop`
  delay slot. It skips only the two-word, halfword-aligned `SVECTOR` copy.
  The target rematerializes `&render_state.view_rotation`, calls `RotMatrix`
  at `0x8001c274`, then zeros stack halfwords 20 and 18. These operations are
  unconditional, including when the rotation input is null.
- The second `RotMatrix` call at `0x8001c298` uses a stack `SVECTOR` with
  persistent pitch and zero yaw/roll. Its delay slot stores pitch at stack
  offset 16. The first call's delay slot derives `view_matrix` by -180;
  `pitch_matrix` is derived by -148 before the second call. The 32-byte frame
  is released in the `jr $ra` delay slot at `0x8001c2ac`.
- There are two proven direct calls, both to `RotMatrix`, and seven validated
  HI16/LO16 pairs, in order: `render_state+0xa4`, `+0xac`, `+0xbc`, `+0xbe`,
  and three `+0xb4` references. No strings, outgoing candidate references,
  indirect calls, or new data identities are involved.
- The preceding `tmd_select_object_vertices` (`0x8001c148`, 60 bytes) and
  following `tmd_prepare_primitive_indices` (`0x8001c2b0`, 768 bytes) were
  inspected. Their boundaries, typed TMD policy, and bodies remain unchanged.
- Vendor negative control: the target is game-owned persistent-view policy,
  absent from `functions_vendored.tsv`. Its callee at `GAME:0x8004e9b8` is
  independently attributed to the exact Release 2.5 `LIBGTE.LIB/GEO` section,
  member offset `0x6c`. The supplied `LIBGTE.H` declares
  `MATRIX *RotMatrix(SVECTOR *r, MATRIX *m)`; SDK layouts stay authentic.
- The exact OPEN counterpart at `0x80016f04` has the same copy-only branch
  scope and unconditional calls/zero stores. Its different matrix offsets
  and wider graphics owner are not transferred to GAME.
- Source history and `source-shapes-gcc257.md` were inspected. The historical
  note describes the copy-address register mismatch but misses the earlier
  branch-target mismatch. A fresh `kf try --unit game.render` first differs
  at the nullable rotation branch: compiled `+0xf8` versus retail `+0xe8`.
  The current C incorrectly skips the first matrix calculation and leaves
  two consumed stack fields uninitialized on null input.

Hypothesis: end the null check immediately after the rotation copy, then
execute the first matrix call and zero assignments unconditionally. Make no
other source, type, ownership, relocation, compiler-profile, or declaration
changes. Rebuild the unit and inspect the first real divergence; require
strict 100%, all 75 encoded words and ordered relocations for closure.

## Final verdict

The one branch-scope correction closes the function at strict **100%**.
`kf match --unit game.render` rebuilt the real object and generated the
canonical report; `kf try`'s textual exact result was not used alone.
All 75 encoded words match, including the conditional branch target,
unaligned copy instructions, division checks, both call delay slots and the
return delay slot. All 16 relocation entries have identical relative offsets,
kinds and targets; low-word addends also agree in the encoded words.

GAME advances from 267 to **268/362 exact** (94 partial functions remaining),
and `game.render` from 14 to **15/18 exact**. All other 483 function report
rows across GAME, OPEN and PSX, including SDK source controls, retain their
previous extents and scores. Only `GAME:0x8001c184` is selected for banking.
No remaining difference is attributed to a compiler mechanism.

Ruff passes, all 551 repository tests pass, and `git diff --check` passes.
Full `kf build` was run and still fails the pre-existing data-comparison,
known-reference ownership and section-placement gates: 11/59 source data
owners pass, and six target units have conflicting section bases. These
failures are not function compile errors and were not weakened for closure.

The historical copy-register symptom was a consequence of an incorrect source
branch scope, not evidence that a matching source construct was unavailable.
For nullable inputs, compare the first branch target before classifying later
address rematerialization or register differences as an unexplained residue.

## Other inspected candidate

`GAME:0x800328e0 audio_initialize` remains untouched at 95.121950%. A fresh
focused comparison differs only in the ordering of the voice-loop constant
loads: retail initializes -1 before 9, while the current source emits the
reverse. Calls and ordered referents agree. This pass does not invent a local
or change the profile to force those two instructions into place.
