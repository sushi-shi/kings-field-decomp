# Retained header literals

Current accounting covers **53 headers / 169 numeric or character occurrences**.
The lexer excludes comments, strings, identifier digits, named enum/macro
definitions and address claims, as in the [C source census](source-literal-coverage.md).
Each row below records one token and its complete expression; repeated tokens
have repeated rows. Headers with zero retained tokens are listed explicitly.

The final bound review names actor attachments, both shared resource paths and
the floor-five actor texture-pair capacity. Ordinary layout witnesses remain
literal so that checks can independently detect drift in modeled declarations.
Opaque byte extents preserve known offsets without inventing field semantics.
This accounts for literals; it does not resolve opaque fields or establish
binary equivalence. Builds, compiler checks and tests were deferred as requested.

## Header coverage

| Header | Retained occurrences |
| --- | ---: |
| [include/kf/address.h](../../include/kf/address.h) | 0 |
| [include/kf/audio.h](../../include/kf/audio.h) | 2 |
| [include/kf/cd_file.h](../../include/kf/cd_file.h) | 0 |
| [include/kf/debug.h](../../include/kf/debug.h) | 0 |
| [include/kf/enum.h](../../include/kf/enum.h) | 1 |
| [include/kf/floor.h](../../include/kf/floor.h) | 0 |
| [include/kf/game.h](../../include/kf/game.h) | 0 |
| [include/kf/game_actor.h](../../include/kf/game_actor.h) | 11 |
| [include/kf/game_asset.h](../../include/kf/game_asset.h) | 0 |
| [include/kf/game_cd.h](../../include/kf/game_cd.h) | 0 |
| [include/kf/game_collision.h](../../include/kf/game_collision.h) | 1 |
| [include/kf/game_effect.h](../../include/kf/game_effect.h) | 16 |
| [include/kf/game_equipment.h](../../include/kf/game_equipment.h) | 4 |
| [include/kf/game_graphics.h](../../include/kf/game_graphics.h) | 48 |
| [include/kf/game_map.h](../../include/kf/game_map.h) | 13 |
| [include/kf/game_math.h](../../include/kf/game_math.h) | 0 |
| [include/kf/game_menu.h](../../include/kf/game_menu.h) | 11 |
| [include/kf/game_player.h](../../include/kf/game_player.h) | 8 |
| [include/kf/game_render.h](../../include/kf/game_render.h) | 3 |
| [include/kf/game_resources.h](../../include/kf/game_resources.h) | 0 |
| [include/kf/game_save.h](../../include/kf/game_save.h) | 17 |
| [include/kf/game_state.h](../../include/kf/game_state.h) | 0 |
| [include/kf/game_system.h](../../include/kf/game_system.h) | 0 |
| [include/kf/game_types.h](../../include/kf/game_types.h) | 0 |
| [include/kf/gpu_packets.h](../../include/kf/gpu_packets.h) | 0 |
| [include/kf/item.h](../../include/kf/item.h) | 2 |
| [include/kf/magic.h](../../include/kf/magic.h) | 1 |
| [include/kf/map_data.h](../../include/kf/map_data.h) | 0 |
| [include/kf/memory.h](../../include/kf/memory.h) | 0 |
| [include/kf/notify.h](../../include/kf/notify.h) | 0 |
| [include/kf/notify_types.h](../../include/kf/notify_types.h) | 0 |
| [include/kf/open_audio.h](../../include/kf/open_audio.h) | 0 |
| [include/kf/open_camera_path.h](../../include/kf/open_camera_path.h) | 0 |
| [include/kf/open_controller.h](../../include/kf/open_controller.h) | 0 |
| [include/kf/open_opening_helpers.h](../../include/kf/open_opening_helpers.h) | 0 |
| [include/kf/open_opening_render.h](../../include/kf/open_opening_render.h) | 0 |
| [include/kf/open_opening_scenes.h](../../include/kf/open_opening_scenes.h) | 0 |
| [include/kf/open_render.h](../../include/kf/open_render.h) | 27 |
| [include/kf/open_resources.h](../../include/kf/open_resources.h) | 2 |
| [include/kf/open_scene0.h](../../include/kf/open_scene0.h) | 0 |
| [include/kf/overlay.h](../../include/kf/overlay.h) | 0 |
| [include/kf/pool.h](../../include/kf/pool.h) | 0 |
| [vendor/include/psyq/sdk.h](../../vendor/include/psyq/sdk.h) | 0 |
| [vendor/include/psyq/audio.h](../../vendor/include/psyq/audio.h) | 0 |
| [vendor/include/psyq/cd.h](../../vendor/include/psyq/cd.h) | 0 |
| [vendor/include/psyq/kernel.h](../../vendor/include/psyq/kernel.h) | 0 |
| [vendor/include/psyq/libc.h](../../vendor/include/psyq/libc.h) | 0 |
| [vendor/include/psyq/pad.h](../../vendor/include/psyq/pad.h) | 0 |
| [include/kf/render_types.h](../../include/kf/render_types.h) | 0 |
| [include/kf/resources.h](../../include/kf/resources.h) | 0 |
| [include/kf/tmd.h](../../include/kf/tmd.h) | 2 |
| [vendor/include/sys/fcntl.h](../../vendor/include/sys/fcntl.h) | 0 |
| [vendor/include/sys/types.h](../../vendor/include/sys/types.h) | 0 |

