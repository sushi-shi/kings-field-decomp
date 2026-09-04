#ifndef KF_SEMANTIC_TYPES_H
#define KF_SEMANTIC_TYPES_H

#include <kf/game_types.h>
#include <kf/psyq.h>
#include <kf/audio.h>
#include <kf/game_math.h>
#include <kf/game_actor.h>

/*
 * Layout identities supported by the semantic inventory. Their original
 * source spellings and translation-unit ownership remain unknown.
 */
/*
 * Each display buffer owns one primitive allocation interval. Frame begin
 * selects one of two 0x0c-byte records and resets cursor to start.
 */
typedef struct KfPrimitiveBuffer {
    u8 *start;
    u8 *end;
    u8 *cursor;
} KfPrimitiveBuffer;

/* Texture rectangle followed by its screen-space rectangle. */
typedef struct KfSpriteQuad {
    u8 u;
    u8 v;
    u8 u_span;
    u8 v_span;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} KfSpriteQuad;

/* One row of the sentinel-terminated HUD gauge and status-sprite table. */
typedef struct KfHudSprite {
    u8 state;
    u8 unknown_01;
    KfSpriteQuad sprite;
} KfHudSprite;

/* One 14-byte row of the six-sprite on-screen notification display. */
typedef struct KfNotificationSprite {
    u8 active;
    u8 unknown_01;
    KfSpriteQuad sprite;
} KfNotificationSprite;

/* Animated screen-facing model entry; state 0xff terminates the two-row list. */
typedef struct KfEffectSprite {
    u8 state;
    u8 visibility_tag;
    u16 asset_variant;
    u16 scale;
    s16 translation_x;
    s16 translation_y;
    s16 translation_z;
    u8 unknown_0c[2];
    SVECTOR rotation;
    u8 unknown_16[2];
    u8 anchor[4];
} KfEffectSprite;

/*
 * Optional output from the world collision query.  The query copies a
 * transform from the selected player, actor, map object, or map event and
 * records the selected object's collision radius.  The final six bytes are
 * retained because the next independently referenced state begins at +0x20;
 * their meaning is not yet known.
 */
typedef struct KfCollisionTarget {
    VECTOR position;
    SVECTOR rotation;
    u16 radius;
    u8 unknown_1a[0x06];
} KfCollisionTarget;

typedef struct KfMapCell {
    u8 z;
    u8 x;
} KfMapCell;

typedef struct KfMapCopyRegion {
    u8 source_x;
    u8 source_z;
    u8 destination_x;
    u8 destination_z;
    u8 width;
    u8 height;
} KfMapCopyRegion;

/*
 * Effect pool record (60-byte stride, pool in effect_pool_records). The pool
 * general and specialised effect-pool constructors
 * establish the field layout: byte 0 is the type tag (0xff marks a free
 * slot), byte 1 the dispatch/magic_records index, byte 6 the owning id, a
 * VECTOR world position at 0x0c, and three SVECTOR-shaped 16-bit triples
 * (rotation, scale, direction) that individual effect kinds reuse. The
 * default scale is 0x1000 (1.0 fixed point); the direction triple is copied
 * from the constructor's SVECTOR argument.
 */
typedef struct KfEffectRecord {
    u8 type;             /* 0x00 */
    u8 kind;             /* 0x01 */
    u8 unknown_02;       /* 0x02 */
    u8 unknown_03;       /* 0x03 */
    u8 unknown_04;       /* 0x04 */
    u8 unknown_05;       /* 0x05 */
    u8 id;               /* 0x06 */
    u8 unknown_07;       /* 0x07 */
    u16 unknown_08;      /* 0x08 */
    u16 unknown_0a;      /* 0x0a */
    VECTOR position;     /* 0x0c */
    u16 rotation_x;      /* 0x1c */
    u16 rotation_y;      /* 0x1e */
    u16 rotation_z;      /* 0x20 */
    u16 unknown_22;      /* 0x22 */
    u16 scale_x;         /* 0x24 */
    u16 scale_y;         /* 0x26 */
    u16 scale_z;         /* 0x28 */
    u16 unknown_2a;      /* 0x2a */
    u16 direction_x;     /* 0x2c */
    u16 direction_y;     /* 0x2e */
    u16 direction_z;     /* 0x30 */
    u16 unknown_32;      /* 0x32 */
    u8 unknown_34[4];    /* 0x34 */
    u16 unknown_38;      /* 0x38 */
    u16 unknown_3a;      /* 0x3a */
} KfEffectRecord;

