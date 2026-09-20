#pragma once
#include <kf/game/menu.h>

// Original Japanese atlas sequences, not Unicode code points. Keep partial
// updates at callers: several labels intentionally reuse the preceding row.
namespace menu_glyphs {
inline constexpr s16 on[] = {0xf9, 0xfa, MENU_TEXT_END};
inline constexpr s16 off[] = {0xf9, 0xfb, 0xfb, MENU_TEXT_END};
inline constexpr s16 experience[] = {0x82, 0x83, 0x84, MENU_TEXT_END};
inline constexpr s16 level[] = {0x2b, MENU_TEXT_DAKUTEN | 0x1c, 0x2a, MENU_TEXT_END};
inline constexpr s16 character_class[] = {0x7, 0x28, 0xc, MENU_TEXT_END};
inline constexpr s16 floor[] = {0xcc, 0xcd, MENU_TEXT_END};
inline constexpr s16 hp[] = {0xf0, 0xf2, MENU_TEXT_END};
inline constexpr s16 mp[] = {0xf1, 0xf2, MENU_TEXT_END};
inline constexpr s16 status[] = {0x85, 0x86, MENU_TEXT_END};
inline constexpr s16 gold[] = {MENU_TEXT_DAKUTEN | 0x9, 0x2d, 0x2a, MENU_TEXT_DAKUTEN | 0x13, MENU_TEXT_END};
inline constexpr s16 healthy[] = {0xc5, 0xc6, MENU_TEXT_END};
inline constexpr s16 slowed[] = {0xc9, MENU_TEXT_END};
inline constexpr s16 poison[] = {0x88, MENU_TEXT_END};
inline constexpr s16 darkness[] = {0xc7, MENU_TEXT_END};
inline constexpr s16 curse[] = {0xc8, MENU_TEXT_END};
inline constexpr s16 physical_power[] = {0x8c, 0x8b, MENU_TEXT_END};
inline constexpr s16 magic_power[] = {0x78, 0x8b, MENU_TEXT_END};
inline constexpr s16 total_attack[] = {0xce, 0xcf, 0x89, 0x8a, 0x8b, MENU_TEXT_END};
inline constexpr s16 total_defense[] = {0xce, 0xcf, 0x7a, 0xd0, 0x8b, MENU_TEXT_END};
inline constexpr s16 attack[] = {0x89, 0x8a, 0x8b, MENU_TEXT_END};
inline constexpr s16 defense[] = {0x7a, 0xd0, 0x8b, MENU_TEXT_END};
inline constexpr s16 cutting[] = {MENU_TEXT_BLANK, 0xd1, 0x6a, MENU_TEXT_END};
inline constexpr s16 striking[] = {MENU_TEXT_BLANK, 0xd2, 0x51, MENU_TEXT_END};
inline constexpr s16 piercing[] = {MENU_TEXT_BLANK, 0xd3, 0x4c, MENU_TEXT_END};
inline constexpr s16 holy[] = {MENU_TEXT_BLANK, 0xbf, 0x58, 0x78, 0x79, MENU_TEXT_END};
inline constexpr s16 fire[] = {MENU_TEXT_BLANK, 0xd4, 0x58, 0x78, 0x79, MENU_TEXT_END};
inline constexpr s16 poison_resistance[] = {MENU_TEXT_BLANK, 0x88, MENU_TEXT_END};
inline constexpr s16 magic_defense[] = {MENU_TEXT_BLANK, 0x78, 0x58, 0x78, 0x79, MENU_TEXT_END};
inline constexpr s16 quantity[] = {0xca, 0xcb, MENU_TEXT_END};
inline constexpr s16 use[] = {0x72, 0x42, MENU_TEXT_END};
inline constexpr s16 drop[] = {0x75, 0x52, 0x6a, MENU_TEXT_END};
inline constexpr s16 yes[] = {0x59, 0x41, MENU_TEXT_END};
inline constexpr s16 no[] = {0x41, 0x41, 0x43, MENU_TEXT_END};
inline constexpr s16 buy[] = {0x74, 0x42, MENU_TEXT_END};
inline constexpr s16 sell[] = {0x73, 0x6a, MENU_TEXT_END};
inline constexpr s16 equip[] = {0x70, 0x71, MENU_TEXT_END};
inline constexpr s16 cancel[] = {0x63, 0x61, 0x6a, MENU_TEXT_END};
inline constexpr s16 pickup[] = {0x53, 0x6a, MENU_TEXT_END};
inline constexpr s16 unequip[] = {0x59, MENU_TEXT_DAKUTEN | 0x4c, 0x4c, MENU_TEXT_END};
}
