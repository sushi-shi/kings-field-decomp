# GAME resource-parser coverage

This is the parser census for the Japanese retail `GAME.EXE`
(`SLPS-00017`). It separates byte interpretation from disc I/O, allocation,
GPU/SPU submission, and runtime consumption so that a format is not silently
excluded merely because Sony code performs part of the parse. The local retail
image used for this census has SHA-256
`394ba335c0a179073c0b7bee306513785fdb3bbc162dbd024a57e82130f3cdb8`.
Curated function/data/relocation inventories remain working models and may
change independently of this hash-identical resource corpus.

The intended comparison is byte-for-byte state after an isolated call on the
same input: retail machine code, reconstructed C machine code, and the Rust
codec. "Exact C" means strict objdiff 100%, not a loose score. Differential
agreement is a separate claim: it covers the enumerated inputs and captured
state, not every possible input or full-game execution. Every format/layer in
the census below now has a Rust implementation and passing comparison coverage
at its stated boundary. `python -m scripts.kf.codec_oracle` runs all ten suites
without case or event limits and rebuilds the candidates by default.

The [latest exact-closure attempt](patterns/game-parser-exact-closure.md)
leaves **23 of the 29 explicit GAME functions strict-exact (79.31%)**.
Size-weighted objdiff similarity is 99.470158%, up from 98.291691% after the
[failure-path review](patterns/game-parser-verification.md), and distinct from
the passing finite test corpus. Five of the six remaining functions improved,
but none reached 100%; Sony providers are not included in these counts.

## Complete parser census

