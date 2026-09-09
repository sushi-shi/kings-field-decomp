#include <kf/bool.h>
#include <kf/address.h>
#include <kf/debug.h>
#include <kf/game_types.h>

/*
 * The formatter consumes O32 word argument slots and returns a byte count
 * including the terminating NUL. The diagnostic sink currently emits nothing.
 * Left padding writes before the numeric scratch anchor; the enclosing GAME
 * allocation bounds remain unresolved, so the anchor claim is not a proof of
 * backing storage for those preceding bytes.
 */

/* "DEBUG STOP !!!" literal owned by this unit in the shared rodata pool. */
RODATA(0x80012dd4, 0x10)

DATA(0x80057b98, 0x4)
KfBool32 debug_stop_flag = KF_FALSE;

DATA(0x800598a8, 0x18)
static char format_number_buffer[24];

ADDRESS(0x8003a7dc, 0x40)
void debug_stop(void)
{
    debug_printf_sink("DEBUG STOP !!!\n");
    debug_stop_flag = debug_stop_flag == KF_FALSE;
}

ADDRESS(0x8003a81c, 0xe0)
char *format_int_dec(s32 value)
{
    s32 divisor = KF_FORMAT_DECIMAL_HIGHEST_PLACE;
    char *out = format_number_buffer;
    KfFormatDigitState digit_state = KF_FORMAT_DIGITS_LEADING;
    u8 i;

    if (value < 0) {
        *out++ = '-';
        value = -value;
    }
    for (i = 0; i < KF_FORMAT_DECIMAL_DIGITS; i++) {
        s32 digit = value / divisor;
        value = value % divisor;
        if (digit != 0 || digit_state != KF_FORMAT_DIGITS_LEADING || i == KF_FORMAT_DECIMAL_DIGITS - 1) {
            *out++ = digit + '0';
            digit_state = KF_FORMAT_DIGITS_EMITTED;
        }
        divisor /= 10;
    }
    *out = '\0';
    return format_number_buffer;
}

ADDRESS(0x8003a8fc, 0x8c)
char *format_int_hex(u32 value)
{
    u32 divisor = KF_FORMAT_HEX_HIGHEST_PLACE;
    KfFormatDigitState digit_state = KF_FORMAT_DIGITS_LEADING;
    char *out = format_number_buffer;
    u8 i;

    for (i = 0; i < KF_FORMAT_HEX_DIGITS; i++) {
        u32 digit = value / divisor;
        value = value % divisor;
        if (digit != 0 || digit_state != KF_FORMAT_DIGITS_LEADING || i == KF_FORMAT_HEX_DIGITS - 1) {
            if (digit < 10) {
                *out++ = digit + '0';
            } else {
                *out++ = digit + ('A' - 10);
            }
            digit_state = KF_FORMAT_DIGITS_EMITTED;
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
    KfFormatParserState parser_state = KF_FORMAT_PARSER_TEXT;
    u8 width;
    KfFormatPaddingMode padding_mode;
    u8 c;
    char *s;

    while ((c = *format++) != 0) {
        if (c >= '1' && c <= '8') {
            if (parser_state != KF_FORMAT_PARSER_TEXT) {
                width = c - '0';
                continue;
            }
            goto literal;
        }
        switch (c) {
        case '%':
            parser_state = KF_FORMAT_PARSER_CONVERSION;
            padding_mode = KF_FORMAT_PAD_SPACES;
            width = KF_FORMAT_WIDTH_UNSPECIFIED;
            continue;
        case '0':
            if (parser_state == KF_FORMAT_PARSER_TEXT) {
                goto literal;
            }
            padding_mode = KF_FORMAT_PAD_ZEROES;
            continue;
        case 'D':
        case 'd':
            if (parser_state == KF_FORMAT_PARSER_TEXT) {
                goto literal;
            }
            parser_state = KF_FORMAT_PARSER_TEXT;
            s = format_int_dec(*args++);
        emit_padded:
            if (width != KF_FORMAT_WIDTH_UNSPECIFIED) {
                if (padding_mode == KF_FORMAT_PAD_SPACES) {
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
            if (parser_state == KF_FORMAT_PARSER_TEXT) {
                goto literal;
            }
            parser_state = KF_FORMAT_PARSER_TEXT;
            s = format_int_hex(*args++);
            goto emit_padded;
        case 'S':
        case 's':
            if (parser_state == KF_FORMAT_PARSER_TEXT) {
                goto literal;
            }
            parser_state = KF_FORMAT_PARSER_TEXT;
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
void debug_printf_sink(const char *format, ...)
{
}

ADDRESS(0x8003ac4c, 0x8)
void func_8003ac4c(void)
{
    /* Unreferenced return stub at the game/runtime boundary. */
}
