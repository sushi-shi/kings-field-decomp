# Resource loading and remaining OPEN audio literal ledger

Complete current accounting for five modules: 60 retained numeric/character
occurrences, including authored initializers. See the [evidence review](resource-audio-coverage.md).
Each duplicate token has its own row. Claims, definitions and string contents
are excluded. Coverage records why literals remain; outstanding selector type
propagation is identified separately in the review.

## `src/game/cd_file.c`

35 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 12 | `5` | `char cd_path_prefix[5] = "\\KF\\";` | Five bytes hold the four-character directory prefix and its terminating NUL; the local extent describes this literal string. |
| `initializers` | 14 | `3` | `char cd_version_suffix[3] = ";1";` | Three bytes hold the two-character CD file-version suffix and its terminating NUL. |
| `cd_file_load_allocated` | 29 | `0` | `if (CdSearchFile(&cd_search_file, path) == 0) {` | Null SDK search result enters the error-screen path; this is a pointer/API result boundary. |
| `cd_file_load_allocated` | 32 | `1` | `if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {` | Sector size minus one is the remainder mask for the named power-of-two sector size. |
| `cd_file_load_allocated` | 33 | `1` | `loaded = (cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1;` | Round a nonzero partial sector upward by one whole sector; loaded temporarily holds this numeric count before completion bookkeeping. |
| `cd_file_load_allocated` | 37 | `0` | `loaded = 0;` | Initialize the local completion flag to false after allocation/rounding; no successful read has yet been observed. |
| `cd_file_load_allocated` | 41 | `0` | `for (attempt = 0; attempt < CD_PATH_READ_ATTEMPTS; attempt++) {` | Start the retry index at zero; the separately named limit controls the number of attempts. |
| `cd_file_load_allocated` | 44 | `0` | `CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);` | Null optional SDK response buffer; the preceding named command sets the read location. |
| `cd_file_load_allocated` | 46 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer and positive remaining-sector predicate. Zero completion and negative failure both end polling but have different retry consequences. |
| `cd_file_load_allocated` | 46 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer and positive remaining-sector predicate. Zero completion and negative failure both end polling but have different retry consequences. |
| `cd_file_load_allocated` | 48 | `0` | `if (result == 0) {` | Exactly zero from the SDK poll means completion; a negative error keeps the retry path available. |
| `cd_file_load_allocated` | 50 | `1` | `loaded = 1;` | Set the ordinary local completion flag to true only on the exact-zero SDK result. |
| `cd_file_load_allocated` | 53 | `0` | `if (loaded == 0) {` | False local completion flag selects the read-failure screen after retries; it is not the SDK result code. |
| `cd_file_load_allocated` | 56 | `0` | `return 0;` | The wrapper returns zero after its read/error-screen path; it does not propagate negative SDK read errors. Error-screen return behavior remains separate. |
| `cd_file_load_table_entry` | 67 | `0` | `loaded = 0;` | Initialize the local completion flag to false after allocation/rounding; no successful read has yet been observed. |
| `cd_file_load_table_entry` | 71 | `0` | `for (attempt = 0; attempt < CD_TABLE_READ_ATTEMPTS; attempt++) {` | Start the indexed-file retry count at zero; this path has its own named retry limit. |
| `cd_file_load_table_entry` | 74 | `0` | `CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);` | Null optional SDK response buffer; the preceding named command sets the read location. |
| `cd_file_load_table_entry` | 76 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer and positive remaining-sector predicate. Zero completion and negative failure both end polling but have different retry consequences. |
| `cd_file_load_table_entry` | 76 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer and positive remaining-sector predicate. Zero completion and negative failure both end polling but have different retry consequences. |
| `cd_file_load_table_entry` | 78 | `0` | `if (result == 0) {` | Exactly zero from the SDK poll means completion; a negative error keeps the retry path available. |
| `cd_file_load_table_entry` | 80 | `1` | `loaded = 1;` | Set the ordinary local completion flag to true only on the exact-zero SDK result. |
| `cd_file_load_table_entry` | 83 | `0` | `if (loaded == 0) {` | False local completion flag selects the read-failure screen after retries; it is not the SDK result code. |
| `cd_file_load_table_entry` | 86 | `0` | `return 0;` | The wrapper returns zero after its read/error-screen path; it does not propagate negative SDK read errors. Error-screen return behavior remains separate. |
| `cd_file_load_into` | 100 | `0` | `if (CdSearchFile(&cd_search_file, path) == 0) {` | Null SDK search result enters the error-screen path; this is a pointer/API result boundary. |
| `cd_file_load_into` | 103 | `1` | `if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {` | Sector size minus one is the remainder mask for the named power-of-two sector size. |
| `cd_file_load_into` | 104 | `1` | `loaded = (cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1;` | Round a nonzero partial sector upward by one whole sector; loaded temporarily holds this numeric count before completion bookkeeping. |
| `cd_file_load_into` | 107 | `0` | `loaded = 0;` | Initialize the local completion flag to false after allocation/rounding; no successful read has yet been observed. |
| `cd_file_load_into` | 111 | `0` | `for (attempt = 0; attempt < CD_PATH_READ_ATTEMPTS; attempt++) {` | Start the retry index at zero; the separately named limit controls the number of attempts. |
| `cd_file_load_into` | 114 | `0` | `CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);` | Null optional SDK response buffer; the preceding named command sets the read location. |
| `cd_file_load_into` | 116 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer and positive remaining-sector predicate. Zero completion and negative failure both end polling but have different retry consequences. |
| `cd_file_load_into` | 116 | `0` | `while ((result = CdReadSync(KF_CD_READ_POLL, 0)) > 0) {` | Null optional result buffer and positive remaining-sector predicate. Zero completion and negative failure both end polling but have different retry consequences. |
| `cd_file_load_into` | 118 | `0` | `if (result == 0) {` | Exactly zero from the SDK poll means completion; a negative error keeps the retry path available. |
| `cd_file_load_into` | 120 | `1` | `loaded = 1;` | Set the ordinary local completion flag to true only on the exact-zero SDK result. |
| `cd_file_load_into` | 123 | `0` | `if (loaded == 0) {` | False local completion flag selects the read-failure screen after retries; it is not the SDK result code. |
| `cd_file_load_into` | 126 | `0` | `return 0;` | The wrapper returns zero after its read/error-screen path; it does not propagate negative SDK read errors. Error-screen return behavior remains separate. |

