# King's Field — Linux / WebAssembly source port

A Linux and browser port of the original Japanese King's Field (SLPS-00017).
Supply your own disc image; game data is not bundled.

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
| `master` | Reconstruction and matching |
| `source` | C++ PS1 build, codecs, and base for porting |
| `classic` | C PS1 build |
| `port` | Linux and browser port (default) |

## Play on Linux

On x86_64 Linux with Nix flakes enabled:

```sh
KF_DISC="/path/to/King's Field (Japan).iso" nix run github:sushi-shi/kings-field-decomp/port
```

Use a Japanese ISO, BIN, or single-track CUE/BIN disc image.
Keep the CUE and its BIN file together.

The first launch extracts and caches game data locally. Saves use three separate
slots. No manual extraction or `--impure` is needed.

Optional arguments go after `--`:

| Option | Purpose |
| --- | --- |
| `--saves DIRECTORY` | Use an existing save directory |
| `--data DIRECTORY` | Use an extracted disc tree instead of `KF_DISC` |

In a local checkout, use `KF_DISC=/path/to/disc.iso nix run .`.

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
| Pause | P or Escape during gameplay |

Controllers are supported. Click to capture the mouse; switching away pauses
the game and releases it. Press a key or button to resume from pause.

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

Open [the game](http://localhost:8000/kings-field.html), select your disc and press
Play. Data and saves stay in browser storage; clearing it removes them.
