# Reviewing parameter names

`kf parameters` supplies the complete argument lists seen by pylibclang in
every selected manifest C variant, then marks parameters with enum, struct,
union or class domains for individual semantic review. A type's name alone
does not determine a parameter's role: two vectors may be a source and a
destination, while one overlay selector should keep the same name through
declarations and definitions.

Run in `nix develop`:

```sh
kf parameters > build/parameters.tsv
kf parameters --all > build/all-parameters.tsv
kf parameters --json > build/parameters.json
kf parameters --by-type > build/parameter-types.tsv
kf parameters --image open --unit open.opening_scenes --json
```

The default TSV includes every selected enum/record parameter source site,
its callable, zero-based argument index, name, domains, modern/retail type
spellings, declaration/definition flag, and image/unit contexts. `--all`
also includes scalar and unresolved dependent parameters. JSON always includes
all parameters, full per-variant observations, callable groups, and coverage.
An empty name records an unnamed argument. IDs such as `P00001` identify one
sorted snapshot; use file, callable source location, argument index and domain
when comparing reviews across edits.

Both frontend modes use the pinned MIPS-I/O32 flags and manifest defines.
The modern view preserves domains hidden by the retail `KF_ENUM_BEGIN`,
`KF_ENUM_PARAM`, `KF_ENUM_PROMOTED`, and `KF_ENUM_STORAGE` macros. The retail
view records the ABI type and catches declarations hidden from the modern
view. Typedefs, const qualification, pointers, references, arrays, unions,
and SDK record types on project function parameters are included. Concrete
`KfEnumStorage` specializations identify their enum domain. Dependent template
parameters remain in the all-parameter list without an invented concrete type.

Shared header sites retain all image/unit/define observations without becoming
separate review rows. Modern-only declarations and definitions remain visible
through `modes`; this includes typed overloads and checking helpers. Nested
callback signatures have their own parameter rows, including function-pointer
typedefs and fields, and are marked `callback_signature`. Vendored source
parameters remain in the inventory with `origin: vendored`.

JSON groups reconcile external functions by image and argument index and
internal functions by image, unit and argument index. Modern overloads retain
distinct Clang identities, separately from their retail ABI bridge; the tool
does not claim those signatures are interchangeable. Macro-generated callable
occurrences sharing an expansion site retain separate ordinals and identities.
Source excerpts come from literal expansion ranges, which can be empty for
macros; token expansion text is not treated as a written parameter name.

The separate JSON `by_type` index maps each canonical enum/record domain to
all parameter names and source sites across functions. It retains per-name
buckets, functions, domain routes, written type spellings and image/unit/define
contexts. The key includes image and the type declaration's identity and source
location, so unrelated types with the same spelling are not combined.
`--by-type` presents those buckets as TSV, with one row per type and parameter
name. This is the explicit type-to-names comparison; callable groups separately
check redeclaration agreement. Both supply review leads without deciding that
a source vector and destination vector should share one name.

For each row, inspect the declaration, body, callers and relevant data domain.
Record whether its spelling agrees with its actual role, which peer names
should agree, and why a different spelling is warranted. Group spelling flags
only identify leads. Equal names can conceal different meanings, and different
names can correctly distinguish roles. The command never renames parameters
or writes curated inventories.

Coverage records each parse, included project files, unseen project headers,
unselected source files, diagnostics, and any parameter cursor without an
identified owner. Parse errors abort with status 2 and no partial report.
The scope is active declarations in selected manifest variants and included
project headers: an empty unseen-header list does not certify inactive
preprocessor branches. External SDK function declarations are excluded.
Generated JSON and TSV reports belong under `build/` and are not committed;
the reviewed decisions belong in a separate curated ledger.
