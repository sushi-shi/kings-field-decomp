#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/notify.h>
#include <kf/game_menu.h>

enum {
    NOTIFICATION_HOLD_FRAMES = 15,
    NOTIFICATION_EXIT_ANGLE_STEP = 128,
    NOTIFICATION_GOLD_DIGITS = 4,
    NOTIFICATION_DIGIT_ROW_HEIGHT = 11,
    NOTIFICATION_ATLAS_COLUMN_MASK = 0xf0,
    NOTIFICATION_ATLAS_COLUMN_SHIFT = 3,
    NOTIFICATION_ATLAS_ROW_MASK = 0xf,
    NOTIFICATION_ATLAS_ROW_SHIFT = 4
};

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
KfNotificationSprite notification_sprites[KF_NOTIFICATION_SPRITE_COUNT] = {
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0, 0, 0x7f, 0x0f, 0xffc0, 0xffa0, 0x7f, 0x0f}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0, 0, 0x7f, 0x0f, 0xffc4, 0xffa0, 0x7f, 0x0f}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffc4, 0xffa3, 7, 0x0b}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffba, 0xffa3, 7, 0x0b}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffb0, 0xffa3, 7, 0x0b}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffa6, 0xffa3, 7, 0x0b}},
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
 * The trailing varargs slot carries the id 0x13 payload.
 */

ADDRESS(0x8001fa44, 0xa0)
void notify_enqueue(KfNotificationArgument message_id, ...)
{
    u8 *head;

    if (message_id == KF_NOTIFICATION_NONE) {
        return;
    }
    head = &game_graphics_runtime.notification_state.control.queue_head;
    if (game_graphics_runtime.notification_message_ids[*head] == KF_NOTIFICATION_NONE) {
        game_graphics_runtime.notification_message_ids[*head] = message_id;
        if (message_id == KF_NOTIFICATION_GOLD) {
            u16 *payload = game_graphics_runtime.notification_state.message_payloads;
#if KF_MODERN_TYPES
            __builtin_va_list arguments;
            __builtin_va_start(arguments, message_id);
            payload[*head] = __builtin_va_arg(arguments, s32);
            __builtin_va_end(arguments);
#else
            /* The pinned compiler spills four-byte argument homes. */
            payload[*head] = *(u16 *)(&message_id + 1);
#endif
        }
        *head = (*head + 1) & (KF_NOTIFICATION_CAPACITY - 1);
    }
}

ADDRESS(0x8001fae4, 0x18)
void notification_digit_set_v(KfSpriteQuad *sprite, s32 digit)
{
    sprite->v = digit * NOTIFICATION_DIGIT_ROW_HEIGHT;
}

