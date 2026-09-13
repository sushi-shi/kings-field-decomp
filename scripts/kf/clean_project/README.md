# King's Field

Source for the original Japanese PlayStation game, SLPS-00017.
The game builds as three C programs: the loader, main game, and opening.

## Build and run

On x86-64 Linux with Nix flakes enabled:

```sh
nix build
nix run . -- --disc "/path/to/King's Field (Japan).cue"
```

`nix build` writes `PSX.EXE`, `GAME.EXE`, and `OPEN.EXE` under `result/`.
`nix run` builds those programs, inserts them into a cached copy of your disc,
and starts PCSX-Redux with its bundled OpenBIOS. The original disc is unchanged.
The SDK, compiler, assembler, linker and emulator are supplied by the pinned
Nix configuration. A graphical desktop is required to launch the emulator.

Supply the Japanese disc as a CUE with one BIN track, a raw Mode 2/2352 BIN,
or a directory containing one such disc. Alternatively set `KF_RETAIL_DISC`.
The supported BIN SHA-256 is
`ae74beba377d686bfaa292ea40df8ade4454ec3139c2b5152364e02aac90b3d9`.
No extracted resource directory or additional initialization is needed.

Generated discs live under `${XDG_CACHE_HOME:-~/.cache}/kings-field`.
Emulator working files live under `${XDG_DATA_HOME:-~/.local/share}/kings-field`.
Use PCSX-Redux's settings to configure controllers and memory cards.
Use `--prepare-only` to prepare the disc without launching; arguments after `--`
are forwarded to PCSX-Redux.

For local C development:

```sh
nix develop -c python3 build.py
```

The three outputs are under `build/psx`, `build/game`, and `build/open`.
The build uses GCC 2.5.7, ASPSX 1.07, PSYLINK and CPE2X with the Psy-Q 2.5 SDK.
Game resources are required only when preparing and running the disc.

## Resource codecs

`codecs/` contains the optional allocation-free, dependency-free `kf-codec`
Rust library. It provides readers and writers for game assets and saves and
does not participate in the PlayStation build.

```sh
nix develop .#codecs -c cargo build --offline --manifest-path codecs/Cargo.toml
```

## Development

This `source` branch is generated from the reconstruction branch. Make upstream
source changes there and regenerate with:

```sh
nix develop -c kf clean --out build/clean-source --verify \
  --publish source --worktree build/source
```

Create a separate branch from `source` for Linux or WebAssembly port work.
This branch currently builds the PlayStation target; it is not a native port.
Generation provenance is recorded in Git commit messages.

## License

Original project contributions are dedicated to the public domain under
[CC0 1.0](LICENSE), to the extent of the contributors' rights. This does not
grant rights to FromSoftware's game or Sony/Psy-Q material. Separately licensed
material retains its terms. Game assets are supplied locally and are not
included in this repository. Nix obtains the SDK separately.
