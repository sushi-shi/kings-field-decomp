# Port technical notes

These are current constraints and unresolved attribution questions. The
[status page](port-status.md) tracks remaining work; [PORTING.md](../PORTING.md)
describes ownership and verification. Implementation history lives in Git.

## Reconstruction boundary

A native failure can expose a reconstruction mistake, retail undefined behavior,
or a port assumption. Establish the image, callers, fields and retail instructions
before backporting a fix. Exact means objdiff 100%; an exact body still does not
prove unique source spelling or host-safe layout. Keep defensive host checks and
deliberate behavior changes in the port unless independent evidence supports a
reconstruction correction. None of the port changes below is a new matching claim.

## Resource formats and ownership

Two COM table copies intentionally cross nominal chunk boundaries:

| Table | Payload offset | Nominal payload | Copied bytes |
| --- | ---: | ---: | ---: |
| Armor | `0x1134` | 756 | 1,176 |
| Map-object definitions | `0x1610` | 1,128 | 1,280 |

Armor includes the next four-byte header and 416 magic bytes; object definitions
include the next header and 148 growth bytes. `resource_stream_tail` checks the
file extent for these two copies while retaining the normal next-chunk cursor.
Other records remain chunk-bounded. Checking these copies against the nominal
chunk breaks GAME startup.

`KfTmdResource` carries a borrowed pointer and actual loaded extent through slots,
asset registries and selection. Standalone menu TMDs have no preceding MIX header.
Embedded asset counts, sizes and offsets are bounded by their containing file;
the TMD extent is the asset tail, not an independently proved subregion.
Registration checks the 12-byte header and 28-byte object records. Release clears
selected references to the released resource; phase arenas retain ownership.

GAME's three enqueuers decode little-endian packet values, check body length,
normal/vertex indices and projected-array capacity, and advance by the original
`input_length * 4` stride even for skipped modes. Their 12/4/2 accepted-case sets,
winding, materials, lighting, depth and ordering remain distinct. OPEN still has
legacy packet-union consumers; neither this conversion nor slot bounds establish
complete animation or projection-source lifetime safety.

## Floor-item facing and frame count

The appearance byte packs facing in the high nibble and frame count in the low
nibble. B1 MIXA has base-sprite 4 records with appearance `0x23` and `0x13`, plus
20 base-sprite 0 records with `0x04`. Multiplying the random value by the full byte
can exceed the seven-entry sprite table; ASan observed that native overflow.

The port decodes separate facing/count fields and uses only the count for initial
frame selection, retaining random-call count and scaling. Placement extent,
capacity, sprite range and cell coordinates are checked. Zero-frame records remain
valid. Retail attribution of the earlier unmasked expression is unresolved:
inspect GAME and OPEN independently before changing reconstruction sources. The
port's seed-one startup policy is also not a proved retail boot seed.

## Rendering contracts

| Evidence | Contract retained by the port |
| --- | --- |
| GAME `tmd_project_vertices`, `0x8001c60c` | Full current depth from `ReadSZ2`; projection flags do not reject vertices. |
| GAME `render_enqueue_map`, `0x8001de18` | Projected winding and signed depth arithmetic; wall bias 200. |
| GAME `render_enqueue_tmd`, `0x8001c7f8` | Projected winding; averaged depth plus signed bias reaches five before 14-bit masking. |
| GAME `render_map_object`, `0x8001ebb8` | Hinged-door bias 15; lift-door bias 180. |
| GAME `AddPrim`, `0x80054290` | Equal-depth head insertion. |

Sort whole faces before the original quad split. Reject each triangle with a
screen span above 1023 in X or 511 in Y before viewport clipping. This missing
rasterizer rule caused the behind-camera door intrusion. Preserve projected
winding and authored biases; the starting-room plaque artifact is also visible
in retail and does not justify adding a depth buffer.

Projection uses integer reciprocal refinement, Q16 scale capped at `0x1ffff`,
signed coordinate rounding/saturation and a signed fog coefficient. GAME
`SetFogNear` (`0x8004f4b4`) and OPEN's counterpart (`0x8002f288`) compute wrapped
`-320 * near`, signed division by reference distance 200, a signed 16-bit
coefficient and intercept `0x01400000`. Weapon projection changes retain that fog
reference. Lighting retains Q12 matrix stages, intermediate saturation, signed
fog inputs and each producer's intermediate byte-color quantization.

