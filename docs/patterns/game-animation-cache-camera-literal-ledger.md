# Retained animation-cache and camera literals

Complete current ledger for the [cache/camera review](game-animation-cache-camera-coverage.md).
Named definitions and ADDRESS/DATA/RODATA claims are excluded. Every row represents one token, including repeated tokens on a line.

## `src/game/asset_registry.c`

1 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `asset_registry_load_tmd_archive` | 16 | `0` | `while (count-- != 0) {` | Archive countdown exhaustion; the archive header supplies the count and the named header extent supplies its byte stride. |

## `src/game/pool.c`

35 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 16 | `1` | `u32 keyframes[1];   /* +4: byte offsets to KfAnimKeyframe (from asset base) */` | Legacy C variable-tail declaration; the serialized header supplies the actual count. One is not a one-element resource limit. |
| `initializers` | 25 | `1` | `u16 morph_indices[1]; /* +8: object-table indices of the morph targets */` | Legacy C variable-tail declaration; the serialized header supplies the actual count. One is not a one-element resource limit. |
| `initializers` | 33 | `1` | `SVECTOR deltas[1];    /* +12: signed vertex deltas */` | Legacy C variable-tail declaration; the serialized header supplies the actual count. One is not a one-element resource limit. |
| `copy_vertices` | 54 | `0` | `} while (--count != 0);` | Vertex-copy countdown exhaustion. Each iteration copies the two words of one SDK SVECTOR; retain the existing do/while behavior. |
| `render_bind_animated_instance` | 77 | `0` | `if (asset_header->animation_clip_count == 0) {` | An empty clip count selects static rendering. Zero is a count boundary, distinct from the named static-success pointer sentinel. |
| `render_bind_animated_instance` | 78 | `0` | `if (record != 0) {` | A nonnull owner-slot pointer refers to an existing cache record for reuse or release; this is not the record lifecycle state. |
| `render_bind_animated_instance` | 82 | `0` | `tmd_select_object_vertices(0);` | Select the first TMD subobject. Retail does not consume the morph header TMD selector here; do not invent a model identity for this ordinary subobject index. |
| `render_bind_animated_instance` | 86 | `0` | `if (record != 0) {` | A nonnull owner-slot pointer refers to an existing cache record for reuse or release; this is not the record lifecycle state. |
| `render_bind_animated_instance` | 90 | `0` | `if (record == 0) {` | Null allocation result means that no free cache record was found. |
| `render_bind_animated_instance` | 91 | `0` | `return (u16 *)0;` | Null result on cache exhaustion; the separate static-success sentinel and live record pointer retain their distinct meanings. |
| `render_bind_animated_instance` | 99 | `0` | `if (record->cached_vertices == 0) {` | Null vertex allocation triggers release-all and retry; preserve the existing failure path. |
| `render_bind_animated_instance` | 115 | `0` | `phase_end = 0;` | Begin accumulated keyframe duration at the origin of the clip phase interval. |
| `render_bind_animated_instance` | 116 | `0` | `phase_start = 0;` | The first keyframe interval begins at clip phase zero. |
| `render_bind_animated_instance` | 123 | `0` | `while (keyframes_left-- != 0) {` | Stored keyframe-count exhaustion with the existing post-decrement and halfword wrapping behavior. |
| `render_bind_animated_instance` | 132 | `0` | `if (keyframe->reverse != 0) {` | Unsigned Boolean reversal flag: zero keeps the forward fraction, every nonzero halfword complements it. All 813 shipped references contain zero; no distinct authored nonzero enum member is established. |
| `render_bind_animated_instance` | 151 | `0` | `tmd_select_object_vertices(0);` | Select the first TMD subobject. Retail does not consume the morph header TMD selector here; do not invent a model identity for this ordinary subobject index. |
| `render_bind_animated_instance` | 159 | `0` | `while (morphs_left-- != 0) {` | Stored morph-count exhaustion; zero is an ordinary element-count boundary. |
| `render_bind_animated_instance` | 175 | `1` | `copy_vertices(&((SVECTOR *)(game_graphics_runtime.unknown_projection_morph_20318 + MORPH_SCRATCH_OFFSET_IN_PROJECTION_STORAGE))[1], record->cached_vertices, vertex_count);` | Published vertices start after the one leading SDK vector temporarily used by the rest-morph header blend. |
| `render_bind_animated_instance` | 180 | `0` | `u32 saved_xy_word = ((u32 *)scratch_vertex)[0];` | First 32-bit lane of one SDK vector holds X/Y; save it before the temporary header blend. |
| `render_bind_animated_instance` | 181 | `1` | `u32 saved_z_pad_word = ((u32 *)scratch_vertex)[1];` | Second 32-bit lane holds Z and the SDK alignment halfword; save it before the temporary header blend. |
| `render_bind_animated_instance` | 185 | `1` | `morph_object->vertex_count + 1, blend_fraction);` | Blend the one extra header-sized vector as well as the stored vertex count, then restore the extra scratch entry. |
| `render_bind_animated_instance` | 186 | `0` | `((u32 *)scratch_vertex)[0] = saved_xy_word;` | Restore the first X/Y word after the blend; zero is the ordinary vector-lane index. |
| `render_bind_animated_instance` | 187 | `1` | `((u32 *)scratch_vertex)[1] = saved_z_pad_word;` | Restore the second Z/alignment word after the blend; one is the ordinary vector-lane index. |
| `render_bind_animated_instance` | 189 | `1` | `tmd_set_current_vertices(&((SVECTOR *)(game_graphics_runtime.unknown_projection_morph_20318 + MORPH_SCRATCH_OFFSET_IN_PROJECTION_STORAGE))[1]);` | Publish vertices after the extra leading vector; the same one-vector displacement is used by the initial scratch copy. |
| `pool_reset` | 202 | `0` | `record->cached_vertices = 0;` | Clear an owned vertex pointer after reset or release; zero denotes null rather than a cache state. |
| `pool_reset` | 204 | `0` | `} while (--records_left != 0);` | Exhaust the named twelve-record capacity with the existing pre-decrement loop. |
| `pool_mark_allocated` | 218 | `0` | `} while (--records_left != 0);` | Exhaust the named twelve-record capacity with the existing pre-decrement loop. |
| `pool_record_release` | 225 | `0` | `*record->owner_slot = 0;` | Clear the owning instance cache pointer on release. |
| `pool_record_release` | 226 | `0` | `if (record->cached_vertices != 0) {` | Free only a nonnull owned vertex allocation. |
| `pool_record_release` | 228 | `0` | `record->cached_vertices = 0;` | Clear an owned vertex pointer after reset or release; zero denotes null rather than a cache state. |
| `pool_release_all` | 243 | `1` | `for (records_left = KF_ANIMATION_CACHE_CAPACITY - 1; records_left != -1; records_left--) {` | Capacity minus one initializes the signed countdown; minus one terminates after all twelve records. These are loop bounds, not lifecycle values. |
| `pool_release_all` | 243 | `1` | `for (records_left = KF_ANIMATION_CACHE_CAPACITY - 1; records_left != -1; records_left--) {` | Capacity minus one initializes the signed countdown; minus one terminates after all twelve records. These are loop bounds, not lifecycle values. |
| `pool_release_stale` | 267 | `0` | `} while (--records_left != 0);` | Exhaust the named twelve-record capacity with the existing pre-decrement loop. |
| `pool_allocate` | 282 | `0` | `} while (--records_left != 0);` | Exhaust the named twelve-record capacity with the existing pre-decrement loop. |
| `pool_allocate` | 283 | `0` | `return 0;` | Return a null record pointer when the entire pool scan finds no free record. |

## `src/game/camera_path.c`

1 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `camera_path_begin` | 51 | `0` | `path->point_index = 0;` | GAME starts the ordinary point index at zero; its segment routine selects point zero before incrementing, from the current player-camera transform. |

## `src/open/camera_path.c`

3 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `opening_camera_path_begin` | 58 | `0` | `opening_camera_path_state.position = points[0].position;` | OPEN takes its initial position from the first serialized point; its segment routine pre-increments to point one for the first target. |
| `opening_camera_path_begin` | 59 | `0` | `opening_camera_path_state.rotation = points[0].rotation;` | OPEN takes its initial rotation from the same first serialized point as its position. |
| `opening_camera_path_begin` | 60 | `0` | `opening_camera_path_state.point_index = 0;` | Initialize the ordinary point index before the OPEN segment routine advances it to the next target; zero is not a path-state enum. |
