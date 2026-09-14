#ifndef KF_DEBUG_H
#define KF_DEBUG_H

#include <kf/lib/types.h>
#include <kf/lib/enum.h>

/* Custom diagnostic formatter and sink shared by GAME.EXE and OPEN.EXE. */
KF_ENUM_BEGIN(KfFormatDigitState, u8)
    KF_FORMAT_DIGITS_LEADING = 0,
    KF_FORMAT_DIGITS_EMITTED = 1
KF_ENUM_END(KfFormatDigitState)

KF_ENUM_BEGIN(KfFormatParserState, u8)
    KF_FORMAT_PARSER_TEXT = 0,
    KF_FORMAT_PARSER_CONVERSION = 1
KF_ENUM_END(KfFormatParserState)

KF_ENUM_BEGIN(KfFormatPaddingMode, u8)
    KF_FORMAT_PAD_SPACES = 0,
    KF_FORMAT_PAD_ZEROES = 1
KF_ENUM_END(KfFormatPaddingMode)

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
