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

/* OPEN.EXE and GAME.EXE run in turn. GAME returns the next OPEN mode
 * through the result word; the loader always repeats the same order. */
ADDRESS(0x80010028, 0xd0)
void main(void)
{
    struct EXEC header;
    KfOverlayArguments entry_args;

    _96_remove();
    entry_args.result = KF_GAME_EXIT_INTRO;
    for (;;) {
        entry_args.request = KF_ENUM_DECODE(KfOpenMode, KF_ENUM_ENCODE(u32, entry_args.result));
        _96_init();
        if (Load(overlay_path_table[KF_OVERLAY_OPEN_PATH], &header) == 1) {
            _96_remove();
            header.s_addr = 0;
            header.s_size = 0;
            EnterCriticalSection();
            Exec(&header, KF_OVERLAY_ENTRY_ARGC, (char **)&entry_args);
        }
        _96_init();
        if (Load(overlay_path_table[KF_OVERLAY_GAME_PATH], &header) == 1) {
            _96_remove();
            header.s_addr = 0;
            header.s_size = 0;
            EnterCriticalSection();
            Exec(&header, KF_OVERLAY_ENTRY_ARGC, (char **)&entry_args);
        }
    }
}
