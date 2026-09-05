# Target-section relink review

Tooling campaign, 2026-09-05, starting at `1dd222d`. No C function, DATA/RODATA
claim, identity, relocation inventory or banked score is changed. The tool is
`kf verify roundtrip`; use `--output build/roundtrip/all.json` for all symbol
witnesses. Run after `kf init` and a target rebuild in `nix develop`.

## Independent comparison contract

For each manifested ELF32 MIPS target object, every owned symbol requires:

```text
section base = claimed retail VA - actual ELF st_value
```

All claims in one section must imply the same base and satisfy the section's
alignment. Function symbol sizes and the contiguous text extent are checked;
RODATA supplies an explicit section claim. GNU `mipsel-linux-gnu-ld` places
whole sections at these bases, resolves undefined identities against the
image-local curated inputs and performs the actual REL fixups. The check does
not use `relocations_used.tsv` or the delinker's inverse encoding helpers to
manufacture its expected values. Defined symbols are never overridden with
their desired VAs. Unsupported allocated sections, unknown external identities,
invalid relocation sites and linker errors fail.

Every initialized output section byte is compared with the hash-verified retail
load image, including any extra section tail. BSS remains NOBITS: placement and
allocation are checked, not invented initialization. Full placed extents are
also checked for overlap within each image. GNU ld is an independent relocation
implementation, not evidence that the historical game used that linker.

This verifies manifested **target** objects, not reconstructed linked programs,
standalone vendor objects or config-only data. A successful roundtrip cannot
prove semantic ownership or that the reference/relocation census is complete.
The report explicitly keeps both exhaustive-coverage and whole-image-equality
flags false. Its initialized-byte total sums compared sections; it is not a
reachable-image coverage denominator. A unit with conflicting placement is not
partially linked and reported as verified.

## Synthetic tails contradicted by retail

The old `_module_object` rounded DATA/BSS extents to 16 bytes and jump-table
RODATA extents to eight bytes, appending zeros to imitate the probe assembler.
This was a target-model error: source object alignment does not establish
retail byte ownership. Before correction, 24 initialized sections relinked to
different retail bytes. Every first mismatch was beyond the unrounded claim.
The table records section-relative offsets and the retail bytes beginning at
the old claimed end (hex, including intervening zero bytes).

| Image-qualified unit | Section | Claimed end | First mismatch | Retail tail, not owned padding |
| --- | --- | ---: | ---: | --- |
| psx.main | .data | 0x8 | 0x9 | 0080000043504501 |
| game.game | .data | 0x8 | 0x8 | 20000000ff00f000 |
| game.player_update | .data | 0x4 | 0x4 | 0b00f401010001005c4b465c |
| game.collision | .data | 0x238 | 0x238 | 42305c0000000000 |
| game.resources | .data | 0x8 | 0x8 | 5c45302e3b310000 |
| game.render | .data | 0x7 | 0x8 | 008080800080808000 |
| game.render_enqueue | .rodata | 0x74 | 0x74 | 5c4b465c |
| game.geometry_render | .data | 0xc4 | 0xc8 | 000000007f0fc0ffa0ff7f00 |
| game.render_frame | .data | 0x38 | 0x3a | 0000f40110000100 |
| game.menu | .rodata | 0x1c | 0x1c | 4d41505c |
| game.actor | .data | 0x108 | 0x108 | 0900480a004d0b00 |
| game.actor_behavior | .rodata | 0x264 | 0x264 | 84130380 |
| game.map_object_pool | .data | 0x18 | 0x1a | 0000240407010101 |
| game.map_object | .rodata | 0x18c | 0x18c | 56414220 |
| game.player_warp | .rodata | 0x14 | 0x14 | c0700380 |
| game.magic | .rodata | 0x14 | 0x14 | 44454255 |
| game.debug_text | .data | 0x4 | 0x4 | 6e6f6e650000000000000000 |
| open.opening_ending_scene | .data | 0xfc | 0xfc | 888a0100 |
| open.opening_fade | .data | 0x14 | 0x14 | 42305c4c302e00005c4b465c |
| open.opening_controller | .data | 0x6 | 0x8 | 00005c4b465c00000000 |
| open.render_tmd | .data | 0x4 | 0x4 | 808080008080800000000000 |
| open.render_map | .data | 0x4 | 0x9 | 000000000010000009000000 |
| open.render_sprite | .data | 0x8 | 0x8 | 0900000000000000 |
| open.audio_play_voice | .data | 0x4 | 0x8 | 000000006e6f6e6500000000 |

