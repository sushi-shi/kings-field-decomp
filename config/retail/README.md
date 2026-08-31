# Retail executable censuses

These four TSVs are the admitted, manually curated work-in-progress model of
the original Japanese `SLPS-00017` executables:

- `functions.tsv`: current function starts and explicit WIP extents;
- `functions_vendored.tsv`: provider/library ownership layered over those
  structural functions;
- `data.tsv`: defined data plus `unclassified` gaps; and
- `relocs.tsv`: reconstructed relocation candidates and their review status.

`PSX.EXE`, `GAME.EXE`, and `OPEN.EXE` are separate linked programs. Every row
therefore carries an `image` even where `GAME.EXE` and `OPEN.EXE` reuse the
same virtual address. Never merge rows on address alone.

The same rule extends to decompilation output: the repository holds three link
graphs and generates `build/delink/{psx,game,open}` plus three corresponding
objdiff projects. See `docs/decompilation-layout.md` and
`docs/delinking-and-matching.md`.

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
investigation. Ghidra-supplied names remain `ghidra-analysis` candidates.
Library identity is kept out of the structural census: `functions_vendored.tsv`
references an existing `(image, va, size)` and adds provider, archive member,
member offset, version witness, evidence channel, and confidence. This is a
second hand-owned list, so later seeds are diffs to review rather than truth.

The initial vendored inventory contains 514 functions: 8 in `PSX.EXE`, 261 in
`GAME.EXE`, and 245 in `OPEN.EXE`. Of these, 375 have Release 2.5 evidence
(369 within relocation-masked exact object sections and six reviewed complete
16-byte objects). The remaining 139 are candidates from unique matches against
the later Psy-Q 2.60 wildcard signature corpus bundled by `ghidra_psx_ldr`.
That version boundary is explicit in every row. There are 502 symbol-named
rows and 12 anonymous functions whose containing object is nevertheless known.

`library` and `module` use `|` for unresolved archive aliases. For example,
identical members can prove Sony ownership while leaving the precise member
name ambiguous. `member_offset` is relative to the matched `.text` section or
signature start, not to the PS-X EXE.

## Editing rules

- A missing function is added to `functions.tsv`; correct neighbouring WIP
  extents and split/remove any `unclassified` data row it disproves.
- A newly understood datum replaces the corresponding `unclassified` span in
  `data.tsv`. Data rows may lie inside a function's coarse extent when they are
  an owned jump table or literal pool.
- A relocation candidate is promoted by setting `status=reviewed`. A disproven
  row becomes `status=rejected` so the false positive remains documented.
- Generated target objects never edit these lists. `kf-delink` consumes them
  and records applied and withheld decisions under `build/delink`; a correction
  to a missing function, datum, or relocation is curated here first.
- A provider claim is added only after the structural function exists. Prefer
  Release 2.5 object evidence; keep later-signature guesses in their explicit
  `psyq260-signature*` confidence channel.
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

## Vendored-function seed

The Release 2.5 match spans and reviewed tiny complete objects are preserved in
`config/evidence`. The seeder reconstructs the object bytes and relocation
masks, rechecks each executable occurrence, and reads XDEF names live from the
exact SDK archive members. A second lane scans the hash-verified executable
payloads against the Psy-Q 2.60 JSON signatures supplied by the pinned Ghidra
plugin:

```sh
kf-vendored-seed \
  --exe-dir /path/to/kings-field-japan-retail/disc
```

The command writes `build/vendored-seed/functions_vendored.tsv` and refuses to
write under `config/retail`. Review that proposed diff, then manually admit the
desired rows. A 2.60 signature hit is accepted only when it is unique in an
image and at least one meaningful signature label lands on an existing
structural function start; this rejects short incidental byte matches inside
game code. It is still a WIP inference, not proof that the linked member came
from Psy-Q 2.60.
