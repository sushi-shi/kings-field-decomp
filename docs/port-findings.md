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

## KFIII catalogue coverage

The imported mesh count is not a count of entries in the
[39-entry character catalogue](https://models.spriters-resource.com/playstation/kingsfieldiii/).
Comparison of the locally cached catalogue thumbnails against all textured
previews gives the following visual correspondences. These are presentation
names, not reconstructed game symbols. IDs 0..42 retain canonical MO slots;
ID 43 appends a model from MOF without renumbering existing selections.

| Catalogue character | Presentation ID / source |
| --- | --- |
| Airon Green (Healthy) | 36 |
| Airon Green (Sick) | Not located |
| Alexander Thornton Regginis | 42 |
| Christy Clements | 0 |
| Ed Edmund | 22 |
| Franz Stoppenbach | 39 |
| Gullick Risty | 3 |
| Jack Leininger | 14 |
| James Seward McCain V | 41 |
| Jamie Porter | 17 |
| Janan Green (Cup) | 37 |
| Janan Green (Prayer) | 7 |
| Jane Cowley | 25 |
| Jens Stensland | 4 |
| Joe Santos | 38 |
| John Creel | 26 |
| Krone Licht | 24 |
| Leon Shore | 34 |
| Lyn Reinhardt | 1 |
| Lyn Reinhardt (Dead) | 15 |
| Lyn Reinhardt (Sitting) | 2 |
| Marcus Peppers | 16 |
| Marilyn Miller | 21 |
| Mark Johnson | 8 |
| Michael Hansen | 6 |
| Olivier Veyrac | 27 |
| Orladin | 43 / MOF 545 |
| Priscilla Gomez | 20 |
| Rene Thomas | 5 |
| Robert Shreve | 23 |
| Sal Estrada | 28 |
| The Skeleton of Light | 30 |
| The Skeleton of the Giant | 29 |
| Tim Lindquist | 40 |
| Toni Gomez | 19 |
| The Tree of Dragon King Plant | 31 |
| Varde | 13 |
| Yvette Bince | MO 18; excluded for unresolved palette |
| Zul Arifin | 9 |

MO 32 and 33 are additional elf meshes; do not count them as coverage of the
missing entries. MO 10..12 alias 13, and MO 35 is a literal DUMMY model.
The resulting 39-mesh pack represents 37 catalogue entries plus those two extra
meshes. Authored walking/combat rigs remain a separate requirement from import.

Orladin's model is MOF slot 545 (area 17, local map object 1), with model SHA-256
`81c53ef94b8b84490f089a076bb478a9a94d98707705e9e1d1eacbcbbecb32c2`.
It decodes with common FDAT 84 textures and RTIM 17's first rectangle stream.
The textured throne and skeleton match catalogue entry 353110. Preserve the
throne's original pose; it has no walking rig. A broader source-geometry review
found MO 59 as a possible standing figure, but its area-19 textured preview is
a mummy, not sick Airon. Textured previews also rule out MO 81 (area 1),
92 (area 19), 93 (area 7) and 94 (area 15): these are armored/statue/doll-like
figures, without the catalogue body's human head and skin. No substitute for
the missing body was imported.

MOF 129..133 were also checked with their area-4 character texture stream
(RTIM 4 offset 32896), plus common FDAT 84. They are prone, damaged human
remains, not the intact bare-limbed child shown by catalogue entry 353059.
The first map texture stream does not cover their materials; use the second
stream when reproducing this comparison. None has an identical geometry entry
in MO. Their source SHA-256 values, in slot order, are:

```text
129 ae91a3d136b4a2dfa8ae99ede013508e3caf0b59c3baff5dd3b9c5f836def26b
130 4ff8a82061a5217f62fd1fff7255e5e0ca33b9f6a5106e64c497d4d36ea39ac8
131 7018020b15e6045a5b35b04713868482538d5ec16b1f97d9955f530c3945ad1f
132 72e29780268a417ec4324898ff7646771d4720dc20574eb4fa9de15635c76d80
133 c3558e345573d5313e912a92149306cecc07bb41ad13ee343cc86156874eed7d
```

An alternate Japanese-disc comparison remains open. Archive.org metadata for
`rr-sony-playstation-j` lists both Japanese KFIII revisions, but marks both
files private. That listing does not establish any regional model difference;
no Japanese source bytes were obtained or imported during this comparison.

## KFIII character palette still unresolved

This evidence is for the locally imported **SLUS-00255** presentation assets,
not the SLPS-00017 reconstruction images. MO slot 18 has four textured triangles
on its small neck ornament referencing page 14 and CLUT `0x7d06`: VRAM `(96,500)`.
Their UVs span `(128,64)..(158,95)`. The image pixels are present and use multiple
palette indices; this is not an all-transparent placeholder. Other body faces
use loaded palettes. Do not substitute the image block's accompanying palette:
that does not prove the palette selected by these primitives.

The opening-image lead was checked against OPEN.EXE SHA-256
`bf5b2c820007157c2bf3bd380b0e5b280843f579c2e7fbc84605c22c5eb13edb`.
OPEN `0x80016390` is the LoadImage wrapper (its diagnostic string is at
`0x800112a4`); the TIM-stream consumer calls it at `0x800135f0` and `0x8001360c`.
The opening data file `OP/OP.D`, SHA-256
`d7adfc84ef4c6e80b749d1d9c03f4eea31628cd5286d37f697a038aee245391d`,
contains these length-validated TIM palette rectangles:

| File offset | Palette `(x,y,width,height)` |
| --- | --- |
| `0x00000` | `(0,480,256,1)` |
| `0x0f220` | `(0,481,256,1)` |
| `0x14d40` | `(0,482,256,1)` |
| `0x1a860` | `(0,500,16,3)` |
| `0x1c4e0` | `(960,200,16,2)` |

None covers `(96,500)`. OPEN's MoveImage wrapper at `0x80016458` has no direct
`jal` callers. This eliminates those opening TIMs as a direct source, but does
not prove that all runtime VRAM writes or material remapping are understood.
An actual VRAM capture at the character or a proven runtime remap is the next
useful evidence; do not repeat broad archive/header scans or invent a palette.
