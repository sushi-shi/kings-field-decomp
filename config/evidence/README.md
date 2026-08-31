# Vendored-library evidence

This directory preserves the small, reviewable inputs behind
`config/retail/functions_vendored.tsv`; it does not contain Sony binaries.

`psyq_release_25_text_sections.tsv` is the `.text` subset of the relocation-
aware object match report produced during the original King's Field
investigation. Each address was found by masking only link-editable bits in a
Psy-Q object and comparing every remaining bit against a hash-verified retail
PS-X EXE. Duplicate `S_R`/`S_W` rows intentionally retain an archive ambiguity.

`psyq_release_25_complete_objects.tsv` records six reviewed 16-byte `LIBAPI`
members in `PSX.EXE`. These were below the original matcher's 32-byte cutoff,
so their complete-byte comparisons were reviewed separately.

`kf-vendored-seed` does not blindly trust either table. It extracts the named
members from the hash-pinned Release 2.5 archive with `psy-k`, reconstructs
interleaved `.text` records, applies the retained MIPS relocation masks, checks
sizes/bit counts/relocation counts, and verifies each recorded occurrence in
the clean retail executables. It also checks XDEF offsets before using a symbol
as a function name.

The external SDK archive is supplied by the Nix environment and remains
hash-pinned there. The executable inputs are user-supplied; their SHA-256,
header, load address, and load size must match `scripts/kf/retail.py` before
the evidence or the secondary Psy-Q 2.60 signature lane is accepted.