/* Rendering view of the same 60-byte effect-pool record. The renderer reads
 * the low halfwords of the VECTOR position and interprets kind-specific header
 * bytes as sprite selectors. */
typedef struct KfEffectRenderView {
    u8 unknown_00[3];
    u8 sprite_id;
    u8 mode;
    u8 unknown_05[3];
    u16 asset_variant;
    u8 unknown_0a[2];
    u16 position_x;
    u16 unknown_0e;
    u16 position_y;
    u16 unknown_12;
    u16 position_z;
    u8 unknown_16[6];
    struct KfEulerAngles rotation;
    u8 unknown_22[2];
    s16 scale_x;
    s16 scale_y;
    s16 scale_z;
    u8 unknown_2a[10];
    u8 anchor[4];
    u8 unknown_38[4];
} KfEffectRenderView;

typedef struct KfMapObjectLink {
    u8 link_id;
    u8 action_parameter;
    u16 spawn_sequence;
    s16 vertical_velocity;
    u8 unknown_06[2];
} KfMapObjectLink;

typedef struct KfMapObjectPlacement {
    u8 object_id;
    u8 unknown_01;
    u8 tile_z;
    u8 tile_x;
    u16 yaw;
    s16 local_z;
    s16 local_x;
    s16 local_y;
    KfMapObjectLink link;
} KfMapObjectPlacement;

typedef struct KfMapObjectDefinition {
    u8 behavior_type;
    u8 unknown_01;
    u16 collision_radius;
    u16 interaction_radius;
    u8 unknown_06[2];
} KfMapObjectDefinition;

/* Header shared by static and animated model assets in the asset registry. */
typedef struct KfAssetHeader {
    u32 byte_size;
    s32 animation_data;
    u32 tmd_data_offset;
    u32 object_table_offset;
    u32 clip_table_offset;
} KfAssetHeader;

typedef struct KfMapObject {
    u8 object_id;
    u8 unknown_01;
    u16 cell_x;
    u16 cell_z;
    u8 unknown_06[2];
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 unknown_14[4];
    struct KfEulerAngles rotation;
    u16 unknown_1e;
    KfMapObjectLink link;
    u8 action;
    u8 unknown_29;
    u16 action_timer;
} KfMapObject;

/*
 * Serialized floor-item placement record (12 bytes) from the map resource
 * stream, and the runtime floor-item entry (24 bytes) the loader expands it
 * into.  The tile bytes index map_floor_height_grid[tile_z][tile_x]; the world
 * position is tile*2000 plus the signed local offset, and the height byte times
 * -100 sinks the item onto the floor.
 */
typedef struct KfFloorItemPlacement {
    u16 item_id;
    u8 facing_and_frame_count;
    u8 unknown_03;
    u8 tile_z;
    u8 tile_x;
    s16 local_z;
    s16 local_x;
    s16 local_y;
} KfFloorItemPlacement;

typedef struct KfFloorItem {
    u16 item_id;
    u8 facing_and_frame_count;
    u8 unknown_03;
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 unknown_10[4];
    u8 animation_frame;
    u8 unknown_15[3];
} KfFloorItem;

/*
 * Per-yaw visible-cell window selected by render_map_cells and consumed by
 * render_entities.  The cell payload is row-major with width*height entries.
 */
typedef struct KfCellWindow {
    u16 width;
    u16 height;
    u16 origin_x;
    u16 origin_z;
    u8 cells[196];
} KfCellWindow;

/*
 * Cutscene camera paths use 0x1c-byte serialized points and a 0x64-byte
 * runtime interpolator.  The fourth vector lane and two trailing halfwords
 * are retained because their meanings are not yet evidenced.
 */