| Format/layer | GAME entry point(s) | Candidate unit | Serialized input | Observable output | Rust/oracle status |
| --- | --- | --- | --- | --- | --- |
| Generic `u32 length; payload` stream | `common_resources_load` `0x8001b180`; `map_resources_load` `0x8001b558` | `game.resources` | `COM/COM.DAT`, `B1..B5/MIXA.DAT`, `B1..B5/MIXB.DAT` | Named borrowed chunks and the callback state below | Rust `common`/`map_resources`; COM outer loader passes including actual record/registry/TMD callees; MIX outer loader passes all five floors at explicit inner-parser boundaries |
| Common asset and stat records | `weapon_records_load_and_mirror_angles` `0x800150a8`; `armor_records_load` `0x800150fc`; `magic_load_records` `0x8003a274`; `map_object_definitions_load` `0x80030fdc` | `game.equipment`, `game.magic`, `game.map_object_pool` | COM chunks 2 through 5 | 704-byte weapon table with 16 angle negations; 1,176-byte armor table; 480-byte magic table; 1,280-byte object-definition table | Rust `records`; three-way pass on six COM table copies/transforms and six synthetic byte-pattern controls, including preserved tails |
| Five map grids | `map_resource_copy_words` `0x8001b3e4`, called five times by `0x8001b558` | `game.resources` | MIXA chunk 2 | Five 100-by-100 byte grids, 50,000 bytes total | Rust `map_resources`; complete 50,000-byte three-way comparison for each of the five floors, executing the actual retail/C word-copy helper |
| Floor-item placements | `item_load_floor_placements` `0x80020b4c` | `game.item` | MIXA chunk 3; 12-byte records, `item_id == 0xffff` sentinel | `floor_item_count` plus a prefix of `KfFloorItem[64]`, 24 bytes each; position and random initial frame are derived | Rust `placements`; three-way pass on all five floors and empty-sentinel control: full pool, count, grids, ordered callbacks |
| Map-object placements | `map_object_pool_load` `0x80031008` | `game.map_object_pool` | MIXA chunk 4; 20-byte records, `object_id == 0xff` sentinel | All 190 44-byte pool slots are visited; live records derive position/link/collision and special-effect side effects, remaining IDs become `0xff` | Rust `placements`; three-way pass on all five floors and empty-sentinel control: full pool, grids, ordered callbacks |
| Actor placements | `actor_pool_load_placements` `0x800308c0` | `game.actor_pool` | MIXA chunk 5; 16-byte records, `slot_state == 0xff` sentinel | All 128 72-byte actor slots are visited; live records derive fields/position, remaining slots become free | Rust `placements`; three-way pass on all five floors and empty-sentinel control: full pool, grids, ordered callbacks |
| Actor definitions | `actor_definitions_load` `0x80030a6c` | `game.actor_pool` | First 12 of the 20 152-byte records in MIXA chunk 6 | `KfActorDefinition[12]`, 1,824 bytes | Rust `records`; three-way pass on all five floor tables, including untouched destination tail |
| Map-event definitions | `map_event_pool_load` `0x800338b8` | `game.map_event` | First at most eight 24-byte records in MIXA chunk 7; `state == 0xff` sentinel | `KfMapEvent[8]`, 68 bytes each, plus collision occupancy callbacks | Rust `placements`; three-way pass on all five floors and empty-sentinel control: full pool, grids, ordered callbacks |
| Model-asset archive | `asset_registry_load_tmd_archive` `0x800204c0`; `asset_registry_set` `0x8002055c`; offset resolver `asset_registry_select` `0x8002059c` | `game.asset_registry` | `u16 count`, two reserved bytes, then 20-byte `KfAssetHeader` entries advanced by `byte_size` | Consecutive `asset_registry_entries`; current TMD is `asset + tmd_data_offset`; each TMD is prepared in place | Rust `asset_archive`/`registry`; three-way pass on 14 MIXB archives, three CHR archives, COM asset, 16 WEP assets and empty control: full mutated payload, 64-slot registry window, selected pointer |
| Standard TMD packet preparation | `tmd_register` `0x8001c5b0`; parser `tmd_prepare_primitive_indices` `0x8001c2b0` | `game.render` | 12-byte TMD header, 28-byte object records, packets advanced by `4 + ilen * 4` | Selected `u16` normal/vertex indices in eight polygon modes are shifted left by three in place | Rust `tmd`; three-way pass on all 246 shipped payloads plus four synthetic controls (including unknown modes and wrapping). Registry wrapper also passes all ten raw MIXB and 67 item TMDs, including all eight slots and selected pointer |
| King's Field animation tables | `render_bind_animated_instance` `0x800205d4` | `game.pool` | `KfAssetHeader` object/clip tables, clip and keyframe offsets, morph/rest objects and base TMD vertices | 20-byte pool record, caller cache, shared scratch including its leading reserved vector, full TMD state/current pointer, return status and lifecycle requests | Rust `animation`; 1,136-case three-way pass: 1,027 shipped animated selections/cache hits, 99 static assets, three reverse/wrap/fallback controls, seven allocation/reinitialization/release controls |
| `STAT.DAT` fixed banks | `item_load_database` `0x80020cfc` | `game.item` | Exactly 5,708 bytes | Six copies: 912, 2,376, 1,600, 180, 320, and 320 bytes | Rust `stat`; three-way pass at all four source byte alignments on all six banks, 80 20-byte CD file entries, and all 80 lookup-name buffers with deterministic found/missing responses |
| TIM image | game loop `tim_upload_images` `0x8001b100`; Sony `OpenTIM` `0x800529a0`, `ReadTIM` `0x800529b0`, actual worker `get_tim_addr` `0x80052cb0` | `game.resources`; vendored `LIBGPU.LIB/TMD` | TIM magic `0x10`, mode, optional CLUT block, pixel block; files may concatenate records | Full 20-byte `TIM_IMAGE` descriptor, CLUT/pixel rectangles and bytes, submission order, SDK cursor | Rust `tim`; three-way pass on 297 GAME files plus two synthetic controls, 678 uploads total. C wrapper explicitly shares the three retail Sony decoder providers; Rust is independent |
| Save icon TIM extraction | `save_file_initialize_buffers` `0x8002c304` | `game.save_system` | `TIM/ICO1.TIM` through `ICO3.TIM`, 192 bytes each | ICO1 CLUT bytes `[0x14,0x34)` and pixel bytes `[0x40,0xc0)`; ICO2/3 pixel bytes `[0x40,0xc0)` copied into the 0x200-byte card header | Rust `save`; three-way pass using all three retail icons, comparing complete initialized buffers and ordered file callbacks |
| VAB/VH/VB | game wrapper `audio_load_vab` `0x80032984`; Sony parser `SsVabOpenHead` `0x800446a8`; transfer `SsVabTransBody` `0x80044aa8`, completion `0x80044ce4` | `game.audio`; vendored `LIBSND.LIB/VMANAGER` | MIXA chunks 0 and 1 | Full mutated VH, GAME audio state, touched Sony program/tone/sample tables and aliases, SPU transfer requests | Rust `audio`/`audio_vab_state`; all five shipped banks pass normalization/roundtrip and successful runtime comparisons; 16 additional busy/allocation/validation/transfer/fade controls compare partial state, adjacent cleanup byte and service arguments. Sony routines are explicit shared retail providers; Rust is independent |
| SEQ | game wrapper `audio_play_map_sequence` `0x80032a4c`; Sony open `SsSeqOpen` `0x800468d8`; initializer `InitSoundSeq` `0x800471a4`; event pump `SeqPlay` `0x800476e8`, `GetSeqData` `0x800478c4`, `ReadDeltaValue` `0x80049d80`, `GetMetaEvent` `0x80049888`; tick dispatcher `SsSeqCalledTbyT` `0x8004a55c` | `game.audio`; vendored `LIBSND.LIB/SSOPEN`, `SEQINIT`, `SEQREAD`, `SSCALL` | Nine `B1..B5/SND*.SEQ` files | Full 176-byte score after initialization and each event, ordered SPU arguments, 0x90-byte GAME audio state, Sony open flag and unchanged loaded SEQ | Rust `audio`; all nine files and 15,880 events (5,154 running-status) pass full state/flag/input/command comparisons, including nonzero state seeds. Sony parser bodies are shared retail providers; Rust is independent |
| Memory-card header/catalog | `save_file_read_header` `0x8002bd08`; `save_system_read_catalog` `0x8002b078` | `game.save_system` | First 0x280 bytes of `bu00:BISLPS-00017KF      ` | `KfSaveHeader`; catalog copies six-word summaries selected by four directory slot IDs | Rust `save`; three-way pass on success, short-read retry, retry exhaustion, open failure, cached catalog and unavailable catalog |
| Memory-card slot payload | `save_file_read_slot` `0x8002beb0`; inverse writer `save_file_write_slot` `0x8002b73c` | `game.save_system` | One 0x2580-byte payload at `0x280 + entry * 0x2580`, after rereading/comparing the header | Copies 0xe0 player bytes, 0x2134 world bytes, 0xf0 auxiliary bytes, and 24 sampled magic flags; preserves two live player pointers/words | Rust `save`; reader passes slots 1–4 and retry/open/missing/summary-mismatch cases. Writer passes 14 cases covering rotation, creation, absent previous entry, retries and failures: full buffers, simulated file and I/O trace |
| Nested per-floor world state | serializer `map_world_state_persist` `0x80035b5c`; deserializer `map_restore_floor_state` `0x80035e44` | `game.map_events`, `game.map_load` | Variable-length record within the saved 0x2134-byte world block | Eight events, sparse actor lifecycle overrides, 190 object IDs, sparse object-link payloads, and two effect-object pools | Rust `world_state`/`world_persist`; restore passes 15 cases across floors 1–5, zero sparse counts, missing/skipped actors, floor override branches and marker-clear with complete pools/callbacks. Persistence passes five cases comparing the full 8,500-byte world block, including exactly 1,690 bytes for floor five |

