#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

DATA(0x80055d20, 0x54)
KfNotificationSprite notification_sprites[6] = {
    {0, 0, {0, 0, 0x7f, 0x0f, 0xffc0, 0xffa0, 0x7f, 0x0f}},
    {0, 0, {0, 0, 0x7f, 0x0f, 0xffc4, 0xffa0, 0x7f, 0x0f}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffc4, 0xffa3, 7, 0x0b}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffba, 0xffa3, 7, 0x0b}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffb0, 0xffa3, 7, 0x0b}},
    {0, 0, {0xf0, 0, 7, 0x0b, 0xffa6, 0xffa3, 7, 0x0b}},
};


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
 * The trailing varargs slot carries the id 0x13 payload.  The payload table is
 * reached from the head pointer because that source shape reproduces retail's
 * reuse of the head-address base register.
 */

ADDRESS(0x8001fa44, 0xa0)
void notify_enqueue(s32 message_id, ...)
{
    u8 *head;

    if (message_id == 0xff) {
        return;
    }
    head = &notification_queue_head;
    if (notification_message_ids[*head] == 0xff) {
        notification_message_ids[*head] = message_id;
        if (message_id == 0x13) {
            u16 *payload = (u16 *)(head - 17);
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
 * type; record 0 also anchors the four digit descriptors.  The payload table
 * is reached from the phase pointer to reproduce retail's shared base-register
 * schedule.
 *
 * The menu_format_number digit scratch reserves 24 stack bytes (a fixed buffer
 * larger than the four digits used); the exact element count is unverified but
 * codegen-invariant across it.  With that 56-byte frame the phase/tail cursors
 * bind $a3/$t0 where retail binds $t0/$a3 -- an unattributed register-allocation
 * residue that leaves the body otherwise structurally exact.
 */

ADDRESS(0x8001fafc, 0x2cc)
void notify_effect_update(void)
{
    u8 *phase = &notification_effect_phase;

    switch (*phase) {
    case 0: {
        u8 tail = notification_queue_tail;
        u8 id = notification_message_ids[tail];
        if (id == 0xff) {
            return;
        }
        *phase = 2;
        notification_effect_angle_x = 0;
        notification_hold_frames = 15;
        if (id == 0x13) {
            KfNotificationSprite *sprite_records = notification_sprites;
            KfNotificationDigitBuffer digits;
            sprite_records[0].active = 0;
            notification_sprites[1].active = 1;
            notification_sprites[1].sprite.u = (id & 0xf0) << 3;
            notification_sprites[1].sprite.v = (id & 0xf) << 4;
            menu_format_number(
                ((u16 *)((char *)phase - 18))[tail], 4, 0, digits.formatted);
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
        u8 counter = notification_hold_frames - 1;
        notification_hold_frames = counter;
        if (counter == 0) {
            *phase = 3;
        }
        break;
    }
    case 3: {
        s16 angle_x = notification_effect_angle_x + 128;
        notification_effect_angle_x = angle_x;
        if (angle_x >= 512) {
            u8 *tail = phase - 2;
            u8 id;
            notification_effect_angle_x = 512;
            notification_sprites[5].active = 0;
            notification_sprites[4].active = 0;
            notification_sprites[3].active = 0;
            notification_sprites[2].active = 0;
            notification_sprites[1].active = 0;
            notification_sprites[0].active = 0;
            id = notification_message_ids[notification_queue_tail];
            do {
                notification_message_ids[*tail] = 0xff;
                *tail = (*tail + 1) & 7;
            } while (id == notification_message_ids[*tail] && id != 0x13);
            tail[2] = 0;
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