typedef struct KfCameraPathPoint {
    VECTOR position;
    SVECTOR rotation;
    s16 speed;
    s16 unknown_1a;
} KfCameraPathPoint;

typedef struct KfCameraPathState {
    const KfCameraPathPoint *points;
    VECTOR position;
    SVECTOR rotation;
    VECTOR position_fixed;
    VECTOR rotation_fixed;
    VECTOR position_delta;
    VECTOR rotation_delta;
    s16 point_index;
    s16 unknown_5e;
    s32 frames_remaining;
} KfCameraPathState;

/*
 * Map-event definitions are expanded from a 0x18-byte resource record into
 * one of eight 0x44-byte runtime slots.  Only fields supported by the loader
 * and reviewed update routines are named.
 */
/*
 * Five opaque bytes that map_event_pool_load copies from a definition into its
 * runtime slot as one block (an unaligned word plus a byte), which only a
 * struct assignment produces.
 */
typedef struct KfMapEventTag {
    u8 bytes[5];
} KfMapEventTag;

typedef struct KfMapEventDefinition {
    u8 state;
    u8 kind;
    u8 variant;
    u8 cell_z;
    u8 cell_x;
    KfMapEventTag tag;
    u8 image_limit;
    u8 unknown_0b;
    u8 unknown_0c;
    u8 unknown_0d;
    s16 position_z_offset;
    s16 position_x_offset;
    u16 initial_rotation;
    u16 radius;
    u16 unknown_16;
} KfMapEventDefinition;

typedef struct KfMapEvent {
    u8 state;
    u8 kind;
    u8 variant;
    KfMapEventTag tag;
    u8 image_limit;
    u8 image_index;
    u8 image_dirty;
    u8 image_delay;
    u8 unknown_0c;
    u8 unknown_0d;
    u8 unknown_0e;
    u8 unknown_0f;
    u8 unknown_10;
    u8 unknown_11;
    u16 rotation_phase;
    s32 position_x;
    s32 position_z;
    u16 cell_x;
    u16 cell_z;
    u16 radius;
    u16 unknown_22;
    s32 reference_x;
    s32 position_y;
    s32 reference_z;
    u8 unknown_30[4];
    u16 unknown_34;
    s16 rotation;
    u16 unknown_38;
    u8 unknown_3a[6];
    s16 rotation_target;
    u16 unknown_42;
} KfMapEvent;

typedef struct KfPlayerProgressState {
    u8 level;
    u8 unknown_01;
    u8 current_floor;
    u8 highest_floor;
} KfPlayerProgressState;

/*
 * Record zero seeds a new player's values.  Later records provide the next
 * absolute HP/MP values, stat steps, and cumulative experience threshold.
 * After level 40 the game extrapolates from the final two records.
 */
typedef struct KfPlayerLevelGrowth {
    u16 maximum_hp;
    u16 maximum_mp;
    u16 physical_power_step;
    u16 magic_step;
    u32 experience_threshold;
} KfPlayerLevelGrowth;

/*
 * The HUD, save summary, adjustment helpers, and full-restoration effect all
 * agree on these four adjacent unsigned halfwords.  Linked bytes do not
 * establish whether the original object had external or internal linkage.
 */
/* Armor and accessory record (item ids 13..54 index armor_records[id - 13]). */
typedef struct KfArmorRecord {
    u16 unknown_00;
    u16 defense_component0;
    u16 defense_component1;
    u16 defense_component2;
    u16 status_effect2_resistance;
    u16 defense_component3;
    u16 defense_component4;
    u16 hp_regen_interval;
    u16 hp_drain_interval;
    u8 unknown_12[0x0a];
} KfArmorRecord;

typedef struct KfPlayerVitals {
    u16 maximum_hp;
    u16 current_hp;
    u16 maximum_mp;
    u16 current_mp;
} KfPlayerVitals;

typedef struct KfPlayerAttackChargeState {
    u16 current;
    u16 committed;
} KfPlayerAttackChargeState;

/*
 * The player input loop ramps the two signed movement components, derives the
 * unsigned horizontal speed, and independently ramps the yaw and pitch steps.
 * The grouped clear routine proves the complete five-halfword extent.
 */
