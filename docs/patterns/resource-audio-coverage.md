# Resource loading and remaining OPEN audio coverage

## Review plan

Complete current literal accounting for GAME `cd_file.c` and `resources.c`,
and OPEN `audio_spatial.c`, `audio_play_voice.c` and `sound_ref.c`. These modules
connect file loading and resource registration to sequence selection and voice
dispatch. Earlier grouped notes describe their contracts but do not give a
complete current occurrence ledger.

Capture each function's image-qualified disassembly, CFG, callers, callees,
strings, current match and history. Inspect SDK call boundaries and adjacent
functions before deciding which remaining literals need names. Preserve raw
instructions, delay slots and ordered referents. The SDK CD, TIM and sound
providers remain library code; this review concerns the surrounding game
orchestration and data conventions.

Review local retry/completion bookkeeping separately from SDK return codes,
path character offsets separately from resource IDs, and playback dispatch
results separately from voice allocation success. Check both image variants
before borrowing a semantic interpretation from a similarly named function.
Force the five units before and after review, compare raw words and references,
and run inventory, lint, repository tests and full build before committing.

The map sequence selector also reaches `map_scripts.c`. Its cross-module enum
propagation remains outstanding; named constants alone do not provide that
type safety.

## Retained contracts and remaining type work

GAME's path loaders use the same signed-word local first as a rounded sector
count and then as a completion flag. The indexed loader uses it only as a
completion flag. The flag begins at zero and becomes one only when the SDK
read poll returns exactly zero. Positive remaining-sector counts keep polling;
negative results allow another attempt. These local Boolean values are distinct
from SDK return codes. The wrappers themselves return zero even after invoking
the read-failure screen. That screen can return after a press/release cycle;
its name does not establish a nonreturning failure path.

The prefix and suffix extents include their terminators. Path positions one,
three and six identify the floor digit, start of the filename, and CHR variant
digit in the local `B0\CHR0.MIM` template. Adding `'0'` performs character
encoding; these offsets are not asset registry identities. Resource payloads
use the existing named chunk-header size, typed grid extents and named registry
slots. A zero `use_variant` selects the embedded actor archive; every nonzero
value selects the allocated CHR archive path. Its caller-wide Boolean interface
remains a separate type-propagation task.

OPEN's all-zero program/tone/note tuple suppresses a voice request before the
round-robin slot advances. No individual zero component is independently a
no-sound sentinel. After incrementing, the voice index wraps at the named
ten-slot capacity. The initializer is capacity minus one so the first request
uses slot zero. The key-on call receives numeric zero fine pitch; its return
value is stored without a success check.

The spatial wrapper returns zero at or beyond the distance cutoff and one
after calling the void voice helper. The latter can suppress the all-zero tuple,
so one means dispatch, not confirmed allocation or audible playback. The
folded bearing is halved before the sine/cosine pair. At Q7 attenuation 96 or
above, OPEN assigns attenuation itself to both channels, rather than the
volume-scaled level. This existing image-specific behavior is retained. The
key-off wrapper packs the note in the upper byte, leaves fine pitch zero and
does not read the active VAB state; no validated caller establishes a more
specific game use.

No source or enum changes follow from this review. The
[complete occurrence ledger](resource-audio-literal-ledger.md) accounts for all
61 retained literals in these five modules, including the zero-token forwarding
wrapper. Existing named policies and unresolved selector type work remain
distinguished from this accounting.

## Per-function evidence and final verdicts

Each function began and remains at strict 100%. The table records the retail
extent and direct call/branch instruction counts from its own disassembly.
All candidate words, delay slots and ordered referents remain equal to the
delinked target and raw retail words. No function was newly banked.

| Image | Function | VA / bytes | Calls / branches | Final verdict |
| --- | --- | --- | --- | --- |
| GAME.EXE | `cd_file_load_allocated` | `0x8001acf0 / 368` | 9 / 6 | 100%; unchanged. |
| GAME.EXE | `cd_file_load_table_entry` | `0x8001ae60 / 316` | 5 / 4 | 100%; unchanged. |
| GAME.EXE | `cd_file_load_into` | `0x8001af9c / 356` | 8 / 6 | 100%; unchanged. |
| GAME.EXE | `tim_upload_images` | `0x8001b100 / 128` | 6 / 3 | 100%; unchanged. |
| GAME.EXE | `common_resources_load` | `0x8001b180 / 528` | 10 / 4 | 100%; unchanged. |
| GAME.EXE | `map_resource_path_set_floor` | `0x8001b390 / 20` | 0 / 0 | 100%; unchanged. |
| GAME.EXE | `map_resource_load_file` | `0x8001b3a4 / 64` | 2 / 0 | 100%; unchanged. |
| GAME.EXE | `map_resource_copy_words` | `0x8001b3e4 / 48` | 0 / 2 | 100%; unchanged. |
| GAME.EXE | `map_variant_assets_load` | `0x8001b414 / 136` | 2 / 0 | 100%; unchanged. |
| GAME.EXE | `audio_play_current_map_sequence` | `0x8001b49c / 188` | 1 / 7 | 100%; unchanged. |
| GAME.EXE | `map_resources_load` | `0x8001b558 / 600` | 31 / 1 | 100%; unchanged. |
| OPEN.EXE | `audio_play_spatial` | `0x80019f44 / 488` | 5 / 6 | 100%; unchanged. |
| OPEN.EXE | `audio_play_spatial_default_range` | `0x8001a12c / 48` | 1 / 0 | 100%; unchanged. |
| OPEN.EXE | `audio_play_spatial_range` | `0x8001a15c / 44` | 1 / 0 | 100%; unchanged. |
| OPEN.EXE | `sound_ref_key_off_bank0` | `0x8001a188 / 40` | 1 / 0 | 100%; unchanged. |
| OPEN.EXE | `audio_set_listener_transform` | `0x8001a1b0 / 112` | 0 / 2 | 100%; unchanged. |
| OPEN.EXE | `sound_ref_play` | `0x8001a220 / 72` | 1 / 0 | 100%; unchanged. |
| OPEN.EXE | `audio_play_voice` | `0x8001a268 / 404` | 2 / 5 | 100%; unchanged. |

## Verification

The five reviewed source files are byte-identical to their captured baseline.
All eighteen forced-rebuilt functions remain exact, preserving 990 words,
85 ordered calls and 77 address materializations. Twenty initialized data
bytes across four claims agree with both object views and retail: the CD
prefix/suffix, TIM filename and initial voice-slot index.

Inventory, Ruff, whitespace and all 684 repository tests pass (90.751 seconds).
Modern checking retains the same 300 diagnostics, with 65/112 variants passing.
Full `kf build` retains source-data mismatches (PSX 0/1, GAME 9/42, OPEN 2/19)
and target-relink gaps (PSX 1/1, GAME 75/77, OPEN 34/38), with six conflicting
section bases and zero artifact failures. These existing failures prevent a
claim that the full build is clean.