## `include/kf/audio.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 58 | `2` | `u8 unknown_06[2];` | Byte extent of the explicitly opaque `unknown_06` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 61 | `2` | `u8 unknown_0e[2];` | Byte extent of the explicitly opaque `unknown_0e` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/enum.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 5 | `202002L` | `#if defined(__cplusplus) && __cplusplus >= 202002L` | Standard C++20 feature-test value supplied by the compiler language-version contract; the adjacent comparison states its role directly. |

## `include/kf/game_actor.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 205 | `2` | `u8 unknown_38[2];` | Byte extent of the explicitly opaque `unknown_38` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 236 | `3` | `u8 unknown_07[3];` | Byte extent of the explicitly opaque `unknown_07` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 239 | `2` | `u8 unknown_0e[2];` | Byte extent of the explicitly opaque `unknown_0e` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 255 | `2` | `u8 unknown_0c[2];` | Byte extent of the explicitly opaque `unknown_0c` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 275 | `2` | `u8 unknown_46[2];` | Byte extent of the explicitly opaque `unknown_46` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 279 | `0` | `static_assert(__builtin_offsetof(KfActorPlacement, slot_state) == 0);` | Independent byte-offset witness for `slot_state`; zero denotes the object origin where present. |
| 280 | `0` | `static_assert(__builtin_offsetof(KfActor, slot_state) == 0);` | Independent byte-offset witness for `slot_state`; zero denotes the object origin where present. |
| 281 | `0x06` | `static_assert(__builtin_offsetof(KfActor, lifecycle) == 0x06);` | Independent byte-offset witness for `lifecycle`; zero denotes the object origin where present. |
| 282 | `0x08` | `static_assert(__builtin_offsetof(KfActor, action) == 0x08);` | Independent byte-offset witness for `action`; zero denotes the object origin where present. |
| 283 | `0x0b` | `static_assert(__builtin_offsetof(KfActor, vertical_state) == 0x0b);` | Independent byte-offset witness for `vertical_state`; zero denotes the object origin where present. |
| 284 | `0x39` | `static_assert(__builtin_offsetof(KfActor, collision_state) == 0x39);` | Independent byte-offset witness for `collision_state`; zero denotes the object origin where present. |

