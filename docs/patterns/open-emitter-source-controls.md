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
| `80018344 render_enqueue_unlit_triangles` | 676; 99.455620% | 8 / 1 / 1 | u16 object, s16 bias; F3/FT3; frame 64 retail/56 probe, two vertex-register roles |
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

## Refreshed source controls at `4cd8fae`

The unlit and map snapshots above retain the same extents, strict scores,
ABI, calls, address pairs and delay slots. Six semantic views, complete CFGs,
callers, neighboring bodies, shared types, SDK providers and history were
reviewed before these isolated edits. No source or profile change is kept.

Unlit's three projected-vertex pointers travel together through clipping,
packed XY copies and the common depth sum. Test one three-element pointer
array in place of the three scalar locals, preserving every lane, assignment
order and use. No fourth element, extra initialization or padding is added.
The probe stores the tuple at sp+16/20/24 and reloads it, unlike retail's
register-only values. It grows from 680 to 768 bytes with 158 unequal aligned
words. Its frame happens to become 64 bytes, but fewer saved registers and
unsupported local traffic make this a rejection, not a frame solution.
Eight calls and both compiled address targets remain unchanged.

Map's stream loads a word header, advances four bytes, then advances by the
header's input-word count times four. Test a `u32 *packet` initialized from
the unchanged asset-plus-byte-offset expression, using `*packet`, `packet++`
and `packet += (header >> 8) & 0xff`. Keep the two typed polygon views and
all their accesses unchanged. This actual format-unit hypothesis emits the
identical 952 bytes, eighteen calls and two address targets. All five
normal/header spill-slot operands still differ. The word cursor is not kept;
it supplies no basis for declaration permutations to exchange those slots.

The driver-default control is recorded [separately](open-driver-default-control.md).
The final full build preserves 97/108 OPEN exact functions and thirteen SDK
controls, reporting only the known OPEN TMD addend mismatch and existing GAME
deficits. Ruff, the existing 401 tests (16.580 seconds) and `git diff --check`
pass. Only evidence notes are retained; no new exact result is banked.

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
