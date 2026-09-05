# MIPS address expressions are not allocation identities

A decoded HI16/LO16 address is `S+A`. It can lie before, after, or numerically
inside an unrelated object even when the real indexed access belongs to `S`.
Do not infer an allocation owner solely from the bytes at the decoded address.

The SDK's `sin_1` provides an independently reproducible control: its third
quadrant builds `rsin_tbl-0x1000`, then adds twice an angle in 2048..3071. The
actual signed-halfword reads are offsets `0..0x7fe` in the 2048-byte table.
Psy-Q `GEO.OBJ` explicitly carries this negative linker addend, and the complete
table plus relocated consumer bodies equal retail in both GAME and OPEN.
See the [SDK/retail dossier](../../config/evidence/psyq_rsin_table.md).

The implementation contract is:

- Preserve the raw decoded target and the signed addend; do not rewrite an
  instruction constant to the owner's start to improve a comparison.
- Resolve explicit reviewed data names within one image. Ambiguous names fail
  delinking and remain candidate evidence in navigation. Byte validation alone
  does not review a candidate owner spelling.
- Traverse the named allocation, not the unrelated bytes containing `S+A`.
  Keep competing hypotheses separate and genuine code/data overlaps visible.
- Express a unit-owned literal/table through its actual RODATA contribution,
  even if the biased base lies outside that contribution. Conversely, numerical
  coincidence with unrelated RODATA does not transfer data ownership.
- Address construction and one-past pointers are not dereferences. A direct
  memory-op low still contributes its real address and access width to bounds
  diagnostics. This rule does not prove arbitrary later index arithmetic.
- Delink the pair to the same named symbol with its original signed addend.
  Applying the real owner address, including carry-adjusted HI16 semantics,
  must restore every retail word without masking relocation bits.

`tests/test_data_referents.py` supplies synthetic controls for these boundaries;
`tests/test_psyq_rsin_table.py` checks the original SDK patches and retail pairs.
A faithful owner/addend model does not by itself supply a default comparison
for config-only allocations or prove complete reachable-byte closure.
