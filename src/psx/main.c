#include <kf/address.h>
#include <kf/game_types.h>

/*
 * Psy-Q executable header consumed by Load/Exec. The Release 2.5 headers in
 * the toolchain bundle do not declare it; this is the LIBAPI layout (15
 * words, 0x3c bytes).
 */
struct EXEC {
    u32 pc0;
    u32 gp0;
    u32 t_addr;
    u32 t_size;
    u32 d_addr;
    u32 d_size;
    u32 b_addr;
    u32 b_size;
    u32 s_addr;
    u32 s_size;
    u32 sp;
    u32 fp;
    u32 gp;
    u32 ret;
    u32 base;
};

typedef char EXEC_size_is_60[(sizeof(struct EXEC) == 0x3c) ? 1 : -1];

extern char *overlay_path_table[2];
extern s32 _96_init(void);
extern s32 _96_remove(void);
extern s32 Load(const char *name, struct EXEC *header);
extern s32 Exec(struct EXEC *header, s32 argc, char **argv);
extern void EnterCriticalSection(void);

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
