#ifndef KF_DEBUG_H
#define KF_DEBUG_H

#include <kf/lib/types.h>

typedef u8 KfFormatDigitState; enum {
    KF_FORMAT_DIGITS_LEADING = 0,
    KF_FORMAT_DIGITS_EMITTED = 1
};

typedef u8 KfFormatParserState; enum {
    KF_FORMAT_PARSER_TEXT = 0,
    KF_FORMAT_PARSER_CONVERSION = 1
};

typedef u8 KfFormatPaddingMode; enum {
    KF_FORMAT_PAD_SPACES = 0,
    KF_FORMAT_PAD_ZEROES = 1
};

enum {
    KF_FORMAT_LEADING_PAD_BYTES = 8,
    KF_FORMAT_DECIMAL_DIGITS = 10,
    KF_FORMAT_DECIMAL_HIGHEST_PLACE = 1000000000,
    KF_FORMAT_HEX_DIGITS = 8,
    KF_FORMAT_HEX_HIGHEST_PLACE = 0x10000000,
    KF_FORMAT_WIDTH_UNSPECIFIED = 0xff
};

extern void debug_printf_sink(const char *format, ...);

#endif
