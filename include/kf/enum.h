#ifndef KF_ENUM_H
#define KF_ENUM_H

/* Modern checking keeps enum domains; the retail C view keeps their ABI. */
#if defined(__cplusplus) && __cplusplus >= 202002L
#define KF_MODERN_TYPES 1
#define KF_ENUM_BEGIN(name, storage) enum class name : storage {
#define KF_ENUM_END(name) }; using enum name;

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

#define KF_ENUM_DECODE(type, value) kf_enum_decode<type>(value)
#define KF_ENUM_ENCODE(storage, value) kf_enum_encode<storage>(value)
#else
#define KF_MODERN_TYPES 0
#define KF_ENUM_BEGIN(name, storage) typedef storage name; enum {
#define KF_ENUM_END(name) };
#define KF_ENUM_DECODE(type, value) ((type)(value))
#define KF_ENUM_ENCODE(storage, value) ((storage)(value))
#endif

#endif
