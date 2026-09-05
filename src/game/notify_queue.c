#include <kf/address.h>
#include <kf/notify.h>
#include <kf/game_menu.h>

typedef char notification_state_size[sizeof(KfNotificationState) == 0x16 ? 1 : -1];
typedef char notification_control_size[sizeof(KfNotificationControl) == 6 ? 1 : -1];
typedef char notification_control_offset[
    (u32)&((KfNotificationState *)0)->control == 0x10 ? 1 : -1];
typedef char notification_tail_offset[
    (u32)&((KfNotificationControl *)0)->queue_tail == 0 ? 1 : -1];
typedef char notification_head_offset[
    (u32)&((KfNotificationControl *)0)->queue_head == 1 ? 1 : -1];
typedef char notification_phase_offset[
    (u32)&((KfNotificationControl *)0)->effect_phase == 2 ? 1 : -1];
typedef char notification_hold_offset[
    (u32)&((KfNotificationControl *)0)->hold_frames == 3 ? 1 : -1];
typedef char notification_angle_offset[
    (u32)&((KfNotificationControl *)0)->effect_angle_x == 4 ? 1 : -1];

DATA(0x80055d20, 0x54)
KfNotificationSprite notification_sprites[6] = {
    {0, 0, {0, 0, 0x7f, 0x0f, 0xffc0, 0xffa0, 0x7f, 0x0f}},
    {0, 0, {0, 0, 0x7f, 0x0f, 0xffc4, 0xffa0, 0x7f, 0x0f}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffc4, 0xffa3, 7, 0x0b}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffba, 0xffa3, 7, 0x0b}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffb0, 0xffa3, 7, 0x0b}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffa6, 0xffa3, 7, 0x0b}},
};

DATA(0x8009506e, 0x8)
u8 notification_message_ids[8];

DATA(0x80095076, 0x16)
KfNotificationState notification_state;

/*
 * Notification queue and effect, one contiguous run
 * 0x8001fa44..0x8001fde4 (GAME.EXE): the notification ring enqueue/dequeue and
 * the on-screen notification effect. Module boundary is WIP.
 */


/*
 * On-screen notification ring.  Player stat routines (level, training,
 * experience, item use) enqueue a message id here; the notification effect
 * state machine (notify_effect_update) consumes it a frame later.  Message id 0x13
 * carries a u16 payload stored in a parallel table.
 *
 * The trailing varargs slot carries the id 0x13 payload.
 */

ADDRESS(0x8001fa44, 0xa0)
void notify_enqueue(s32 message_id, ...)
{
    u8 *head;

    if (message_id == 0xff) {
        return;
    }
    head = &notification_state.control.queue_head;
    if (notification_message_ids[*head] == 0xff) {
        notification_message_ids[*head] = message_id;
        if (message_id == 0x13) {
            u16 *payload = notification_state.message_payloads;
            payload[*head] = *(u16 *)(&message_id + 1);
        }
        *head = (*head + 1) & 7;
    }
}


ADDRESS(0x8001fae4, 0x18)
void notification_digit_set_v(KfSpriteQuad *sprite, s32 digit)
{
    sprite->v = digit * 11;
}


/*
 * Notification effect state machine, stepped once per frame by the frame
 * renderer (render_frame).  It consumes the notification ring filled by
 * notify_enqueue and drives the six on-screen sprite records that
 * render_enqueue_sprite draws.
 *
 *   phase 0: a queued id starts an effect.  Id 0x13 is a numeric popup whose
 *            payload is split into four digit sprites; any other id shows one
 *            sprite whose atlas U/V cell comes from the id nibbles.
 *   phase 2: hold for fifteen frames.
 *   phase 3: rotate out, then clear the records and dequeue the entry.
 *
 * The six 14-byte notification_sprites rows share one complete descriptor
 * type; record 0 also anchors the four digit descriptors.
 *
 * The menu_format_number digit scratch reserves 24 stack bytes; only four
 * digits are read here, and the formatter's full capacity remains unverified.
 */

ADDRESS(0x8001fafc, 0x2cc)
void notify_effect_update(void)
{
    u8 *phase = &notification_state.control.effect_phase;

    switch (*phase) {
    case 0: {
        u8 tail = notification_state.control.queue_tail;
        u8 id = notification_message_ids[tail];
        if (id == 0xff) {
            return;
        }
        *phase = 2;
        notification_state.control.effect_angle_x = 0;
        notification_state.control.hold_frames = 15;
        if (id == 0x13) {
            KfNotificationSprite *sprite_records = notification_sprites;
            KfNotificationDigitBuffer digits;
            sprite_records[0].active = 0;
            notification_sprites[1].active = 1;
            notification_sprites[1].sprite.u = (id & 0xf0) << 3;
            notification_sprites[1].sprite.v = (id & 0xf) << 4;
            menu_format_number(
                notification_state.message_payloads[tail],
                4, 0, digits.formatted);
            notification_sprites[2].active = 1;
            notification_digit_set_v(
                &sprite_records[2].sprite, digits.values[3]);
            notification_sprites[3].active = 1;
            notification_digit_set_v(
                &sprite_records[3].sprite, digits.values[2]);
            notification_sprites[4].active = 1;
            notification_digit_set_v(
                &sprite_records[4].sprite, digits.values[1]);
            notification_sprites[5].active = 1;
            notification_digit_set_v(
                &sprite_records[5].sprite, digits.values[0]);
        } else {
            notification_sprites[0].active = 1;
            notification_sprites[0].sprite.u = (id & 0xf0) << 3;
            notification_sprites[0].sprite.v = (id & 0xf) << 4;
            notification_sprites[5].active = 0;
            notification_sprites[4].active = 0;
            notification_sprites[3].active = 0;
            notification_sprites[2].active = 0;
            notification_sprites[1].active = 0;
        }
        break;
    }
    case 2: {
        u8 counter = notification_state.control.hold_frames - 1;
        notification_state.control.hold_frames = counter;
        if (counter == 0) {
            *phase = 3;
        }
        break;
    }
    case 3: {
        s16 angle_x = notification_state.control.effect_angle_x + 128;
        notification_state.control.effect_angle_x = angle_x;
        if (angle_x >= 512) {
            KfNotificationControl *control;
            u8 id;
            notification_state.control.effect_angle_x = 512;
            notification_sprites[5].active = 0;
            notification_sprites[4].active = 0;
            notification_sprites[3].active = 0;
            notification_sprites[2].active = 0;
            notification_sprites[1].active = 0;
            notification_sprites[0].active = 0;
            control = &notification_state.control;
            id = notification_message_ids[notification_state.control.queue_tail];
            do {
                notification_message_ids[control->queue_tail] = 0xff;
                control->queue_tail = (control->queue_tail + 1) & 7;
            } while (id == notification_message_ids[control->queue_tail] && id != 0x13);
            control->effect_phase = 0;
        }
        break;
    }
    }
}


ADDRESS(0x8001fdc8, 0x1c)
void display_flip_buffer_index(void)
{
    display_state.buffer_index = (display_state.buffer_index == 0);
}
