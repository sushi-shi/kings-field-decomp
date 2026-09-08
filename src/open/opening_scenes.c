#include <kf/address.h>
#include <kf/audio.h>
#include <kf/game_math.h>
#include <kf/open_audio.h>
#include <kf/open_camera_path.h>
#include <kf/open_opening_helpers.h>
#include <kf/open_opening_render.h>
#include <kf/open_opening_scenes.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/open_scene0.h>
#include <kf/psyq_audio.h>

enum {
    SCENE0_ROTATION_START_POINT = 8,
    SCENE0_FADE_OUT_START_POINT = 15,
    SCENE0_YAW_STEP = 40,
    SCENE0_SOUND_VOLUME = 100,
    OPENING_COLOR_FADE_STEP = 0x100,
    SCENE1_LEFT_TPAGE_X = 0x140,
    SCENE1_RIGHT_TPAGE_X = 0x1c0,
    SCENE1_PANEL_WIDTH = 192,
    SCENE1_RIGHT_PANEL_X = 128,
    SCENE1_SHADE_STEP = 4,
    SCENE1_SEQUENCE_STOP_FRAME = 600,
    SCENE1_HOLD_FRAMES = 1000,
    TRANSITION_FIRST_ENTITY_SLOT = 24,
    TRANSITION_ENTITY_COUNT = 4,
    TRANSITION_TALL_SCALE_Y = 0x2000,
    TRANSITION_SCALE_STEP = 0x100,
    TRANSITION_YAW_STEP = 0x200,
    TRANSITION_ENTITY_DELAY_SHIFT = 3,
    TRANSITION_FRAMES = 48
};

enum {
    SCENE3_YAW_STEP = 0x10,
    SCENE3_CLUT_WORK_CAPACITY = 6,
    SCENE_CAMERA_WAVE_SHIFT = 7,
    SCENE_CAMERA_WAVE_ANGLE_STEP = 100,
    TRANSITION_BASE_Y = -10000,
    PANEL_TPAGE_FIRST_X = 0x1c0,
    PANEL_TPAGE_X_STRIDE = 0x40,
    PANEL_CLUT_FIRST_Y = 0x1ed,
    PANEL_CLIP_Y_BIAS = 255,
    PANEL_CLIP_SPAN = PANEL_CLIP_Y_BIAS + KF_DISPLAY_HEIGHT,
    PANEL_OT_DEPTH = 4,
    ENDING_ROTATION_START_POINT = 4,
    ENDING_MAX_BRIGHTNESS = 255,
    ENDING_BRIGHTEN_STEP = 4,
    ENDING_FADE_DARKEN_STEP = 0x10,
    ENDING_PANEL_COUNT = 9,
    ENDING_PANEL_STOP_Y = 50,
    ENDING_BACKGROUND_OT_DEPTH = 0x2f65,
    ENDING_TMD_PROJECTION_SHIFT = 2,
    ENDING_MODEL_START_Y_OFFSET = 1500,
    ENDING_MODEL_FINAL_Y = -8000,
    ENDING_MODEL_Y_STEP = 3,
    ENDING_LIGHT_MIDPOINT_STEP = 0x40,
    ENDING_LIGHT_GREEN_STEP = 3,
    ENDING_BACKGROUND_BLEND_STEP = 3,
    ENDING_SEQUENCE_DELAY_START = 20,
    ENDING_SEQUENCE_VOLUME_DIVISOR = 3,
    ENDING_SCROLL_CAMERA_POINT_COUNT = 3
};

KF_ENUM_BEGIN(KfEndingLightingPhase, s16)
    ENDING_LIGHT_TO_MIDPOINT = 0,
    ENDING_LIGHT_TO_GREEN = 1,
    ENDING_LIGHT_FINISHED = 2
KF_ENUM_END(KfEndingLightingPhase)

KF_ENUM_BEGIN(KfEndingSequencePhase, s16)
    ENDING_SEQUENCE_WAIT_SCROLL = 0,
    ENDING_SEQUENCE_DELAY = 1,
    ENDING_SEQUENCE_FADE = 2,
    ENDING_SEQUENCE_REPLACED = 3
KF_ENUM_END(KfEndingSequencePhase)

KF_ENUM_BEGIN(KfEndingScrollState, s16)
    ENDING_SCROLL_WAIT_DISK = 0,
    ENDING_SCROLL_ACTIVE = 1
KF_ENUM_END(KfEndingScrollState)

