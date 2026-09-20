#include <kf/lib/bool.h>
#include <stdarg.h>

#include <kf/lib/debug.h>
#include <kf/lib/types.h>

KfBool32 debug_stop_flag = KF_FALSE;

static char format_number_storage[24];

void debug_stop(void)
{
    debug_printf_sink("DEBUG STOP !!!\n");
    debug_stop_flag = debug_stop_flag == KF_FALSE;
}

#include "../lib/format.inc"

#include "../lib/debug_sink.inc"

void func_8003ac4c(void)
{

}


void debug_text_reset_module_state(void)
{
    kf::restore_initial_value<debug_stop_flag>();
    kf::restore_initial_value<format_number_storage>();
}
