# Build and progress system

The build workflow borrows the portable part of Gruntz: an umbrella command,
an explicit unit manifest, a generated Ninja graph, content-aware artifacts,
and a manual high-water ledger. It does not import Gruntz's PE/PDB model,
COFF normalization, source-label extraction, Windows linker, or C++ gates.

## Unit manifest

`config/units.toml` is the only source-enrolment mechanism. A unit has a stable
ID, one of the three canonical executable names, an admitted retail function
VA, a repo-relative source, and a named complete profile. Duplicate IDs or
`(image, va)` identities, missing sources, fragmented functions, and Sony/Psy-Q
vendored functions are errors.

The first topology remains one contiguous function per unit. This is a
calibration topology, not a claim about original translation units. Grouping
functions and data must wait for ownership evidence and a target-object model
that can represent it.

The current `probe-gcc260-o2-g0` name is deliberately non-attributive. It makes
the practical GCC 2.6.0/maspsx route reproducible without claiming that the
historical compiler, optimization profile, assembler, or linker is proven.

## Incremental graph

`kf configure` writes `build/build.ninja`. Each image has independent
`base-IMAGE`, `target-IMAGE`, `compare-IMAGE`, `verify-IMAGE`, and `all-IMAGE`
aliases; the unsuffixed aliases join the three graphs only as build targets.
Addresses and objdiff reports never cross image boundaries.

The graph tracks retail executables, curated TSVs, source files, transitive
repo-local headers, manifest profiles, analysis scripts, and a generated
toolchain identity. Target objects, reconstruction objects, TSVs, projects,
and empty reports are written only when content changes. Removing a unit
prunes its orphan base object at configure time.

## Status and banking

The status universe is every contiguous, non-vendored target object: currently
1 PSX, 557 GAME, and 306 OPEN functions. Only manifested units with real base
objects enter objdiff. This keeps an absent reconstruction distinct from a
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