## `src/game/resources.c`

14 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 21 | `8` | `char map_mix_tim_filename[8] = "MIX.TIM";` | Eight bytes hold the seven-character TIM filename and its terminating NUL. |
| `tim_upload_images` | 44 | `0` | `while (ReadTIM(&image) != 0) {` | A nonzero SDK iterator result supplies another TIM descriptor. |
| `tim_upload_images` | 45 | `0` | `if (image.caddr != 0) {` | A nonnull CLUT payload enables palette upload. |
| `tim_upload_images` | 47 | `0` | `DrawSync(0);` | The SDK zero mode waits for GPU work to finish after the upload. |
| `tim_upload_images` | 49 | `0` | `if (image.paddr != 0) {` | A nonnull pixel payload enables image upload. |
| `tim_upload_images` | 51 | `0` | `DrawSync(0);` | The SDK zero mode waits for GPU work to finish after the upload. |
| `map_resource_path_set_floor` | 90 | `1` | `map_resource_path[1] = KF_ENUM_ENCODE(s32, floor) + '0';` | Byte one is the floor digit in B0\<filename>; ASCII zero encodes the explicitly converted floor value at the text boundary. |
| `map_resource_path_set_floor` | 90 | `'0'` | `map_resource_path[1] = KF_ENUM_ENCODE(s32, floor) + '0';` | Byte one is the floor digit in B0\<filename>; ASCII zero encodes the explicitly converted floor value at the text boundary. |
| `map_resource_load_file` | 98 | `3` | `strcpy(&map_resource_path[3], filename);` | Byte three is the first filename character after the B0\ prefix; this is a local string-layout offset. |
| `map_resource_copy_words` | 109 | `0` | `while (word_count-- != 0) {` | Copy while the pre-decrement unsigned count is nonzero; preserve the post-decrement expression and advanced source return. |
| `map_variant_assets_load` | 121 | `3` | `memcpy(&map_resource_path[3], "CHR0.MIM", sizeof "CHR0.MIM");` | Replace the filename starting after B0\; the literal offset belongs to the adjacent template, not an asset registry ID. |
| `map_variant_assets_load` | 122 | `6` | `map_resource_path[6] = player_state.map_variant + '0';` | Byte six is the CHR filename digit in B0\CHR0.MIM; ASCII zero formats the numeric variant at this text boundary. |
| `map_variant_assets_load` | 122 | `'0'` | `map_resource_path[6] = player_state.map_variant + '0';` | Byte six is the CHR filename digit in B0\CHR0.MIM; ASCII zero formats the numeric variant at this text boundary. |
| `map_resources_load` | 203 | `0` | `if (use_variant == 0) {` | The exact-zero Boolean selector uses the embedded actor archive; any nonzero value allocates the variant buffer and loads CHR assets. A closed enum would need caller-wide propagation. |

