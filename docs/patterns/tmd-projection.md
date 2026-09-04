# TMD projection ownership and SDK call contract

The three GAME projection functions at `0x8001c60c..0x8001c7f8` now extend
`game.render`, rather than forming a separate `game.tmd_project` unit.
Their evidence pass is recorded in
`config/evidence/game_semantic_tmd_projection_owner.tsv`.

## Why these functions belong together

The preceding renderer already owns `tmd_set_current_vertices` and
`tmd_select_object_vertices`. Those functions write the same
`current_tmd_vertices` pointer that all three projection routines read.
Callers repeatedly select an object, obtain its vertex count through
`tmd_get_object`, and invoke projection before emitting polygons. Animated
call paths supply the same cursor through the animation-binding interface.
All eleven projection call sites were inspected; this is shared state and
call workflow evidence, not adjacency alone.

The text runs are contiguous, use the same existing compiler profile, and
the projection tail adds no data or literal-pool claim. The consolidated
run is `GAME.EXE:0x8001b7b0..0x8001c7f8`; claims retain their original
addresses, extents, and order. The original historical file boundary is
still unproven. The following polygon-emitter unit is not merged by this
campaign, and no wider graphics-state aggregate is invented.

## Names follow the data flow

The shared input cursor is `vertex`, the output cursor is `projected`, and
the countdown is `remaining`. SDK output locals are `perspective`,
`gte_flags`, `depth`, and the non-perspective `transformed` vector. SDK
`long`, `SVECTOR`, and `VECTOR` types are retained at the library boundary.
The existing low-halfword conversions and arithmetic depth shift are
unchanged.

The second local passed to `ReadSZ2` is `unused_depth`, not a second
observed depth result. GAME callers materialize pointers to stack offsets
24 and 28. Retail `ReadSZ2` at `GAME.EXE:0x8004cb58` is exactly:

```asm
swc2 $19,0(a0)
jr   ra
nop
```

It writes only through the first argument. Its containing `LIBGTE/REG`
section is an exact Release 2.5 archive match, documented in
`psyq_release_25_text_sections.tsv` (member offset `0x1ac`, whole section
`0x2a0` bytes). The pinned `LIBGTE.H` omits this export, so `psyq.h` retains
the observed two-argument GAME/OPEN compatibility declaration and documents
the unused second pointer. It does not claim that the SDK has two FIFO
outputs or that this was its original C prototype. The extra argument and
its addressable storage are evidenced by retail, not added to steer codegen.

`RotTransPers` stores the interpolation value from GTE data register 8 and
returns the register-19 depth shifted right by two. Reading full depth
through `ReadSZ2` therefore has a distinct purpose. `RotTransPers` and
`RotTrans` remain separately vendored as supported SMP lineage with an
unresolved earlier SDK revision; this campaign does not promote them to an
exact Release 2.5 attribution. The game-owned array loops are not SDK
progress.

## Results and remaining differences

| Projection function | Before | After |
| --- | ---: | ---: |
| `tmd_project_vertices` | 98.846150% | 98.846150% |
| `tmd_project_vertices_shift` | 98.953490% | 98.953490% |
| `tmd_transform_vertices` | 100% | 100% |

In the two non-exact functions, the first differing instruction is the
count-minus-one assignment: retail uses `$s0`, while the probe uses `$s1`.
The depth-member pointer uses the opposite register, and later uses follow
those roles. The current raw comparison otherwise agrees on frame shape,
instructions, control flow, calls, widths, constants, and ordered
relocations. This is an observed register-identity difference, not proof of
an allocation priority, scheduler mechanism, compiler revision, or a
source-level impossibility. The old source and pattern note's stronger
mechanistic claims have been removed.

The preceding fifteen render functions retain their scores: eleven remain
exact; `display_show_error_screen` is 97.922680%, `display_initialize`
94.253010%, `render_set_view_transform` 96.800000%, and
`tmd_prepare_primitive_indices` 94.135414%. The consolidated unit is 12/18
exact. All 340 historically exact non-vendored functions remain exact, and
all 55 data-owning units match retail. The manifest now has 109 units.

After deleting the old source, `kf build --reconfigure` is needed to
replace the generated Ninja graph's dependency on that path. Comparisons
against the old module target are stale until the new graph and retail
target have been rebuilt.
