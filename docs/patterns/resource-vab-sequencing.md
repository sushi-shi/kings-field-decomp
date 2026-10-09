# Resource VAB header/body sequencing

## Source verdict

The five GAME/OPEN resource loaders capture the header payload in a separate
statement before calling `audio_load_vab`. The first argument then reads only
that saved pointer; the second advances `stream` to the body chunk. In OPEN,
that same expression also saves the body header as `vab_chunk` for later arena
reuse. Argument evaluation order cannot change the chunk pairing.

The cursor macro remains appropriate for calls with one cursor argument.
There is no remaining conflicting read of `stream` in these five calls. The
saved header is a real audio argument, not a stack reservation or compiler
carrier. Its original historical source spelling remains unknown.

## Retail contract

Each chunk is `u32 payload_size; payload[payload_size]`. The header payload
starts at stream +4, the body header at stream + header_size +4, and the body
payload another four bytes later. Both audio implementations pass the first
pointer to `SsVabOpenHead` and the second to `SsVabTransBody`. GAME additionally
stores the header pointer; these are distinct object identities.

| Image/function | Audio call | Header/body setup | Retained body header |
| --- | --- | --- | --- |
| GAME `map_resources_load` | `8001b5d4` | `8001b5c4..8001b5d8` | `s2`, then copied to `s0` at `8001b5e0` |
| OPEN `opening_resources_load_scene0` | `80016398` | `80016384..8001639c` | `s0`, also stored at `sp+16` |
| OPEN `opening_resources_load_scene1` | `80016560` | `8001654c..80016564` | `s0`, also stored at `sp+16` |
| OPEN `opening_resources_load_ending` | `80016720` | `8001670c..80016724` | `s0`, also stored at `sp+20` |
| OPEN `opening_resources_load_ending_sequence` | `800168a8` | `80016894..800168ac` | `s0`, also stored at `sp+16` |

At each call, `a0` contains the first payload and the delay slot finishes `a1`
as body-header +4. Subsequent walks advance past the body. Arena reuse starts
at body-header +16: GAME stores it at `8001b6d8`; OPEN scene0 at `80016488`,
scene1 at `80016590/80016598`, ending at `80016784`, and ending-sequence at
`800168c4`. Scene1 and ending also preserve the cursor for later scene loads.
Ending-sequence does not need a further resource advance after its VAB call.

The shipped OPEN files support these boundaries (offsets from file start):

| File | Header bytes | Body bytes | Body payload | Arena reuse |
| --- | ---: | ---: | ---: | ---: |
| `B0/MIXA0.` | 26144 | 487408 | 26152 | 26164 |
| `B0/MIXA1.` | 11296 | 483232 | 11304 | 11316 |
| `B0/MIXAE.` | 11296 | 483232 | 11304 | 11316 |
| `B0/MIXAG.` | 9760 | 362128 | 9768 | 9780 |

All four header payloads begin with the little-endian VAB magic `pBAV`.
This is a serialized-input and instruction audit, not a full OPEN runtime
oracle or an exhaustive resource-state claim.

## Rejected candidates and tooling observation

The earlier separate advance before the call added an instruction to GAME
(body 0x258 became 0x25c). A new candidate computing the body pointer separately
and assigning `stream` after the call also changes emitted instructions: its
first body-pointer setup uses `lw s0,0(s2)` and a body payload displacement of
8; arena reuse becomes an offset of 20 from that intermediate base. That
candidate was not retained. The header-capture form preserves the original
instructions and ordered referents without using unsequenced access.

Long candidate debug filenames can stall the pinned assembler. A persistent
trial's 123-byte `.file` directive produced empty output and timed out; changing
only that debug filename to `resources.c` allowed both UNIT and SIZES assembly
to finish. Running the actual candidate from a short source path then allowed
normal `kf try` comparison. This observation does not attribute a specific
assembler buffer limit or change the pinned toolchain.

## Verification and per-function verdicts

All 97 native-derived ELF comparison objects remain byte-identical to the
captured `19df9a3e` baseline, including instructions, data, and relocations.
These are `.o` comparison views; this claim excludes native debug records.
Refreshed strict objdiff reports give these individual verdicts:

| Unit/function | Verdict |
| --- | --- |
| GAME `tim_upload_images` | 100%, unchanged |
| GAME `common_resources_load` | 100%, unchanged |
| GAME `map_resource_path_set_floor` | 100%, unchanged |
| GAME `map_resource_load_file` | 100%, unchanged |
| GAME `resource_stream_copy_words` | 100%, unchanged |
| GAME `map_variant_assets_load` | 100%, unchanged |
| GAME `audio_play_current_map_sequence` | 100%, unchanged |
| GAME `map_resources_load` | 100%, header capture sequenced |
| OPEN `cd_file_load_allocated` | 100%, unchanged |
| OPEN `cd_file_load_into` | 100%, unchanged |
| OPEN `tim_upload_images` | 100%, unchanged |
| OPEN `resource_stream_copy_words` | 100%, unchanged |
| OPEN `opening_resources_load_scene0` | 100%, header capture sequenced |
| OPEN `opening_resources_load_scene1` | 100%, header capture sequenced |
| OPEN `opening_resources_load_scene3` | 100%, unchanged |
| OPEN `opening_resources_load_ending` | 100%, header capture sequenced |
| OPEN `opening_resources_load_ending_entities` | 100%, unchanged |
| OPEN `opening_resources_load_ending_sequence` | 100%, header capture sequenced |

The existing GAME retail/C/Rust outer-walk oracle passes all five floors,
including the floor-five external variant. It checks consecutive VAB payloads,
50,000 copied grid bytes per floor, subsequent typed callbacks, final path,
variant buffer, and the retained arena cursor. Strict total remains 465/471;
no function was newly banked. Existing data/section-placement failures remain.

Focused Clang C89 controls use `-Werror=unsequenced`: the original GAME source
has one diagnostic and original OPEN has four. Both corrected sources pass
that gate; both corrected C++20 views also compile. This diagnostic control
checks source sequencing independently of the identical target instructions.
All three linked CPE/EXE pairs are byte-identical to the captured baseline.

Full `kf build` succeeds for all three images. `kf check-types` passes all
97 image variants with zero enum-domain literals. Ruff, all 885 local tests
(no skips), and diff whitespace checks pass. This package changes no tooling
or flake inputs; the preceding actor package's flake gate remains recorded
under its own verdict.
