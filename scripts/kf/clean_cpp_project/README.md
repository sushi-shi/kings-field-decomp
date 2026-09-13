# King's Field — C++ source

Clean C++20 sources for the original Japanese King's Field (SLPS-00017).
This branch is the base for the crossplatform port. It builds for the PlayStation so
the game can be exercised before changing platforms.

```text
              master
                 |
     +-----------+-----------+
     |                       |
     v                       v
  source (you are here)   classic
     |
     v
   port
```

| Branch | Purpose |
| --- | --- |
| `master` | Reconstruction and matching |
| `source` | C++ PS1 build, codecs, and base for porting |
| `classic` | C PS1 build |
| `port` | Crossplatform port |

## Build and run

On x86_64 Linux with Nix flakes enabled:

```sh
nix build
nix run . -- --disc "/path/to/King's Field (Japan).cue"
# Run the original disc instead:
nix run . -- --retail --disc "/path/to/King's Field (Japan).cue"
```

Supply your own original Japanese disc image. Game assets are not included.
The runner creates a cached disc containing the rebuilt executables and starts
PCSX-Redux. It leaves the original image intact.

`nix develop -c python3 build.py` builds into `build/` for local development.
The PS1 build uses Clang, a MIPS linker, and the pinned Psy-Q SDK libraries.
Game code lives in `src/` and `include/kf/`; PS1 build support lives under
`scripts/psxbuild/` and `vendor/include/psyq/`.

## Codecs

`codecs/` contains the optional Rust codec library:

```sh
nix develop .#codecs -c cargo build --offline --manifest-path codecs/Cargo.toml
```

## Regeneration

The reconstruction branch generates this tree. Matching annotations, inventories,
analysis tooling and tests are absent. Keep platform-port changes on `port` so
this base can be regenerated from `master`.
