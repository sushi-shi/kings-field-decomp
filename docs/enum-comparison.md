# Comparing enum constants

`kf enums` collects evaluated enum members into a comparison table. Use the
names and their source consumers to decide whether two constants describe the
same quantity. Equal values are leads for that review, not a merge rule.

Run in `nix develop`:

```sh
kf enums > build/enum-comparison.tsv
kf enums --duplicates > build/enum-duplicates.tsv
kf enums --value 10
kf enums --value 0x18e --value 0x1c7
kf enums --image game --unit game.actor --json > build/actor-enums.json
```

The TSV columns are evaluated decimal value, hex value, name, source file,
line, enum tag, images, and manifest units. Rows with the same value are
adjacent. `--duplicates` keeps values declared at multiple sites, including
the same name declared independently in two TUs. A shared header declaration
is one row with all its image/unit contexts. JSON additionally retains column,
byte offset, and each context's preprocessor defines. A declaration whose value
changes between variants has separate rows for those values.

The helper extends the existing target-C analysis infrastructure: manifest
variant selection and compiler arguments from the type/cast tooling, and the
pylibclang traversal used by `scripts.kf.pointer_zeros`. It uses the pinned
MIPS-I/O32 frontend flags, project and SDK include paths, and each unit's
defines. Clang resolves decimal/hex/octal spellings, constant expressions,
aliases, negative values, unsigned values, and implicit increments. Anonymous
enums have an empty tag column; their member names and locations remain explicit.

Coverage is every active enum declaration in the selected manifest C variants
and included project headers, including function-local enums. External SDK
declarations, inactive preprocessor branches, and headers that no selected TU
includes are outside this report. Parsing errors abort with exit status 2 and
no partial table. The command never edits source or curated inventories.
Generated tables belong under `build/` and are not committed.

For each plausible pair, read all uses with `rg`, check the quantities and
operations involved, and follow shared producers/consumers. A shared unit
conversion, storage convention, or effect parameter can warrant one definition
in its owning header. Unrelated IDs, separate tuning parameters, and quantities
with different units retain their own names. The helper supplies names and
values; the semantic decision remains an evidence review.

The [shared-constant review](patterns/shared-constant-review.md) records the
value-10 example, both actor angle thresholds, wider cross-TU findings, source
changes, and binary verification.
