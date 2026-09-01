# Audio system

The reviewed GAME audio block at `0x800328e0..0x8003329c` owns game policy
around Sony's Psy-Q sequence, VAB, spatialization, and voice APIs. The game
functions choose files, volume and fade policy, listener-relative panning, and
managed voice replacement while calling linked `LIBSND`/`LIBGPU` routines
whose bodies do not count as game decompilation progress.

## Storage

| Address | Extent | Identity | Evidence |
| --- | ---: | --- | --- |
| `0x80012a48` | `0x0b` | `audio_sequence_path_template` | copied as `B0\SND0.SEQ`, then patched at the map-bank and sequence digits |
| `0x80057b84` | `0x04` | `audio_voice_slot_index` | loaded `s32` initialized to nine and advanced modulo ten before each managed voice |
| `0x80059738` | `0x158` | `audio_sequence_table` | `SsSetTableSize(table, 2, 1)` and Psy-Q 2.5 `SS_SEQ_TABSIZ == 172` prove `172 * 2 * 1` bytes |
| `0x80095868` | `0x04` | `audio_vab_header` | VAB header pointer retained until close |
| `0x8009586c` | `0x02` | `audio_active_vab_id` | returned by the VAB-head operation and set to minus one after `SsVabClose` |
| `0x80095870` | `0x04` | `audio_sequence_buffer` | `0x3000`-byte allocation used for sequence-file loading and `SsSeqOpen` |
| `0x80095874` | `0x02` | `audio_sequence_id` | access number returned by `SsSeqOpen` and passed to play/volume/stop/close operations |
| `0x80095878` | `0x04` | `audio_sequence_active` | word flag set after play and cleared after stop/close |
| `0x8009587c` | `0x10` | `audio_listener_position` | `KfVec4i` copied from the per-frame player position snapshot |
| `0x8009588c` | `0x08` | `audio_listener_rotation` | `KfVec4s` copied from the per-frame player rotation snapshot; yaw is field `+2` |
| `0x80095894` | `0x64` | `audio_voice_slots` | ten-lane `KfAudioVoiceSlots` aggregate containing voice, VAB, program, tone, and note IDs |
| `0x800a0816` | `0x01` | `audio_effects_enabled` | settings byte guarding sound-effect voice allocation |
| `0x800a0817` | `0x01` | `audio_music_enabled` | settings byte guarding map-sequence loading and playback |

All BSS identities retain `scope=unknown`. Multiple users prove shared storage,
but the linked image does not distinguish external linkage from file-local
statics used by one original translation unit. The reviewed HI16/LO16 pair at
`0x800328f0/0x800328f4` admits the previously missing sequence-table owner;
interior addresses were not modeled as separate globals.

The voice region is one aggregate rather than five overlapping globals. The
code retains the base at `0x80095894` and accesses five `s16[10]` lanes at
offsets `0x00`, `0x14`, `0x28`, `0x3c`, and `0x50`. `KfAudioVoiceSlots` names
those fields and checks both the `0x64` size and every lane offset. The adjacent
`game_exit_code` at `0x800958f8` proves the complete-object extent.

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
- `audio_set_listener_transform` copies independently optional position and
  rotation snapshots. The main loop supplies the player snapshots every frame.
- `audio_play_spatial` rejects sources beyond `max_distance`, scales the base
  volume through `attenuation_distance`, and derives left/right volume from the
  listener-relative x/z angle. The two wrappers supply either explicit ranges
  or the defaults 16000 and 28000.
- `audio_play_voice` advances the ten-slot ring, keys off an occupied slot with
  its retained VAB/program/tone/note tuple, keys on the replacement, and stores
  the returned voice ID. `sound_ref_play` supplies the three packed
  program/tone/note bytes with equal left and right volume.

## SDK boundary

Psy-Q Release 2.5 `LIBSND.H` supplies the authentic widths and prototypes for
`SsSetTableSize`, VAB transfer, sequence open/play/volume/stop/close, master
volume, reverb, and voice calls. Several targets have exact Release 2.5 FIDs.
The remaining anonymous sequence targets are API-name candidates supported by
their ABI, call position, and surrounding library sequence; they are not
promoted to exact FID results or reconstructed as game bodies.

Two VMANAGER functions require a different proof channel. GAME
`0x80044fac/0x80045378` and OPEN `0x80024dcc/0x80025198` have respectively
243/243 and 73/73 identical instruction shapes. In both images they occupy the
same contiguous archive-order slots between exact Release 2.5 `note2pitch2`
and `SsUtPitchBend` anchors. Their bodies and official seven- and five-`short`
prototypes identify `SsUtKeyOn` and `SsUtKeyOff`. The key-off implementation is
`0x1c` bytes larger than the Release 2.5 corpus member, so all four copies are
classified `sdk-lineage-supported`, not falsely reported as exact Release 2.5
FIDs and not counted as game functions.

Per-function evidence is in
`config/evidence/game_semantic_audio_control.tsv` and
`config/evidence/game_semantic_audio_spatial.tsv`; the cross-overlay SDK proof
is in `config/evidence/overlay_lineage.tsv`.

## Next questions

- Establish exact Release 2.5/version-skew evidence for the currently anonymous
  `SsInit`, `SsSetTableSize`, `SsVabOpenHead`, `SsSeqOpen`, `SsSeqStop`, and
  `SsSeqClose` targets before adding them to the vendored census. `VSync` and
  its private worker are now admitted through cross-overlay `LIBGPU/VSYNC`
  lineage; their exact SDK revision remains unresolved.
- Resolve the three-byte voice-mask layout and any indirect caller of
  `audio_key_off_mask`; no pointer to the function appears in loaded GAME data.
- Recover translation-unit boundaries before assigning global versus `static`
  linkage to the audio state.