/* Countdown order: combined update, hold, panel update, hold. */
KF_ENUM_BEGIN(KfEndingScrollTick, s16)
    ENDING_SCROLL_TICK_WRAP = -1,
    ENDING_SCROLL_TICK_STARFIELD_AND_PANELS = 0,
    ENDING_SCROLL_TICK_HOLD_AFTER_PANELS = 1,
    ENDING_SCROLL_TICK_PANELS = 2,
    ENDING_SCROLL_TICK_HOLD_AFTER_STARFIELD = 3
KF_ENUM_END(KfEndingScrollTick)

typedef char KfOpeningEntityPositionOffsetCheck[
    (u32)&((KfOpeningEntity *)0)->position == 0x08 ? 1 : -1];

DATA(0x800354f4, 0x1dc)
KfCameraPathPoint opening_scene0_camera_path[KF_OPENING_SCENE0_CAMERA_POINT_COUNT] = {
    {{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},
    {{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},
    {{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},
    {{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},
    {{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},
    {{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},
    {{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},
    {{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},
    {{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},
    {{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},
    {{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},
    {{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},
    {{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},
    {{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},
    {{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},
    {{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},
    {{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x800356d0, 0x54)
KfCameraPathPoint opening_scene3_camera_path[KF_OPENING_SCENE3_CAMERA_POINT_COUNT] = {
    {{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},
    {{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},
    {{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035724, 0xfc)
KfCameraPathPoint opening_ending_camera_path[KF_OPENING_ENDING_CAMERA_POINT_COUNT] = {
    {{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},
    {{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},
    {{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},
    {{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},
    {{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},
    {{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},
    {{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},
    {{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},
    {{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035820, 0x54)
static KfCameraPathPoint opening_ending_scroll_camera_path[ENDING_SCROLL_CAMERA_POINT_COUNT] = {
    {{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},
    {{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},
    {{KF_CAMERA_PATH_END_X, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035874, 0x3)
SoundRef opening_scene0_sound = {9, 0, 0x43};

DATA(0x80035878, 0x10)
KfScreenRect opening_scene3_overlay_rects[KF_OPENING_SCENE3_OVERLAY_COUNT] = {
    {32, 256, 255, 254},
    {32, 512, 255, 254},
};

DATA(0x80035888, 0x48)
static KfScreenRect opening_ending_scroll_panels[ENDING_PANEL_COUNT] = {
    {32, 256, 255, 254},
    {32, 512, 255, 254},
    {32, 768, 255, 254},
    {32, 1024, 255, 254},
    {32, 1261, 255, 254},
    {32, 1490, 255, 254},
    {32, 1746, 255, 254},
    {32, 2002, 255, 254},
    {32, 2258, 255, 254},
};

DATA(0x80037284, 0x8)
u8 opening_scene3_overlay_uv[KF_QUAD_TEX_DESCRIPTOR_BYTES] = {0, 0, 0, 0, 255, 0, 254, 0};

DATA(0x8003728c, 0x4)
CVECTOR opening_scene3_overlay_color = {200, 200, 200, 0};

DATA(0x80037290, 0x10)
static KfScreenRect opening_ending_scroll_backgrounds[2] = {
    {0, 0, KF_DISPLAY_WIDTH, 160}, {0, 160, KF_DISPLAY_WIDTH, 160},
};

DATA(0x800372a0, 0x4)
static CVECTOR opening_ending_scroll_top_start = {32, 0, 32, 0};

DATA(0x800372a4, 0x4)
static CVECTOR opening_ending_scroll_bottom_start = {255, 0, 0, 0};

DATA(0x800372a8, 0x4)
static CVECTOR opening_ending_scroll_top_end = {0, 0, 32, 0};

DATA(0x800372ac, 0x4)
static CVECTOR opening_ending_scroll_bottom_end = {0, 0, 32, 0};

DATA(0x800372b0, 0x4)
static CVECTOR opening_ending_scroll_panel_color = {200, 200, 200, 0};

DATA(0x800372b4, 0x4)
static CVECTOR opening_ending_scroll_background_color = {0, 0, 0, 0};

DATA(0x800372b8, 0x8)
static u8 opening_ending_scroll_uv[KF_QUAD_TEX_DESCRIPTOR_BYTES] = {0, 0, 1, 0, 255, 0, 254, 0};

RODATA(0x80012000, 0x20)

ADDRESS(0x80014268, 0x174)
void opening_scene0_run(void)
{
    KfOpeningEntity *entity_11;
    KfOpeningEntity *entity_12;
    s16 blend;
    s16 next_blend;

    blend = 0;
    opening_resources_load_scene0();
    entity_11 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_SCENE0_DECREASING_YAW_MODEL);
    entity_12 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_SCENE0_INCREASING_YAW_MODEL);
    entity_12->rotation.y = KF_ANGLE_THREE_QUARTER_TURN;
    entity_11->rotation.y = KF_ANGLE_THREE_QUARTER_TURN;
    opening_camera_path_begin(opening_scene0_camera_path);

    for (;;) {
        opening_poll_input();
        opening_camera_path_step(0);
        if (opening_camera_path_state.point_index >= SCENE0_ROTATION_START_POINT) {
            if (entity_11->rotation.y == KF_ANGLE_THREE_QUARTER_TURN) {
                sound_ref_play(&opening_scene0_sound, SCENE0_SOUND_VOLUME);
            }
            entity_11->rotation.y -= SCENE0_YAW_STEP;
            entity_12->rotation.y += SCENE0_YAW_STEP;
        }

        if (opening_camera_path_state.point_index >= SCENE0_FADE_OUT_START_POINT) {
            goto fade_out;
        }
        if (opening_input_action == KF_OPENING_INPUT_NONE) {
            goto fade_in;
        }

fade_out:
        if (blend < 0) {
            goto scene_complete;
        }
        next_blend = blend - OPENING_COLOR_FADE_STEP;
        blend = next_blend;
        goto update_color;

fade_in:
        if (blend >= KF_FIXED12_ONE) {
            goto render_frame;
        }
        next_blend = blend + OPENING_COLOR_FADE_STEP;
        blend = next_blend;

update_color:
        lighting_set_color_matrix(
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_BLACK)],
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_DEFAULT)], next_blend);

render_frame:
        audio_set_listener_transform(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_scene0_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
    }

scene_complete:
    audio_stop_sequence(KF_AUDIO_STOP_FADE);
}

ADDRESS(0x800143dc, 0x180)
void opening_scene1_draw_fade(u8 shade)
{
    POLY_FT4 *left;
    POLY_FT4 *right;
    u32 **ordering_table_slot;

    display_begin_frame();
    left = (POLY_FT4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;
    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    right = (POLY_FT4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;
    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);

    SetPolyFT4(left);
    SetPolyFT4(right);
    left->tpage = GetTPage(
        KF_GPU_TEXTURE_16BIT, KF_GPU_BLEND_AVERAGE,
        SCENE1_LEFT_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    right->tpage = GetTPage(
        KF_GPU_TEXTURE_16BIT, KF_GPU_BLEND_AVERAGE,
        SCENE1_RIGHT_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);

    left->x0 = 0;
    left->y0 = 0;
    left->x1 = SCENE1_PANEL_WIDTH;
    left->y1 = 0;
    left->x2 = 0;
    left->y2 = KF_DISPLAY_HEIGHT;
    left->x3 = SCENE1_PANEL_WIDTH;
    left->y3 = KF_DISPLAY_HEIGHT;

    right->x0 = SCENE1_RIGHT_PANEL_X;
    right->y0 = 0;
    right->x1 = KF_DISPLAY_WIDTH;
    right->y1 = 0;
    right->x2 = SCENE1_RIGHT_PANEL_X;
    right->y2 = KF_DISPLAY_HEIGHT;
    right->x3 = KF_DISPLAY_WIDTH;
    right->y3 = KF_DISPLAY_HEIGHT;

    left->u0 = 0;
    left->v0 = 0;
    left->u1 = SCENE1_PANEL_WIDTH;
    left->v1 = 0;
    left->u2 = 0;
    left->v2 = KF_DISPLAY_HEIGHT;
    left->u3 = SCENE1_PANEL_WIDTH;
    left->v3 = KF_DISPLAY_HEIGHT;

    right->u0 = 0;
    right->v0 = 0;
    right->u1 = SCENE1_PANEL_WIDTH;
    right->v1 = 0;
    right->u2 = 0;
    right->v2 = KF_DISPLAY_HEIGHT;
    right->u3 = SCENE1_PANEL_WIDTH;
    right->v3 = KF_DISPLAY_HEIGHT;

    left->r0 = shade;
    left->g0 = shade;
    left->b0 = shade;
    right->r0 = shade;
    right->g0 = shade;
    right->b0 = shade;

    ordering_table_slot = &open_graphics_runtime.ordering_table;
    AddPrim(*ordering_table_slot, left);
    AddPrim(*ordering_table_slot, right);
    display_present_frame();
}

ADDRESS(0x8001455c, 0xac)
void opening_scene1_run(void)
{
    s32 frame;
    s32 shade;

    opening_resources_load_scene1();
    shade = 0;
    do {
        opening_scene1_draw_fade((u8)shade);
        opening_poll_input();
        shade += SCENE1_SHADE_STEP;
    } while (shade < KF_TEXTURE_BASE_BRIGHTNESS + 1);

    frame = 0;
    do {
        if (frame == SCENE1_SEQUENCE_STOP_FRAME) {
            audio_stop_sequence(KF_AUDIO_STOP_IMMEDIATE);
        }
        VSync(0);
        opening_poll_input();
        if (opening_input_action != KF_OPENING_INPUT_NONE) {
            break;
        }
        frame++;
    } while (frame < SCENE1_HOLD_FRAMES);

    audio_stop_sequence(KF_AUDIO_STOP_FADE);
    shade = KF_TEXTURE_BASE_BRIGHTNESS;
    do {
        opening_scene1_draw_fade((u8)shade);
        shade -= SCENE1_SHADE_STEP;
    } while (shade >= 0);
}

ADDRESS(0x80014608, 0x1fc)
void opening_entity_transition(KfOpeningTransitionMode mode, const VECTOR *position)
{
    struct KfOpeningTransformSnapshot {
        VECTOR position;
        SVECTOR rotation;
    } transform_snapshot;
    KfOpeningEntity *entity;
    s16 entity_index;
    s16 frame;
    s16 initial_scale_y;
    s16 scale_step;

    switch (mode) {
    case KF_OPENING_TRANSITION_GROW:
        initial_scale_y = 0;
        scale_step = TRANSITION_SCALE_STEP;
        break;
    case KF_OPENING_TRANSITION_REMOVE:
        goto deactivate;
    case KF_OPENING_TRANSITION_SHRINK:
    case KF_OPENING_TRANSITION_CREATE:
        initial_scale_y = TRANSITION_TALL_SCALE_Y;
        scale_step = -TRANSITION_SCALE_STEP;
        break;
    }

    entity = &opening_entity_state.entities[TRANSITION_FIRST_ENTITY_SLOT];
    entity_index = TRANSITION_ENTITY_COUNT - 1;
    /* Retail retains these stack coordinates without a subsequent consumer. */
    transform_snapshot.position.vx = position->vx;
    transform_snapshot.position.vz = position->vz;
    transform_snapshot.position.vy = position->vy;
    do {
        entity->object_id = KF_OPENING_TRANSITION_CYLINDER;
        entity->position = *position;
        entity->rotation.z = 0;
        entity->rotation.y = 0;
        entity->rotation.x = 0;
        entity->scale.vz = KF_FIXED12_ONE;
        entity->scale.vx = KF_FIXED12_ONE;
        entity->scale.vy = initial_scale_y;
        entity++;
        entity_index--;
    } while (entity_index != -1);

    if (mode == KF_OPENING_TRANSITION_CREATE) {
        return;
    }

    frame = 0;
    do {
        entity = &opening_entity_state.entities[TRANSITION_FIRST_ENTITY_SLOT];
        entity_index = 0;
        do {
            if ((entity_index << TRANSITION_ENTITY_DELAY_SHIFT) < frame) {
                u16 scale_y = entity->scale.vy;

                if (scale_y < TRANSITION_TALL_SCALE_Y + 1) {
                    entity->scale.vy = scale_step + scale_y;
                }
            }
            entity_index++;
            entity->rotation.y =
                (entity->rotation.y + TRANSITION_YAW_STEP) & KF_ANGLE_WRAP_MASK;
            entity++;
        } while (entity_index < TRANSITION_ENTITY_COUNT);
        opening_render_frame(0, 0);
        VSync(0);
        frame++;
    } while (frame < TRANSITION_FRAMES);

    if (mode == KF_OPENING_TRANSITION_GROW) {
        return;
    }

deactivate:
    entity = &opening_entity_state.entities[TRANSITION_FIRST_ENTITY_SLOT];
    entity_index = TRANSITION_ENTITY_COUNT - 1;
    do {
        entity->object_id = KF_OPENING_ENTITY_FREE;
        entity++;
        entity_index--;
    } while (entity_index != -1);
}

ADDRESS(0x80014804, 0x330)
void opening_scene3_run(void)
{
    KfOpeningEntity *entity_13;
    KfOpeningEntity *entity_14;
    VECTOR transition_position;
    u32 texture_pages[KF_OPENING_SCENE3_OVERLAY_COUNT];
    /* Retail reserves six CLUT work slots; this scene populates the first two. */
    u32 cluts[SCENE3_CLUT_WORK_CAPACITY];
    KfScreenRect *overlay_rect;
    s16 *overlay_y;
    s16 blend;
    s16 overlay_index;
    s32 wave_angle;

    wave_angle = 0;
    opening_resources_load_scene3();
    texture_pages[0] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[0] = GetClut(0, PANEL_CLUT_FIRST_Y);
    texture_pages[1] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[1] = GetClut(0, PANEL_CLUT_FIRST_Y + 1);

    entity_13 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_SCENE3_INCREASING_YAW_MODEL);
    entity_14 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_SCENE3_DECREASING_YAW_MODEL);
    entity_14->rotation.y = 0;
    entity_13->rotation.y = 0;
    opening_camera_path_begin(opening_scene3_camera_path);

    blend = 0;
    for (;;) {
        lighting_set_color_matrix(
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_BLACK)],
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_DEFAULT)], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_poll_input();
        blend += OPENING_COLOR_FADE_STEP;
        if (blend >= KF_FIXED12_ONE + 1) {
            break;
        }
    }

    while (entity_13->rotation.y < KF_ANGLE_QUARTER_TURN) {
        entity_13->rotation.y += SCENE3_YAW_STEP;
        entity_14->rotation.y -= SCENE3_YAW_STEP;
        audio_set_listener_transform(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_poll_input();
        if (opening_input_action != KF_OPENING_INPUT_NONE) {
            break;
        }
    }

    do {
        opening_camera_path_step(rsin(wave_angle) >> SCENE_CAMERA_WAVE_SHIFT);
        if (opening_camera_path_state.frames_remaining == KF_CAMERA_PATH_FINISHED) {
            break;
        }
        wave_angle = (wave_angle + SCENE_CAMERA_WAVE_ANGLE_STEP) & KF_ANGLE_WRAP_MASK;
        render_set_view_transform(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        display_begin_frame();
        SetGeomScreen(KF_DEFAULT_PROJECTION_DISTANCE);
        opening_render_entities();
        overlay_index = 0;
        overlay_rect = opening_scene3_overlay_rects;
        overlay_y = &overlay_rect->y;
        do {
            /* Retain quads while their signed Y span can still cross the screen. */
            if ((u16)(--*overlay_y + PANEL_CLIP_Y_BIAS) < PANEL_CLIP_SPAN) {
                sprite_add_ft4(
                    overlay_rect,
                    opening_scene3_overlay_uv,
                    texture_pages[overlay_index],
                    cluts[overlay_index],
                    &opening_scene3_overlay_color,
                    PANEL_OT_DEPTH);
            }
            overlay_index++;
            overlay_y += sizeof(*overlay_rect) / sizeof(*overlay_y);
            overlay_rect++;
        } while (overlay_index < KF_OPENING_SCENE3_OVERLAY_COUNT);
        display_present_frame();
        opening_poll_input();
    } while (opening_input_action == KF_OPENING_INPUT_NONE);

    transition_position.vx = opening_camera_path_state.position.vx;
    transition_position.vy = TRANSITION_BASE_Y;
    transition_position.vz = opening_camera_path_state.position.vz;
    if (opening_input_action == KF_OPENING_INPUT_NONE) {
        opening_entity_transition(KF_OPENING_TRANSITION_GROW, &transition_position);
    }

    blend = 0;
    do {
        lighting_set_color_matrix(
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_DEFAULT)],
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_BLACK)], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        blend += OPENING_COLOR_FADE_STEP;
    } while (blend < KF_FIXED12_ONE + 1);

    if (opening_input_action == KF_OPENING_INPUT_NONE) {
        opening_entity_transition(KF_OPENING_TRANSITION_REMOVE, &transition_position);
    }
}

ADDRESS(0x80014b34, 0x2f4)
void opening_ending_scene_run(void)
{
    KfOpeningEntity *entity_13;
    KfOpeningEntity *entity_14;
    VECTOR transition_position;
    s16 brightness;
    s16 blend;
    s32 wave_angle;

    wave_angle = 0;
    opening_resources_load_ending();
    entity_13 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_SCENE3_INCREASING_YAW_MODEL);
    entity_14 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_SCENE3_DECREASING_YAW_MODEL);
    entity_14->rotation.y = 0;
    entity_13->rotation.y = 0;
    opening_camera_path_begin(opening_ending_camera_path);

    transition_position.vy = TRANSITION_BASE_Y;
    transition_position.vx = opening_camera_path_state.position.vx;
    transition_position.vz = opening_camera_path_state.position.vz;
    SetDispMask(1);
    blend = 0;
    opening_entity_transition(KF_OPENING_TRANSITION_CREATE, &transition_position);

    do {
        lighting_set_color_matrix(
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_BLACK)],
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_DEFAULT)], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        blend += OPENING_COLOR_FADE_STEP;
    } while (blend < KF_FIXED12_ONE + 1);

    opening_entity_transition(KF_OPENING_TRANSITION_SHRINK, &transition_position);
    brightness = 0;
    blend = 0;
    for (;;) {
        opening_camera_path_step(rsin(wave_angle) >> SCENE_CAMERA_WAVE_SHIFT);
        if (opening_camera_path_state.frames_remaining == KF_CAMERA_PATH_FINISHED) {
            break;
        }
        if (opening_camera_path_state.point_index >= ENDING_ROTATION_START_POINT) {
            if (entity_13->rotation.y < KF_ANGLE_QUARTER_TURN) {
                entity_13->rotation.y += SCENE3_YAW_STEP;
                entity_14->rotation.y -= SCENE3_YAW_STEP;
                audio_set_listener_transform(
                    &opening_camera_path_state.position,
                    &opening_camera_path_state.rotation);
                opening_render_frame(
                    &opening_camera_path_state.position,
                    &opening_camera_path_state.rotation);
                SetBackColor(brightness, brightness, brightness);
                SetFarColor(brightness, brightness, brightness);
                setRGB0(
                    &open_graphics_runtime.display_draw_environments[0],
                    brightness,
                    brightness,
                    brightness);
                setRGB0(
                    &open_graphics_runtime.display_draw_environments[1],
                    brightness,
                    brightness,
                    brightness);
                if (brightness >= ENDING_MAX_BRIGHTNESS) {
                    brightness = ENDING_MAX_BRIGHTNESS;
                } else {
                    brightness += ENDING_BRIGHTEN_STEP;
                }
                if (blend < 0xfff) {
                    blend += 0x40;
                } else {
                    blend = 0xfff;
                }
            }
        } else {
            wave_angle = (wave_angle + SCENE_CAMERA_WAVE_ANGLE_STEP) & KF_ANGLE_WRAP_MASK;
        }
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
    }

    brightness = ENDING_MAX_BRIGHTNESS;
    blend = 0;
    do {
        if (brightness < 0) {
            brightness = 0;
        }
        SetBackColor(brightness, brightness, brightness);
        SetFarColor(brightness, brightness, brightness);
        setRGB0(
            &open_graphics_runtime.display_draw_environments[0],
            brightness, brightness, brightness);
        setRGB0(
            &open_graphics_runtime.display_draw_environments[1],
            brightness, brightness, brightness);
        lighting_set_color_matrix(
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_DEFAULT)],
            &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_BLACK)], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        blend += OPENING_COLOR_FADE_STEP;
        brightness -= ENDING_FADE_DARKEN_STEP;
    } while (blend < KF_FIXED12_ONE + 1);
}

ADDRESS(0x80014e28, 0x798)
void opening_ending_scroll_run(void)
{
    VECTOR transition_position;
    MATRIX light_matrix = {
        {{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}
    };
    u32 texture_pages[ENDING_PANEL_COUNT];
    u32 cluts[ENDING_PANEL_COUNT];
    CVECTOR top_color;
    CVECTOR bottom_color;
    KfOpeningEntity *entity_26;
    KfOpeningEntity *entity_27;
    KfEndingLightingPhase lighting_phase;
    KfEndingScrollState scroll_state;
    KfEndingSequencePhase sequence_phase;
    s32 sequence_delay;
    s32 sequence_volume;
    s16 lighting_blend;
    s16 background_blend;
    KfEndingScrollTick scroll_tick;
    s16 panel_index;
    KfScreenRect *panel;

    open_graphics_runtime.render_state.light_matrix = light_matrix;
    lighting_blend = 0;
    opening_resources_load_ending_entities();
    texture_pages[0] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[0] = GetClut(0, PANEL_CLUT_FIRST_Y);
    texture_pages[1] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[1] = GetClut(0, PANEL_CLUT_FIRST_Y + 1);
    texture_pages[2] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + 2 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[2] = GetClut(0, PANEL_CLUT_FIRST_Y + 2);
    texture_pages[3] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + 3 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[3] = GetClut(0, PANEL_CLUT_FIRST_Y + 3);
    texture_pages[4] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + 4 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[4] = GetClut(0, PANEL_CLUT_FIRST_Y + 4);
    texture_pages[5] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + 5 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[5] = GetClut(0, PANEL_CLUT_FIRST_Y + 5);
    texture_pages[6] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + 6 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[6] = GetClut(0, PANEL_CLUT_FIRST_Y + 6);
    texture_pages[7] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + 7 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[7] = GetClut(0, PANEL_CLUT_FIRST_Y + 7);
    texture_pages[8] = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        PANEL_TPAGE_FIRST_X + 8 * PANEL_TPAGE_X_STRIDE, KF_TEXTURE_LOWER_PAGE_Y);
    cluts[8] = GetClut(0, PANEL_CLUT_FIRST_Y + 8);

    entity_26 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_ENDING_ORANGE_DISK);
    entity_27 = opening_entity_find_by_object_id(
        opening_entity_state.entities, KF_OPENING_ENDING_STARFIELD);
    entity_26->position.vy -= ENDING_MODEL_START_Y_OFFSET;
    entity_27->object_id = KF_OPENING_ENTITY_FREE;
    opening_camera_path_begin(opening_ending_scroll_camera_path);
    lighting_phase = ENDING_LIGHT_TO_MIDPOINT;
    background_blend = 0;
    sequence_phase = ENDING_SEQUENCE_WAIT_SCROLL;
    scroll_state = ENDING_SCROLL_WAIT_DISK;
    scroll_tick = ENDING_SCROLL_TICK_STARFIELD_AND_PANELS;
    SetFogNear(KF_INITIAL_FOG_NEAR_DISTANCE, KF_DEFAULT_PROJECTION_DISTANCE);
    SetBackColor(0, 0, 0);
    SetFarColor(0, 0, 0);
    open_graphics_runtime.tmd_projection_shift = ENDING_TMD_PROJECTION_SHIFT;
    /* Retail retains this otherwise unconsumed stack-owned position snapshot. */
    transition_position.vy = TRANSITION_BASE_Y;
    transition_position.vx = opening_camera_path_state.position.vx;
    transition_position.vz = opening_camera_path_state.position.vz;
    open_graphics_runtime.floor_item_state.material.color.r = 0;
    open_graphics_runtime.floor_item_state.material.color.g = 0;
    open_graphics_runtime.floor_item_state.material.color.b = 0;
    top_color = opening_ending_scroll_top_start;
    bottom_color = opening_ending_scroll_bottom_start;

    for (;;) {
        switch (lighting_phase) {
        case ENDING_LIGHT_TO_MIDPOINT:
            if (lighting_blend <= KF_FIXED12_ONE) {
                lighting_set_color_matrix(
                    &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_BLACK)],
                    &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_ENDING_MIDPOINT)], lighting_blend);
                lighting_blend += ENDING_LIGHT_MIDPOINT_STEP;
            } else {
                lighting_phase = ENDING_LIGHT_TO_GREEN;
                lighting_blend = 0;
            }
            break;
        case ENDING_LIGHT_TO_GREEN:
            if (lighting_blend <= KF_FIXED12_ONE) {
                lighting_set_color_matrix(
                    &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_ENDING_MIDPOINT)],
                    &color_matrix_table[KF_ENUM_ENCODE(s32, KF_OPEN_COLOR_ENDING_GREEN)], lighting_blend);
                lighting_blend += ENDING_LIGHT_GREEN_STEP;
            } else {
                lighting_phase = ENDING_LIGHT_FINISHED;
                lighting_blend = 0;
            }
            break;
        }
        switch (sequence_phase) {
        case ENDING_SEQUENCE_DELAY:
            if (--sequence_delay == -1) {
                sequence_phase = ENDING_SEQUENCE_FADE;
                sequence_volume = KF_AUDIO_MAX_VOLUME * ENDING_SEQUENCE_VOLUME_DIVISOR;
            }
            break;
        case ENDING_SEQUENCE_FADE:
            --sequence_volume;
            SsSetMVol(sequence_volume / ENDING_SEQUENCE_VOLUME_DIVISOR,
                      sequence_volume / ENDING_SEQUENCE_VOLUME_DIVISOR);
            if (sequence_volume == 0) {
                sequence_phase = ENDING_SEQUENCE_REPLACED;
                opening_resources_load_ending_sequence();
            }
            break;
        }
        if (opening_camera_path_state.frames_remaining != KF_CAMERA_PATH_FINISHED) {
            opening_camera_path_step(0);
        }
        if (entity_26->position.vy < ENDING_MODEL_FINAL_Y) {
            entity_26->position.vy += ENDING_MODEL_Y_STEP;
        } else if (scroll_state == ENDING_SCROLL_WAIT_DISK) {
            entity_26->object_id = KF_OPENING_ENTITY_FREE;
            scroll_state = ENDING_SCROLL_ACTIVE;
            entity_27->object_id = KF_OPENING_ENDING_STARFIELD;
        }
        render_set_view_transform(
            &opening_camera_path_state.position, &opening_camera_path_state.rotation);
        display_begin_frame();
        SetGeomScreen(KF_DEFAULT_PROJECTION_DISTANCE);
        opening_render_entities();

        background_blend += ENDING_BACKGROUND_BLEND_STEP;
        if (background_blend > KF_FIXED12_ONE) {
            background_blend = KF_FIXED12_ONE;
        }
        color_lerp_cvector(&opening_ending_scroll_top_start,
                          &opening_ending_scroll_top_end, &top_color, background_blend);
        color_lerp_cvector(&opening_ending_scroll_bottom_start,
                          &opening_ending_scroll_bottom_end, &bottom_color, background_blend);
        sprite_add_g4(&opening_ending_scroll_backgrounds[0],
                      &top_color, &top_color, &bottom_color, &bottom_color);
        sprite_add_f4(&opening_ending_scroll_backgrounds[1],
                      &opening_ending_scroll_background_color, ENDING_BACKGROUND_OT_DEPTH);

        if (scroll_tick == ENDING_SCROLL_TICK_STARFIELD_AND_PANELS) {
            entity_27->rotation.z = (entity_27->rotation.z - 1) & KF_ANGLE_WRAP_MASK;
            if (entity_27->object_id != KF_OPENING_ENTITY_FREE &&
                open_graphics_runtime.floor_item_state.material.color.r < ENDING_MAX_BRIGHTNESS) {
                ++open_graphics_runtime.floor_item_state.material.color.r;
                open_graphics_runtime.floor_item_state.material.color.b =
                    open_graphics_runtime.floor_item_state.material.color.r;
                open_graphics_runtime.floor_item_state.material.color.g =
                    open_graphics_runtime.floor_item_state.material.color.r;
            }
        }
        if (scroll_state > ENDING_SCROLL_WAIT_DISK) {
            panel_index = 0;
            panel = opening_ending_scroll_panels;
            do {
                if (scroll_tick == ENDING_SCROLL_TICK_STARFIELD_AND_PANELS ||
                    scroll_tick == ENDING_SCROLL_TICK_PANELS) {
                    if ((s16)opening_ending_scroll_panels[ENDING_PANEL_COUNT - 1].y >
                        ENDING_PANEL_STOP_Y) {
                        --panel->y;
                    } else if (sequence_phase == ENDING_SEQUENCE_WAIT_SCROLL) {
                        sequence_phase = ENDING_SEQUENCE_DELAY;
                        sequence_delay = ENDING_SEQUENCE_DELAY_START;
                    }
                }
                if ((u16)(panel->y + PANEL_CLIP_Y_BIAS) < PANEL_CLIP_SPAN) {
                    sprite_add_ft4(panel, opening_ending_scroll_uv,
                                   texture_pages[panel_index], cluts[panel_index],
                                   &opening_ending_scroll_panel_color, PANEL_OT_DEPTH);
                }
                ++panel_index;
                panel++;
            } while (panel_index < ENDING_PANEL_COUNT);
            scroll_tick = KF_ENUM_DECODE(
                KfEndingScrollTick, KF_ENUM_ENCODE(s16, scroll_tick) - 1);
            if (scroll_tick == ENDING_SCROLL_TICK_WRAP) {
                scroll_tick = ENDING_SCROLL_TICK_HOLD_AFTER_STARFIELD;
            }
        }
        display_present_frame();
    }
}