typedef struct KfPlayerMotionState {
    s16 strafe_velocity;
    s16 forward_velocity;
    u16 movement_speed;
    s16 yaw_step;
    s16 pitch_step;
} KfPlayerMotionState;

/*
 * Equipped weapons use a 0x2c-byte runtime-loaded record.  Reviewed combat
 * and attack code establishes only the fields below; the remaining resource
 * bytes deliberately retain opaque identities.
 */
typedef struct KfWeaponRecord {
    u8 unknown_00;
    u8 charge_rate;
    u16 attack_components[5];
    u16 hp_regen_interval;
    u16 mp_regen_interval;
    u8 unknown_10[0x02];
    u16 attack_z_offset;
    u8 unknown_14[0x12];
    u16 mirrored_angle;
    u8 unknown_28[0x04];
} KfWeaponRecord;

typedef struct KfMagicRecord {
    u8 learned;
    u8 charge_rate;
    SoundRef sounds[2];
    u16 damage_components[4];
    u16 mp_cost;
    u8 unknown_12[0x02];
} KfMagicRecord;

/* === game-math-death layouts === */
/* === end game-math-death === */

/* === player layouts === */

/*
 * The saved player object. save_file_write_slot copies exactly 0xe0 bytes
 * from 0x800a0780 into the save payload with the compiler's inline struct
 * loop, and the player routines address these members from a registered
 * base; both prove one object rather than adjacent globals. Field names keep
 * their former identity spellings; unknown_ spans remain opaque.
 */
typedef struct KfPlayerState {
    s32 experience;
    s32 next_level_experience;
    KfPlayerProgressState progress_state;
    u8 map_variant;
    u8 unknown_0d;
    u8 weapon_charge_delay;
    u8 unknown_0f;
    KfPlayerVitals vitals;
    KfPlayerAttackChargeState attack_charge_state;
    u16 magic_charge;
    u16 physical_power_training;
    u16 magic_training;
    u16 base_physical_power;
    u16 base_magic;
    u16 physical_power;
    u16 magic;
    u16 status_effect_flags;
    u32 gold;
    u16 attack_component0;
    u16 attack_component1;
    u16 attack_component2;
    u16 attack_component3;
    u16 attack_component4;
    u8 unknown_3a[2];
    u16 damage_defense_component0;
    u16 damage_defense_component1;
    u16 damage_defense_component2;
    u16 status_effect2_resistance;
    u16 damage_defense_component3;
    u16 damage_defense_component4;
    s16 status_effect0_timer;
    s16 status_effect1_timer;
    s16 status_effect2_timer;
    s16 status_effect3_timer;
    s16 status_effect4_timer;
    s16 light_effect_timer;
    u8 unknown_54[4];
    u32 unknown_58;
    u8 selected_magic_id;
    u8 unknown_5d[3];
    KfMagicRecord *selected_magic_record;
    u8 equipped_weapon_id;
    u8 unknown_65[3];
    const KfWeaponRecord *equipped_weapon_record;
    u8 *weapon_asset_buffer;
    s16 weapon_attack_phase;
    u8 unknown_72[2];
    u32 unknown_74;
    u8 unknown_78;
    u8 unknown_79;
    u8 weapon_attack_fully_charged;
    u8 unknown_7b[1];
    KfArmorRecord *equipped_shield_record;
    KfArmorRecord *equipped_head_armor_record;
    KfArmorRecord *equipped_body_armor_record;
    KfArmorRecord *equipped_arm_armor_record;
    KfArmorRecord *equipped_leg_armor_record;
    u8 equipped_shield_id;
    u8 equipped_head_armor_id;
    u8 equipped_body_armor_id;
    u8 equipped_arm_armor_id;
    u8 equipped_leg_armor_id;
    u8 equipped_accessory_id;
    u8 audio_effects_enabled;
    u8 audio_music_enabled;
    u8 unknown_98[2];
    SVECTOR view_rotation_offset;
    u8 update_state;
    u8 unknown_a3;
    VECTOR camera_position;
    s32 floor_height;
    SVECTOR camera_rotation;
    KfPlayerMotionState motion_state;
    KfMapCell map_cell;
    KfMapCell previous_map_cell;
    u8 unknown_ce[6];
    s16 view_bob_offset;
    u16 view_bob_phase;
    u16 death_camera_pitch_step;
    s16 death_visual_blend;
    s16 vertical_velocity;
    u8 vertical_state;
    u8 unknown_df[1];
} KfPlayerState;

