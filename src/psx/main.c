#include <kf/address.h>
#include <kf/overlay.h>
#include <kf/game_types.h>
#include <psyq/kernel.h>

RODATA(0x80010000, 0x28)

const char overlay_path_GAME[20] = "cdrom:GAME.EXE;1";
const char overlay_path_OPEN[20] = "cdrom:OPEN.EXE;1";

DATA(0x80010224, 0x8)
const char *overlay_path_table[KF_OVERLAY_PATH_COUNT] = {
    overlay_path_OPEN,
    overlay_path_GAME,
};

/* Caller-owned EXEC and argument lvalues persist across overlay returns. */
#define OVERLAY_LAUNCH(path_index, header, args) do { \
    _96_init(); \
    if (Load(overlay_path_table[(path_index)], &(header)) == 1) { \
        _96_remove(); \
        (header).s_addr = 0; \
        (header).s_size = 0; \
        EnterCriticalSection(); \
        Exec(&(header), KF_OVERLAY_ENTRY_ARGC, (char **)&(args)); \
    } \
} while (0)

/* OPEN.EXE and GAME.EXE run in turn. GAME returns the next OPEN mode
 * through the result word; the loader always repeats the same order. */
ADDRESS(0x80010028, 0xd0)
void main(void)
{
    struct EXEC header;
    KfOverlayArguments entry_args;

    _96_remove();
    entry_args.result = KF_OVERLAY_MODE_INTRO;
    for (;;) {
        entry_args.request = entry_args.result;
        OVERLAY_LAUNCH(KF_OVERLAY_OPEN_PATH, header, entry_args);
        OVERLAY_LAUNCH(KF_OVERLAY_GAME_PATH, header, entry_args);
    }
}
