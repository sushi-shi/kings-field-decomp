# King's Field — Linux / WebAssembly source port

A Linux and browser port of the original Japanese King's Field (SLPS-00017).
Supply your own disc image; game data is not bundled.

## Play on Linux

On x86_64 Linux with Nix flakes enabled:

```sh
KF_DISC="/path/to/King's Field (Japan).iso" nix run github:sushi-shi/kings-field-decomp/port
```

Supply your own disc image of the Japanese King's Field (SLPS-00017).
Tested image: `King's Field (Japan).bin`, SHA-256:

```text
ae74beba377d686bfaa292ea40df8ade4454ec3139c2b5152364e02aac90b3d9
```

The first launch extracts and caches game data locally. Later launches can
reuse that cache without `KF_DISC`. Saves use three separate slots.

The game starts in English. Switch languages during play in **Configuration → Language**.
English uses John Osborne's translation, prepared from your Japanese disc.
See [translation details](docs/english-resources.md).

With `nix run`, pass game options after `--`. For example:

```sh
KF_DISC=/path/to/disc.iso nix run . -- --saves /path/to/saves
```

| Option | Purpose |
| --- | --- |
| `--saves DIRECTORY` | Use an existing save directory |
| `--language ja\|en` | Choose the starting language |

In a local checkout, use `KF_DISC=/path/to/disc.iso nix run .`.

## Install with a NixOS flake

For an x86_64 Linux system, add the game and a local directory containing your
disc to your flake inputs:

```nix
inputs.kings-field.url = "github:sushi-shi/kings-field-decomp/port";
inputs.kings-field-disc = {
  url = "path:/path/to/disc-directory";
  flake = false;
};
```

Import the module and set your disc's filename:

```nix
outputs = { nixpkgs, kings-field, kings-field-disc, ... }: {
  nixosConfigurations."<host>" = nixpkgs.lib.nixosSystem {
    modules = [
      ./configuration.nix
      kings-field.nixosModules.default
      {
        programs.kings-field = {
          enable = true;
          disc = "${kings-field-disc}/King's Field (Japan).iso";
        };
      }
    ];
  };
};
```

Nix verifies and extracts the disc into its store during installation. Rebuild
your configuration, replacing `<host>` with your host's name, then launch:

```sh
sudo nixos-rebuild switch --flake '.#<host>'
kings-field
```

## Controls

| Action | Keyboard / mouse |
| --- | --- |
| Move / strafe | WASD |
| Move / turn | Arrow keys |
| Look | Mouse or Page Up / Page Down |
| Attack | Space or left mouse button |
| Magic | Q or right mouse button |
| Interact / confirm | E or Enter |
| Inventory / skip intro | Tab |
| Back | Backspace or Escape in menus |
| Compare text languages (hold) | R |
| Pause | P or Escape during gameplay |

Hold **R** or **right-stick click** on a controller to read dialogue, inventory,
menus and messages such as “Empty” in the other language. Release to return to
your selected language. Comparison does not advance dialogue; messages stay
visible while held.

## Build from source

From the `port` branch:

```sh
nix develop
cmake --preset linux
cmake --build --preset linux
build/linux/kings-field --data /path/to/extracted/disc
```

To extract a disc with this executable, replace `--data` with
`--disc IMAGE --extract-to NEW_DIRECTORY`. The destination must not already exist.

## Browser

Inside `nix develop`:

```sh
emcmake cmake --preset wasm
cmake --build --preset wasm
python3 -m http.server --directory build/wasm
```

For the manual Windows browser-build recipe, see [browser builds](docs/browser-build.md).

Open [the game](http://localhost:8000/kings-field.html), select your disc and press
Play. The language selector also works during play. Data and saves stay in
browser storage; clearing it removes them.

## Branches

```text
              master
                 |
     +-----------+-----------+
     |                       |
     v                       v
  source                  classic
     |
     v
   port (you are here)
```

| Branch | Purpose |
| --- | --- |
| [`master`](https://github.com/sushi-shi/kings-field-decomp/tree/master) | Reconstruction and matching |
| [`source`](https://github.com/sushi-shi/kings-field-decomp/tree/source) | C++ PS1 build, codecs, and base for porting |
| [`classic`](https://github.com/sushi-shi/kings-field-decomp/tree/classic) | C PS1 build |
| [`port`](https://github.com/sushi-shi/kings-field-decomp/tree/port) | Linux and browser port |

## Development

See [analysis tools](docs/analysis.md) for sanitizers, static checks and parser fuzzing.
