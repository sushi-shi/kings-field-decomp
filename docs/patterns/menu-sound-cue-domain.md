# Menu sound cue domain

## Function Match Plan

For GAME.EXE `menu_play_input_sound`, `0x8002b150 / 0x84`, carry the three
existing cue identities through a signed-word `KfMenuSoundCue` parameter.
All current direct source callers already pass these names. Name the paired
stereo volume as `MENU_INPUT_SOUND_VOLUME` without changing its value 64.
Keep the original two comparisons and final fallback; values other than zero
and one still select the cancel/error voice.

Current source/history and the cached retail disassembly/CFG, caller/callee,
string and match dossier were reviewed alongside the adjacent catalogue reader
and card initializer. The helper is game menu policy around three SDK calls,
not a reconstructed SDK body. Its frame is 32 bytes. Retail checks zero at
`0x8002b154` and one at `0x8002b170`. The selected byte program/note pairs are:

| Cue | Encoded value | Program / note |
| --- | ---: | --- |
| `MENU_SOUND_CURSOR` | 0 | 14 / 68 |
| `MENU_SOUND_CONFIRM` | 1 | 13 / 60 |
| `MENU_SOUND_CANCEL_OR_ERROR` | 2 | 15 / 63 |

At `0x8002b194` and `0x8002b198`, both channel arguments receive 64. The helper
calls `SsVoKeyOn` at `0x8002b1a4`, waits with `VSync(0)`, then calls `SsVoKeyOff`
at `0x8002b1bc`. The key-on/off delay slots shift the note into the high byte.
These are direct calls; there are no string or global-data references.

The incoming dossier has 92 rows: **90 proven calls and two validated internal
branches**, not 92 calls. The current identity note conflates those counts and
will be corrected. Its source identity/evidence tier otherwise stays the same.
The cached 100% match describes an earlier source version and does not verify
this edit. Builds, compiler checks, tests and post-edit matches remain deferred.

## Retained values

The per-cue program/note pairs remain authored VAB data, already explained by
the branch cue and individual ledger rows. Their original timbre/pitch choices
are unproven; naming another selector enum would confuse them with menu actions.
The two shifts by eight express the SDK note/fine-pitch packing, and `VSync(0)`
is the authentic SDK wait mode. The common channel volume deserves one name
because it is a shared menu-feedback setting. Its original loudness rationale
is unproven; 64 is not claimed to be an exact half-gain or perceptual midpoint.

## Source result

The helper and shared prototype now use `KfMenuSoundCue`. All 90 source calls
already carry the domain's constants: 45 cursor, 19 confirm and 26 cancel/error.
The two shared-volume uses are named. The identity signature and incoming-call
count are corrected without promoting its evidence tier. Branches, program/note
values, SDK arguments and key-on/wait/key-off order remain unchanged in source.

Final verdict for the planned helper: source review complete; compiler and
post-edit match verification deferred. No builds, compiler checks, tests or
banking ran. The save-system ledger now accounts for 133 retained occurrences;
the whole-source accounting covers 111 C files and 6,000 occurrences with
individual reasons. This does not establish byte matching or naming completion.
