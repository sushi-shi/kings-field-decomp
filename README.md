# King's Field

Clean reverse-engineering and matching-decompilation project for the original
Japanese King's Field (`SLPS-00017`).

## Environment

Enter the complete analysis/build shell with:

```sh
nix develop
```

On the first load, Nix downloads 6.3 MiB of hash-pinned original media and runs
`scripts/create-toolchain.py`. The resulting 7.7 MiB Psy-Q candidate toolchain
is cached in the Nix store; later shell loads reuse it. The shell exports
`PSYQ_DIR`, `PSYQ_BIN`, `PSYQ_INCLUDE`, `PSYQ_LIB`, and the three compiler
candidate directories. It also provides Ghidra/PyGhidra, DOSBox, MIPS
little-endian GNU binutils, maspsx, psy-k, disc-image utilities, and the normal
C/C++/Python build tools.

The Splat Python stack is locked separately because Rabbitizer and spimdisasm
are not in the pinned Nixpkgs revision. Run it through `uv run`, for example:

```sh
uv sync --frozen
uv run splat --help
```

`nix develop` initializes the toolchain automatically through `flake.nix`; no
separate Nix entrypoint or derived toolchain release archive is needed.

The shell's `ghidra` is Ghidra 12.0.4 with the pinned PSX loader extension
already installed. `GHIDRA_INSTALL_DIR` identifies base Ghidra, while
`NIX_GHIDRAHOME` exposes the same pinned extension tree to the GUI, headless
analyzer, and PyGhidra. The extension supplies PS-X EXE and overlay loading,
PSX GTE language support, Psy-Q OBJ/LIB loading, and upstream Psy-Q signatures.

See [docs/ghidra.md](docs/ghidra.md) for the plugin pin, packaging model, and
the important signature-version boundary.

## Retail executable model

The admitted WIP inventories live in `config/retail/functions.tsv`,
`config/retail/functions_vendored.tsv`, `config/retail/data.tsv`, and
`config/retail/relocs.tsv`. They cover the three separate linked programs and
intentionally retain confidence/provenance rather than presenting
machine-assisted recovery as final truth.

The current function census contains 1,617 starts: 9 in `PSX.EXE`, 939 in
`GAME.EXE`, and 669 in `OPEN.EXE`. It combines the Ghidra baseline with an
independent MIPS frame/return carving pass; both sources remain explicit in the
confidence and provenance columns.

The separate vendored-function layer currently identifies 514 Sony/Psy-Q
functions. It distinguishes 375 Release 2.5 exact-object claims from 139
version-skewed Psy-Q 2.60 signature candidates. Regenerate a review seed with
`kf-vendored-seed --exe-dir /path/to/retail/disc`; the command cannot overwrite
the hand-owned list. Vendored functions remain reference evidence and call
targets; they are excluded from objdiff units and decomp progress.

Run `kf-retail-validate` to check schemas, sorting, virtual/file offsets, and
complete payload accounting. See
[`config/retail/README.md`](config/retail/README.md) for the manual-curation and
provider-layer contract, and [docs/vendored-functions.md](docs/vendored-functions.md)
for the evidence rules and current breakdown.

## Three decompilation targets

This is three decomps in one repository. `PSX.EXE` is the bootstrap/loader,
`GAME.EXE` is the main game, and `OPEN.EXE` is the opening/title program.
`GAME.EXE` and `OPEN.EXE` reuse the same RAM window at different times, so each
has an independent address namespace, link graph, target-object set, and
objdiff project. Tooling, evidence, headers, and proven shared source stay
common.

The initial TSV-driven delinker emits conservative ELF32 little-endian MIPS
objects and a complete used/withheld relocation audit. Configure the local,
hash-verified extraction once, then use the unified build loop:

```sh
kf init --retail-dir /path/to/hash-identical/retail
kf build
kf match
kf status
```

`kf build [all|base|target|compare|verify]` accepts repeatable
`--image psx|game|open`, `--retail-dir`, `-j`, and `-v`. It configures
`build/build.ninja` when needed and is content-incremental; a no-op second
build runs no commands.
`kf match` builds and reports changed reconstruction objects. `kf status` is
read-only, `kf check` enforces the manually owned high-water ledger, and
`kf bank` is the only command that updates it.

With no `--image` selector, `kf status` reads all three generated objdiff
reports and prints one row for each independently linked executable plus a
combined total:

```text
$ kf status
exact threshold: 100.000%
image  eligible  started   built  scored   exact   coverage  fuzzy/built
psx           1        0       0       0       0     0.000%       0.000%
game        673        0       0       0       0     0.000%       0.000%
open        422        0       0       0       0     0.000%       0.000%
total      1096        0       0       0       0     0.000%       0.000%
ledger: no changes
```

Use `kf status --image psx`, `--image game`, or `--image open` for one
executable. Repeat `--image` to select any combination, and add `--json` for a
machine-readable document containing the same per-image and total fields.

Exact means exactly `100%`. `kf status --loose` and `kf match --loose` use a
`99.995%` navigation threshold; loose mode never rounds the banked score.

Reconstruction units are explicit in `config/units.toml`. Each unit binds one
source, image, retail VA, and fully named compiler-probe profile. Reconstructed
objects go under `build/objdiff/{psx,game,open}/base`. Unstarted functions stay
in the 1,096-function coverage denominator but do not enter objdiff through a
dummy placeholder. The pinned `objdiff-cli` and GUI are included in
`nix develop`; the earlier low-level commands remain available for focused
experiments.

See [docs/decompilation-layout.md](docs/decompilation-layout.md) for the
three-target architecture and
[docs/delinking-and-matching.md](docs/delinking-and-matching.md) for relocation
policy, artifacts, commands, and the Vostok/Gruntz boundary. See
[docs/build-system.md](docs/build-system.md) for manifest, graph, and ledger
contracts.