For example, the fake `game.render_enqueue` tail replaced the beginning of
`game.item`'s string range; `game.menu` replaced `game.menu_map_viewer`'s `MAP\`
prefix. `game.actor_behavior` extended into `game.map_object_pool`'s jump table,
and `game.map_object` into `game.audio`'s `VAB ` string. The mismatch at
PSX.EXE 0x8001022d lies beyond `psx.main`'s eight-byte pointer table starting
at 0x80010224. These examples disprove unconditional target tail synthesis.

Removing only these appended tails eliminates every observed initialized-byte
mismatch and every cross-unit section overlap. It does not enlarge the source
claims, strip compiler-emitted source bytes, or infer ownership of neighboring
zeros. Inner packing remains an explicit hypothesis subject to placement and
byte verification. Forty-four module target objects change; all per-function
target objects remain identical.

## Remaining unplaceable sections

Ten sections have inconsistent implied bases. The following representative
pairs are sufficient contradictions; the JSON retains every claim and size.
Each witness is written as `symbol: retail VA / object offset`.

| Unit / section | First witness | Contradicting witness |
| --- | --- | --- |
| game.cd_file / .data | cd_path_prefix: 0x80057b3c / 0 | cd_read_location: 0x80057e80 / 0xc |
| game.pad / .bss | pad_buf: 0x80058020 / 0 | PadIdentifier: 0x8006bd88 / 0x10 |
| open.opening_scene0 / .data | opening_scene0_camera_path: 0x800354f4 / 0 | opening_scene0_sound: 0x80035874 / 0x1dc |
| open.opening_scene3 / .data | opening_scene3_camera_path: 0x800356d0 / 0 | opening_scene3_overlay_rects: 0x80035878 / 0x54 |
| open.opening_ending_scroll / .data | opening_ending_scroll_camera_path: 0x80035820 / 0 | opening_ending_scroll_panels: 0x80035888 / 0x54 |
| open.resources / .data | cd_path_prefix: 0x800372dc / 0 | opening_scene1_arena_cursor: 0x800375e0 / 0x14 |
| open.render_map_cells / .bss | render_cell_windows: 0x800439d8 / 0 | active_cell_window: 0x8006e1c8 / 0xcc0 |
| open.entity_render / .data | floor_item_sprites: 0x800358e0 / 0 | floor_item_light_matrix: 0x800359e4 / 0x54 |
| open.audio / .bss | audio_sequence_table: 0x80037808 / 0 | audio_state: 0x8006e1d0 / 0x158 |
| open.pad / .data | pad_buf: 0x80037760 / 0 | pad_status: 0x80037768 / 4 |

Two more BSS sections conflict with the target ELF's four-byte alignment:
`game.notify_queue` at 0x8009506e (30 bytes) and `open.format` at 0x80037971
(19 bytes). This establishes a mismatch in the current section model, not
permission to change the address, add padding or force a linker alignment.
The next source/ownership campaign must inspect the underlying complete objects
and section classes before changing their claims or shared declarations.

After correction: PSX 1/1, GAME 72/75 and OPEN 32/41 target units verify
(105/117 total), compared with 0/1, 51/75 and 25/41 initially. The twelve
remaining placement failures are a default-build gate. No linked-executable
equality is claimed.
