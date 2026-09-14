#ifndef KF_GAME_SYSTEM_H
#define KF_GAME_SYSTEM_H

/* Top-level GAME.EXE lifecycle and frame-pacing interface. */

extern void frame_pacer_vsync_callback(void);
extern void frame_pacer_wait(void);
extern void game_main_loop(void);
extern void game_shutdown(void);

#endif
