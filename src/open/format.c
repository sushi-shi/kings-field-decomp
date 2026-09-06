#include <kf/address.h>
#include <kf/debug.h>
#include <kf/game_types.h>

enum {
    FORMAT_LEADING_PAD_BYTES = 7
};

/* Minimum accessed span: seven leading pad bytes and twelve number bytes.
 * The original allocation's outer bounds remain unresolved. */
DATA(0x80037971, 0x13)
static char format_number_storage[19];

ADDRESS(0x8001a3fc, 0xe0)
char *format_int_dec(s32 value)
{
    s32 divisor = KF_FORMAT_DECIMAL_HIGHEST_PLACE;
    char *out = (format_number_storage + FORMAT_LEADING_PAD_BYTES);
    u8 started = 0;
    u8 i;

    if (value < 0) {
        *out++ = '-';
        value = -value;
    }
    for (i = 0; i < KF_FORMAT_DECIMAL_DIGITS; i++) {
        s32 digit = value / divisor;
        value = value % divisor;
        if (digit != 0 || started != 0 || i == KF_FORMAT_DECIMAL_DIGITS - 1) {
            *out++ = digit + '0';
            started = 1;
        }
        divisor /= 10;
    }
    *out = '\0';
    return (format_number_storage + FORMAT_LEADING_PAD_BYTES);
}

ADDRESS(0x8001a4dc, 0x8c)
char *format_int_hex(u32 value)
{
    u32 divisor = KF_FORMAT_HEX_HIGHEST_PLACE;
    u8 started = 0;
    char *out = format_number_storage + FORMAT_LEADING_PAD_BYTES;
    u8 i;

    for (i = 0; i < KF_FORMAT_HEX_DIGITS; i++) {
        u32 digit = value / divisor;
        value = value % divisor;
        if (digit != 0 || started != 0 || i == KF_FORMAT_HEX_DIGITS - 1) {
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
    return format_number_storage + FORMAT_LEADING_PAD_BYTES;
}

ADDRESS(0x8001a568, 0x6c)
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

ADDRESS(0x8001a5d4, 0x240)
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
            width = KF_FORMAT_WIDTH_UNSPECIFIED;
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
            if (width != KF_FORMAT_WIDTH_UNSPECIFIED) {
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
