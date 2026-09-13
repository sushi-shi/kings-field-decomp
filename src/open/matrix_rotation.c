#include <kf/address.h>
#include <kf/debug.h>
#include <kf/game_math.h>
#include <kf/open_render.h>

RODATA(0x80012038, 0x24)

static const char debug_matrix_label[16] = "Dump Matrix\n";

#include "../shared/matrix_rotation.inc"

#include "../shared/debug_matrix.inc"

#include "../shared/pitch_yaw_to_forward_vector.inc"
