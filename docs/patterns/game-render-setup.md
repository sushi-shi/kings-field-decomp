# GAME error-screen brightness and TMD countdown setup

## Function Match Plan at `4199d60`

Use hash-verified GAME retail and the unchanged `probe-gcc257-o2-g0`.
Both functions live in `game.render`; its fifteen exact siblings are controls.
The six image-qualified semantic views, complete retail CFGs and delay slots,
source history, adjacent boundaries, shared types and SDK evidence were read
before editing. These are game-owned display/resource policies, not SDK bodies.

| Function | Starting strict result | Evidence and first source hypothesis |
| --- | --- | --- |
| `8001b7b0 display_show_error_screen` | 97.922680%; 772 compiled / 776 retail bytes | Word stage, seven call sites passing 0/1/2/3, no consumed return; nineteen calls, 22 validated address pairs, no indirect control or candidate references. Retail has an 80-byte frame and a 40-byte SDK POLY_FT4 at sp+16. Preserve the seven-byte error path, digit at +2, sector rounding, fifty-attempt CD loop, input press/release loops and environment flags. The current chained colour assignment writes B/G/R, versus retail R/G/B. Test the supplied SDK `setRGB0` macro. |
| `8001c2b0 tmd_prepare_primitive_indices` | 98.333336%; 768 bytes | Three callers select the current asset before entry; no arguments, direct calls or strings. Four validated address pairs, seven internal jumps and one unresolved indirect dispatch; its 29 incoming table rows remain candidates. Preserve halfword-narrowed disk counts, byte packet lengths, eight mode bodies, `0xfd` mode mask and wrapping index shifts by three. Retail prepares the countdown on both outcomes of the primitive-zero guard. Test the two-statement unconditional setup established independently in OPEN. |

The error-screen caller sites are `8001480c`, `80018910`, `8001ad60`,
`8001ae3c`, `8001af70`, `8001b00c` and `8001b0dc`. Their argument delay
slots and following result handling were inspected, along with the CD-loader,
main-loop and player-update source. `tim_upload_images` retains its separate
TIM reader/upload loop. The preceding resource loader ends at `8001b7ac`;
the following colour-matrix selector remains exact.

Psy-Q Release 2.5 LIBGPU.H defines `setRGB0` as three comma-separated R/G/B
assignments. Its real POLY_FT4, DrawSync, GetClut/GetTPage and ordering-table
interfaces remain unchanged. PRIM's independently matched archive contribution
and SYS FID evidence attribute those services separately; the error path,
retry policy and selected display state are not vendor progress.

The TMD callers are `tmd_register`, `asset_registry_load_tmd_archive` and
`asset_registry_set`; all three complete retail bodies were inspected.
The preceding view helper and following registration/release boundaries,
shared word-sized disk headers and halfword packet fields retain their models.
The SDK's separate OpenTMD/ReadTMD/unpack_packet family does not implement this
custom in-place index preparation. An indirect-jump case arm is not proved
unreachable merely because the current CFG builder lacks its table edges.

## Error-screen value lifetime

The literal-valued SDK macro recovers the RGB store order but retains the
72-byte frame and late brightness materialization. Retail instead executes
`li s3,0x60` in CdSearchFile's delay slot at `8001b8bc`, before testing its
result. The same value supplies the stores at `8001ba08/0c/10`. Test a real
signed-word brightness value assigned immediately before the CD lookup and
consumed by that macro; do not add a register constraint, dummy storage,
extra call, early RGB write or alternate source-order permutations.

That source form closes the function at strict **100%**. All **776 linked
bytes**, nineteen ordered calls, 22 materialized addresses, branch targets
and load/control delay slots agree. The 80-byte frame and saved s3 emerge
from the actual brightness lifetime; no padding is introduced. All fifteen
previously exact unit siblings remain exact. The shared graphics-owner pilot
also reproduces all 776 bytes, but its unresolved storage intervals remain
temporary and no new production data claim is made.

## TMD countdown verdict

Move `primitives_left = primitive_count; primitives_left--;` immediately
before `if (primitive_count != 0)`, retaining the preceding packet calculation.
The retail guard at `8001c2f0` owns `addu a1,a0,t1` with t1=65535;
the previous source put packet addition in the branch slot and prepared
the countdown afterward. This two-statement form preserves the retail
packet/count register roles, unlike the older rejected decrement experiment.

Strict objdiff improves to **99.375000%**. Both canonical and temporary-owner
comparisons contain 768 bytes; exactly two linked words remain unequal:

| Offset | Retail | Compiled |
| --- | --- | --- |
| `+8` | `addiu sp,sp,-8` | `nop` |
| `+2fc` | `addiu sp,sp,8`, the return delay slot | `nop` |

All other instructions, four materialized addresses and seven internal jump
targets agree. The unused frame has no established source owner; no artificial
local is added to create it. This function is **not exact and is not banked**.
The existing owner control now checks this complete two-word residue set;
the exact error screen instead follows its full-body equality path.

## Verification

Focused canonical and temporary-owner compiles independently resolve every
instruction relocation before raw comparison. Strict reports confirm both
results above. All 63 ordered error-screen relocation rows agree in relative
site, kind, symbol and encoded addend. The native full-image report changes
only these two functions among 484 rows; no existing exact function regresses.
GAME advances **289 -> 290 / 362 exact**, OPEN stays **97/108**, and PSX
stays 1/1: **388/471** overall. Only GAME `8001b7b0` is selected for banking.

Ruff, `git diff --check` and all **642** repository tests pass (81.316s,
no skips). The complete, freshly rebuilt TMD oracle passes **250 payloads**:
246 shipped plus four synthetic, covering 1,911 shipped objects and 117,119
packets. Retail/C/Rust complete mutated bytes agree; retail and candidate
each execute 4,783,525 instructions. This finite corpus does not replace
the strict byte criterion or establish arbitrary-input equivalence.

The subsequent full `kf build -j4` retains the existing explicit gates:
source data **5/60**, config-backed SDK data **4/4**, target relink
**110/116**, six conflicting section bases and zero artifact failures.
Known-reference ownership remains incomplete. These failures are neither
waived nor evidence of function-exact regression. The OPEN coordinate trial
is restored and recompiled; no OPEN source, shared type, inventory, compiler
setting, SDK implementation or complete graphics-owner claim changes.
