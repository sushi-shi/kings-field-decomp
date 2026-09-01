#include <kf/address.h>
#include <kf/game_types.h>

extern void lighting_set_color_matrix(const struct KfMatrix *from, const struct KfMatrix *to, s32 blend);
extern void func_8001fde4(s32 first, s32 second);
extern void frame_pacer_wait(void);

ADDRESS(0x80033d80, 0x68)
void lighting_transition_color_matrix(const struct KfMatrix *from, const struct KfMatrix *to)
{
    s32 blend = 0;

    do {
        lighting_set_color_matrix(from, to, blend);
        func_8001fde4(0, 0);
        frame_pacer_wait();
        blend += 0x400;
    } while (blend <= 0x1000);
}

ADDRESS(0x80033de8, 0x28)
void color_matrix_set_rgb(s16 red, s16 green, s16 blue, struct KfMatrix *matrix)
{
    matrix->m[0][2] = red;
    matrix->m[0][1] = red;
    matrix->m[0][0] = red;
    matrix->m[1][2] = green;
    matrix->m[1][1] = green;
    matrix->m[1][0] = green;
    matrix->m[2][2] = blue;
    matrix->m[2][1] = blue;
    matrix->m[2][0] = blue;
}
