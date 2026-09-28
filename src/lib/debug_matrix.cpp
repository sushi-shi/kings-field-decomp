#include <kf/platform/prelude.hpp>
#include <kf/lib/debug.h>
#include <kf/lib/math.h>

static const char debug_matrix_label[16] = "Dump Matrix\n";

void debug_dump_matrix(const MATRIX *matrix)
{
    debug_printf_sink(debug_matrix_label);
    debug_printf_sink("  [%05d,%05d,%05d]\n",
        matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);
    debug_printf_sink("  [%05d,%05d,%05d]\n",
        matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);
    debug_printf_sink("  [%05d,%05d,%05d]\n",
        matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);
}
