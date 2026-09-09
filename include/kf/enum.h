#ifndef KF_ENUM_H
#define KF_ENUM_H

/* Modern checking keeps enum domains; the retail C view keeps their ABI. */
#if defined(__cplusplus) && __cplusplus >= 202002L
#define KF_MODERN_TYPES 1
#define KF_ENUM_BEGIN(name, storage) enum class name : storage {
#define KF_ENUM_END(name) }; using enum name;
/* Keep the domain when legacy C promotes the underlying integer value. */
#define KF_ENUM_PROMOTED(name) name
#define KF_ENUM_STORAGE(name, storage) KfEnumStorage<name, storage>
#define KF_ENUM_PARAM(name, storage) name

/* Opt in only for state bytes that retail also advances as counters.
 * Increment/decrement keep the domain; integer assignment remains invalid. */
#define KF_ENUM_COUNTER(name, storage) \
    inline name& operator++(name& value) \
    { value = static_cast<name>(static_cast<storage>(value) + 1); return value; } \
    inline name operator++(name& value, int) \
    { name previous = value; ++value; return previous; } \
    inline name& operator--(name& value) \
    { value = static_cast<name>(static_cast<storage>(value) - 1); return value; } \
    inline name operator--(name& value, int) \
    { name previous = value; --value; return previous; }

/* Flag combinations retain their domain; integers and other enums do not mix. */
#define KF_ENUM_FLAGS(name, storage) \
    constexpr name operator|(name lhs, name rhs) \
    { return static_cast<name>(static_cast<storage>(lhs) | static_cast<storage>(rhs)); } \
    constexpr name operator&(name lhs, name rhs) \
    { return static_cast<name>(static_cast<storage>(lhs) & static_cast<storage>(rhs)); } \
    constexpr name operator^(name lhs, name rhs) \
    { return static_cast<name>(static_cast<storage>(lhs) ^ static_cast<storage>(rhs)); } \
    constexpr name operator~(name value) \
    { return static_cast<name>(~static_cast<storage>(value)); } \
    inline name& operator|=(name& lhs, name rhs) { return lhs = lhs | rhs; } \
    inline name& operator&=(name& lhs, name rhs) { return lhs = lhs & rhs; } \
    inline name& operator^=(name& lhs, name rhs) { return lhs = lhs ^ rhs; }

/* One domain may use different field widths in runtime and serialized data. */
template <typename Enum, typename Storage>
    requires (__is_enum(Enum) && __is_integral(Storage))
class KfEnumStorage {
public:
    KfEnumStorage() = default;
    constexpr KfEnumStorage(Enum value) : value_(static_cast<Storage>(value)) {}

    constexpr operator Enum() const { return static_cast<Enum>(value_); }
    constexpr Storage encoded_value() const { return value_; }

    constexpr KfEnumStorage& operator=(Enum value)
    {
        value_ = static_cast<Storage>(value);
        return *this;
    }

private:
    Storage value_;
};

template <typename Enum, typename Left, typename Right>
constexpr bool operator==(KfEnumStorage<Enum, Left> lhs, KfEnumStorage<Enum, Right> rhs)
{
    return lhs.encoded_value() == rhs.encoded_value();
}

/* Explicit encoded-data boundaries. These conversions do not validate values. */
template <typename Enum, typename Integer>
    requires (__is_enum(Enum) && __is_integral(Integer))
constexpr Enum kf_enum_decode(Integer value)
{
    return static_cast<Enum>(value);
}

template <typename Integer, typename Enum>
    requires (__is_integral(Integer) && __is_enum(Enum))
constexpr Integer kf_enum_encode(Enum value)
{
    return static_cast<Integer>(value);
}

template <typename Integer, typename Enum, typename Storage>
    requires (__is_enum(Enum) && __is_integral(Integer))
constexpr Integer kf_enum_encode(KfEnumStorage<Enum, Storage> value)
{
    return static_cast<Integer>(value.encoded_value());
}

#define KF_ENUM_DECODE(type, value) kf_enum_decode<type>(value)
#define KF_ENUM_ENCODE(storage, value) kf_enum_encode<storage>(value)
#else
#define KF_MODERN_TYPES 0
#define KF_ENUM_BEGIN(name, storage) typedef storage name; enum {
#define KF_ENUM_END(name) };
#define KF_ENUM_PROMOTED(name) __typeof__(+((name)0))
#define KF_ENUM_STORAGE(name, storage) storage
#define KF_ENUM_PARAM(name, storage) storage
#define KF_ENUM_COUNTER(name, storage)
#define KF_ENUM_FLAGS(name, storage)
#define KF_ENUM_DECODE(type, value) ((type)(value))
#define KF_ENUM_ENCODE(storage, value) ((storage)(value))
#endif

#endif
