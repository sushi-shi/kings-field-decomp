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
`config/retail/data.tsv`, and `config/retail/relocs.tsv`. They cover the three
separate linked programs and intentionally retain confidence/provenance rather
than presenting machine-assisted recovery as final truth.

The current function census contains 1,617 starts: 9 in `PSX.EXE`, 939 in
`GAME.EXE`, and 669 in `OPEN.EXE`. It combines the Ghidra baseline with an
independent MIPS frame/return carving pass; both sources remain explicit in the
confidence and provenance columns.

Run `kf-retail-validate` to check schemas, sorting, virtual/file offsets, and
complete payload accounting. See
[`config/retail/README.md`](config/retail/README.md) for the manual-curation and
future label-provider contract.
