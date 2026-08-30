# Retail executable censuses

These three TSVs are the admitted, manually curated work-in-progress model of
the original Japanese `SLPS-00017` executables:

- `functions.tsv`: current function starts and explicit WIP extents;
- `data.tsv`: defined data plus `unclassified` gaps; and
- `relocs.tsv`: reconstructed relocation candidates and their review status.

`PSX.EXE`, `GAME.EXE`, and `OPEN.EXE` are separate linked programs. Every row
therefore carries an `image` even where `GAME.EXE` and `OPEN.EXE` reuse the
same virtual address. Never merge rows on address alone.

## Authority

The lists were seeded once from:

- hash-verified clean Japanese retail executables;
- Ghidra 12.0.4 with `ghidra_psx_ldr` `2026.07.08`;
- the investigation relocation/string census; and
- exact reviewed `PSX.EXE` bootstrap labels.

They are now hand-owned in the same sense as Gruntz's retail function and data
censuses. `scripts/kf/seed_retail.py` only writes review candidates below
`build/retail-seed`; it must never overwrite this directory. A new analysis
run is a proposed diff, not authority.

The first independent MIPS carving pass added 569 anonymous WIP starts that
Ghidra had not created: 228 stack-framed routines and 341 routines delimited by
clean return sequences. The admitted census is therefore 1,617 functions, not
the original 1,048-function Ghidra baseline. These rows use
`mips-frame-carve` or `mips-return-carve` confidence so they remain easy to
review or retract.

The `name` and `labels` columns preserve labels already established during the
investigation. Ghidra-supplied names remain `ghidra-analysis` candidates. As
library and game ownership improves, dedicated provider TSVs such as
`functions_psyq.tsv` and `data_psyq.tsv` can carry identity/ownership claims
while these three files remain the structural census.

## Editing rules

- A missing function is added to `functions.tsv`; correct neighbouring WIP
  extents and split/remove any `unclassified` data row it disproves.
- A newly understood datum replaces the corresponding `unclassified` span in
  `data.tsv`. Data rows may lie inside a function's coarse extent when they are
  an owned jump table or literal pool.
- A relocation candidate is promoted by setting `status=reviewed`. A disproven
  row becomes `status=rejected` so the false positive remains documented.
- Keep rows sorted by image and address. `GAME.EXE` and `OPEN.EXE` are sorted
  independently.
- Confidence and provenance describe evidence; neither is a substitute for
  manual review.

Validate structure, sorting, offsets, and complete payload accounting with:

```sh
nix develop
kf-retail-validate
```

The validator also requires each PS-X entry point and every non-rejected
reachable `jal` target to have a function row. Function extents and data rows
must not overlap others of their own class.

To produce a new comparison seed, point the explicit audit command at a clean
retail extraction and the preserved investigation evidence:

```sh
kf-retail-seed \
  --exe-dir /path/to/kings-field-japan-retail/disc \
  --evidence-dir /path/to/investigation/config/evidence/kings-field-japan-retail \
  --reviewed-symbols /path/to/investigation/config/reviewed/kings-field-japan-retail/symbols.tsv
```

The Ghidra-independent number-expansion pass can be reproduced separately:

```sh
kf-function-audit --exe-dir /path/to/kings-field-japan-retail/disc
kf-function-propose
kf-retail-validate build/function-audit/proposed-retail \
  --exe-dir /path/to/kings-field-japan-retail/disc
```

The audit combines MIPS stack allocation/RA saves, matching epilogues, stored
code pointers, cross-extent jumps, and clean return-delimited instruction
sequences. It writes candidates and a proposed merge under
`build/function-audit`; neither command overwrites `config/retail`.

The current upstream PSX analyzer emits a non-fatal exception while looking up
one GTE datatype in headless mode. Executable loading, Sleigh disassembly, and
function discovery complete; the seed deliberately does not treat those
plugin-generated GTE datatypes as retail data authority.
