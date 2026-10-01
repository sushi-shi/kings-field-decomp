# Character presentation notes

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
