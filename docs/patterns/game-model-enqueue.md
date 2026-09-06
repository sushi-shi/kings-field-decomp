# GAME model polygon dispatch and shared depth tails

## Function Match Plan (2026-09-06)

GAME `8001d730` / `0x6e8` (1768 bytes), `render_enqueue_model`, starts
at strict 60.490950% on `d867e20`, under `probe-gcc257-o2-g0`.
Hash validation, all six semantic views, complete retail body and CFG,
the sole actor caller, adjacent emitters, TMD accessor, source history,
shared types and SDK provider evidence were inspected. The candidate
`void(u16 object_index, s16 depth_bias)` signature agrees with the callee's
halfword mask and sign extension; the sole caller passes zero for both.
Primitive counts and headers are words, prepared vertex/normal byte offsets
are unsigned halfwords, and projected depth/fog values are signed halfwords.

Retail has a 96-byte frame, fifteen ordered direct calls, ten validated
HI16/LO16 pairs, five internal absolute jumps and one return, with no
strings or unresolved indirect transfers. Every transfer owns its delay
slot. The four clipping rejections advance the packet; all four allocation
overflows exit through the complete restore tail. These returns already
exist in source. Allocation advances the cursor before checking the end.
Retain SDK GT3/GT4/FT3/FT4 sizes 40/52/32/40, prepared packet layouts,
triangle checked division by three then shift two, quad shift four,
signed biased depth greater than four, mask 3fff and stride mask 3fc.
GT4's fourth lighting call deliberately uses vertex zero's fog value.

LIBGPU PRIM packet constructors/AddPrim have exact Release 2.5 archive
attribution; LIBGTE SMP lighting/clip helpers have cross-overlay lineage
support with an unresolved pre-2.5 revision. Their SDK prototypes constrain
the O32 arguments. The surrounding model allocation, material and traversal
policy is game-owned; do not reconstruct SDK bodies as progress.

The first source discrepancy is asset setup: retail loads the word count
first and adds twelve to each object-relative offset before adding the
selected asset. First recover those source expressions. The broader GAME
graphics owner is unresolved: retail derives projected vertices from the
selected-asset field plus 488, texture selectors from that base plus
16040/16042, and the ordering table from it minus 756. Keep existing honest
identities rather than fabricate cross-object pointer arithmetic.

Next advance the packet by its four-byte header immediately after loading
that header, leaving only its payload length for the loop tail. Retail's
four-way dispatch has physical arm order 34, 3c, 24, 2c; test a switch with
that order instead of the current nested if chain. This is a source-shape
hypothesis, not proof of original syntax. Then compare the word
postdecrement guard and guarded projected-base lifetime. Inspect material
code publication after the UV writes and the decoded quad sum order.
Retail shares the full quad-depth calculation between GT4 and FT4, then
shares depth bias, threshold and AddPrim across all four modes.

Change one evidenced source cause at a time, rebuilding the affected unit
and comparing strict scores plus raw instructions/ordered relocations.
Preserve the neighboring map emitter's 98.770996% and every banked match.
Record remaining source/ownership questions or unattributed codegen
residue, run the full build, lint and tests, and bank only strict 100%.

## Focused reconstruction

Offset-before-base expressions reach 62.208145%. Immediate header advance
reaches 62.027150% but restores the decoded cursor position and removes the
extra four bytes from the tail. Retain that independently evidenced fact.
The four-case switch reaches 88.500000%, restoring physical arm order and
the dispatch tree. A single word-postdecrement while loop reaches
90.581450%, recovering the count load/test/decrement shape. Loop-local
projected storage reaches 90.359726%; its address is now formed only after
the entry guard, but the extra address pair remains because the broader
owner is unresolved. Other unit functions retain their strict scores.

The first remaining raw discrepancy is the selected-asset address setup
and separately materialized projected base. Record this ownership question
without inventing a source alias. Dispatch now differs by signed `slti`
versus `sltiu`; the extracted mode is 0..255, so a signed word local is
both range-safe and directly supported by the decoded comparison. Next
check the existing material-code store after the UV writes, and the quad
fog/depth sum order (vertex 0, 1, 2, 3). These are actual instruction/data
dependencies, not a register assignment prescription.

The signed mode local changes only `sltiu` to the retail `slti`, reaching
90.509050%. Publishing the command byte after UV writes reaches 90.518100%
and restores all four store positions. Quad fog/depth sums in decoded
0/1/2/3 order reach 92.959274% and permit a shared quad-depth tail.
The fourth vertex is still loaded too late: retail reads its prepared
offset at `8001d9bc`/`8001dc94`, before writing the advanced allocation
cursor, then forms the pointer in the overflow branch's delay slot.
Move the existing fourth-vertex expression before that cursor write,
keeping it inside the successfully clipped arm and preserving overflow
returns. Do not force a register or delay-slot instruction in source.

## Final verdict

The fourth-vertex lifetime restores both allocation-guard schedules and
reaches strict **93.126690%**, up from 60.490950%. Compiled text is 1776
bytes versus 1768 retail. All four overflow paths still enter the complete
96-byte restore/return tail; clipping and depth rejection advance packets.
The command publication, triangle division, quad sums, shared depth tails,
fog arguments, packet widths and constants agree with decoded retail.

A focused read-only audit of the two raw object listings finds fifteen
identical ordered calls, 25 conditional branches, five direct jumps, six
division-check traps, one return and 41 blocks on each side. The ordered
branch/jump destinations agree after mapping addresses to block indices;
the full twelve-instruction restore/return tail is byte-identical. This
checks this direct-control function, not general CFG equivalence or data
semantics, and does not replace the strict score.

The first raw difference is a NOP after the count load, where retail starts
forming the selected-asset address. Source uses a direct load of that
field, materializes the projected base separately and exchanges the
normal/projected saved registers (`s6`/`s7`). Count/header spills are
32/40 instead of retail's 40/32. Source has twenty HI16/LO16 pairs versus
ten retail: the extra pairs materialize the projected buffer, four uses
each of CLUT and texture page, and the ordering-table field. These are
correct separate identities; retail derives their addresses from a common
base whose enclosing GAME owner remains unresolved. Material loads/stores
therefore also schedule differently. No alias, fake local, forced register,
assembly or compiler-profile change is retained; the remaining register
and stack symptoms are unattributed codegen residue.

Only this function changes among all 484 native comparison rows. Every
banked function and the other three unit functions retain their scores,
including the map renderer's 98.770996%. GAME remains 288/362 exact and
OPEN 97/108; GAME aggregate fuzzy moves 94.45% to 94.81%. OPEN sources are
unchanged. This is a verified partial reconstruction, not a new exact or
bank entry.

Focused recompiles and the final recorded match retain 93.126690%. Ruff,
all 635 tests (79.485s, no skips) and `git diff --check` pass.
Full `kf build -j4` exits 1 on the existing
data/ownership/placement gates: source data 5/60, config-backed SDK data
4/4, target relink 110/116, six conflicting section bases and zero artifact
failures. No partial baseline is updated or banked.