## Format corpus and invariants

### Comparison boundaries

All agreement counts describe finite retail/synthetic inputs, not universal
equivalence on malformed byte strings. Rust rejects unsafe extents and indices
where retail trusts them. Destination poison and write guards detect changes
outside the declared outputs; actual runtime helpers remain distinct from the
parser boundary:

- Placement and world-restore tests compare full parser-owned pools, selected
  grids, and ordered deterministic helper requests. They do not prove the
  omitted collision, effect, actor-death or link helper bodies.
- TIM tests compare all five words of each temporary `TIM_IMAGE` descriptor,
  upload rectangles, pixel bytes, ordering and the SDK cursor. Descriptor
  addresses are PSX `u32` values, never host pointers. The C wrapper shares the
  declared Sony decoder functions; Rust parses independently.
- MIX outer tests execute actual retail/C chunk walking and grid copying.
  Inner parsers are declared hooks and are tested separately. To avoid code
  overlap in isolated RAM, only the length words and grid bytes read by this
  outer closure are loaded; Rust receives the complete files.
- Animation uses shared GAME selectors and an explicit Psy-Q fixed-point GTE
  interpolation model. Scratch captures include the leading reserved vector;
  allocation/release services are deterministic hooks whose requests, return
  status and parser-owned state are compared.
- Audio executes actual Sony parser bodies as shared retail/C providers. Rust
  independently derives VAB runtime state and complete per-event SEQ records.
  SPU hardware/voice allocation is a declared service boundary. VAB runtime
  agreement covers successful loads and the 16 explicit failure/fade controls;
  it does not claim all malformed-input, inherited-register or hardware states.
- Save reads and writes use deterministic in-memory BIOS/file services. No
  real memory card or host save file is changed.

