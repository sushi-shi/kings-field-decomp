# GAME effect-map collision: GCC 2.5.7 trace revisit

## Function Match Plan

The target is `GAME.EXE effect_map_collision` at `0x80037850`, retail extent
1900 bytes. The baseline source hash is
`daffff39cd4f75c638e5deeb147becfa8841e388b594389c39800793d3b2a9b5`:
strict objdiff **98.221054%**, 1888 emitted bytes. The current native and
instrumented compiler objects are byte-identical. The compiler source
fingerprint is
`222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`.
Generated evidence is under `build/gcc257/game-effect-collision-traces/`.

The six image-qualified semantic views, complete retail CFG/disassembly,
fresh resolved actual/target listings, six proven incoming call sites,
adjacent `effect_pool_set_current` and `effect_magic_power`, shared collision
types and `collision_query_world`, and the previous collision reconstruction
notes/history were inspected. This is game policy over the game's typed map
grids and effect state, with no vendored identification or SDK wrapper shape.

Callers pass a world `VECTOR *` and signed word radius in `$a0/$a1`, and test
the word result against `-1` or decode its upper halfword. The sole direct
callee is `collision_query_world`; its six arguments use the normal O32
register/stack slots. Retail has a 32-byte frame saving only `$ra`, fourteen
checked division expansions, fifteen internal `j` instructions, one `jal`,
the shape-table `jr`, and the final `jr $ra` with the frame restoration in its
slot. The twelve ordered numeric address pairs agree in the two resolved
objects; the target's resolved words equal the initialized retail image.
The source object has one fewer internal jump. No relocation or call-set
change is indicated. The six shape-table pointer rows remain candidate
semantic-navigation evidence; internal jump relocations are validated and
direct calls proven.

Signed halfword X/Z cells, byte grid elements, signed halfword height and
rectangle fields, tile size 2000, center 1000, and floor step 100 are already
modeled. The seven initialized eight-byte rectangles include intentional
reversed vertical bounds. Keep the existing typed data owner and all numeric
referents. The current differences are the missing local bounds-return pair,
oriented-coordinate register/conversion sequence, and third-query/default
flag setup; see the previous
[collision campaign](game-effect-map-collision.md) for rejected controls.

The baseline trace retains separate constant-return blocks through `sched2`.
`jump2` then applies cross-jumping to the bounds-return jump UID 65 (event
30572), sharing a later terrain constant return. This observation concerns
the pinned probe; it is not attribution of the retail compiler or a claim
that the source CFG is already reconstructed.

First source hypothesis: the final effect selector is exhaustive over the
masked values 0, 1, 2, and 3. Retail returns 1 for zero and forwards the other
three values with flags `0xe1`, `0x71`, and `0x61`. Test an explicit zero case
with the three query cases. All possible selector values must have defined
results; do not omit a reachable return or imitate a stale register on an
impossible above-three input. This isolates selector coverage from the
previous, ineffective relocation of the switch default after the switch.
Compare the raw final branch tree, flags and delay slots before accepting
any source change. The bounds and coordinate differences remain independent.

Second independent hypothesis: retail starts the selected coordinate's
halfword extension before loading `record->x_min`, whereas the baseline
expands the record load before extending the coordinate. Test the same
inclusive lower-bound predicate with the coordinate as its first operand,
`coordinate >= record->x_min`. Keep the halfword local, upper-bound test,
backward join, selection assignments and all other statements. This is an
operand-evaluation observation, distinct from the rejected changes to the
coordinate's width or lifetime. Inspect expansion, allocation and the raw
conversion sequence; the two return-CFG differences may remain.

## Results

The explicit zero case is rejected: **94.200000%, 1976 bytes**, with three
physical query calls instead of retail's one. It does restore the separate
bounds-return block, but neither the final selector tree nor the query tails
match. All four masked values remain covered; no undefined reachable path
was needed for this negative control. The source hash is
`06a3d66a871d5648337a290b57327f1482f8e2038bbe469b2b4b544631d9690b`.

The coordinate-first comparison is retained. Its source hash is
`b2ef8f1d238e6229f4b0fedbd45ee08c10c828c63a20f5a9643f21ba7354d88e`.
Exactly six of the 472 emitted instruction words change, at offsets
`0x1c8`, `0x1e4`, `0x1f0`, `0x1f4`, `0x228`, and `0x230`.
Each changed word now equals retail at the corresponding location, eight
bytes later because of the still-missing bounds return. All other 466 words
remain identical to the baseline, including their raw constants and control
destinations. In particular, all four orientation assignments now select
`v0`; the shared comparison starts with:

```text
sll  v1,v0,16
lh   v0,0(t0)
sra  v1,v1,16
slt  v0,v1,v0
```

This is the retail conversion/load sequence. No width, lifetime, predicate,
table, call, branch, or source object is added. The source simply evaluates
the coordinate side of the inclusive comparison first.

Two small rectangle controls reproduce the expansion difference. The old
operand order expands the record load before extending the coordinate. The
new order expands the coordinate extension first; the final control likewise
selects `v0` and interleaves the rectangle load with the two extension shifts.
Native, traced, and debug compiler builds emit whole-ELF-identical objects for
each control. The baseline actual unit and both actual-unit trials also pass
native/traced/debug whole-ELF parity at the same respective source paths.

Two additional small bounds/height/query controls validate the return-sharing
observation. With a default return, `jump2` applies cross-jumping to the bounds
return (UID 46, instruction 44 to instruction 62) and both duplicate query
tails. With an explicit zero case, it applies none of those merges. Both
controls pass the same three-compiler parity check. This validates the trace
interpretation for the current probe; it does not establish the historical
compiler or justify keeping the nonmatching exhaustive-switch spelling.

The complete executable-relocation audit proves that all `.text`, `.rodata`,
and `.data` relocation records and the switch payload remain identical to
the baseline. An initial object-wide comparison also included `.debug_line`;
its sole difference is the relocation offset associated with the different
source path. No executable referent changes. All 56 initialized rectangle
bytes still match retail.

**Partial, not banked.** Strict objdiff changes from **98.221054% to
98.126310%**, with the same 1888-byte extent. The lower fuzzy result does not
override the complete six-word correction and unchanged remainder. The
separate bounds return and final third-query/default flag sequence still
account for the twelve-byte deficit. No exact-function count increases.

## Verification checkpoint

The focused `kf match --unit game.effect_map_collision` invocation rebuilt
the actual source (`[109/119] compile game.effect_map_collision`); a fresh
production-object audit then proved the same six corrected words and strict
score. Modern scoped type checking passes. All **733 repository tests pass**
(102.348 seconds), including the existing rectangle/orientation/boundary and
query-flag controls. Ruff and `git diff --check` pass. The literal ledger's
43 tokens and line references remain unchanged by this one-line predicate
correction.

The subsequent full `kf build` still exits nonzero on the existing data,
ownership and placement checks. Source-data matches remain PSX 1/1, GAME
11/41 and OPEN 3/19; target relinks remain 1/1, 75/77 and 34/38. No artifact
failure or exact-function regression is reported. The collision unit's
56-byte rectangle payload is exact, but its existing alignment conflict and
the eight-byte switch-target layout displacement remain. GAME stays at
**336/362**, OPEN **106/108**, and PSX **1/1** strict exact functions. This
correction is committed as evidence-backed partial source recovery, with no
bank entry; completing the remaining 26 GAME and two OPEN functions remains
the active objective.
