# Audio control

The reviewed GAME audio-control block at `0x800328e0..0x80032c78` owns game
policy around Sony's Psy-Q sequence and VAB APIs. The five new identities are
game functions: they choose files, volumes, fade timing, and shared state while
calling linked `LIBSND`/`LIBGPU` routines whose bodies do not count as game
decompilation progress.

## Storage

| Address | Extent | Identity | Evidence |
| --- | ---: | --- | --- |
| `0x80012a48` | `0x0b` | `audio_sequence_path_template` | copied as `B0\SND0.SEQ`, then patched at the map-bank and sequence digits |
| `0x80059738` | `0x158` | `audio_sequence_table` | `SsSetTableSize(table, 2, 1)` and Psy-Q 2.5 `SS_SEQ_TABSIZ == 172` prove `172 * 2 * 1` bytes |
| `0x80095868` | `0x04` | `audio_vab_header` | VAB header pointer retained until close |
| `0x8009586c` | `0x02` | `audio_active_vab_id` | returned by the VAB-head operation and set to minus one after `SsVabClose` |
| `0x80095870` | `0x04` | `audio_sequence_buffer` | `0x3000`-byte allocation used for sequence-file loading and `SsSeqOpen` |
| `0x80095874` | `0x02` | `audio_sequence_id` | access number returned by `SsSeqOpen` and passed to play/volume/stop/close operations |
| `0x80095878` | `0x04` | `audio_sequence_active` | word flag set after play and cleared after stop/close |
| `0x80095894` | `0x14` | `audio_voice_ids[10]` | ten halfwords initialized to minus one and indexed by the rotating effect-voice path |
| `0x800a0816` | `0x01` | `audio_effects_enabled` | settings byte guarding sound-effect voice allocation |
| `0x800a0817` | `0x01` | `audio_music_enabled` | settings byte guarding map-sequence loading and playback |

All BSS identities retain `scope=unknown`. Multiple users prove shared storage,
but the linked image does not distinguish external linkage from file-local
statics used by one original translation unit. The reviewed HI16/LO16 pair at
`0x800328f0/0x800328f4` admits the previously missing sequence-table owner;
interior addresses were not modeled as separate globals.

## Operations

- `audio_initialize` initializes sequence services, master volume and studio-C
  reverb, allocates the sequence buffer, and resets the ten voice IDs.
- `audio_load_vab` receives separate VAB header/body pointers from the resource
  loader, transfers both parts, and retains the active VAB state.
- `audio_play_map_sequence` selects sequence 0, 1, or 2, patches the current map
  bank into the path, loads it, opens it against the active VAB, and starts it at
  volume 75 when music is enabled.
- `audio_stop_sequence_fade` lowers sequence volume once per vertical sync from
  75 through zero before stop/close.
- `audio_stop_sequence_master_fade` lowers symmetric master volume from
  fixed-point `0x4b00` by its caller-provided step, then stops and closes the
  sequence. The main-loop exit path supplies `0x80`.

Psy-Q Release 2.5 `LIBSND.H` supplies the authentic widths and prototypes for
`SsSetTableSize`, VAB transfer, sequence open/play/volume/stop/close, master
volume, and reverb calls. Several target functions already have exact Release
2.5 FIDs. The remaining anonymous targets are API-name candidates supported by
their ABI, call position, and surrounding library sequence; this pass does not
promote them to exact FID results or reconstruct their bodies.

Per-function evidence is in
`config/evidence/game_semantic_audio_control.tsv`.

## Next questions

- Establish exact Release 2.5/version-skew evidence for the currently anonymous
  `SsInit`, `SsSetTableSize`, `SsVabOpenHead`, `SsSeqOpen`, `SsSeqStop`,
  `SsSeqClose`, and `VSync` targets before adding them to the vendored census.
- Recover the four parallel ten-element sound-effect parameter arrays following
  `audio_voice_ids` while reviewing the sound-effect playback family.
- Recover translation-unit boundaries before assigning global versus `static`
  linkage to the audio state.