The native shaders preserve raw/modulated and flat/Gouraud properties, RGB5
writeback, dithering and integer blending. Zero texture words are transparent;
STP black is visible. Blended textured faces blend only STP texels. Each blended
triangle reads a separate clipped framebuffer snapshot. Its cost is unmeasured.
Modulation follows later GPU arithmetic; launch-model truncation and GL pixel
coverage/interpolation remain fidelity gaps.

Retained modal calls accumulate on one target; resize/expose re-presents stored
pixels. GAME `display_present_frame` (`0x8001c050`) displays the opposite retail
buffer while drawing; native drawing and presentation happen in one call.
Equivalent frame chronology and complete render inputs remain unverified.

The fixed sine samples in `src/lib/fixed_math.cpp` derive from Psy-Q Release 2.5
`LIBGTE.LIB` / `GEO.OBJ`, also identified in GAME and OPEN. The 2,048-byte payload
SHA-256 is `74743e361fc4d78cbd41bd99b2acf5c75c9b5b0268ccd753ed282ec4394fb25a`.
Do not regenerate them with host trigonometry or combine the XYZ constructor
with the game's separate Y-X-Z helper.

## Timing policy

GAME world rendering owns the three-tick deadline, including scripted redraws;
callers do not wait another complete interval. OPEN scene0 uses a local 22-update/s
deadline. Its 490 intervals took 22.313425276 seconds in the measured retail
emulator path (about 21.96 updates/s); 22 is a chosen stable approximation.
Other opening loops, menu waits, explicit holds and sample-based audio keep their
own timing. Focus pauses are excluded. Long stalls rebase the deadline without
injecting catch-up steps. No global elapsed-time scaling replaces per-step logic.

## Audio and menu quirks

GAME `audio_play_spatial` treats SoundRef byte one as tone/flags. Retail GAME
`0x80032e64..0x80032e78` masks with `0x80` then compares to `1`, so the pan-narrowing
gain adjustment is unreachable. Preserve that comparison; changing it to a
nonzero test changes behavior. `0x80032f8c` masks the tone with `0x0f`. OPEN passes
the selector unchanged and uses distance gain directly on its near/equal-pan path.

Menu labels intentionally reuse existing glyphs: MP retains HP's second glyph,
and total defense retains total attack's prefix. Whole-row replacements can change
those labels. Effect update IDs passed to `player_apply_damage` are damage
multipliers in tenths, not actor-owner identities.

## Deliberate port behavior corrections

- **Return home:** GAME action 33 in `actor_update_current_action` (`0x8002fa88`)
  uses uninitialized home registers at `0x80030500`; its proven caller supplies
  residue `(1, 65535)`. The port computes the actor's actual placement coordinates
  before either branch uses them.
- **Player-homing pitch:** GAME effect kind 20 (`effect_update_dispatch`,
  `0x80038a38`) reads an unwritten stack distance at `0x80039760` on the player
  path. The actor-target branch obtains that distance from its search. The port
  computes player horizontal distance using the existing fixed-point length
  operation, retaining pitch sign, offset, angle mask and turn limit.

Both corrections were user-approved port policies; they do not restore missing
retail instructions. Their builds passed, but the specific combat paths were not
runtime-verified by that change.

## MAGIC charge evidence

The one-off empty bar report remains deferred without a reproduction. Ordinary
refill requires a selected spell, no new Magic-button edge, gameplay updates and
body armor other than Skull Armor. Charge is distinct from MP. Eligible gain is
`2 * (((magic * 64) / (charge_rate + 1)) + 1)`, followed by the original halfword
assignment and clamp to 5,000 (GAME `0x800193cc..0x80019430`). Shipped offensive
rates for IDs 4–8 are 70, 28, 45, 20 and 14; starting Light Needle gains 172 per
eligible update and fills in 30 updates.

Spell selection and successful casting clear charge; insufficient MP does not
block later refill. Load-return reselects the saved spell and rebuilds its pointer.
The HUD derives width as `magic_charge / 100` (GAME `0x8001ffb8..0x8001ffc4`).
These gates do not explain the reported session. If reproduced, inspect spell
ID/pointer, charge, magic power, armor, input edges, update state and HUD width;
do not repeat the static trace or reset state speculatively.
