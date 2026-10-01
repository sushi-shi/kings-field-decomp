#pragma once

#include <coroutine>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <utility>

namespace kf {

// A task yields to its owner, never to a platform wait that runs gameplay.
// Each root has its own continuation and deadline; nested menus retain their
// locals across frames without retaining an active C++ call stack.
struct FrameContinuation {
    std::coroutine_handle<> leaf {};
    unsigned delay {};
};

struct FramePromise {
    FrameContinuation local {};
    FrameContinuation *schedule = &local;
    std::coroutine_handle<> caller = std::noop_coroutine();

    std::suspend_always initial_suspend() const noexcept { return {}; }
    struct Complete {
        bool await_ready() const noexcept { return false; }
        template<class P>
        std::coroutine_handle<> await_suspend(std::coroutine_handle<P> task) const noexcept {
            return task.promise().caller;
        }
        void await_resume() const noexcept {}
    };
    Complete final_suspend() const noexcept { return {}; }
    [[noreturn]] void unhandled_exception() const noexcept { std::abort(); }
};

template<class T> struct FrameResult : FramePromise {
    std::optional<T> result;
    void return_value(T value) { result.emplace(std::move(value)); }
};

template<> struct FrameResult<void> : FramePromise {
    void return_void() const noexcept {}
};

template<class T = void> class [[nodiscard]] FrameTask {
public:
    struct promise_type : FrameResult<T> {
        FrameTask get_return_object() noexcept {
            return FrameTask(std::coroutine_handle<promise_type>::from_promise(*this));
        }
    };
    using Handle = std::coroutine_handle<promise_type>;

    FrameTask() = default;
    FrameTask(const FrameTask &) = delete;
    FrameTask &operator=(const FrameTask &) = delete;
    FrameTask(FrameTask &&other) noexcept : handle(std::exchange(other.handle, {})) {}
    FrameTask &operator=(FrameTask &&other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = std::exchange(other.handle, {});
        }
        return *this;
    }
    ~FrameTask() { if (handle) handle.destroy(); }

    bool done() const noexcept { return !handle || handle.done(); }
    void advance() {
        if (done()) return;
        auto &schedule = handle.promise().local;
        if (schedule.delay && --schedule.delay) return;
        const auto next = schedule.leaf ? schedule.leaf : handle;
        next.resume();
    }

    bool await_ready() const noexcept { return done(); }
    template<class P>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<P> parent) noexcept {
        handle.promise().schedule = parent.promise().schedule;
        handle.promise().caller = parent;
        return handle;
    }
    T await_resume() {
        if constexpr (!std::is_void_v<T>) return std::move(*handle.promise().result);
    }

private:
    explicit FrameTask(Handle value) : handle(value) {}
    Handle handle {};
};

struct FrameDelay {
    unsigned ticks = 1;
    bool await_ready() const noexcept { return ticks == 0; }
    template<class P>
    void await_suspend(std::coroutine_handle<P> task) const noexcept {
        auto &schedule = *task.promise().schedule;
        schedule.leaf = task;
        schedule.delay = ticks;
    }
    void await_resume() const noexcept {}
};

}
