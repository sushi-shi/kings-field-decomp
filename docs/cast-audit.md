# Target-C cast audit

`kf casts` parses the reconstruction with Clang in the target C89 language
mode and reports explicit C-style cast expressions written under `src/` and
`include/`. Enter the pinned environment before running it:

```sh
nix develop
kf casts
```

The default audit parses every C translation-unit variant in
`config/units.toml`. This includes shared sources once for each image or
preprocessor context. A cast written in a shared source, header, or macro is
counted once by its Clang spelling location; the report retains every unit,
function, source type, and target type observed at that location.

Use filters to choose the next review band:

```sh
kf casts --kind pointer --scope source --list
kf casts --image open --path src/open/render_ --list
kf casts --unit game.effect_dispatch --json
kf casts --kind pointer --scope source --max 400
```

`--kind` accepts `pointer`, `scalar`, `mixed`, or `all`. `mixed` identifies a
macro spelling whose expansions produced more than one target category.
`--scope` separates C source from project headers, and repeated `--path`,
`--image`, or `--unit` options form a focused selection. `--max N` exits with
status 1 when the filtered written-site count exceeds the limit, so a campaign
can lower an explicit ceiling without changing the report.

`--json` emits schema version 1 with the summary and full site ledger. Each
site includes its spelling location, source line, category, observed source
and target types, and image-qualified unit/function contexts. `--list` emits
the same sites as tab-separated review lines.

The audit fails closed on target-C parse errors, a source change during the
run, or a project header that the full manifest did not parse. It excludes
implicit conversions and casts written only in external SDK headers. The raw
AST expansion count remains in the report to show why spelling-location
deduplication matters.

The count is a navigation and regression tool. A cast should be removed only
when source and retail evidence support the resulting type or object model;
ABI, serialization, SDK, and explicitly generic-storage boundaries can still
require an explicit conversion.
