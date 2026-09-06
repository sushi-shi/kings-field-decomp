#ifndef KF_DEBUG_H
#define KF_DEBUG_H

/* Custom diagnostic formatter and sink shared by GAME.EXE and OPEN.EXE. */
enum {
    KF_FORMAT_DECIMAL_DIGITS = 10,
    KF_FORMAT_DECIMAL_HIGHEST_PLACE = 1000000000,
    KF_FORMAT_HEX_DIGITS = 8,
    KF_FORMAT_HEX_HIGHEST_PLACE = 0x10000000,
    KF_FORMAT_WIDTH_UNSPECIFIED = 0xff
};

extern void debug_printf_sink(const char *format, ...);

#endif
