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
| `port` | Linux and browser port |

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

The first launch extracts and caches game data locally. Saves use three separate
slots.

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

After the WASM build, serve `build/wasm` over localhost or HTTPS and open
`kings-field.html`. Select an ISO or BIN (with its CUE if applicable), then
choose **Play solo**, **Host multiplayer**, or enter a friend's room code and
choose **Join multiplayer**. Hosting displays a copyable room code and a waiting
lobby with the party's avatars. Once everyone connected is ready, the host clicks
**Start game** to launch together. The host can also start alone. Every player
loads their own resources; extraction stays local, and resources and saves use
separate IndexedDB stores. Browser storage can be cleared or evicted.

Only the host stores campaign saves. Select a slot under **Host campaign** before
clicking **Host multiplayer** to restore it. The lobby shows each saved avatar
with a character code underneath: share that code with a friend, who enters it
in **Join multiplayer** to resume that character, including from another device.
Absent characters remain reserved; new players can take unused slots, up to four
characters in total. The host may start with any number of connected players.
Character codes last for that hosted room; rehosting creates new codes.

Multiplayer also keeps a private random profile in save storage for reconnecting
to the same room. Preserve the host's browser storage to retain its campaigns and
ownership. The campaign selector shows the saved floor and level; unreadable,
incompatible or other-profile saves cannot be selected. Native clients retain
their profile in the save directory and can rehost with `--host --campaign 1`.

The website's room service handles codes and WebRTC signaling. Gameplay runs in
the host's browser, with direct peer connections when available and a configured
TURN relay when a direct connection cannot be established. Co-op is still under
development; implementation boundaries and remaining validation are described in [multiplayer notes](docs/multiplayer.md).

For multiplayer characters, select your US King's Field II disc (`SLUS-00255`,
the western release of KFIII) in the **Multiplayer characters: KFIII disc** field,
then choose a body. Drag its preview (or focus it and use the arrow keys) to look
around it before joining. The browser reads only the character archives, extracts them
locally, and caches the result and your selection for next time. Solo play needs
only the Japanese KF1 disc. Native multiplayer clients use
`--avatar-disc /path/to/kfiii.bin` (and optionally `--avatar 41`).

The standalone extraction tool is also available:

```sh
nix develop --command python scripts/kf3_assets.py /path/to/disc.bin --output build/characters
```

Its `build/characters/characters.kfa` output can be selected in that same browser
field or supplied with native `--avatars`. Each player needs the same character
resources; the derived pack and presentation recipe are hashed together and
checked when joining. No character data is sent between players. This first pass
has 39 textured base meshes, including seated figures and Orladin's throne.
Standing bodies 1, 5, 14, 19, 22, 24, 26, 27, 39 and 41 have walking, melee and casting poses with KF1 weapon/shield
attachments; slot 41 is the default. The browser lists catalogue names and groups
animated characters separately from bodies that retain their original pose.
The lobby's Pose selector previews walking, sideways steps, swings, thrusts and
casting. Use the animation-position slider to inspect a motion and drag the
preview to view it from another angle; no game needs to be started.
Other bodies still need curation, rigs and mesh corrections.
This is not the catalogue's complete 39-character roster: it includes extra pose
variants, while Airon's sick form is still missing and Yvette's unresolved
palette prevents her import.
Select the KFIII disc again to add newly supported bodies to an older cached pack.

Game resources must also match. Before hosting or joining, the client hashes the
paths and contents in the selected extracted disc directory. Keep it separate
from saves and other files; native multiplayer expects ordinary files, without
symbolic links. Different resource sets cannot join the same room or load each
other's campaign checkpoints.

To serve the application and room service together locally:

```sh
cd services/rooms
npm ci
WEB_ROOT=../../build/wasm node server.mjs
```

Open `http://localhost:8787`. For a public website, serve it over HTTPS and proxy
WebSocket upgrades at `/rooms` to this service. The launcher uses that same-site
endpoint by default; `kf-room-service` in `web/shell.html` can select a separate
`wss://` endpoint. Keep the backend bound to its default `127.0.0.1` behind the
proxy, or explicitly set `BIND` and `PORT` for the deployment.

Set `STUN_URL`, `TURN_URL`, and `TURN_SECRET` on the room service for internet
connections. The TURN server must use the same shared authentication secret;
the room service issues short-lived credentials to players. The secret stays
on the servers. Both clients receive fresh credentials before new connections,
including late joins and reconnection after host recovery. `TURN_URL` accepts up
to seven comma-separated relay URLs sharing that secret. Browsers can use TCP
or TLS TURN when their network blocks UDP. The pinned native networking library
is validated with UDP TURN; native TCP/TLS relay support remains unverified. TURN needs its own
publicly reachable listener and relay ports;
an HTTP reverse proxy alone cannot relay WebRTC traffic. Room state is in memory,
so a room-service restart ends its rooms.

The [deployment examples](services/rooms/deploy) use one Linux server with
systemd, nginx, coturn and Node.js 22 or newer. They are templates, not an
installed deployment. Set the website and relay DNS records to the server's
public address, and use your existing certificate setup for the website domain.

Use this release layout on the server:

```text
/srv/kings-field/web/
  kings-field.html
  kings-field.js
  kings-field.wasm
/srv/kings-field/rooms/
  server.mjs
  package.json
  package-lock.json
  node_modules/       # npm ci --omit=dev in this directory
```

