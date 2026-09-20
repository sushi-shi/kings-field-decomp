#include <stdarg.h>

#include <kf/lib/debug.h>
#include <kf/lib/types.h>

static char format_number_storage[24];

#include "../lib/format.inc"


void format_reset_module_state(void)
{
    kf::restore_initial_value<format_number_storage>();
}