## `include/kf/game_collision.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 53 | `0x06` | `u8 unknown_1a[0x06];` | Byte extent of the explicitly opaque `unknown_1a` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/game_effect.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 256 | `0x1c` | `KF_EFFECT_OFFSET_CHECK(rotation, rotation, 0x1c);` | Independent byte-offset witness for `rotation` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 257 | `0x1e` | `KF_EFFECT_OFFSET_CHECK(rotation_y, rotation.vy, 0x1e);` | Independent byte-offset witness for `rotation_y` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 258 | `0x20` | `KF_EFFECT_OFFSET_CHECK(rotation_z, rotation.vz, 0x20);` | Independent byte-offset witness for `rotation_z` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 259 | `0x22` | `KF_EFFECT_OFFSET_CHECK(rotation_pad, rotation.pad, 0x22);` | Independent byte-offset witness for `rotation_pad` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 260 | `0x2c` | `KF_EFFECT_OFFSET_CHECK(direction, direction, 0x2c);` | Independent byte-offset witness for `direction` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 261 | `0x32` | `KF_EFFECT_OFFSET_CHECK(direction_pad, direction.words.pad, 0x32);` | Independent byte-offset witness for `direction_pad` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 262 | `0x08` | `KF_EFFECT_OFFSET_CHECK(visual, visual, 0x08);` | Independent byte-offset witness for `visual` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 263 | `0x38` | `KF_EFFECT_OFFSET_CHECK(control, control, 0x38);` | Independent byte-offset witness for `control` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 264 | `0x39` | `KF_EFFECT_OFFSET_CHECK(control_high_byte, control.bytes.high, 0x39);` | Independent byte-offset witness for `control_high_byte` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 265 | `0x3a` | `KF_EFFECT_OFFSET_CHECK(propagation, propagation, 0x3a);` | Independent byte-offset witness for `propagation` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 286 | `3` | `u8 unknown_05[3];` | Byte extent of the explicitly opaque `unknown_05` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 288 | `2` | `u8 unknown_0a[2];` | Byte extent of the explicitly opaque `unknown_0a` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 294 | `6` | `u8 unknown_16[6];` | Byte extent of the explicitly opaque `unknown_16` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 296 | `2` | `u8 unknown_22[2];` | Byte extent of the explicitly opaque `unknown_22` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 300 | `10` | `u8 unknown_2a[10];` | Byte extent of the explicitly opaque `unknown_2a` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 302 | `4` | `u8 unknown_38[4];` | Byte extent of the explicitly opaque `unknown_38` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/game_equipment.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 61 | `0x0a` | `u8 unknown_12[0x0a];` | Byte extent of the explicitly opaque `unknown_12` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 75 | `0x02` | `u8 unknown_10[0x02];` | Byte extent of the explicitly opaque `unknown_10` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 77 | `0x12` | `u8 unknown_14[0x12];` | Byte extent of the explicitly opaque `unknown_14` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 79 | `0x04` | `u8 unknown_28[0x04];` | Byte extent of the explicitly opaque `unknown_28` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/game_graphics.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 17 | `8` | `u8 unknown_20108[8];` | Byte extent of the explicitly opaque `unknown_20108` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 19 | `0xf0` | `u8 unknown_registry_20134[0xf0];` | Byte extent of the explicitly opaque `unknown_registry_20134` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 22 | `0x3e88` | `u8 unknown_projection_morph_20318[0x3e88];` | Byte extent of the explicitly opaque `unknown_projection_morph_20318` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 24 | `10` | `u8 unknown_241a6[10];` | Byte extent of the explicitly opaque `unknown_241a6` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 26 | `10` | `u8 unknown_241b6[10];` | Byte extent of the explicitly opaque `unknown_241b6` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 46 | `6` | `u8 unknown_241fa[6];` | Byte extent of the explicitly opaque `unknown_241fa` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 60 | `0x0` | `KF_GAME_GRAPHICS_OFFSET_CHECK(display_state, 0x0);` | Independent byte-offset witness for `display_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 61 | `0x20028` | `KF_GAME_GRAPHICS_OFFSET_CHECK(display_draw_environments, 0x20028);` | Independent byte-offset witness for `display_draw_environments` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 62 | `0x200e0` | `KF_GAME_GRAPHICS_OFFSET_CHECK(display_disp_environments, 0x200e0);` | Independent byte-offset witness for `display_disp_environments` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 63 | `0x20108` | `KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_20108, 0x20108);` | Independent byte-offset witness for `unknown_20108` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 64 | `0x20110` | `KF_GAME_GRAPHICS_OFFSET_CHECK(tmd_state, 0x20110);` | Independent byte-offset witness for `tmd_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 65 | `0x20134` | `KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_registry_20134, 0x20134);` | Independent byte-offset witness for `unknown_registry_20134` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 66 | `0x20224` | `KF_GAME_GRAPHICS_OFFSET_CHECK(current_tmd_vertices, 0x20224);` | Independent byte-offset witness for `current_tmd_vertices` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 67 | `0x20228` | `KF_GAME_GRAPHICS_OFFSET_CHECK(pool_records, 0x20228);` | Independent byte-offset witness for `pool_records` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 68 | `0x20318` | `KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_projection_morph_20318, 0x20318);` | Independent byte-offset witness for `unknown_projection_morph_20318` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 69 | `0x241a0` | `KF_GAME_GRAPHICS_OFFSET_CHECK(effect5_texture_pages, 0x241a0);` | Independent byte-offset witness for `effect5_texture_pages` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 70 | `0x241a6` | `KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241a6, 0x241a6);` | Independent byte-offset witness for `unknown_241a6` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 71 | `0x241b0` | `KF_GAME_GRAPHICS_OFFSET_CHECK(effect5_texture_cluts, 0x241b0);` | Independent byte-offset witness for `effect5_texture_cluts` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 72 | `0x241b6` | `KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241b6, 0x241b6);` | Independent byte-offset witness for `unknown_241b6` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 73 | `0x241c0` | `KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_clut, 0x241c0);` | Independent byte-offset witness for `active_render_clut` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 74 | `0x241c2` | `KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_tpage, 0x241c2);` | Independent byte-offset witness for `active_render_tpage` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 75 | `0x241c4` | `KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_red, 0x241c4);` | Independent byte-offset witness for `active_render_red` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 76 | `0x241c5` | `KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_green, 0x241c5);` | Independent byte-offset witness for `active_render_green` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 77 | `0x241c6` | `KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_blue, 0x241c6);` | Independent byte-offset witness for `active_render_blue` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 78 | `0x241c7` | `KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_code, 0x241c7);` | Independent byte-offset witness for `active_render_code` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 79 | `0x241c8` | `KF_GAME_GRAPHICS_OFFSET_CHECK(hud_clut, 0x241c8);` | Independent byte-offset witness for `hud_clut` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 80 | `0x241ca` | `KF_GAME_GRAPHICS_OFFSET_CHECK(hud_tpage, 0x241ca);` | Independent byte-offset witness for `hud_tpage` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 81 | `0x241cc` | `KF_GAME_GRAPHICS_OFFSET_CHECK(hud_brightness, 0x241cc);` | Independent byte-offset witness for `hud_brightness` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 82 | `0x241cd` | `KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241cd, 0x241cd);` | Independent byte-offset witness for `unknown_241cd` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 83 | `0x241ce` | `KF_GAME_GRAPHICS_OFFSET_CHECK(notification_text_clut, 0x241ce);` | Independent byte-offset witness for `notification_text_clut` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 84 | `0x241d0` | `KF_GAME_GRAPHICS_OFFSET_CHECK(notification_text_tpage, 0x241d0);` | Independent byte-offset witness for `notification_text_tpage` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 85 | `0x241d2` | `KF_GAME_GRAPHICS_OFFSET_CHECK(notification_digit_clut, 0x241d2);` | Independent byte-offset witness for `notification_digit_clut` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 86 | `0x241d4` | `KF_GAME_GRAPHICS_OFFSET_CHECK(notification_digit_tpage, 0x241d4);` | Independent byte-offset witness for `notification_digit_tpage` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 87 | `0x241d6` | `KF_GAME_GRAPHICS_OFFSET_CHECK(notification_message_ids, 0x241d6);` | Independent byte-offset witness for `notification_message_ids` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 88 | `0x241de` | `KF_GAME_GRAPHICS_OFFSET_CHECK(notification_state, 0x241de);` | Independent byte-offset witness for `notification_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 89 | `0x241f4` | `KF_GAME_GRAPHICS_OFFSET_CHECK(floor_item_clut, 0x241f4);` | Independent byte-offset witness for `floor_item_clut` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 90 | `0x241f6` | `KF_GAME_GRAPHICS_OFFSET_CHECK(floor_item_tpage, 0x241f6);` | Independent byte-offset witness for `floor_item_tpage` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 91 | `0x241f8` | `KF_GAME_GRAPHICS_OFFSET_CHECK(floor_item_count, 0x241f8);` | Independent byte-offset witness for `floor_item_count` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 92 | `0x241fa` | `KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241fa, 0x241fa);` | Independent byte-offset witness for `unknown_241fa` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 93 | `0x24200` | `KF_GAME_GRAPHICS_OFFSET_CHECK(floor_items, 0x24200);` | Independent byte-offset witness for `floor_items` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 94 | `0x24800` | `KF_GAME_GRAPHICS_OFFSET_CHECK(DAT_80095698, 0x24800);` | Independent byte-offset witness for `DAT_80095698` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 95 | `0x24804` | `KF_GAME_GRAPHICS_OFFSET_CHECK(DAT_8009569c, 0x24804);` | Independent byte-offset witness for `DAT_8009569c` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 96 | `0x24808` | `KF_GAME_GRAPHICS_OFFSET_CHECK(render_state, 0x24808);` | Independent byte-offset witness for `render_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 97 | `0x24948` | `KF_GAME_GRAPHICS_OFFSET_CHECK(light_quadrant_matrices, 0x24948);` | Independent byte-offset witness for `light_quadrant_matrices` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 98 | `0x249c8` | `KF_GAME_GRAPHICS_OFFSET_CHECK(active_cell_window, 0x249c8);` | Independent byte-offset witness for `active_cell_window` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 100 | `0x249cc` | `typedef char check_game_graphics_size[sizeof(KfGraphicsRuntimeGame) == 0x249cc ? 1 : -1];` | Independent total byte-size witness 0x249cc for the graphics owner; +1/-1 are the compile-time assertion success/failure extents. |
| 100 | `1` | `typedef char check_game_graphics_size[sizeof(KfGraphicsRuntimeGame) == 0x249cc ? 1 : -1];` | Independent total byte-size witness 0x249cc for the graphics owner; +1/-1 are the compile-time assertion success/failure extents. |
| 100 | `1` | `typedef char check_game_graphics_size[sizeof(KfGraphicsRuntimeGame) == 0x249cc ? 1 : -1];` | Independent total byte-size witness 0x249cc for the graphics owner; +1/-1 are the compile-time assertion success/failure extents. |

