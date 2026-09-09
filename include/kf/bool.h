#ifndef KF_BOOL_H
#define KF_BOOL_H

#include <kf/game_types.h>
#include <kf/enum.h>

/* Preserve the retail integer representation while checking Boolean writes. */
#if KF_MODERN_TYPES
template <typename Storage>
    requires (__is_integral(Storage))
class KfBoolStorage {
public:
    KfBoolStorage() = default;
    constexpr KfBoolStorage(bool value) : value_(static_cast<Storage>(value)) {}

    template <typename Other>
    constexpr KfBoolStorage(KfBoolStorage<Other> value)
        : KfBoolStorage(static_cast<bool>(value)) {}

    template <typename Value>
        requires (!__is_same(Value, bool))
    KfBoolStorage(Value) = delete;

    constexpr operator bool() const { return value_ != 0; }

    constexpr KfBoolStorage& operator=(bool value)
    {
        value_ = static_cast<Storage>(value);
        return *this;
    }

    template <typename Other>
    constexpr KfBoolStorage& operator=(KfBoolStorage<Other> value)
    {
        return *this = static_cast<bool>(value);
    }

    template <typename Value>
        requires (!__is_same(Value, bool))
    KfBoolStorage& operator=(Value) = delete;

private:
    Storage value_;
};

typedef KfBoolStorage<int> KfBool;
typedef KfBoolStorage<s32> KfBool32;
typedef KfBoolStorage<u32> KfBoolU32;
typedef KfBoolStorage<u8> KfBool8;
typedef KfBoolStorage<u16> KfBool16;
#define KF_FALSE false
#define KF_TRUE true
#else
/* int and long are distinct source types even though both use an O32 word. */
typedef int KfBool;
typedef s32 KfBool32;
typedef u32 KfBoolU32;
typedef u8 KfBool8;
typedef u16 KfBool16;
#define KF_FALSE 0
#define KF_TRUE 1
#endif

#endif
