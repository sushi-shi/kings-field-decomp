# Build and progress system

The build workflow borrows the portable part of Gruntz: an umbrella command,
an explicit unit manifest, a generated Ninja graph, content-aware artifacts,
and a manual high-water ledger. It does not import Gruntz's PE/PDB model,
COFF normalization, source-label extraction, Windows linker, or C++ gates.

## Unit manifest and ADDRESS() claims

`config/units.toml` is the only source-enrolment mechanism. A unit has a stable
ID, one of the three canonical executable names, a repo-relative source, and a
named complete profile. It carries no address: every function a source
reconstructs claims its retail address in the source itself,

```c
#include <kf/address.h>

ADDRESS(0x8001499c, 0x38)
void game_shutdown(void)
```

`ADDRESS()` expands to nothing; `scripts/kf/model.py` reads the claims and the
manifest loader binds them (`build/gen/bindings.tsv` records the result). The
binding rules are structural, in the spirit of Gruntz's `RVA()`:

- a claim must name an admitted, non-vendored, non-fragmented function of the
  unit's image, spelled exactly as `function_identities.tsv` names it, so a
  labelled function is never called by an address-derived name in source, and
  its size must equal the admitted retail body size, so the claim and the
  census cannot drift apart silently;
- claims inside one source ascend by address, and a unit owns every admitted
  function between its first and last claim, so a source file is a contiguous
  run of the linked image (address-order incrementalism);
- units are listed in ascending address order within each image, so the
  manifest itself reads as the recovered link order; and
- an address is claimed by at most one unit.

A unit with several claims is a module: a translation-unit hypothesis whose
target object is the whole run carved as one section. Address proximity alone
does not prove the original file boundary, so module names stay WIP (a class
name, with an address suffix while a class is split by unreconstructed gaps)
and modules merge as gaps are filled. Data ownership is not yet part of the
model.

Profile names are deliberately non-attributive. `probe-gcc260-o2-g0` keeps
the original GCC 2.6.0/maspsx route reproducible; `probe-gcc257-o2-g0` is the
default for new units because the GCC 2.5.7 rebuild natively emits the retail
framed epilogue and load hoisting, and its `maspsx_flags = ["--expand-div"]`
selects the checked `div` expansion retail contains. Neither name claims that
the historical compiler, optimization profile, assembler, or linker is proven;
the evidence and corpus numbers are in
[`patterns/gcc257-epilogue-and-scheduling.md`](patterns/gcc257-epilogue-and-scheduling.md).
A profile may set `compiler` to any listed native probe and may pass extra
`cc1_flags` and `maspsx_flags`; every field is part of the unit fingerprint.

## Incremental graph

`kf configure` writes `build/build.ninja`. Each image has independent
`base-IMAGE`, `target-IMAGE`, `compare-IMAGE`, `verify-IMAGE`, and `all-IMAGE`
aliases; the unsuffixed aliases join the three graphs only as build targets.
Addresses and objdiff reports never cross image boundaries.

The graph tracks retail executables, curated TSVs, source files, transitive
repo-local headers, manifest profiles, analysis scripts, and a generated
toolchain identity. The delink edge also depends on the manifest and unit
sources because module target objects are carved from their claims. Target objects, reconstruction objects, TSVs, projects,
and empty reports are written only when content changes. Removing a unit
prunes its orphan base object at configure time.

## Status and banking

The status universe is every contiguous, non-vendored function: currently
1 PSX, 497 GAME, and 246 OPEN functions. Progress is counted per function even
when several functions share a module unit; the objdiff report lists each
function inside its unit. Only manifested units with real base objects enter
objdiff. This keeps an absent reconstruction distinct from a
real 0% comparison and prevents zero-total dummy objects from reporting 100%.

The committed `config/match_baseline.tsv` is keyed by `(image, va)`. Its input
fingerprint includes source, transitive local headers, the complete profile,
and toolchain identity. An unchanged fingerprint below its best score is a
regression. A changed fingerprint is reported separately; `--strict` also
gates it against the historical best. A banked identity that disappears from
the report is a loss.

Only `kf bank` mutates the ledger. It refuses stale reports and dirty build
inputs unless `--dirty` is explicit. Normal builds never update committed
progress automatically.

## Generated README status

The compact status block at the top of `README.md` is derived state, not
hand-written documentation. `scripts/kf/readme.py` renders it from the same
strict-100% snapshot as `kf status` and replaces only the text between
`<!-- match-score:start -->` and `<!-- match-score:end -->`.

`kf check` refreshes the block, and `kf build` ends in that check. `kf bank`
also refreshes it after updating the manual ledger. Concurrent per-image checks
serialize snapshot and replacement through `build/gen/readme.lock`; content is
written atomically and only when it changes. README generation never changes
`config/match_baseline.tsv`.
