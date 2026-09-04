#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/psyq_kernel.h>

RODATA(0x80010000, 0x28)

const char overlay_path_GAME[20] = "cdrom:GAME.EXE;1";
const char overlay_path_OPEN[20] = "cdrom:OPEN.EXE;1";

DATA(0x80010224, 0x8)
const char *overlay_path_table[2] = {
    overlay_path_OPEN,
    overlay_path_GAME,
};

/* OPEN.EXE and GAME.EXE are run in turn; each returns its successor through
 * the argument block, so the loop never exits. */
ADDRESS(0x80010028, 0xd0)
void main(void)
{
    struct EXEC header;
    s32 entry_args[2];

    _96_remove();
    entry_args[1] = 1;
    for (;;) {
        entry_args[0] = entry_args[1];
        _96_init();
        if (Load(overlay_path_table[0], &header) == 1) {
            _96_remove();
            header.s_addr = 0;
            header.s_size = 0;
            EnterCriticalSection();
            Exec(&header, 1, (char **)entry_args);
        }
        _96_init();
        if (Load(overlay_path_table[1], &header) == 1) {
            _96_remove();
            header.s_addr = 0;
            header.s_size = 0;
            EnterCriticalSection();
            Exec(&header, 1, (char **)entry_args);
        }
    }
}
