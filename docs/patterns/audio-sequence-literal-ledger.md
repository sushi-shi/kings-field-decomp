# Retained audio and opening-resource literals

Complete current ledger after the [audio sequence-domain review](audio-sequence-domains.md).
Every row is one retained token, including duplicates. Named definitions, string contents and address claims are excluded.

## `src/game/audio.c`

38 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `audio_initialize` | 40 | `1` | `index = KF_AUDIO_VOICE_SLOTS - 1;` | Start at the last zero-based reusable voice slot; the named capacity supplies the count. |
| `audio_initialize` | 43 | `0` | `} while (--index >= 0);` | Include slot zero before exhausting the signed voice-slot countdown. |
| `audio_play_map_sequence` | 67 | `20` | `char path[20] = "B0\\SND0.SEQ";` | Twenty-byte local path workspace retained from the original frame; the eleven-character template plus terminator fits, but does not establish why that extra capacity was chosen. |
| `audio_play_map_sequence` | 71 | `6` | `path[6] = sequence_id + '0';` | Position six is the sequence digit in the adjacent B0\SND0.SEQ template; adding the ASCII zero glyph encodes that digit. Retain these local template offsets and character encoding. |
| `audio_play_map_sequence` | 71 | `'0'` | `path[6] = sequence_id + '0';` | Position six is the sequence digit in the adjacent B0\SND0.SEQ template; adding the ASCII zero glyph encodes that digit. Retain these local template offsets and character encoding. |
| `audio_play_map_sequence` | 72 | `1` | `path[1] = KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor) + '0';` | Position one is the floor digit in the adjacent path template. The floor domain is explicitly encoded at this text boundary; ASCII zero is character data. |
| `audio_play_map_sequence` | 72 | `'0'` | `path[1] = KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor) + '0';` | Position one is the floor digit in the adjacent path template. The floor domain is explicitly encoded at this text boundary; ASCII zero is character data. |
| `audio_play_map_sequence` | 73 | `0` | `if (cd_file_load_into(audio_state.sequence_buffer, path) == 0) {` | The existing CD wrapper uses zero for its accepted load path; preserve the exact return-code test before issuing sequence playback. |
| `audio_stop_sequence_fade` | 91 | `0` | `VSync(0);` | The SDK zero mode waits for the next vertical sync; it is the ordinary synchronization call argument. |
| `audio_stop_sequence_fade` | 93 | `0` | `} while (--volume >= 0);` | The GAME per-sequence fade includes volume zero before its signed countdown becomes negative. |
| `audio_stop_sequence_master_fade` | 108 | `0` | `VSync(0);` | The SDK zero mode waits for the next vertical sync; it is the ordinary synchronization call argument. |
| `audio_stop_sequence_master_fade` | 111 | `0` | `} while (volume > 0);` | The fixed-point master fade stops when its numeric volume reaches or crosses zero, then explicitly silences both channels. |
| `audio_stop_sequence_master_fade` | 112 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_stop_sequence_master_fade` | 112 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_stop_sequence_master_fade` | 113 | `0` | `SsSeqSetVol(audio_state.sequence_id, 0, 0);` | Set both numeric sequence-channel amplitudes to silence before stop/close. |
| `audio_stop_sequence_master_fade` | 113 | `0` | `SsSeqSetVol(audio_state.sequence_id, 0, 0);` | Set both numeric sequence-channel amplitudes to silence before stop/close. |
| `audio_close_vab` | 135 | `0` | `audio_state.vab_header = 0;` | Clear the borrowed VAB-header pointer after closing its handle. |
| `audio_play_spatial` | 161 | `0` | `return 0;` | No spatial voice dispatch beyond the cutoff; this is the Boolean dispatch result. |
| `audio_play_spatial` | 165 | `0` | `if (level < 0) {` | Lower bound of numeric attenuation-scaled volume; clamp negative values to silence. |
| `audio_play_spatial` | 166 | `0` | `level = 0;` | Silence is the lower endpoint of the numeric volume clamp. |
| `audio_play_spatial` | 167 | `1` | `} else if (level >= KF_AUDIO_MAX_VOLUME + 1) {` | Convert the inclusive maximum volume into the first out-of-range threshold without altering the retail comparison. |
| `audio_play_spatial` | 178 | `1` | `angle >>= 1;` | Halve the folded half-turn bearing for the quarter-turn sine/cosine panning pair; one is the arithmetic divide-by-two shift. |
| `audio_play_spatial` | 179 | `0x80` | `if ((sound->tone & 0x80) == 1) {` | Preserve the impossible high-bit-mask-equals-one predicate exactly. The branch cannot identify a working sound mode; do not invent a semantic flag name or normalize it to nonzero. |
| `audio_play_spatial` | 179 | `1` | `if ((sound->tone & 0x80) == 1) {` | Preserve the impossible high-bit-mask-equals-one predicate exactly. The branch cannot identify a working sound mode; do not invent a semantic flag name or normalize it to nonzero. |
| `audio_play_spatial` | 181 | `1` | `if (attenuation >= KF_AUDIO_MAX_VOLUME + 1) {` | The first value above the inclusive attenuation cap; retain the exact threshold inside the existing unreachable tone branch. |
| `audio_play_spatial` | 187 | `2` | `* (KF_FIXED7_ONE * 2 - attenuation * 2)) >> KF_FIXED7_BITS)` | Both factors of two belong to the linear pan-narrowing equation, scaling the distance from Q7 unity; they are arithmetic coefficients, not selector values. |
| `audio_play_spatial` | 187 | `2` | `* (KF_FIXED7_ONE * 2 - attenuation * 2)) >> KF_FIXED7_BITS)` | Both factors of two belong to the linear pan-narrowing equation, scaling the distance from Q7 unity; they are arithmetic coefficients, not selector values. |
| `audio_play_spatial` | 191 | `1` | `if (left >= KF_AUDIO_MAX_VOLUME + 1) {` | Clamp the computed left-channel amplitude at the inclusive named maximum. |
| `audio_play_spatial` | 195 | `1` | `if (right >= KF_AUDIO_MAX_VOLUME + 1) {` | Clamp the computed right-channel amplitude at the inclusive named maximum. |
| `audio_play_spatial` | 205 | `1` | `return 1;` | A spatial dispatch was requested; this Boolean result does not independently confirm SDK voice playback. |
| `audio_set_listener_transform` | 246 | `0` | `if (position_or_null != 0) {` | A nonnull optional listener-position pointer requests a transform update. |
| `audio_set_listener_transform` | 249 | `0` | `if (rotation_or_null != 0) {` | A nonnull optional listener-rotation pointer requests a transform update. |
| `audio_play_voice` | 270 | `0` | `if (program == 0 && tone == 0 && note == 0) {` | The all-zero sound triple suppresses voice dispatch. Individual zero program/tone/note values remain valid when the whole triple is not zero. |
| `audio_play_voice` | 270 | `0` | `if (program == 0 && tone == 0 && note == 0) {` | The all-zero sound triple suppresses voice dispatch. Individual zero program/tone/note values remain valid when the whole triple is not zero. |
| `audio_play_voice` | 270 | `0` | `if (program == 0 && tone == 0 && note == 0) {` | The all-zero sound triple suppresses voice dispatch. Individual zero program/tone/note values remain valid when the whole triple is not zero. |
| `audio_play_voice` | 278 | `0` | `audio_voice_slot_index = 0;` | Wrap the reusable voice-slot index to the first zero-based entry after reaching the named capacity. |
| `audio_play_voice` | 293 | `0` | `SsUtKeyOn(vab_id, program, tone, note, 0, left_volume, right_volume);` | Zero fine pitch in the authentic SDK key-on interface; the ordinary note and channel amplitudes remain separate arguments. |
| `angle_shortest_delta` | 309 | `1` | `if (signed_difference < -KF_ANGLE_HALF_TURN + 1) {` | Preserve the asymmetric inclusive negative-half-turn endpoint with its explicit one-unit adjustment. |

