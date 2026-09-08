# GCC 2.5.7 automatic aggregate spacing

These are observations of the pinned probe and OPEN retail accesses, not
historical compiler attribution or a claim that stack gaps are C members.

OPEN `80014e28 opening_ending_scroll_run` provides four concrete witnesses:

| Object | Observed bytes accessed | Retail stack base | Distance to next object |
| --- | ---: | ---: | ---: |
| Texture-page table | Nine words, 36 bytes | `sp+72` | 40 |
| CLUT table | Nine words, 36 bytes | `sp+112` | 40 |
| Top CVECTOR | Four bytes | `sp+152` | 8 |
| Bottom CVECTOR | Four bytes | `sp+160` | 8 to the next local's home |

The first texture/CLUT writes are at `80014f04` and `80014f18`; their ninth
writes are at `80015084` and `80015098`. The panel loop loads the tables at
`8001552c/80015530`, using offsets 48/88 from its `sp+24` indexed base.
The color addresses are formed at `800150dc` and `800150f4`, retained at
`sp+208` and `sp+216`, then consumed by both interpolation and gradient calls.
The initial four-byte copies end in `swr` at `80015198` and `800151b4`.
The kept source's complete raw audit matches all these accesses.

The pinned source explains why independent aggregates can have these gaps:

- `stmt.c:expand_decl` passes the declared byte size to `assign_stack_temp`
  and gives BLKmode declarations `BIGGEST_ALIGNMENT` alignment.
- `function.c:assign_stack_temp` allocates a new BLKmode slot with alignment
  argument -1 to `assign_stack_local`.
- `assign_stack_local` uses `BIGGEST_ALIGNMENT` for -1 and rounds the size
  up to that alignment.
- The supplied MIPS configuration defines `BIGGEST_ALIGNMENT` as 64 bits.

Thus a 36-byte array can occupy a 40-byte slot, and a four-byte CVECTOR can
occupy an eight-byte slot without any padding member or unused array entry
in source. This explanation agrees with the current native/traced compiler
parity and retail's concrete addresses.

Conversely, a compact `CVECTOR[2]` has a four-byte element stride, and a
plain structure containing two consecutive `u32[9]` fields has a 36-byte
field displacement. Alignment of the *whole* aggregate does not change
those internal distances to eight and forty. Neither compact grouping
preserves the observed addresses. Do not add padding or unused elements
merely to restore them.

The accesses prove minimum used extents and distances; they do not uniquely
recover declared capacities, separate-declaration spelling, or original TU
ownership. In particular, a 40-byte slot alone does not establish a ten-word
array. This is a layout constraint, not a proposed source edit or a new match.
Retail disassembly is reproducible with
`kf sema --image open disasm opening_ending_scroll_run --blocks`; current
full-source/isolated-context raw parity is recorded in the campaign's
`build/open-missed-evidence/target-tu-context/` evidence.