/*
 * Notification effect state machine, stepped once per frame by the frame
 * renderer (render_frame).  It consumes the notification ring filled by
 * notify_enqueue and drives the six on-screen sprite records that
 * render_enqueue_sprite draws.
 *
 *   phase 0: a queued id starts an effect. Gold popups split their payload
 *            into four digit sprites; any other id shows one
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
    KfNotificationPhase *phase = &game_graphics_runtime.notification_state.control.effect_phase;

    switch (*phase) {
    case KF_NOTIFICATION_IDLE: {
        u8 tail = game_graphics_runtime.notification_state.control.queue_tail;
        KfNotificationId id = game_graphics_runtime.notification_message_ids[tail];
        if (id == KF_NOTIFICATION_NONE) {
            return;
        }
        *phase = KF_NOTIFICATION_HOLD;
        game_graphics_runtime.notification_state.control.effect_angle_x = 0;
        game_graphics_runtime.notification_state.control.hold_frames = NOTIFICATION_HOLD_FRAMES;
        if (id == KF_NOTIFICATION_GOLD) {
            KfNotificationSprite *sprite_records = notification_sprites;
            KfNotificationDigitBuffer digits;
            sprite_records[KF_NOTIFICATION_TEXT_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].active = KF_NOTIFICATION_SPRITE_VISIBLE;
            notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].sprite.u =
                (KF_ENUM_ENCODE(u8, id) & NOTIFICATION_ATLAS_COLUMN_MASK) << NOTIFICATION_ATLAS_COLUMN_SHIFT;
            notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].sprite.v =
                (KF_ENUM_ENCODE(u8, id) & NOTIFICATION_ATLAS_ROW_MASK) << NOTIFICATION_ATLAS_ROW_SHIFT;
            menu_format_number(
                game_graphics_runtime.notification_state.message_payloads[tail],
                NOTIFICATION_GOLD_DIGITS, 0, digits.formatted);
            notification_sprites[KF_NOTIFICATION_ONES_SPRITE].active = KF_NOTIFICATION_SPRITE_VISIBLE;
            notification_digit_set_v(
                &sprite_records[KF_NOTIFICATION_ONES_SPRITE].sprite, digits.values[3]);
            notification_sprites[KF_NOTIFICATION_TENS_SPRITE].active = KF_NOTIFICATION_SPRITE_VISIBLE;
            notification_digit_set_v(
                &sprite_records[KF_NOTIFICATION_TENS_SPRITE].sprite, digits.values[2]);
            notification_sprites[KF_NOTIFICATION_HUNDREDS_SPRITE].active = KF_NOTIFICATION_SPRITE_VISIBLE;
            notification_digit_set_v(
                &sprite_records[KF_NOTIFICATION_HUNDREDS_SPRITE].sprite, digits.values[1]);
            notification_sprites[KF_NOTIFICATION_THOUSANDS_SPRITE].active = KF_NOTIFICATION_SPRITE_VISIBLE;
            notification_digit_set_v(
                &sprite_records[KF_NOTIFICATION_THOUSANDS_SPRITE].sprite, digits.values[0]);
        } else {
            notification_sprites[KF_NOTIFICATION_TEXT_SPRITE].active = KF_NOTIFICATION_SPRITE_VISIBLE;
            notification_sprites[KF_NOTIFICATION_TEXT_SPRITE].sprite.u =
                (KF_ENUM_ENCODE(u8, id) & NOTIFICATION_ATLAS_COLUMN_MASK) << NOTIFICATION_ATLAS_COLUMN_SHIFT;
            notification_sprites[KF_NOTIFICATION_TEXT_SPRITE].sprite.v =
                (KF_ENUM_ENCODE(u8, id) & NOTIFICATION_ATLAS_ROW_MASK) << NOTIFICATION_ATLAS_ROW_SHIFT;
            notification_sprites[KF_NOTIFICATION_THOUSANDS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_HUNDREDS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_TENS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_ONES_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
        }
        break;
    }
    case KF_NOTIFICATION_HOLD: {
        u8 counter = game_graphics_runtime.notification_state.control.hold_frames - 1;
        game_graphics_runtime.notification_state.control.hold_frames = counter;
        if (counter == 0) {
            *phase = KF_NOTIFICATION_ROTATE_OUT;
        }
        break;
    }
    case KF_NOTIFICATION_ROTATE_OUT: {
        s16 angle_x = game_graphics_runtime.notification_state.control.effect_angle_x + NOTIFICATION_EXIT_ANGLE_STEP;
        game_graphics_runtime.notification_state.control.effect_angle_x = angle_x;
        if (angle_x >= KF_ANGLE_EIGHTH_TURN) {
            KfNotificationControl *control;
            KfNotificationId id;
            game_graphics_runtime.notification_state.control.effect_angle_x = KF_ANGLE_EIGHTH_TURN;
            notification_sprites[KF_NOTIFICATION_THOUSANDS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_HUNDREDS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_TENS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_ONES_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_TEXT_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            control = &game_graphics_runtime.notification_state.control;
            id = game_graphics_runtime.notification_message_ids[game_graphics_runtime.notification_state.control.queue_tail];
            do {
                game_graphics_runtime.notification_message_ids[control->queue_tail] = KF_NOTIFICATION_NONE;
                control->queue_tail = (control->queue_tail + 1) & (KF_NOTIFICATION_CAPACITY - 1);
            } while (id == game_graphics_runtime.notification_message_ids[control->queue_tail]
                && id != KF_NOTIFICATION_GOLD);
            control->effect_phase = KF_NOTIFICATION_IDLE;
        }
        break;
    }
    }
}

ADDRESS(0x8001fdc8, 0x1c)
void display_flip_buffer_index(void)
{
    game_graphics_runtime.display_state.buffer_index = (game_graphics_runtime.display_state.buffer_index == 0);
}
