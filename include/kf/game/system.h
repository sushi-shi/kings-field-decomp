#ifndef KF_GAME_SYSTEM_H
#define KF_GAME_SYSTEM_H

struct WorldState;

struct PlayerContext;

kf::FrameTask<void> game_wait_frame();
kf::FrameTask<void> game_wait_buttons_released(u32 mask = ~u32(0));
kf::FrameTask<void> game_wait_button_press();
kf::FrameTask<void> game_present_frame(const KfDisplayState &display);

extern kf::FrameTask<void> frame_pacer_wait(void);
extern kf::FrameTask<void> game_main_loop(WorldState &world, PlayerContext &player);
extern kf::FrameTask<void> coop_game_loop(WorldState &world, PlayerContext &player);
extern void game_shutdown(void);

#endif