## `include/kf/game_map.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 261 | `2` | `u8 unknown_06[2];` | Byte extent of the explicitly opaque `unknown_06` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 269 | `2` | `u8 unknown_06[2];` | Byte extent of the explicitly opaque `unknown_06` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 273 | `4` | `u8 unknown_14[4];` | Byte extent of the explicitly opaque `unknown_14` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 284 | `0` | `(unsigned long)&((KfMapObject *)0)->link == 0x20 ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 284 | `0x20` | `(unsigned long)&((KfMapObject *)0)->link == 0x20 ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 284 | `1` | `(unsigned long)&((KfMapObject *)0)->link == 0x20 ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 284 | `1` | `(unsigned long)&((KfMapObject *)0)->link == 0x20 ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 286 | `0` | `(unsigned long)&((KfMapObjectPlacement *)0)->link == 0x0c ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 286 | `0x0c` | `(unsigned long)&((KfMapObjectPlacement *)0)->link == 0x0c ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 286 | `1` | `(unsigned long)&((KfMapObjectPlacement *)0)->link == 0x0c ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 286 | `1` | `(unsigned long)&((KfMapObjectPlacement *)0)->link == 0x0c ? 1 : -1];` | Independent link-field offset witness (runtime 0x20, placement 0x0c). Null-pointer member-address syntax measures the offset; +1/-1 are compile-time assertion extents. |
| 411 | `4` | `u8 unknown_30[4];` | Byte extent of the explicitly opaque `unknown_30` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 415 | `2` | `u8 unknown_3a[2];` | Byte extent of the explicitly opaque `unknown_3a` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/game_menu.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 267 | `2` | `typedef char check_menu_glyph_alignment[__alignof__(MenuGlyphString) == 2 ? 1 : -1];` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 267 | `1` | `typedef char check_menu_glyph_alignment[__alignof__(MenuGlyphString) == 2 ? 1 : -1];` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 267 | `1` | `typedef char check_menu_glyph_alignment[__alignof__(MenuGlyphString) == 2 ? 1 : -1];` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 269 | `0` | `(unsigned long)&((MenuGlyphString *)0)->x == 0 &&` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 269 | `0` | `(unsigned long)&((MenuGlyphString *)0)->x == 0 &&` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 270 | `0` | `(unsigned long)&((MenuGlyphString *)0)->y == 2 &&` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 270 | `2` | `(unsigned long)&((MenuGlyphString *)0)->y == 2 &&` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 271 | `0` | `(unsigned long)&((MenuGlyphString *)0)->codes == 4 ? 1 : -1];` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 271 | `4` | `(unsigned long)&((MenuGlyphString *)0)->codes == 4 ? 1 : -1];` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 271 | `1` | `(unsigned long)&((MenuGlyphString *)0)->codes == 4 ? 1 : -1];` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |
| 271 | `1` | `(unsigned long)&((MenuGlyphString *)0)->codes == 4 ? 1 : -1];` | Menu glyph layout witness: halfword alignment 2, x/y/code offsets 0/2/4. Null-pointer member-address syntax measures offsets; +1/-1 are compile-time assertion extents. |

