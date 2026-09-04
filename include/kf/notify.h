#ifndef KF_NOTIFY_H
#define KF_NOTIFY_H

/* On-screen notification queue and its per-frame presentation state machine. */

#include <kf/semantic_types.h>

/*
 * The menu formatter writes signed glyphs (including -1), while the
 * notification renderer reads the four decimal cells with lhu.
 */
typedef union KfNotificationDigitBuffer {
    s16 formatted[12];
    u16 values[12];
} KfNotificationDigitBuffer;

extern u8 notification_message_ids[8];
extern u8 notification_queue_tail;
extern u8 notification_queue_head;
extern u8 notification_effect_phase;
extern u8 notification_hold_frames;
extern u16 notification_effect_angle_x;
extern KfNotificationSprite notification_sprites[6];
extern void notify_enqueue(s32 message_id, ...);
extern void notify_effect_update(void);

#endif