## `src/open/audio.c`

23 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `audio_initialize` | 36 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_initialize` | 36 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_initialize` | 43 | `1` | `index = KF_AUDIO_VOICE_SLOTS - 1;` | Start at the last zero-based reusable voice slot; the named capacity supplies the count. |
| `audio_initialize` | 46 | `0` | `} while (--index >= 0);` | Include slot zero before exhausting the signed voice-slot countdown. |
| `audio_load_vab` | 57 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_load_vab` | 57 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_load_vab` | 61 | `0` | `exit(0);` | The retail OPEN error path terminates with process status zero even after VAB failure; preserve this API argument rather than correcting its error policy. |
| `audio_load_vab` | 68 | `0` | `exit(0);` | The retail OPEN error path terminates with process status zero even after VAB failure; preserve this API argument rather than correcting its error policy. |
| `audio_load_vab` | 71 | `1` | `frame = OPEN_VAB_SETTLE_FRAMES - 1;` | Initialize the settle-loop countdown to the last zero-based frame; the named setting supplies the total wait. |
| `audio_load_vab` | 73 | `0` | `VSync(0);` | The SDK zero mode waits for the next vertical sync; it is the ordinary synchronization call argument. |
| `audio_load_vab` | 74 | `1` | `} while (--frame != -1);` | Stop after all configured settle frames; minus one is signed countdown exhaustion, not an audio handle or state. |
| `audio_play_sequence_file` | 83 | `0` | `if (cd_file_load_into(audio_state.sequence_buffer, path) != 0) {` | A nonzero CD-wrapper result exits before issuing playback; this is an API return-code test, not sequence state. |
| `audio_play_sequence_file` | 89 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_play_sequence_file` | 89 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_play_sequence_file` | 91 | `0` | `volume = 0;` | Begin the numeric master-volume fade-in at silence. |
| `audio_play_sequence_file` | 93 | `0` | `VSync(0);` | The SDK zero mode waits for the next vertical sync; it is the ordinary synchronization call argument. |
| `audio_stop_sequence` | 110 | `0` | `VSync(0);` | The SDK zero mode waits for the next vertical sync; it is the ordinary synchronization call argument. |
| `audio_stop_sequence` | 113 | `0` | `} while (volume >= 0);` | The OPEN master fade emits nonnegative volume steps and exits after crossing zero; explicit zero-volume calls follow. |
| `audio_stop_sequence` | 115 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_stop_sequence` | 115 | `0` | `SsSetMVol(0, 0);` | Zero left and right master amplitudes silence the output; these are numeric volume endpoints, not stop-mode values. |
| `audio_stop_sequence` | 116 | `0` | `SsSeqSetVol(audio_state.sequence_id, 0, 0);` | Set both numeric sequence-channel amplitudes to silence before stop/close. |
| `audio_stop_sequence` | 116 | `0` | `SsSeqSetVol(audio_state.sequence_id, 0, 0);` | Set both numeric sequence-channel amplitudes to silence before stop/close. |
| `audio_close_vab` | 140 | `0` | `audio_state.vab_header = 0;` | Clear the borrowed VAB-header pointer after closing its handle. |

## `src/open/resources.c`

31 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 24 | `5` | `char cd_path_prefix[5] = "\\KF\\";` | Four encoded prefix characters plus the terminating NUL occupy five bytes; this is the authored string object extent. |
| `initializers` | 26 | `3` | `char cd_version_suffix[3] = ";1";` | The two-character CD version suffix plus its terminating NUL occupies three bytes. |
| `initializers` | 28 | `8` | `char opening_ending_sequence_path[8] = "B0\\END.";` | Seven encoded path characters plus the terminating NUL occupy eight bytes. |
| `cd_file_load_allocated` | 57 | `0` | `if (CdSearchFile(&cd_search_file, path) == 0) {` | The SDK null/zero search result means the file lookup failed; return before allocation/read setup. |
| `cd_file_load_allocated` | 58 | `1` | `return 1;` | The wrapper reports file-lookup failure with one before starting reads. |
| `cd_file_load_allocated` | 60 | `1` | `if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {` | Sector size minus one forms its low-bit remainder mask; nonzero remainder requires rounding the byte count upward. |
| `cd_file_load_allocated` | 60 | `0` | `if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {` | Sector size minus one forms its low-bit remainder mask; nonzero remainder requires rounding the byte count upward. |
| `cd_file_load_allocated` | 62 | `1` | `((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;` | Add one whole sector only when a partial sector remains; preserve the existing ceiling-to-sector arithmetic. |
| `cd_file_load_allocated` | 68 | `0` | `for (attempt = 0; attempt < OPEN_CD_READ_ATTEMPTS; attempt++) {` | Start the ordinary retry index at zero and visit the named maximum number of attempts. |
| `cd_file_load_allocated` | 71 | `0` | `CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);` | Null optional SDK result buffer for the set-location command. |
| `cd_file_load_allocated` | 76 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer; a positive returned count keeps polling. Preserve zero completion versus negative failure. |
| `cd_file_load_allocated` | 76 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer; a positive returned count keeps polling. Preserve zero completion versus negative failure. |
| `cd_file_load_allocated` | 78 | `0` | `if (result == 0) {` | Exactly zero ends retries through the named stop-attempt sentinel; a negative read result does not take this path. |
| `cd_file_load_allocated` | 82 | `0` | `return 0;` | The wrapper returns zero after the read-attempt loop, including exhaustion; this does not prove that the last read succeeded. |
| `cd_file_load_into` | 96 | `0` | `if (CdSearchFile(&cd_search_file, path) == 0) {` | The SDK null/zero search result means the file lookup failed; return before allocation/read setup. |
| `cd_file_load_into` | 97 | `1` | `return 1;` | The wrapper reports file-lookup failure with one before starting reads. |
| `cd_file_load_into` | 99 | `1` | `if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {` | Sector size minus one forms its low-bit remainder mask; nonzero remainder requires rounding the byte count upward. |
| `cd_file_load_into` | 99 | `0` | `if ((cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) != 0) {` | Sector size minus one forms its low-bit remainder mask; nonzero remainder requires rounding the byte count upward. |
| `cd_file_load_into` | 101 | `1` | `((cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;` | Add one whole sector only when a partial sector remains; preserve the existing ceiling-to-sector arithmetic. |
| `cd_file_load_into` | 106 | `0` | `for (attempt = 0; attempt < OPEN_CD_READ_ATTEMPTS; attempt++) {` | Start the ordinary retry index at zero and visit the named maximum number of attempts. |
| `cd_file_load_into` | 109 | `0` | `CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);` | Null optional SDK result buffer for the set-location command. |
| `cd_file_load_into` | 114 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer; a positive returned count keeps polling. Preserve zero completion versus negative failure. |
| `cd_file_load_into` | 114 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer; a positive returned count keeps polling. Preserve zero completion versus negative failure. |
| `cd_file_load_into` | 116 | `0` | `if (result == 0) {` | Exactly zero ends retries through the named stop-attempt sentinel; a negative read result does not take this path. |
| `cd_file_load_into` | 120 | `0` | `return 0;` | The wrapper returns zero after the read-attempt loop, including exhaustion; this does not prove that the last read succeeded. |
| `tim_upload_images` | 130 | `0` | `while (ReadTIM(&image) != 0) {` | The SDK nonzero iteration result supplies another TIM image descriptor. |
| `tim_upload_images` | 131 | `0` | `if (image.caddr != 0) {` | A nonnull TIM CLUT payload enables palette upload. |
| `tim_upload_images` | 133 | `0` | `DrawSync(0);` | The SDK zero synchronization mode waits for GPU transfer completion. |
| `tim_upload_images` | 135 | `0` | `if (image.paddr != 0) {` | A nonnull TIM pixel payload enables image upload. |
| `tim_upload_images` | 137 | `0` | `DrawSync(0);` | The SDK zero synchronization mode waits for GPU transfer completion. |
| `resource_stream_copy_words` | 147 | `0` | `while (word_count-- != 0) {` | Exhaust the caller-supplied word count with the existing post-decrement copy loop. |

## `src/open/opening_controller.c`

7 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 37 | `6` | `char opening_initial_tim_path[6] = "B0\\L0.";` | The retail six-byte path object holds the six authored characters without an in-object NUL; preserve its exact extent and existing adjacent-storage behavior. |
| `opening_run` | 49 | `0` | `PadInit(0);` | The original zero SDK initialization argument selects the normal pad initialization path. |
| `opening_run` | 51 | `0` | `memset(&open_graphics_runtime, 0, sizeof open_graphics_runtime);` | Fill the complete graphics/runtime aggregate with zero bytes before initialization. |
| `opening_run` | 52 | `0` | `memset(&opening_entity_state, 0, sizeof opening_entity_state);` | Zero-fill the opening entity-state aggregate before its pool initialization. |
| `opening_run` | 63 | `1` | `SetDispMask(1);` | The SDK Boolean enable value turns display output on for the intro. |
| `opening_run` | 66 | `0` | `opening_initial_tim_path) != 0) {` | A nonzero resource-loader result returns from the opening controller before TIM upload. |
| `opening_run` | 89 | `0` | `allocation_state->stack[KF_MEMORY_STACK_DEPTH_INDEX] = 0;` | Reset the allocation stack depth to the empty count before rebasing the arena cursor. |
