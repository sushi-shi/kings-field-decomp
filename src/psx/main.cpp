#include <kf/lib/overlay.h>
#include <kf/lib/types.h>
#include <psyq/kernel.h>

const char overlay_path_GAME[20] = "cdrom:GAME.EXE;1";
const char overlay_path_OPEN[20] = "cdrom:OPEN.EXE;1";

const char *overlay_path_table[KF_OVERLAY_PATH_COUNT] = {
    overlay_path_OPEN,
    overlay_path_GAME,
};

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

extern "C" void main(void)
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