## `include/kf/game_player.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 180 | `2` | `u8 unknown_3a[2];` | Byte extent of the explicitly opaque `unknown_3a` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 193 | `4` | `u8 unknown_54[4];` | Byte extent of the explicitly opaque `unknown_54` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 196 | `3` | `u8 unknown_5d[3];` | Byte extent of the explicitly opaque `unknown_5d` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 199 | `3` | `u8 unknown_65[3];` | Byte extent of the explicitly opaque `unknown_65` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 203 | `2` | `u8 unknown_72[2];` | Byte extent of the explicitly opaque `unknown_72` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 208 | `1` | `u8 unknown_7b[1];` | Byte extent of the explicitly opaque `unknown_7b` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 233 | `6` | `u8 unknown_ce[6];` | Byte extent of the explicitly opaque `unknown_ce` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 240 | `1` | `u8 unknown_df[1];` | Byte extent of the explicitly opaque `unknown_df` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/game_render.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 110 | `2` | `u8 unknown_0c[2];` | Byte extent of the explicitly opaque `unknown_0c` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 112 | `2` | `u8 unknown_16[2];` | Byte extent of the explicitly opaque `unknown_16` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 119 | `3` | `u8 unknown_01[3];` | Byte extent of the explicitly opaque `unknown_01` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/game_save.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 96 | `28` | `u8 zero_pad[28];` | The 28 zero-filled save-header bytes bridge title end 0x44 to CLUT start 0x60; this is a layout gap, not a count of semantic elements. |
| 115 | `0x1c` | `u8 reserved[0x1c];` | The directory entry reserves 0x1c bytes after its decoded fields; preserve the remaining extent without claiming a behavioral role. |
| 131 | `556` | `u8 unknown_0e0[556];` | Byte extent of the explicitly opaque `unknown_0e0` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 135 | `56` | `u8 unknown_2548[56];` | Byte extent of the explicitly opaque `unknown_2548` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 141 | `0x00` | `KF_PSX_SAVE_HEADER_OFFSET_CHECK(magic, 0x00);` | Independent byte-offset witness for `magic` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 142 | `0x02` | `KF_PSX_SAVE_HEADER_OFFSET_CHECK(icon_type, 0x02);` | Independent byte-offset witness for `icon_type` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 143 | `0x03` | `KF_PSX_SAVE_HEADER_OFFSET_CHECK(block_count, 0x03);` | Independent byte-offset witness for `block_count` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 144 | `0x04` | `KF_PSX_SAVE_HEADER_OFFSET_CHECK(title, 0x04);` | Independent byte-offset witness for `title` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 145 | `0x44` | `KF_PSX_SAVE_HEADER_OFFSET_CHECK(zero_pad, 0x44);` | Independent byte-offset witness for `zero_pad` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 146 | `0x60` | `KF_PSX_SAVE_HEADER_OFFSET_CHECK(clut, 0x60);` | Independent byte-offset witness for `clut` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 147 | `0x80` | `KF_PSX_SAVE_HEADER_OFFSET_CHECK(icon_frames, 0x80);` | Independent byte-offset witness for `icon_frames` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 152 | `0x00` | `KF_SAVE_SUMMARY_OFFSET_CHECK(experience, 0x00);` | Independent byte-offset witness for `experience` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 153 | `0x04` | `KF_SAVE_SUMMARY_OFFSET_CHECK(current_floor, 0x04);` | Independent byte-offset witness for `current_floor` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 154 | `0x08` | `KF_SAVE_SUMMARY_OFFSET_CHECK(current_hp, 0x08);` | Independent byte-offset witness for `current_hp` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 155 | `0x0c` | `KF_SAVE_SUMMARY_OFFSET_CHECK(maximum_hp, 0x0c);` | Independent byte-offset witness for `maximum_hp` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 156 | `0x10` | `KF_SAVE_SUMMARY_OFFSET_CHECK(current_mp, 0x10);` | Independent byte-offset witness for `current_mp` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 157 | `0x14` | `KF_SAVE_SUMMARY_OFFSET_CHECK(maximum_mp, 0x14);` | Independent byte-offset witness for `maximum_mp` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |

