#ifndef KF_BOOL_H
#define KF_BOOL_H

#include <kf/lib/types.h>
#include <kf/lib/enum.h>

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

#endif