The placement family also passes full-capacity actor/object/event inputs
without a sentinel and an object-ID-123 control (28 cases total). STAT includes
a successful already-sector-aligned 4,096-byte file and verifies that it is not
rounded up again, at each of four source byte alignments. TMD defaults include modes below, inside, and above the switch
range, zero-length unknown payloads, and a following wrapping-index packet.
TIM controls cover a no-CLUT record, opaque mode bits, zero-area upload and
word-rounded block length; both absent CLUT pointers must become zero.

### Chunked COM and MIX data

Every shipped top-level DAT stream walks exactly to EOF when each little-endian
length is interpreted as payload bytes and the cursor advances by `4 + length`.
`COM.DAT` has seven chunks with payload lengths
`[420, 3264, 704, 756, 480, 1128, 600]`. Every `MIXA.DAT` has eight chunks:

| Floor | Payload lengths | Live records before sentinel |
| ---: | --- | --- |
| 1 | `21536, 489792, 50000, 276, 3180, 1920, 3040, 384` | items 22; objects 158; actors 119; events 6 |
| 2 | `28192, 481920, 50000, 204, 3200, 1920, 3040, 384` | items 16; objects 159; actors 119; events 4 |
| 3 | `26144, 487408, 50000, 624, 2960, 1840, 3040, 384` | items 51; objects 147; actors 114; events 2 |
| 4 | `26144, 487664, 50000, 36, 2900, 1744, 3040, 384` | items 2; objects 144; actors 108; events 3 |
| 5 | `25632, 488896, 50000, 324, 880, 896, 3040, 384` | items 26; objects 43; actors 55; events 2 |

Floors one through four have five `MIXB.DAT` chunks; floor five has four and
uses an external `CHR1.MIM` through `CHR3.MIM` variant archive. Payload lengths
are:

| Floor | Payload lengths | Nested archive entry counts |
| ---: | --- | --- |
| 1 | `274592, 199824, 132856, 32680, 348452` | `6, 18, 7` |
| 2 | `221480, 232660, 91404, 39344, 414736` | `4, 18, 8` |
| 3 | `251844, 204864, 55296, 39688, 392724` | `2, 18, 6` |
| 4 | `253148, 172968, 40664, 45272, 419796` | `2, 18, 7` |
| 5 | `186728, 196220, 64976, 61708` | `2, 18` |

The three external CHR archives have six entries each. All nested asset walks
end exactly at their containing payload.

Retail intentionally reads across two COM chunk boundaries. The armor loader
copies 1,176 bytes from a nominal 756-byte payload, consuming the next
four-byte magic-length header and 416 magic bytes. The object-definition
loader copies 1,280 bytes from a nominal 1,128-byte payload, consuming the next
four-byte growth-length header and 148 growth bytes. A safe codec must bounds
check these spans against the remaining whole-file tail, not reject them merely
for crossing the nominal payload boundary. Conversely, the actor-definition
chunk contains 20 records but GAME copies the first 12, and the growth chunk
contains 50 records but GAME copies the first 40.

### TMD and animation assets

The fixed TMD corpus is 246 payloads: 67 item TMD files, ten raw MIXB TMD
chunks, one COM asset, 16 weapon assets, and 152 nested MIXB/CHR assets. It has
1,911 objects and 117,119 primitive packets. All packets use one of the eight
masked modes handled by retail: `0x20:27216`, `0x24:16889`, `0x28:2407`,
`0x2c:19844`, `0x30:31150`, `0x34:16440`, `0x38:3129`, and `0x3c:44`.

There are 169 `KfAssetHeader` resources when raw item/MIXB TMDs are excluded.
Ninety-nine are static and 70 are animated. For animated assets the signed
field at `+4` is the clip count: 33 assets have one clip, 3 have two, 1 has
three, 5 have four, 17 have five, 10 have six, and 1 has seven. The 214 clips
contain 813 keyframes and every clip's durations sum to 4,096. Every observed
clip `unknown_02` is 20 and every keyframe `reverse` is zero. There are 1,851
morph references excluding each keyframe's rest reference; observed
`morph_count` is 0 through 14. Every referenced table entry, record, and delta
span is within its asset, and every `base_vertex + vertex_count` is within TMD
object zero. The largest animated vertex count is 651, below the 1,000 usable
entries of the 1,001-vector shared scratch array.

The retail animation body does not initialize `kf_index`. Its starting value
is inherited in a callee-saved register, then it is incremented while scanning
keyframes and written to the pool record's cache field. An oracle must seed the
same registers, and a Rust API must expose the initial value explicitly; using
an invented zero would make the cache metadata deterministic in a way retail
is not. The function also trusts the tag, duration, table offsets, object
indices, and caller vertex count.

