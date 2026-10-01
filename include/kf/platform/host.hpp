#pragma once
#include <kf/lib/types.h>
#include <cstddef>
namespace kf {
struct Renderer;
struct FaceList;
struct DrawFace;
struct FrameStyle;
enum class AppMode : u8 { Opening, Gameplay, Ending, Exit };
struct UpdatePacer {
    std::uint64_t deadline_ns;
    u32 steps_per_second;
    u32 fractional_ns;
};
bool host_start();
void host_shutdown();
void host_poll();
void host_set_session_running(bool running);
void host_online_room(const char *code, bool hosting);
void host_online_status(const char *message);
void host_notice(const char *message);
std::uint64_t host_clock_ns();
std::uint64_t host_clock_tick();
void host_wait_until_tick(std::uint64_t deadline);
void host_wait_frame();
UpdatePacer host_begin_update_pacer(u32 steps_per_second);
void host_wait_update(UpdatePacer &pacer);
Renderer *host_renderer();
void host_present_faces(const FaceList *faces);
void host_begin_frame();
void host_enqueue_face(const DrawFace &face);
void host_present_frame(const FrameStyle &style);
[[noreturn]] void host_fail(const char *message);
}