## `include/kf/item.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 147 | `4` | `u8 unknown_10[4];` | Byte extent of the explicitly opaque `unknown_10` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 149 | `3` | `u8 unknown_15[3];` | Byte extent of the explicitly opaque `unknown_15` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/magic.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 46 | `0x02` | `u8 unknown_12[0x02];` | Byte extent of the explicitly opaque `unknown_12` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/open_render.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 30 | `3` | `u8 unknown_01[3];` | Byte extent of the explicitly opaque `unknown_01` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 65 | `6` | `u8 unknown_08[6];` | Byte extent of the explicitly opaque `unknown_08` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 69 | `4` | `u8 unknown_14[4];` | Byte extent of the explicitly opaque `unknown_14` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 79 | `8` | `u8 unknown_20108[8];` | Byte extent of the explicitly opaque `unknown_20108` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 81 | `4` | `u8 unknown_2011c[4];` | Byte extent of the explicitly opaque `unknown_2011c` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 83 | `0x14` | `u8 unknown_20124[0x14];` | Byte extent of the explicitly opaque `unknown_20124` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 85 | `0x1f68` | `u8 unknown_22078[0x1f68];` | Byte extent of the explicitly opaque `unknown_22078` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 93 | `2` | `u8 unknown_24786[2];` | Byte extent of the explicitly opaque `unknown_24786` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 99 | `0x0` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(display_state, 0x0);` | Independent byte-offset witness for `display_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 100 | `0x20024` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(ordering_table, 0x20024);` | Independent byte-offset witness for `ordering_table` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 101 | `0x20028` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(display_draw_environments, 0x20028);` | Independent byte-offset witness for `display_draw_environments` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 102 | `0x200e0` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(display_disp_environments, 0x200e0);` | Independent byte-offset witness for `display_disp_environments` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 103 | `0x20108` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_20108, 0x20108);` | Independent byte-offset witness for `unknown_20108` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 104 | `0x20110` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(tmd_state, 0x20110);` | Independent byte-offset witness for `tmd_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 105 | `0x2011c` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_2011c, 0x2011c);` | Independent byte-offset witness for `unknown_2011c` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 106 | `0x20120` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(current_tmd_vertices, 0x20120);` | Independent byte-offset witness for `current_tmd_vertices` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 107 | `0x20124` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_20124, 0x20124);` | Independent byte-offset witness for `unknown_20124` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 108 | `0x20138` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(tmd_projected_vertices, 0x20138);` | Independent byte-offset witness for `tmd_projected_vertices` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 109 | `0x22078` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_22078, 0x22078);` | Independent byte-offset witness for `unknown_22078` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 110 | `0x23fe0` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(floor_item_state, 0x23fe0);` | Independent byte-offset witness for `floor_item_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 111 | `0x245f8` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(DAT_8006e040, 0x245f8);` | Independent byte-offset witness for `DAT_8006e040` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 112 | `0x245fc` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(DAT_8006e044, 0x245fc);` | Independent byte-offset witness for `DAT_8006e044` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 113 | `0x24600` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(render_state, 0x24600);` | Independent byte-offset witness for `render_state` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 114 | `0x24700` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(light_quadrant_matrices, 0x24700);` | Independent byte-offset witness for `light_quadrant_matrices` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 115 | `0x24780` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(active_cell_window, 0x24780);` | Independent byte-offset witness for `active_cell_window` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 116 | `0x24784` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(tmd_projection_shift, 0x24784);` | Independent byte-offset witness for `tmd_projection_shift` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |
| 117 | `0x24786` | `KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_24786, 0x24786);` | Independent byte-offset witness for `unknown_24786` in this image-specific layout; keep the expected offset literal instead of deriving it from the declaration being checked. |

## `include/kf/open_resources.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 38 | `2` | `u8 unknown_06[2];` | Byte extent of the explicitly opaque `unknown_06` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |
| 47 | `10` | `u8 unknown_500[10];` | Byte extent of the explicitly opaque `unknown_500` region, preserving the next modeled field or object end. Its purpose is unresolved; an invented domain name would obscure that limitation. |

## `include/kf/tmd.h`

| Line | Token | Expression | Reason |
| ---: | --- | --- | --- |
| 67 | `1` | `sizeof(KfTmdPacketHeader) == KF_TMD_PACKET_HEADER_BYTES ? 1 : -1];` | Compile-time assertion idiom: a matching packet size yields the legal array extent +1, otherwise -1 causes a diagnostic. |
| 67 | `1` | `sizeof(KfTmdPacketHeader) == KF_TMD_PACKET_HEADER_BYTES ? 1 : -1];` | Compile-time assertion idiom: a matching packet size yields the legal array extent +1, otherwise -1 causes a diagnostic. |
