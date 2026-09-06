#ifndef KF_NOTIFY_H
#define KF_NOTIFY_H

/* On-screen notification queue and its per-frame presentation state machine. */

#include <kf/game_render.h>

enum {
    KF_NOTIFICATION_CAPACITY = 8,
    KF_NOTIFICATION_SPRITE_COUNT = 6,
    KF_NOTIFICATION_NONE = 0xff,
    KF_NOTIFICATION_GOLD = 0x13,
    KF_NOTIFICATION_TEXT_SPRITE = 0,
    KF_NOTIFICATION_GOLD_SPRITE = 1,
    KF_NOTIFICATION_ONES_SPRITE = 2,
    KF_NOTIFICATION_TENS_SPRITE = 3,
    KF_NOTIFICATION_HUNDREDS_SPRITE = 4,
    KF_NOTIFICATION_THOUSANDS_SPRITE = 5,
    KF_NOTIFICATION_IDLE = 0,
    KF_NOTIFICATION_HOLD = 2,
    KF_NOTIFICATION_ROTATE_OUT = 3
};

/* One row of the six-sprite on-screen notification display. */
typedef struct KfNotificationSprite {
    u8 active;
    u8 unknown_01;
    KfSpriteQuad sprite;
} KfNotificationSprite;

/*
 * The menu formatter writes signed glyphs (including -1), while the
 * notification renderer reads the four decimal cells with lhu.
 */
typedef union KfNotificationDigitBuffer {
    s16 formatted[12];
    u16 values[12];
} KfNotificationDigitBuffer;

/* The dequeue operation addresses tail and phase through this control base. */
typedef struct KfNotificationControl {
    u8 queue_tail;
    u8 queue_head;
    u8 effect_phase;
    u8 hold_frames;
    u16 effect_angle_x;
} KfNotificationControl;

/* Numeric payloads and the queue's presentation state share one retail base. */
typedef struct KfNotificationState {
    u16 message_payloads[KF_NOTIFICATION_CAPACITY];
    KfNotificationControl control;
} KfNotificationState;

extern u8 notification_message_ids[KF_NOTIFICATION_CAPACITY];
extern KfNotificationState notification_state;
extern KfNotificationSprite notification_sprites[KF_NOTIFICATION_SPRITE_COUNT];
extern void notify_enqueue(s32 message_id, ...);
extern void notify_effect_update(void);

#endif
