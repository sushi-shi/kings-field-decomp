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

A unit may also claim its read-only contribution once:

```c
RODATA(0x8001235c, 0x178)
```

A compiled object keeps its switch jump tables and string literals in
`.rodata`, in emission order; the retail linker laid each object's `.rdata`
into the data region in link order, so that range is a contiguous slice of
the image. The delinker carves the claimed bytes into the module object's
`.rodata`, rewrites entries that point into the unit's own code into
`.text`-relative words, and resolves the code's references to `.rodata`
offsets, so objdiff compares tables and literals too. Source literals must
therefore appear in retail order, which the address-ordered functions give
for free; a string that lives inside the range is spelled as the literal the
original used, not as a named extern.

A unit with several claims is a module: a translation-unit hypothesis whose
target object is the whole run carved as one section. Address proximity alone
does not prove the original file boundary, so module names stay WIP (a class
name, with an address suffix while a class is split by unreconstructed gaps)
and modules merge as gaps are filled.

### DATA() claims

A module owns a global by defining it under a `DATA()` claim; every other unit
declares the same global `extern`:

```c
DATA(0x80057b0c, 0x4)
static u32 frame_pacer_vsync_count = 0;
```

The claim binds the declarator to the `data_identities.tsv` row at that
address: the name and size must match, the storage must be `load` or `bss`, a
datum is claimed by one unit only, and data claims ascend inside a source.
Data need not be contiguous with the unit's code. The bindings file records
data claims with `kind=data`.

The delinker carves claimed `load` data from the retail image into the module
object's `.data` section (claims packed in order, each at the alignment its
retail address implies) and claimed `bss` data as `.bss` sizes, with
`STT_OBJECT` symbols whose binding follows the identity's `scope` (`static`
becomes a local symbol). Relocation candidates sited inside claimed data go
through the shared validator; under the safe policy the raw pointer words of
tables remain withheld (`data:` reasons in `relocations_withheld.tsv`), so a
pointer table compares by bytes only until its rows are reviewed.

On the compiled side, GCC 2.5.7 prints an uninitialized global as
`.comm name,size` (size rounded up to 8) and a `static` one as `.lcomm`;
maspsx's default rewrites both into `.bss` definitions (`--use-comm-section`
would leave COMMON symbols, which objdiff cannot pair with a section symbol).
An explicitly initialized global (`= 0` included) is emitted into `.data`,
which is how retail placed the zero-initialized frame pacer counters in the
load image. GCC 2.5.7 also prints no `.size` for data, so `kf-compile` appends
`.type`/`.size` directives for every `DATA()` claim (the curated size, recorded
in the `.o.json`); without them objdiff would infer the last datum's extent
from the assembler's 16-byte section padding. objdiff pairs data symbols by
name inside the unit and reports the `.data`/`.bss` sections next to the
functions; progress and banking still count functions only.

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
1 PSX, 492 GAME, and 241 OPEN functions. Progress is counted per function even
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
inputs unless `--dirty` is explicit. `kf bank --unit ID` updates only the
selected unit, requires every selected function to be exactly 100%, and
preserves all unrelated baseline rows. Normal builds never update committed
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

## Shared sources across images

`GAME.EXE` and `OPEN.EXE` link the same allocator, display, and TMD code.
A source is written once with the claims of its primary image; another
image reuses it with a unit that binds by name:

```toml
[[unit]]
unit = "open.memory"
image = "OPEN.EXE"
source = "src/game/memory.c"
profile = "probe-gcc257-o2-g0"
bind = "name"
defines = ["KF_OPEN"]
```

- `bind = "name"` resolves every `ADDRESS()` and `DATA()` claim through the
  unit image's own `function_identities.tsv` / `data_identities.tsv` rows by
  the definition name, so addresses stay image-qualified. The claimed size
  must still equal that image's body size; a size mismatch means the images
  do not share the body and the difference has to be expressed in source.
  `RODATA()` ranges are image-specific and cannot be name-bound.
- `defines` adds `-DNAME` (or `-DNAME=value`) to the preprocessor run for
  that unit only. Use it for constants that provably differ between the
  images (the mode-1 arena limit is `0xfefff` in `GAME.EXE` and `0x112fff`
  in `OPEN.EXE`), not to fork whole bodies silently.
