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

The subsequent [OPEN map campaign](open-map-enqueue.md) models the shared
packed screen-coordinate field directly as SDK `long`, retaining the
non-perspective halfword writers through `DVECTOR`. All four existing
producer/consumer objects stayed byte-identical after rebuilding.

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

## Projection and morph-buffer identities

The follow-up evidence pass is in
`config/evidence/game_semantic_tmd_buffer_views.tsv`. It covers the three
projection writers, the three polygon emitters, and
`GAME.EXE:0x800205d4 render_bind_animated_instance`.

`GAME.EXE:0x800911b0` is `tmd_projected_vertices`, an array of
`KfScreenVertex`. The writers advance eight bytes and fill packed screen
coordinates at offset 0, depth at 4, and doubled perspective at 6. The
emitters consume those fields through prepared vertex byte offsets. Their
retail base calculation is `(u8 *)&tmd_state.current_asset + 0x1e8`, numerically
`0x800911b0`; it is not a read through the current-asset pointer. The four
old halfword identities describe fields of one entry, not four globals.
Both images now obtain this array's declaration and `current_tmd_vertices`
from `tmd.h`. OPEN retains its existing 1000-entry definition and DATA claim.

`GAME.EXE:0x800930f0` is `tmd_morph_scratch`, an `SVECTOR` array declared in
`game_render.h`. The binder copies two words per vertex to `0x800930f8`,
then installs that same address as the current vertex cursor. The final
blend starts at scratch plus the morph object's `base_vertex * 8`. It
includes the eight bytes at the object's `base_vertex` and `vertex_count`
fields as an extra input vector, saves/restores the corresponding scratch
entry, and blends the following actual delta vectors. Source now spells
the copy destination and installed cursor as `&tmd_morph_scratch[1]` and
the extra input's address through its first field. The former interior
word identities are removed.

The GAME inventory extents are deliberately only the existing referenced
prefixes: eight bytes for projected vertices and 24 bytes for morph
scratch. They are not complete-object sizes or array capacities. Source
declarations remain unsized, linkage scope remains unknown, and neither
prefix receives a DATA claim. Neighbor spacing of 1000 and 1001 vectors,
and OPEN's separate capacity check, do not alone prove GAME's capacities
or its enclosing graphics-state layout. No such aggregate is introduced.

All five reviewed address-pair rows retain their numeric sites, targets,
opcodes, evidence classes, and review status. In particular, the copy
target is still `0x800930f8`; its delinked referent is now
`tmd_morph_scratch + 8`, not an independent symbol. The three emitters'
existing extra source-level array references are still a wider ownership
residue; no synthetic retail relocation was added for them.

The binder now includes its asset, render, pool, memory, and SDK headers
directly instead of the game umbrella. All five callers were inspected:
the anchor is their record slot, the asset and clip select offset tables,
the phase selects a keyframe interval, and the fifth O32 argument is the
TMD vertex count narrowed to a halfword by the callee. These names are
shared by source, public declaration, and function identity. Local names
identify phase bounds, keyframe offsets, morph indices, copy cursors,
countdowns, saved words, and blend fractions. The private variable-tail
asset views stay private; this pass does not claim their full extents.

The retail keyframe index uses incoming `$s5` without initialization. Its
uninitialized C local remains explicitly marked; no initialization or
control-flow repair is inferred. The zero/one/record return convention,
halfword loops, allocation retry, and full-weight `0x1000` morphs are
unchanged. `gteMIMefunc` is separately vendored: extracting `LIBGTE.LIB`
with the pinned `psyk`, then reading `MSC.OBJ` at XDEF offset `0x61c`,
reproduces all 128 unmasked bytes at `GAME.EXE:0x8004c860`. The real
`LIBGTE.H` declaration supplies `SVECTOR *` and `long` arguments. No SDK
body is reconstructed as game progress.

### Verification

All seven objdiff scores remain unchanged: projection
98.846150% / 98.953490% / 100%; emitters
52.039013% / 60.490950% / 17.954199%; binder 90.793990%.
The first retail differences remain the projection counter registers,
the native emitter's 88-versus-96-byte frame, the other emitters'
current-asset setup, and the binder's early jump to its four-byte-shifted
compiled epilogue. The binder also retains the previously observed loop,
branch-layout, and register differences. These are not attributed to a
compiler mechanism.

Compared with the pre-cleanup objects, render and emitter `.text` bytes
are unchanged. Binder `.text` changes only the low relocation addend at
object offset `0x2e0`, from zero against the old interior symbol to eight
against the shared base. All ordered relocation offsets, kinds, and
referents otherwise agree after identity renaming. The initial isolated
check preserved all 340 historically exact functions and 55 data-owning
units. Integration with the concurrent OPEN campaign at `24d4c7a` expands
the baseline to 348 exact non-vendored functions and 58 data-owning units;
all remain exact with this cleanup applied.
