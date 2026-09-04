#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/game.h>

/*
 * Custom text-formatting / debug band 0x8003a7dc..0x8003ac4b (GAME.EXE).
 *
 * This translation unit is From's own minimal formatted-output library; the
 * Sony LIBAPI printf (0x8005032c) and LIBGPU sprintf (0x80054d5c) live
 * elsewhere as vendored objects. Nothing in the linked GAME.EXE image reaches
 * these functions statically (debug_stop has no callers; format_vsprintf and
 * the itoa helpers are only reached from within this unit), so the band is
 * debug/diagnostic scaffolding retained by the build.
 *
 *   debug_stop        prints "DEBUG STOP !!!" and toggles debug_stop_flag.
 *   format_int_dec    signed base-10 conversion into format_number_buffer.
 *   format_int_hex    unsigned 8-digit base-16 conversion into the same buffer.
 *   format_pad_left   left-pads a string to a byte width, prepending in place.
 *   format_vsprintf   printf-style engine: %d/%D, %x/%X, %s/%S, %<1-8> width,
 *                     %0<width> zero-pad, and '\n' emitted as '\r'. The third
 *                     argument is the base of the caller's 4-byte argument
 *                     slots (a va_list-style pointer).
 *   debug_print       variadic diagnostic sink; in this build it only spills
 *                     its argument registers and returns (output disabled).
 *
 * format_number_buffer is anchored at the address the itoa helpers reference
 * (0x800598a8). format_pad_left prepends field padding by decrementing below
 * that anchor, so the retail object reserved leading slack ahead of it; only
 * the referenced anchor is evidenced, so the claim starts there (the padding
 * underflow stays inside the free BSS gap above audio_sequence_table).
 */

/* "DEBUG STOP !!!" literal owned by this unit in the shared rodata pool. */
RODATA(0x80012dd4, 0x10)

DATA(0x80057b98, 0x4)
s32 debug_stop_flag = 0;

DATA(0x800598a8, 0x18)
static char format_number_buffer[24];

ADDRESS(0x8003a7dc, 0x40)
void debug_stop(void)
{
    debug_print("DEBUG STOP !!!\n");
    debug_stop_flag = debug_stop_flag == 0;
}

ADDRESS(0x8003a81c, 0xe0)
char *format_int_dec(s32 value)
{
    s32 divisor = 1000000000;
    char *out = format_number_buffer;
    u8 started = 0;
    u8 i;

    if (value < 0) {
        *out++ = '-';
        value = -value;
    }
    for (i = 0; i < 10; i++) {
        s32 digit = value / divisor;
        value = value % divisor;
        if (digit != 0 || started != 0 || i == 9) {
            *out++ = digit + '0';
            started = 1;
        }
        divisor /= 10;
    }
    *out = '\0';
    return format_number_buffer;
}

ADDRESS(0x8003a8fc, 0x8c)
char *format_int_hex(u32 value)
{
    u32 divisor = 0x10000000;
    u8 started = 0;
    char *out = format_number_buffer;
    u8 i;

    for (i = 0; i < 8; i++) {
        u32 digit = value / divisor;
        value = value % divisor;
        if (digit != 0 || started != 0 || i == 7) {
            if (digit < 10) {
                *out++ = digit + '0';
            } else {
                *out++ = digit + ('A' - 10);
            }
            started = 1;
        }
        divisor >>= 4;
    }
    *out = '\0';
    return format_number_buffer;
}

ADDRESS(0x8003a988, 0x6c)
char *format_pad_left(char *string, char pad, u8 width)
{
    u8 len = 0;
    char *p = string + 1;

    if (*string != 0) {
        do {
            len++;
        } while (*p++ != 0);
    }
    if (len < width) {
        width -= len;
        while (width-- != 0) {
            *--string = pad;
        }
    }
    return string;
}

ADDRESS(0x8003a9f4, 0x240)
s32 format_vsprintf(u8 *out, u8 *format, s32 *args)
{
    s32 count = 0;
    u8 in_format = 0;
    u8 width;
    u8 zero_pad;
    u8 c;
    char *s;

    while ((c = *format++) != 0) {
        if (c >= '1' && c <= '8') {
            if (in_format != 0) {
                width = c - '0';
                continue;
            }
            goto literal;
        }
        switch (c) {
        case '%':
            in_format = 1;
            zero_pad = 0;
            width = 0xff;
            continue;
        case '0':
            if (in_format == 0) {
                goto literal;
            }
            zero_pad = 1;
            continue;
        case 'D':
        case 'd':
            if (in_format == 0) {
                goto literal;
            }
            in_format = 0;
            s = format_int_dec(*args++);
        emit_padded:
            if (width != 0xff) {
                if (zero_pad == 0) {
                    s = format_pad_left(s, ' ', width);
                } else {
                    s = format_pad_left(s, '0', width);
                }
            }
        copy:
            while ((c = *s++) != 0) {
                *out++ = c;
                count++;
            }
            continue;
        case 'X':
        case 'x':
            if (in_format == 0) {
                goto literal;
            }
            in_format = 0;
            s = format_int_hex(*args++);
            goto emit_padded;
        case 'S':
        case 's':
            if (in_format == 0) {
                goto literal;
            }
            in_format = 0;
            s = (char *)*args++;
            goto copy;
        case '\n':
            *out++ = '\r';
            count++;
            continue;
        default:
        literal:
            *out++ = c;
            count++;
            continue;
        }
    }
    *out = '\0';
    return count + 1;
}

ADDRESS(0x8003ac34, 0x18)
void debug_print(int __builtin_va_alist)
{
}

ADDRESS(0x8003ac4c, 0x8)
void func_8003ac4c(void)
{
}