The animation differential seeds retail and candidate `$s5` with the same
explicit `0x4100` value, exercises every one of the 813 shipped keyframes as a
cache miss and the first keyframe of every one of the 214 clips as a cache hit,
and compares complete pool-record, cache, scratch and TMD-state bytes with Rust.
Together with static assets and ten synthetic selection/lifecycle controls,
all 1,136 cases pass. Lifecycle cases cover null-record allocation, exhausted
pool, vertex-allocation retries (including three consecutive failures),
different-asset reinitialization with retries and static release. Exact GAME
TMD selectors are shared providers. Sony Release 2.5
`gteMIMefunc` at `0x8004c860` is an explicit shared service whose independent
fixed-point model applies the GPF 12-bit shift, signed IR saturation, wrapping
halfword addition, and preserved vector padding; retail and candidate call
traces are also required to agree.

### TIM

GAME can reach 297 TIM-formatted files containing 338 records. Of these, 291
contain one record, `COM/MIX.TIM` contains seven, and each floor's `MIX.TIM`
contains eight. All end exactly at the last parsed record. Mode `0x8` occurs
330 times and mode `0x9` eight times. This census includes three root error
images (`E0.` through `E2.`), the floor transition, all COM/floor texture
bundles, 44 ENE, 39 KAN, ten MAP, 17 PRSN, 149 TALK, 25 numbered menu images,
and three save icons. `E3.` and `B0/MIX0.`, `MIX3.`, and `MIX9.` are valid TIM
files but are OPEN-only and are deliberately excluded from the GAME corpus.

`get_tim_addr` checks only the `0x10` magic, reads the mode, optionally exposes
the CLUT rectangle/data when bit 3 is set, always exposes the pixel
rectangle/data, and returns a word count that advances `ReadTIM` to the next
record. It does not receive a buffer length and cannot prove any block is in
bounds. Rust must offer a bounded form while retaining a faithful descriptor
and consumed-length result for differential comparison.

### VAB and SEQ

The five MIXA VAB headers are respectively 21,536, 28,192, 26,144, 26,144,
and 25,632 bytes; bodies are 489,792, 481,920, 487,408, 487,664, and 488,896
bytes. Each header starts `pBAV`, has version 6 and ID zero, and its `fsize`
equals header plus body size. Program/tone/sample counts are `(37,77,49)`,
`(50,90,51)`, `(46,74,53)`, `(46,78,54)`, and `(45,88,55)`. The header extent
matches the Psy-Q layout `32 + 128*16 + programs*16*32 + 256*2` exactly.
Sony `SsVabOpenHead` is the parser; the GAME wrapper owns stop/error/ID policy.
The Rust codec exposes every program/tone row and bounds sample byte spans
from the length table, preserving VH/VB bytes on roundtrip. VB ADPCM is opaque
sample data passed to SPU hardware; it is not decoded by GAME.

The separate VAB state oracle compares all mutated VH bytes (dense program
indices and cumulative SPU addresses included), the 0x90-byte GAME audio state,
maximum/open-bank counts, 16 bank statuses, six 16-word Sony tables and three
current-pointer aliases. All five banks pass with strict write guards and the
same seven ordered service requests. This independent Rust transform is in
`audio_vab_state`, separate from the on-disc `audio` reader.
The entire VB is also captured and required to remain unchanged, and any
decoded store overlapping it is rejected. This explicitly checks the largest
body's tail even though it overlaps the harness's conservative stack window.

Sixteen additional VAB controls compare failures and fades, including Sony's
partial writes and automatic-ID cleanup at the byte before the status array.
Rust represents that byte explicitly and safely. These controls found and
fixed a Rust model bug: transfer size uses only the low-byte sample count plus
one lengths, not every entry in the 256-entry table. See the
[campaign report](patterns/game-parser-verification.md) for exact cases,
remaining limits and the distinction between this fix, the TMD relocation
inventory correction and C instruction-level improvements.