Copy only the three listed files from `build/wasm` and the three room-service
source/package files. Disc images, extracted resources, character packs and
player saves stay on players' devices. The service reads its release directory;
its dynamic systemd user needs read/traverse access to that directory tree.

1. Copy `deploy/rooms.env.example` to `/etc/kings-field/rooms.env`. Replace the
   relay domain and `TURN_SECRET`. Generate the secret with
   `node -e "console.log(require('node:crypto').randomBytes(32).toString('hex'))"`.
   Keep this environment file readable only by root; systemd reads it before
   starting the service. Both TURN settings must be present together.
2. Install `deploy/kings-field-rooms.service` in `/etc/systemd/system/`.
   Adjust `ExecStart` if Node is installed somewhere other than `/usr/bin/node`.
3. Adapt `deploy/turnserver.conf` for the packaged coturn service. Set its
   `static-auth-secret` to the same generated secret and replace the realm.
   Install a trusted certificate for the relay domain and its private key at the
   configured `cert`/`pkey` paths. The coturn service account must be able to read
   them; keep the key and shared-secret configuration private. Include refreshing
   these files and restarting coturn in your certificate renewal procedure.
   If the server is behind NAT, set the documented `external-ip` mapping and
   forward the ports.
4. Include `deploy/nginx.conf` in nginx's HTTP configuration after replacing the
   website domain and certificate paths. It forwards `/rooms` upgrades and the
   three application files to the loopback room service.
5. Allow TCP 80/443 for the website, TCP/UDP 3478 and TCP 5349 for STUN/TURN, and UDP
   49160–49260 for relay traffic in the server and provider firewalls. Keep
   port 8787 private. This example advertises UDP, TCP and TLS TURN. A network
   that permits only outbound TCP 443 needs TURN TLS on a reachable port 443
   listener, such as a separate public IP. Change `tls-listening-port` and the
   `turns:` URL together for that layout; the website already occupies port 443
   on its own address. An HTTP `/rooms` proxy does not carry TURN TLS traffic.
6. Check `nginx -t`, reload the systemd configuration, and start the room,
   coturn and nginx services. Check `https://YOUR_DOMAIN/health` for `ok` and
   open the website. Public cross-network Host/Join still needs verification
   after installation; a health response alone does not check relay traffic.

The nginx upgrade configuration follows its
[WebSocket documentation](https://nginx.org/en/docs/http/websocket.html);
relay authentication and address/port settings follow the
[coturn configuration reference](https://github.com/coturn/coturn/blob/master/examples/etc/turnserver.conf).
Updating the three web artifacts and restarting the room service closes active
rooms; use a planned break between sessions.

The deployment proxy can be checked locally without starting the game:

```sh
nix develop -c node services/rooms/crossplay.test.mjs \
  build/linux/coop-crossplay-host build/wasm --https --turn-tcp
```

Build both presets first. This uses the nginx template with temporary loopback
ports and a disposable certificate, verifies the web artifacts over HTTPS, and
runs browser/native packet exchange through `/rooms` over WSS and a TCP browser
relay. The isolated browser trusts only that test certificate. The native host
connects directly to the local room service; this does not verify native WSS,
public certificates, server installation or cross-network reachability.
Use `--turn-tls` instead of `--https --turn-tcp` to verify the TLS relay path.
It starts the coturn deployment template with a temporary certificate, verifies
its TLS handshake, restricts the browser to `turns:` candidates and requires the
selected relay protocol to be TLS. Local relay fixtures remove the template's
private-address denies solely to permit isolated loopback traffic.
Use `--https --stun-only` to check a direct connection with STUN and no TURN
configuration. The fixture verifies that neither selected candidate is a relay.
Adding `--browser-host --overflow-packets` also establishes a direct third browser
connection and checks that overflowing its queue leaves the healthy peer usable.
These are local transport checks; public NAT/firewall combinations still need
verification on the deployed service.

### Linux

Inside `nix develop`:

```sh
emcmake cmake --preset wasm
cmake --build --preset wasm
python3 -m http.server --directory build/wasm
```

### Windows

The browser version can be built directly on Windows. Install
[Git](https://git-scm.com/downloads/win), [Python 3](https://www.python.org/downloads/windows/),
[CMake 3.25+](https://cmake.org/download/) and [Ninja](https://ninja-build.org/),
with their commands available on `PATH`. In PowerShell, from your `port` checkout:

```powershell
git clone --depth 1 --branch 5.0.6 https://github.com/emscripten-core/emsdk.git build/emsdk
.\build\emsdk\emsdk.bat install 5.0.6
.\build\emsdk\emsdk.bat activate 5.0.6
Set-ExecutionPolicy -Scope Process RemoteSigned
.\build\emsdk\emsdk_env.ps1
python scripts/english_patch.py fetch --output build/wasm/english-v1.kfdelta
emcmake cmake --preset wasm
cmake --build --preset wasm
python -m http.server --directory build/wasm
```

CMake downloads SDL automatically. For later builds, repeat from
`Set-ExecutionPolicy`, skipping the translation download.

### Play

Open [the game](http://localhost:8000/kings-field.html), select your disc and press
Play. The language selector also works during play. Data and saves stay in
browser storage; clearing it removes them.
