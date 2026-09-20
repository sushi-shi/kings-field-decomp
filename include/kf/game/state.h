#ifndef KF_GAME_STATE_H
#define KF_GAME_STATE_H

#include <kf/lib/audio.h>
#include <kf/lib/types.h>
#include <kf/lib/overlay.h>
#include <kf/lib/geometry_types.h>

// Floor-local script configuration: indices are not global character IDs.
inline constexpr u8 KF_FLOOR1_TRIGGER_ACTOR_TILE_X = 7;
inline constexpr u8 KF_FLOOR1_TRIGGER_ACTOR_TILE_Z = 40;
inline constexpr unsigned KF_FLOOR2_DEPARTING_EVENT = 0;
inline constexpr unsigned KF_FLOOR2_HARP_EVENT = 1;
inline constexpr unsigned KF_FLOOR2_REVEAL_EVENT = 3;
inline constexpr unsigned KF_FLOOR3_FIRE_BALL_EVENT = 1;
inline constexpr unsigned KF_FLOOR5_WEAPON_TRANSFORM_EVENT = 1;
extern u32 DAT_80057d24;
extern u32 DAT_800a0768;
extern KfOverlayResultWord game_next_overlay_mode;

enum {
    KF_GAMEPLAY_SOUND_LIFT_DOOR = 0,
    KF_GAMEPLAY_SOUND_HINGED_DOOR = 1,
    KF_GAMEPLAY_SOUND_CONTAINER_OPEN = 2,
    KF_GAMEPLAY_SOUND_EFFECT_SWITCH = 3,
    KF_GAMEPLAY_SOUND_FLOOR_DEFORM = 4,
    KF_GAMEPLAY_SOUND_FLOOR1_REVIVAL = 5,
    KF_GAMEPLAY_SOUND_WARP_SHIMMER = 6,
    KF_GAMEPLAY_SOUND_STONE_PASSAGE = 7,
    KF_GAMEPLAY_SOUND_HARP = 8,
    KF_GAMEPLAY_SOUND_FLOOR5_EVENT_LOOP = 10,
    KF_GAMEPLAY_SOUND_MAP_PIECE_REVEAL = 11,
    KF_GAMEPLAY_SOUND_KEY_UNLOCK = 12,
    KF_GAMEPLAY_SOUND_COUNT = 13
};
extern SoundRef gameplay_sound_refs[KF_GAMEPLAY_SOUND_COUNT];
inline constexpr unsigned talk_image_path_capacity = 20;
extern char talk_image_path_template[talk_image_path_capacity];

#endif
