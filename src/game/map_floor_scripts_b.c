#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Per-floor script band 0x800342ec..0x8003469f (GAME.EXE), sitting just past
 * the empty floor-4 ambient stub (func_800342e4).
 *
 * func_800342ec is the floor-5 ambient script dispatched by func_8003596c
 * (map_events.c). The remaining routines are the floor-1..3 "action" scripts
 * dispatched by func_80034de4 (map_interaction.c) when the player interacts
 * with the world: they gate on progress flags and the persistent world-state
 * block, copy map regions, teach magic, and run a full-screen colour-matrix
 * fade (map_reveal_fade) that reveals a map event.
 */

extern KfPlayerState player_state;
extern KfActorState actor_state;
extern KfMagicRecord magic_records[24];
extern KfMapEvent map_event_pool[8];
extern KfRenderState render_state;
extern MATRIX color_matrix_table[7];

/* Progress-flag block raised at init and decremented on death restart. */
extern u8 DAT_800652a8[240];
/* Persistent per-floor world-state block. */
extern u8 DAT_8009ddb4[4];
/* One-shot event-fired flags. */
extern u8 DAT_8009f844;
extern u8 DAT_8009f846;
/* Camera-path / positional-audio data block; +0x50 is a colour-matrix target. */
extern u8 DAT_800561c8[0x70];

extern const SoundRef gameplay_sound_ref_7;

extern void screen_show_image_until_input(const char *path);
extern void map_apply_copy_region(u8 region_id);
extern void sound_ref_play(const SoundRef *sound, s16 volume);
extern void lighting_set_color_matrix(const MATRIX *from, const MATRIX *to, s32 blend);
extern void matrix_interpolate(
    const MATRIX *from, const MATRIX *to, MATRIX *matrix, s32 blend);
extern void lighting_set_active_color_matrix(s32 index);
extern void frame_pacer_wait(void);
extern void render_frame(s32 first, s32 second);
extern void notify_enqueue(s32 arg0);

/* The two TIM cut-in paths shown by func_800342ec. */
RODATA(0x80012a54, 0x28)

/* Floor-5 ambient script: a one-time scripted reveal at a fixed cell/heading. */
ADDRESS(0x800342ec, 0xf4)
void func_800342ec(void)
{
    u8 *fired = &DAT_8009f846;

    if (*fired == 0 && player_state.map_cell.x >= 38
        && player_state.map_cell.x < 41 && player_state.map_cell.z == 7
        && (u16)player_state.camera_rotation.vy >= 1808
        && (u16)player_state.camera_rotation.vy < 2289) {
        *fired = 1;
        screen_show_image_until_input("TALK\\C17\\T55171.TIM");
        render_frame(0, 0);
        render_frame(0, 0);
        screen_show_image_until_input("TALK\\C17\\T55172.TIM");
        actor_state.definitions[7].action_animations[2] = 2;
        actor_state.definitions[7].action_animations[8] = 3;
        actor_state.definitions[7].action_animations[9] = 3;
        actor_state.definitions[7].action_animations[10] = 3;
        actor_state.definitions[7].action_animations[11] = 1;
        map_apply_copy_region(4);
    }
}

/* Floor-1 action script: reveal a passage once its progress flag is set. */
ADDRESS(0x800343e0, 0x58)
void func_800343e0(void)
{
    if (DAT_800652a8[0x38] != 0 && DAT_8009ddb4[2] == 0) {
        DAT_8009ddb4[2] = 1;
        map_apply_copy_region(1);
        sound_ref_play(&gameplay_sound_ref_7, 0x64);
    }
}

/* Full-screen colour-matrix fade that reveals map event 3, then fades back. */
ADDRESS(0x80034438, 0x184)
void map_reveal_fade(void)
{
    MATRIX saved;
    s32 blend;

    saved = render_state.light_matrix_copy;

    for (blend = 0; blend < 4097; blend += 128) {
        lighting_set_color_matrix(&color_matrix_table[0], &color_matrix_table[3], blend);
        if (blend >= 1025) {
            map_event_pool[3].position_y -= 130;
            map_event_pool[3].rotation += 128;
        } else {
            matrix_interpolate(&saved, (const MATRIX *)&DAT_800561c8[0x50],
                               &render_state.light_matrix_copy, blend << 2);
        }
        render_frame(0, 0);
        frame_pacer_wait();
    }

    map_event_pool[3].state = 3;
    DAT_8009f844 = 1;

    for (blend = 0x1000; blend >= 0; blend -= 256) {
        lighting_set_color_matrix(&color_matrix_table[0], &color_matrix_table[3], blend);
        render_frame(0, 0);
        frame_pacer_wait();
    }

    lighting_set_active_color_matrix(0);
    render_state.light_matrix_copy = saved;
}

/* Floor-2 action script: run the reveal fade when event 3 is fully open. */
ADDRESS(0x800345bc, 0x54)
void func_800345bc(void)
{
    if ((*(u32 *)&map_event_pool[3].image_limit & 0xffffff00) == 0x28010200
        && map_event_pool[3].state == 1) {
        map_reveal_fade();
    }
}

/* Floor-3 action script: teach two spells gated on flags and event state. */
ADDRESS(0x80034610, 0x90)
void func_80034610(void)
{
    if (DAT_800652a8[0x32] != 0) {
        if (magic_records[7].learned == 0) {
            magic_records[7].learned = 1;
            notify_enqueue(1);
        }
    }
    if ((*(u32 *)&map_event_pool[1].image_limit & 0xffffff00) == 0x28010300) {
        if (magic_records[5].learned == 0) {
            magic_records[5].learned = 1;
            notify_enqueue(1);
        }
    }
}
