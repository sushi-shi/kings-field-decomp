#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/game_render.h>
#include <kf/game.h>
#include <kf/shared_graphics.h>

/*
 * matrix_interpolate, lighting_set_color_matrix, and lighting_set_light_matrix
 * are identical in GAME.EXE and OPEN.EXE and live in src/shared/matrix_lerp.c.
 */

#include "../shared/fog.inc"
