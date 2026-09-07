#include <kf/address.h>
#include <kf/overlay.h>
#include <kf/game_types.h>
#include <kf/psyq_kernel.h>

RODATA(0x80010000, 0x28)

const char overlay_path_GAME[20] = "cdrom:GAME.EXE;1";
const char overlay_path_OPEN[20] = "cdrom:OPEN.EXE;1";

DATA(0x80010224, 0x8)
const char *overlay_path_table[KF_OVERLAY_PATH_COUNT] = {
    overlay_path_OPEN,
    overlay_path_GAME,
};

/* OPEN.EXE and GAME.EXE run in turn. GAME returns the next OPEN mode
 * through the result word; the loader always repeats the same order. */
ADDRESS(0x80010028, 0xd0)
void main(void)
{
    struct EXEC header;
    s32 entry_args[KF_OVERLAY_ARGUMENT_WORDS];

    _96_remove();
    entry_args[KF_OVERLAY_RESULT_WORD] = KF_ENUM_ENCODE(s32, KF_OPEN_MODE_INTRO);
    for (;;) {
        entry_args[KF_OVERLAY_REQUEST_WORD] = entry_args[KF_OVERLAY_RESULT_WORD];
        _96_init();
        if (Load(overlay_path_table[KF_OVERLAY_OPEN_PATH], &header) == 1) {
            _96_remove();
            header.s_addr = 0;
            header.s_size = 0;
            EnterCriticalSection();
            Exec(&header, KF_OVERLAY_ENTRY_ARGC, (char **)entry_args);
        }
        _96_init();
        if (Load(overlay_path_table[KF_OVERLAY_GAME_PATH], &header) == 1) {
            _96_remove();
            header.s_addr = 0;
            header.s_size = 0;
            EnterCriticalSection();
            Exec(&header, KF_OVERLAY_ENTRY_ARGC, (char **)entry_args);
        }
    }
}