All nine SEQ files start with bytes `70 51 45 53` (`pQES` in file order).
`SsSeqOpen` allocates a sequence slot and delegates header/state parsing to
`InitSoundSeq`; playback continues parsing variable-length deltas and events
through the listed SEQREAD family. Treating `audio_play_map_sequence` as the
only parser would omit most of the format. All 15,880 events from nine files,
including 5,154 running-status events, agree on all 176 initialized/post-event
score bytes and exact ordered key-on/key-off/pitch-bend argument words. Both
zero- and nonzero-seeded opening runs also compare the complete 0x90-byte GAME
audio state, Sony's four-byte open flag at `0x8009a728`, and the complete loaded
SEQ bytes. The provider-allocated sequence slot is an explicit Rust input.
Strict non-stack write ranges are score plus flag for direct opening; score,
flag, GAME state and the loaded file for the wrapper; and score only for each
event. Raw VLQ deltas remain unscaled in the format view; Sony runtime
delta fields use the directly evidenced factor of ten. Negative controls
deliberately corrupt score bytes and command arguments and require failure.

### Save data

The on-card file is a 0x280-byte header followed by four 0x2580-byte payload
slots. GAME validates read length and compares six summary words for all four
directory entries before loading a slot, but it does not validate the `SC`
magic, title, block count, or serialized field values. Catalog parsing accepts
every slot byte other than zero and four, then indexes output by `slot - 1`;
a malformed larger ID can write out of bounds. The raw slot reader similarly
trusts directory IDs and copies opaque ranges directly into live state.

The nested world record is variable-length. Its fixed part is one marker byte,
eight seven-byte event summaries, actor count, 190 object IDs, sparse-object
count, ten four-byte effect records, and twenty three-byte effect records.
Actor overrides cost two bytes each and sparse object links cost nine. Counts
and record limits are trusted during restore. The formula
`base - 1690 + floor * 1700` starts floor one ten bytes into the copied world
block and leaves only 1,690 copied bytes after the floor-five start; the shipped
runtime state must be measured before claiming every theoretical maximum is
safe.

`save_file_write_slot` first selects a retired directory entry (ID four), then
an empty entry (ID zero). It writes the payload before updating directory IDs
and six summary words, then writes the header. Each write stage retries five
times. A failed header write leaves the updated in-memory header in place. If
the previous slot is absent, the signed index `-1` stores its retired marker at
header byte `0x1ff`; the Rust writer preserves this within-buffer quirk. A
directory with neither retired nor empty entries would produce a negative
retail file offset; the bounded Rust API rejects that case. This is not a
claim of malformed-directory equivalence.

## I/O and runtime consumers, not omitted parsers

The CD functions `cd_file_load_allocated` `0x8001acf0`,
`cd_file_load_table_entry` `0x8001ae60`, and `cd_file_load_into` `0x8001af9c`
construct paths, sector-round lengths, retry reads, and allocate or fill
storage. They do not interpret resource bytes. Likewise
`map_resource_path_set_floor` `0x8001b390`, `map_resource_load_file`
`0x8001b3a4`, `menu_map_viewer` `0x80022d7c`, `menu_load_item_model`
`0x8002aea4`, `menu_load_item_texture` `0x8002af48`,
`screen_show_image_until_input` `0x8002c794`, and the TALK/ENE/PRSN/KAN path
builders select files or consume already parsed images. MAP files are ordinary
single-record TIM files; there is no separate map-image format in GAME.

`player_equip_weapon` `0x80016a30` loads a `WEPnn.MIM` file and hands it to
`asset_registry_set`; despite the extension it is one `KfAssetHeader`, not a
raw texture. `map_variant_assets_load` `0x8001b414` loads a CHR model-asset
archive, also not a texture. `camera_path_compute_segment` `0x800332e4` reads
28-byte points from a table initialized in `GAME.EXE`; it is a serialized-style
runtime consumer but no disc file feeds it, so it is outside the resource-codec
corpus unless a later xref proves another owner.

## Reproduction commands

All semantic queries were image-qualified and run under `nix develop`. The
newly studied animation, map-image, save, TIM, VAB, and SEQ entries received
`kf sema --image game addr`, `disasm --blocks`, `xref`,
`xref --callees`, `strings`, and `match` passes. The disc census used the
configured retail directory only and did not copy proprietary bytes into the
repository. Representative checks are:

```sh
nix develop --command kf-retail-validate
nix develop --command python -m scripts.kf.codec_oracle
nix develop --command kf sema --image game addr 0x800205d4
nix develop --command kf sema --image game disasm 0x800205d4 --blocks
nix develop --command kf sema --image game xref 0x800205d4 --callees
nix develop --command kf sema --image game addr 0x80052cb0
nix develop --command kf sema --image game disasm 0x80052cb0 --blocks
nix develop --command kf sema --image game addr 0x8002beb0
nix develop --command kf sema --image game xref 0x8002beb0 --callees
nix develop --command python -m scripts.kf.animation_oracle
```
