#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfRenderState render_state;

extern KfPlayerState player_state;

extern void lighting_set_color_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);
extern void matrix_interpolate(
    const MATRIX *from, const MATRIX *to, MATRIX *output, s32 blend);
extern void fog_interpolate_near(s32 start, s32 end, s32 ratio);
extern void player_update_vertical_motion(void);
extern void func_8001fde4(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void player_death_restart(void);
extern void player_death_apply_visual_fade(const MATRIX *color_from, s32 blend);

extern MATRIX color_matrix_table[7];
extern s32 player_death_saved_fog_near;
extern u8 DAT_80095064;
extern MATRIX player_death_saved_color_matrix;

/*
 * Fades the colour matrix and near fog from `color_from` toward the death
 * palette (table entry 4) and darkens the 8-bit brightness in step.
 */
ADDRESS(0x800184b0, 0x90)
void player_death_apply_visual_fade(const MATRIX *color_from, s32 blend)
{
    lighting_set_color_matrix(color_from, &color_matrix_table[4], blend);
    matrix_interpolate(&color_matrix_table[3], &color_matrix_table[4], &render_state.unknown_80, blend);
    fog_interpolate_near(player_death_saved_fog_near, 0, blend);
    DAT_80095064 = ((blend * -86) >> 12) + 86;
}

ADDRESS(0x80018540, 0x184)
void player_death_update(void)
{
    s16 *bob = &player_state.view_bob_offset;
    s32 previous = *bob;
    s32 blend;

    if (previous >= 1000) {
        *bob = 1000;
        *bob = 1060;
        player_state.camera_rotation.vx -= player_state.death_camera_pitch_step;
        player_state.death_camera_pitch_step += 10;
    } else {
        player_state.camera_rotation.vx -= 10;
        player_state.death_camera_pitch_step += 15;
        *bob = previous + player_state.death_camera_pitch_step;
        if (*bob >= 1000) {
            player_state.death_camera_pitch_step = 10;
        }
    }
    if (player_state.camera_rotation.vx < -800) {
        player_state.camera_rotation.vx = -800;
        player_state.death_camera_pitch_step = 0;
    }
    player_state.camera_position.vy = *bob - 1500 + player_state.floor_height;
    player_update_vertical_motion();
    player_state.death_visual_blend += 100;
    blend = player_state.death_visual_blend;
    if (blend >= 0x1000) {
        player_death_apply_visual_fade(&player_death_saved_color_matrix, 0x1000);
        func_8001fde4(0, 0);
        func_8001fde4(0, 0);
        player_death_restart();
    } else {
        player_death_apply_visual_fade(&player_death_saved_color_matrix, blend);
    }
}

ADDRESS(0x800186c4, 0xe0)
void player_death_update_reverse_fade(void)
{
    s16 *blend = &player_state.death_visual_blend;

    lighting_set_color_matrix(&color_matrix_table[4], &color_matrix_table[0], *blend);
    matrix_interpolate(&color_matrix_table[4], &color_matrix_table[3], &render_state.unknown_80, *blend);
    fog_interpolate_near(0, player_death_saved_fog_near, *blend);
    DAT_80095064 = (*blend * 86) >> 12;
    *blend += 100;
    if (*blend >= 0x1000) {
        player_death_apply_visual_fade(&color_matrix_table[0], 0);
        player_state.update_state = 0;
    } else {
        player_death_apply_visual_fade(&color_matrix_table[0], 0x1000 - *blend);
    }
}