/*
 * Per-floor entry cell (one-based floor number). player_warp_to_floor_entry
 * copies byte 0 into previous_map_cell.x and byte 1 into .z, so this table
 * stores x first, unlike KfMapCell.
 */
typedef struct KfFloorEntryCell {
    u8 x;
    u8 z;
} KfFloorEntryCell;

/* === end player === */
/* One reversed ordering table of 0x4000 entries (ClearOTagR/DrawOTag). */
typedef struct KfOrderingTable {
    u32 entries[0x4000];
} KfOrderingTable;

/* === display_state layout === */
/*
 * Double-buffered display state. display_begin_frame addresses the primitive
 * buffers (+8) and the ordering tables (+0x24) from the buffer index through
 * one base register, so the block is one object in the original source.
 */
typedef struct KfDisplayState {
    u8 buffer_index;
    u8 unknown_01[3];
    void *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[2];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[2];
    u32 *ordering_table;
} KfDisplayState;

/* === end display_state === */

/* === tmd_state layout === */
/*
 * Registered TMD slots and the selected asset. tmd_register addresses the slot
 * table 32 bytes below the current-asset pointer through one base register.
 */
typedef struct KfTmdState {
    u8 *slots[8];
    void *current_asset;
} KfTmdState;

/* === end tmd_state === */

/* === render_state layout === */
/*
 * Render state. render_set_view_transform reaches the view and pitch matrices
 * from the rotation (-180, -148) and render_initialize reaches the quadrant
 * matrices from the light matrix (+128..+224) through one base register, so
 * the block is one object in the original source. The light products that
 * follow (light_quadrant_matrices) are addressed absolutely there, so they are
 * a separate object. Matrix roles follow the GTE calls that fill them; the
 * light matrix values are three direction rows.
 */
typedef struct KfRenderState {
    MATRIX view_matrix;
    MATRIX pitch_matrix;
    MATRIX light_matrix;
    MATRIX light_matrix_copy;
    MATRIX unknown_80;
    s32 fog_near_distance;
    VECTOR view_position;
    SVECTOR view_rotation;
    struct KfVecXZs view_cell;
    MATRIX quadrant_matrices[4];
} KfRenderState;

/* === end render_state === */

/* === display_state layout === */
/*
 * OPEN.EXE display state.  Same members as KfDisplayState except that the
 * ordering-table pointer lives outside the block (ordering_table).
 */
typedef struct KfDisplayStateOpen {
    u8 buffer_index;
    u8 unknown_01[3];
    void *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[2];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[2];
} KfDisplayStateOpen;

/* === end display_state === */

/* === tmd_state layout === */
/* OPEN.EXE TMD state: two asset slots and the selected asset. */
typedef struct KfTmdStateOpen {
    u8 *slots[2];
    void *current_asset;
} KfTmdStateOpen;

/* === end tmd_state === */

/* === render_state layout === */
/*
 * OPEN.EXE render state.  Same roles as KfRenderState without the
 * light_matrix_copy and unknown_80 members.
 */
typedef struct KfRenderStateOpen {
    MATRIX view_matrix;
    MATRIX pitch_matrix;
    MATRIX light_matrix;
    s32 fog_near_distance;
    VECTOR view_position;
    SVECTOR view_rotation;
    struct KfVecXZs view_cell;
    MATRIX quadrant_matrices[4];
} KfRenderStateOpen;

/* === end render_state === */

/* === map_object_state layout === */
/*
 * Map object state: the 160 definitions and the 190-object pool.
 * map_object_pool_clear_link reaches the definitions from the pool base
 * register (-1280), so the two arrays are one object in the original source.
 */
typedef struct KfMapObjectState {
    KfMapObjectDefinition definitions[160];
    KfMapObject objects[190];
} KfMapObjectState;

/* === end map_object_state === */

#endif
