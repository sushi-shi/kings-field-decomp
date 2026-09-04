#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/psyq_kernel.h>

extern char *overlay_path_table[2];

/* GAME.EXE and OPEN.EXE are run in turn; each returns its successor through the
 * argument block, so the loop never exits. */
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
