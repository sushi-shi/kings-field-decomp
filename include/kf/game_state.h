#ifndef KF_GAME_STATE_H
#define KF_GAME_STATE_H

/*
 * Shared declarations for the game's global state singletons, pools,
 * grids and data tables. Each datum is DEFINED (with DATA()) in its owning
 * translation unit; every consumer sees it through this header.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

extern char DAT_80013e8c[];
extern char DAT_80013eac[];
extern char DAT_80013ecc[];
extern MATRIX DAT_80055858;
extern SVECTOR DAT_80055878[8];
extern u8 DAT_80055afc[];
extern u8 DAT_80055c5c;
extern u16 DAT_80055c66;
extern u8 DAT_80055c6a;
extern u16 DAT_80055c74;
extern u8 DAT_80055c78;
extern u16 DAT_80055c82;
extern u8 DAT_80055c86;
extern u16 DAT_80055c90;
extern u8 DAT_80055c94;
extern u8 DAT_80055ca2;
extern u8 DAT_80055cb0;
extern u8 DAT_80055cbe;
extern u8 DAT_80055ccc;
extern u8 DAT_80055cda;
extern u8 DAT_80055ce8;
extern u8 DAT_80055cf6;
extern u8 DAT_80055d04; /* effect sprite tint anchor */
extern u16 DAT_80055d86;
extern s16 DAT_80055dac;
extern s16 DAT_80055dae;
extern s16 DAT_80055db4;
extern s16 DAT_80055db6;
extern u16 DAT_80055e9c; /* fixed fallback cell window */
extern u8 DAT_800561c8[0x70];
extern char DAT_80056238[16];
extern u8 DAT_80056247[];
extern MATRIX DAT_80056248;
extern u32 DAT_80057b30;
extern u8 DAT_80057b63;
extern SVECTOR DAT_80057b64;
extern u16 DAT_80057b72;
extern void *DAT_80057d08;
extern u32 DAT_80057d24;
extern u32 DAT_80057d30;
extern u32 DAT_80057d34;
extern u32 DAT_80057d3c;
extern s32 DAT_80057e68;
extern s32 DAT_80057e70;
extern u8 DAT_80058010[8];
extern void *DAT_80058018; /* saved handler */
extern u32 DAT_80058020;
extern u32 DAT_80058028;
extern POLY_FT4 DAT_80058228[2];
extern POLY_FT4 DAT_80058278[2];
extern POLY_F4 DAT_800582c8[2][6];
extern u16 DAT_80058400; /* tpage */
extern u16 DAT_80058402; /* clut */
extern u8 DAT_80058404; /* u */
extern u8 DAT_80058406; /* v */
extern u16 DAT_80058408; /* tile width  */
extern u16 DAT_8005840a; /* tile height */
extern u8 DAT_80058478[];
extern s16 DAT_80059108[];
extern s16 DAT_800591d0[];
extern s16 DAT_8005920c[];
extern s16 DAT_80059374[];
extern s16 DAT_80059450[];
extern int DAT_8005b270[]; /* jmp_buf */
extern void *DAT_8005b274; /* live abort handler slot (jmp_buf + 4) */
extern int DAT_80063278; /* installed abort handler */
extern u32 DAT_8006bd88;
extern SVECTOR DAT_800930f0[]; /* morph scratch base */
extern u32 DAT_800930f8[]; /* base+keyframe scratch (== &DAT_800930f0[1]) */
extern u16 DAT_8009503a;
extern u16 DAT_8009503c;
extern u16 DAT_80095048;
extern u16 DAT_8009504a;
extern u16 DAT_8009504c;
extern u16 DAT_80095058; /* clut; the colour CVECTOR follows four bytes on */
extern u16 DAT_8009505a; /* tpage */
extern u8 DAT_8009505c;
extern u8 DAT_8009505d;
extern u8 DAT_8009505e;
extern u8 DAT_8009505f; /* colour CVECTOR command byte */
extern u16 DAT_80095060;
extern u16 DAT_80095062;
extern u8 DAT_80095064;
extern u16 DAT_80095066;
extern u16 DAT_80095068;
extern u16 DAT_8009506a;
extern u16 DAT_8009506c;
extern u16 DAT_8009508c;
extern u16 DAT_8009508e;
extern u32 DAT_80095698;
extern u32 DAT_8009569c;
extern u32 DAT_8009a740;
extern KfMagicRecord *DAT_8009db80; /* magic_records row of the current effect */
extern KfEffectRecord *DAT_8009db84; /* current effect record */
extern u16 map_event_animation_gate;
extern u16 map_ambient_script_countdown;
extern u8 DAT_8009eafc;
extern u8 DAT_8009f844;
extern u8 DAT_8009f845;
extern u8 DAT_8009f846;
extern u32 DAT_800a0768;
extern KfActorActionProfile actor_action_profiles[25];
extern KfActorState actor_state;
extern KfArmorRecord armor_records[42];
extern u8 audio_sequence_table[];
extern KfAudioState audio_state;
extern s32 audio_voice_slot_index;
extern const SoundRef boss_death_loop_sound;
extern SoundRef boss_death_phase_sounds[4];
extern u8 boss_defeat_complete;
extern char cd_path_buffer[80];
extern KfCollisionTarget collision_target;
extern MATRIX color_matrix_table[7];
extern KfMapEvent *current_map_event;
extern SVECTOR *current_tmd_vertices;
extern DISPENV display_disp_environments[2];
extern DRAWENV display_draw_environments[2];
extern KfDisplayState display_state;
extern char enemy_info_image_path_template[13];
extern const KfFloorEntryCell floor_entry_cells[5];
extern u16 floor_item_count;
extern KfFloorItem floor_items[64];
extern u32 game_exit_code;
extern const SoundRef gameplay_sound_ref_0;
extern const SoundRef gameplay_sound_ref_1;
extern SoundRef gameplay_sound_ref_10;
extern const SoundRef gameplay_sound_ref_11;
extern const SoundRef gameplay_sound_ref_12;
extern SoundRef gameplay_sound_ref_2;
extern const SoundRef gameplay_sound_ref_3;
extern SoundRef gameplay_sound_ref_4;
extern const SoundRef gameplay_sound_ref_5;
extern SoundRef gameplay_sound_ref_6;
extern const SoundRef gameplay_sound_ref_7;
extern const SoundRef gameplay_sound_ref_8;
extern MATRIX light_quadrant_matrices[4];
extern u8 map_cell_attribute_grid[100][100];
extern s16 map_cell_attribute_height_table[284];
extern u8 map_cell_orientation_grid[100][100];
extern u8 map_collision_flag_grid[100][100];
extern u8 map_collision_grid[100][100];
extern KfMapCopyRegion map_copy_regions[4];
extern KfMapEvent map_event_pool[8];
extern u8 map_floor_height_grid[100][100];
extern u16 map_object_effect_sequence_160;
extern u16 map_object_effect_sequence_170;
extern u16 map_object_effect_sequence_180;
extern KfMapObjectState map_object_state;
extern char map_resource_path[12];
extern u8 *map_variant_asset_buffer;
extern const char map_variant_filename_template[9];
extern u32 memory_allocation_stack[17];
extern u8 *memory_arena_cursor;
extern u8 *memory_arena_end;
extern u8 *memory_arena_start;
extern s32 memory_card_error_event;
extern s32 memory_card_io_end_event;
extern char memory_card_message_path_template[];
extern s32 memory_card_new_device_event;
extern const char memory_card_root_path[];
extern s32 memory_card_timeout_event;
extern s32 memory_system_heap_size;
extern u8 *memory_system_heap_start;
extern s32 pending_game_state;
extern char person_image_path_template[14];
extern MATRIX player_death_saved_color_matrix;
extern s32 player_death_saved_fog_near;
extern KfPlayerLevelGrowth player_level_growth_table[40];
extern VECTOR player_position_snapshot;
extern SVECTOR player_rotation_snapshot;
extern KfPlayerState player_state;
extern struct KfPoolRecord pool_records[];
extern MATRIX render_light_matrices[6];
extern KfRenderState render_state;
extern KfSaveHeader *save_header_buffer;
extern const char save_main_file_path[];
extern KfSavePayload *save_payload_buffer;
extern const char save_temporary_file_path[];
extern char talk_image_path_template[];
extern KfTmdState tmd_state;
extern char weapon_image_path_template[15];
extern KfWeaponRecord weapon_records[16];

#endif