## `src/open/audio_spatial.c`

6 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `audio_play_spatial` | 32 | `0` | `return 0;` | Outside the strict distance cutoff, no voice request is dispatched; return false. |
| `audio_play_spatial` | 44 | `1` | `angle >>= 1;` | Halve the folded half-turn bearing into a quarter turn for the sine/cosine pan pair; one is the divide-by-two shift. |
| `audio_play_spatial` | 59 | `1` | `return 1;` | Return true after dispatching to the void voice helper, even if it suppresses the all-zero tuple; this does not certify SDK voice allocation or audible playback. |
| `audio_set_listener_transform` | 100 | `0` | `if (position_or_null != 0) {` | A nonnull optional position supplies a replacement listener position. |
| `audio_set_listener_transform` | 103 | `0` | `if (rotation_or_null != 0) {` | A nonnull optional rotation supplies a replacement listener rotation. |

## `src/open/audio_play_voice.c`

6 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `audio_play_voice` | 10 | `0` | `if (program == 0 && tone == 0 && note == 0) {` | Only the complete all-zero sound tuple is suppressed. Zero in an individual SDK program, tone or note selector is not independently a no-sound enum value. |
| `audio_play_voice` | 10 | `0` | `if (program == 0 && tone == 0 && note == 0) {` | Only the complete all-zero sound tuple is suppressed. Zero in an individual SDK program, tone or note selector is not independently a no-sound enum value. |
| `audio_play_voice` | 10 | `0` | `if (program == 0 && tone == 0 && note == 0) {` | Only the complete all-zero sound tuple is suppressed. Zero in an individual SDK program, tone or note selector is not independently a no-sound enum value. |
| `audio_play_voice` | 15 | `0` | `audio_voice_slot_index = 0;` | Wrap the incremented round-robin index to the first zero-based voice slot after reaching the named capacity. |
| `audio_play_voice` | 30 | `0` | `SsUtKeyOn(vab_id, program, tone, note, 0, left_volume, right_volume);` | Zero fine-pitch offset is passed separately from the selected note and numeric channel amplitudes. |
| `initializers` | 34 | `1` | `s32 audio_voice_slot_index = KF_AUDIO_VOICE_SLOTS - 1;` | Initialize to the last slot so the first pre-increment wraps to slot zero; one converts capacity to its last index. |

## `src/open/sound_ref.c`

0 retained occurrences.

The single forwarding function contains no retained numeric or character tokens.
