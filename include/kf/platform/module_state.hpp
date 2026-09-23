#pragma once
#include <cstddef>
#include <type_traits>

namespace kf {
template <typename T>
void copy_module_value(T &destination, const T &source)
{
    if constexpr (std::is_array_v<T>) {
        for (std::size_t i = 0; i < std::extent_v<T>; ++i)
            copy_module_value(destination[i], source[i]);
    } else {
        static_assert(std::is_copy_assignable_v<T>);
        destination = source;
    }
}

// Owners call this before their first game/opening entry, then on each re-entry.
// Keep one source of truth for authored initializers, including native pointers.
// This is an in-process object copy, not a save format or a PS1 memory image.
template <auto &Object>
void restore_initial_value()
{
    using ObjectType = std::remove_reference_t<decltype(Object)>;
    static_assert(!std::is_const_v<ObjectType> && !std::is_volatile_v<ObjectType>);
    struct Snapshot { ObjectType value; };
    static const Snapshot initial = [] {
        Snapshot snapshot {};
        copy_module_value(snapshot.value, Object);
        return snapshot;
    }();
    copy_module_value(Object, initial.value);
}
}
