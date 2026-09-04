#ifndef KF_SEMANTIC_TYPES_H
#define KF_SEMANTIC_TYPES_H

#include <kf/game_types.h>
#include <kf/psyq.h>
#include <kf/audio.h>
#include <kf/game_math.h>
#include <kf/game_actor.h>
#include <kf/game_map.h>
#include <kf/item.h>
#include <kf/magic.h>
#include <kf/game_effect.h>
#include <kf/game_equipment.h>
#include <kf/game_player.h>

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

/* Header shared by static and animated model assets in the asset registry. */
typedef struct KfAssetHeader {
    u32 byte_size;
    s32 animation_data;
    u32 tmd_data_offset;
    u32 object_table_offset;
    u32 clip_table_offset;
} KfAssetHeader;

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

#endif
