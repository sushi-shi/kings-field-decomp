# GAME map-cell selection and coordinate source

## Function Match Plan at `4d59c88`

GAME `render_map_cell`, `8001e5ec`/`0x250` (592 bytes), starts at strict
76.783780% under the unchanged `probe-gcc257-o2-g0` profile. Retail hashes,
all six image-qualified semantic queries, complete CFG/disassembly, sole
traversal caller, SDK and vertex-selection boundaries, adjacent sprite tail,
source history, shared types and vendor evidence were refreshed before editing.
The [newly exact OPEN counterpart](sdk-vector-publication.md) motivates a
natural XYZ source-order control, not transplantation of GAME-specific policy.

Retail has a 120-byte frame, nine direct calls, ten validated HI16/LO16 pairs,
five internal jumps, twelve conditional branches and one shared return.
There are no strings or candidate outgoing references. Attribute 255 and
postdecrement indices above 99 both reach `8001e824`, whose return delay slot
frees the frame. MATRIX begins at sp+16, position halfwords at sp+80/82/84,
the flag at sp+88 and saved s0/s1/s2/ra at sp+104/108/112/116.

The exact 360-byte `render_map_cells` caller bounds signed word row/column
to 0..99 and passes a nonzero visibility byte at `8001e938/3c`. It uses
204-byte windows, a pitch-dependent fixed window, byte loop counts and
full-width skipped-row strides. Keep its source unchanged.
Its local fixed-window reference is a genuine ELF `.data` section reference;
resolve that section to `80055e9c` when comparing raw words. A `kf try`
symbol-spelling difference here is not a byte or numeric-target mismatch.

The grid is unsigned bytes. Retail loads the selected attribute into s2,
narrows it into a3 with `andi ff`, performs the door policy, then decrements
the same byte-sized state with `addiu a0,s2,255; move s2,a0`. The bounds test
narrows s2 again. Cell value 1 adds 100 to that decremented value, and both
final TMD calls receive its low byte. The original C instead uses separate
word-sized `attr` and `tex` values and narrows only at the bound/call sites.

The light timer is a signed halfword at player_state+0x52. Unless it equals
-1, phases `(light & 3) < 2` remap 0x44/0x45/0x46 to 0x17/0x18/0x19.
Retail compares the middle case first, branches to out-of-line case arms,
and joins their shared decrement. The original nested if/else source has
different branch polarity, arm placement and jump count. Test an ordinary
three-case switch, keeping the same mapping and unchanged default behavior.

Position remains an SDK SVECTOR. Preserve col/row times 2000, unsigned-byte
height times -100, unsigned low-halfword camera reads and the three orientation
adjustment arms. Test `setVector`'s natural XYZ order rather than treating
retail's X/Z/Y stores as C statement order. LIBGPU.H defines this macro as
three field assignments without a padding write.

The grid, door and object-selection policy is game-owned. SDK calls have
separate LIBGTE MTX/SMP attribution: the matrix-register setters have Release
2.5 FIDs, while MulMatrix0/RotTrans retain the established cross-overlay
lineage with older exact SDK revision unresolved. Authentic MATRIX/SVECTOR/
VECTOR types and prototypes are unchanged; RotTrans writes three translation
words at MATRIX.t and MulMatrix0 supplies the rotation.

## Focused results

Each step was independently compiled and checked against raw words, numeric
calls/references and the recorded strict report before the next edit:

| Source step | Strict production match | Evidence recovered |
| --- | --- | --- |
| Natural XYZ coordinate assignments | 86.236490% | Retail's coordinate arithmetic/load order and constant-one placement. |
| Byte-sized attribute and index locals | 86.270270% | Explicit unsigned-byte attribute comparison, while separate values still produce the wrong decrement form. |
| Three-case door switch | 95.567566% | Retail's signed case split, out-of-line arms and shared paths. |
| One byte object index, decremented in place | 96.743240% | Retail's +255 decrement, copied result and later +100, with the object value retained through both final calls. |

The retained source uses one `u8 object_index` for the grid attribute, optional
remapping, decrement and foreground-bank addition. No extra bounds, fields,
padding, fake locals, forced instructions or compiler changes are introduced.
The traversal remains strict 100% and all 360 raw bytes agree.

## Remaining production and shared-owner differences

Production emits 588 bytes versus 592 retail. Its first difference is the
88-byte frame versus 120; it saves two s-registers rather than three. It also
forms `light_quadrant_matrices` independently rather than deriving it from the
retained view-quadrant address. All nine calls and physical data targets are
correct, but production has eleven materialized address pairs versus ten.
This is an improved partial, not a bankable exact match.

The existing [complete-clear owner pilot](game-graphics-owner-pilot.md) uses
the same current function through its already established temporary shared
declaration. It emits 592 bytes, and every branch site/condition/destination,
call, data address and non-stack instruction agrees with retail. All 130
non-differing words are compared without masking. The only eighteen unequal
words are at these function-relative offsets:

```text
000 004 008 00c 010 148 164 190 1a4
1b0 1bc 1e0 1ec 238 23c 240 244 24c
```

They encode the 88/120-byte frame sizes and the 32-byte difference in saved
register, SVECTOR and flag offsets. The MATRIX stays at sp+16 and its
translation output stays at sp+36. No instruction accesses the retail
interval sp+48..79. Its existence does not prove an unused MATRIX or authorize
adding one. Exact historical compiler/profile and the original local storage
remain unresolved; these are observed differences, not an optimizer diagnosis.

The owner pilot is still not a production DATA owner. Registry/projection/
morph boundaries and the remaining consumer audit are independent work; this
text result does not justify inventing capacities or waiving data gates.
The existing raw owner test now asserts precisely these eighteen stack-word
differences, all numeric calls/addresses and the full exact traversal. Other
startup/initializer/view-transform controls keep their complete raw assertions.

## Campaign verification

The focused shared-owner control passes, `ruff check scripts tests` passes,
and all 642 repository tests pass (79.830 seconds). The full `kf build` was
run after fresh affected-unit compilation. Across all 484 function report rows,
only GAME `render_map_cell` changes, from 76.783780% to 96.743240%; no rows
disappear and no exact function regresses. GAME remains 291/362 exact, OPEN
98/108, and the overall non-vendored count remains 390/471. No new banking is
appropriate for this partial result.

The full build retains the pre-existing data/closure/placement failures:
5/60 source data-owning units match, all four independently supplied SDK data
contributions match, and target relinking verifies 110/116 units with six
conflicting section bases. There are no artifact failures. This campaign does
not change data claims, identities, the compiler profile, or the bank baseline.
