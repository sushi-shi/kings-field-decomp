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

`include/kf/lib/audio.h` owns the packed `SoundRef`, the voice-slot lanes, the
complete `KfAudioState` layout, and declarations for the three audio-owned
globals. Vendor calls remain declared by the Psy-Q headers; this header models
only the game's state and policy boundary.

The voice region is one aggregate rather than five overlapping globals. The
code retains the base at `0x80095894` and accesses five `s16[10]` lanes at
offsets `0x00`, `0x14`, `0x28`, `0x3c`, and `0x50`. `KfAudioVoiceSlots` names
those fields and checks every lane offset. The adjacent `game_exit_code` at
`0x800958f8` proves the complete `0x64`-byte object extent.

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
- `sound_ref_key_off_bank0` keys off the selected program/note in VAB bank 0.
  GAME 80033014 and OPEN 8001a188 both ignore the `SoundRef` tone byte and
  discard the SDK result. They do not read `audio_state.active_vab_id`.

## Packed key-on/key-off selectors

The former `audio_key_off_mask` identity misinterpreted the SDK inputs.
Both retail wrappers load unsigned bytes zero and two, then put the latter
in bits 8..15 of the second argument. The exact-FID `SsVoKeyOff` providers
decode the first argument into bank/program and the second into note/fine
pitch. Sony's later
[Run-Time Library Reference 4.7, pages 14-131/132](https://psx.arthus.net/sdk/Psy-Q/DOCS/LibRef47.pdf#page=978)
corroborates those packed fields and documents MIDI note numbers in the
upper byte, with fine pitch in the lower byte. This supports the interface
interpretation, not an attribution of the retail program to that later SDK.

The direct retail control-flow chains are:

| Image | Game wrapper | `SsVoKeyOff` | `SpuVmSeKeyOff` | `SpuVmKeyOff` |
| --- | --- | --- | --- | --- |
| GAME | 80033014 | 8004b6a4 | 80044120 | 80043d8c |
| OPEN | 8001a188 | 8002b478 | 80023f40 | 80023bac |

The game supplies a zero upper byte for bank and a zero lower byte for fine
pitch. Both retail `SpuVmSeKeyOff` bodies forward the bank, program and note,
with internal sequence selector 0x20, and do not consume the fourth decoded
fine-pitch argument. `SpuVmKeyOff` compares selectors against per-voice fields
before keying off matching voices. It does not treat the input bytes as a
voice bitmask. The GAME key-on path (8004688c → 80044030 → 80043988) uses
the same selector packing and converts its two volume arguments to volume
and pan; equal channel values select centered pan.

The established three-byte `SoundRef` view gives semantic field names to
the two bytes consumed by the wrappers. No caller or loaded pointer to these
game wrappers is admitted, so their original use site and enclosing input
object remain unresolved. The typed view makes no additional global extent
or data-ownership claim, and the ignored middle byte remains the shared
record's tone field rather than a new mask word.

The menu cue provider at GAME 8002b150 uses the same bank-zero interface:

| Cue | Program | MIDI note | Reviewed uses |
| --- | ---: | ---: | --- |
| `MENU_SOUND_CURSOR` (0) | 14 | 68 | Cursor changes, opening a dialog, empty-list feedback and config actions. |
| `MENU_SOUND_CONFIRM` (1) | 13 | 60 | Confirmation button/accepted choice, including choosing a back row. |
| `MENU_SOUND_CANCEL_OR_ERROR` (2) | 15 | 63 | Cancellation/dismissal, insufficient gold, an empty save slot and other rejected choices. |

Every value other than 0 or 1 follows the third recipe, as before. All 89
source calls retain their selected recipe; in particular, the config panel
still uses cue 0 for all of its actions. Each recipe keys on with equal
channel volume 64, waits once with `VSync(0)`, then keys off. The level is
64/127 of the channel input range. The actual audible duration also depends
on sample/envelope behavior; the code does not establish a fixed-duration
beep. No designer rationale for these program/note choices or the exact
volume has been recovered, and a sampled note number alone does not prove
an audible frequency or instrument identity.

## SDK boundary

Psy-Q Release 2.5 `LIBSND.H` supplies the authentic widths and prototypes for
`SsSetTableSize`, VAB transfer, sequence open/play/volume/stop/close, master
volume, reverb, and voice calls. Several targets have exact Release 2.5 FIDs.
The `SSOPEN`, `SEPINIT`, `SEQINIT`, `SSPLAY`, `STOP`, `SSCALL`, `DECRE`,
`REPLAY`, `CRES`, `PAUSE`, `TEMPO`, and `SSCLOSE` families are
provider-attributed through archive symbols and XREFs, official public
prototypes where available, function semantics, and complete GAME/OPEN
instruction-shape agreement. Their exact SDK revision remains unresolved, so
the version-skewed bodies use `sdk-lineage-supported` rather than an exact
Release 2.5 confidence class and are not reconstructed as game bodies.

That evidence also identifies the internal data referents `_snd_openflag`,
`_ss_score`, `_snd_seq_s_max`, `_snd_seq_t_max`, and `VBLANK_MINUS` in both
overlays. OPEN's `SsSepOpen` begins at
`0x8002675c`: its first `lui`/`lw` pair hoists the `_snd_openflag` load above the
stack-frame allocation, correcting an earlier false eight-byte data gap.

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
  `SsInit`, `SsSetTableSize`, and `SsVabOpenHead` targets before adding them to
  the vendored census. `VSync` and its private worker are admitted through
  cross-overlay `LIBGPU/VSYNC` lineage; their exact SDK revision remains
  unresolved.
- Resolve the original input owner and any indirect caller of
  `sound_ref_key_off_bank0`; no pointer to the function appears in loaded
  GAME data. Its program/note field interpretation is established above.
- Recover translation-unit boundaries before assigning global versus `static`
  linkage to the audio state.
