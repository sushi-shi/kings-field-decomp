# OPEN emitter unit and value-representation controls

These isolated probes start from master `eaf83a3`, with hash-verified OPEN
retail and unchanged `probe-gcc257-o2-g0`. No canonical C, header, manifest,
relocation or baseline change results. The findings exclude specific source
hypotheses; they do not identify the original compiler or its mechanisms.

## Function Match Plan: shared emitter module

The four contiguous emitters share the graphics runtime, allocator, SDK
packet/shading boundaries and repeated emission structure. This supports a
temporary WIP module experiment, not a historical TU claim. Concatenate their
unchanged bodies in linked order and compare each against a fresh separate
compile, resolving numeric instruction relocations before retail comparison.
Keep the existing data claims; do not invent an owner for the four-byte gap
at `800372f4`. The canonical manifest stays unchanged.

All six semantic views, complete CFG/caller evidence, source history and
shared layouts were reviewed. SDK PRIM/SMP services are separately attributed;
these game-owned emitters are not library reconstructions. There are no strings.
TMD's 29 incoming table rows remain candidates and its indirect dispatch is
not a proven direct branch; the listed encoded address pairs/internal jumps
are validated, and direct calls are proven.

| OPEN function | Retail bytes; strict baseline | Calls / address pairs / internal jumps | Relevant contract |
| --- | --- | --- | --- |
| `8001764c render_enqueue_tmd` | 3320; 99.171080% | 57 / 11 / 11 | u16 object, s16 bias; twelve modes; frame 96 retail/88 probe, extra probe OT pair |
| `80018344 render_enqueue_unlit_triangles` | 676; 98.828400% | 8 / 1 / 1 | u16 object, s16 bias; F3/FT3; frame 64 retail/56 probe, extra probe OT pair |
| `800185e8 render_enqueue_map` | 952; 99.978990% | 18 / 2 / 1 | u16 object; GT3/GT4; frame 80, normal/header slots differ |
| `800189a0 render_enqueue_sprite` | 540; 100% | 6 / 5 / 0 | KfSpriteQuad pointer, s16 bias, s32 flag; exact control |

Every combined function has identical resolved instructions, ordered calls
and data targets to its separate-unit control. TMD remains 3324 bytes/125
unequal aligned words; unlit 680/52; map 952/5; sprite 540/0, raw exact.
Unit concatenation does not explain the current residues. Do not migrate
the manifest or claim new exactness from this negative control.

## Function Match Plan: map packet-header value

The map emitter's five differing words exchange the normal-pointer slot
at retail sp+32/probe sp+24 with the header slot at retail sp+24/probe sp+32.
Retail loads the four-byte packet header once, shifts it by 24 for dispatch,
then extracts the input byte length with a shift by 6 and mask `3fc`.
The established disk bytes are olen/ilen/flags/mode; the supplied SDK's
`TMD_PRIM` is a different, decoded object, not this four-byte header.

Test a complete four-byte word/byte union copied by value, preserving the
word extractions. The older GAME leaf-header control was identical; this
OPEN witness instead retains the header across SDK calls. No pointer helper,
forced memory operation, padding or declaration permutation is allowed.
Then separately test four unsigned eight-bit fields in a word-sized struct,
copying that complete header and consuming `mode` and `ilen * 4` directly.
Both representations have a pinned-compiler four-byte size assertion.

Both emit the identical 952-byte baseline, including all five unequal stack
operands, all eighteen calls and both address targets. Neither representation
is retained. Header representation does not explain those slots under this
probe; adding unused storage would not be a source reconstruction.

## Function Match Plan: TMD vertex-address operands

After the frame and normal/projected-base differences, all 42 projected
vertex additions put the prepared unsigned-halfword byte offset first in
retail, whereas the probe puts the byte base first. The refreshed six views,
complete exact entity caller, neighboring emitters and shared packet layouts
retain the first table's ABI/control/referent contract. Test uniform
`polygon->variant.vN + vertices` notation in place of
`vertices + polygon->variant.vN`, with no new local, integer-pointer cast,
load reordering, type, lifetime or profile change.

All instructions and ordered numeric references remain identical: 3324 bytes,
125 unequal aligned words. The notation is not retained. It supplies no
evidence for further operand/declaration permutations or a compiler mechanism.

## Verification

Focused probes actually recompile the candidates and their unchanged-source
controls. Complete TMD text bytes and ordered text/data/rodata relocation
rows are equal; the different source paths only change a debug-line offset.
The orientation-switch rejection is recorded in
[map rendering](open-map-render-residue.md). Full `kf build` preserves all
97/108 OPEN exact functions and all thirteen SDK controls; it still reports
the existing OPEN TMD default-table addend mismatch, thirteen GAME data
mismatches and four GAME historical-best deficits. Ruff, the existing 401
tests (18.070 seconds), and `git diff --check` pass. No source, type, toolchain,
test, GAME configuration or banked baseline change is retained.
