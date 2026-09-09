#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_asset.h>
#include <kf/game_math.h>
#include <kf/game_player.h>
#include <kf/game_render.h>
#include <kf/psyq.h>
#include <kf/notify.h>
#include <kf/game_menu.h>

DATA(0x80055c5c, 0xc4)
KfHudSprite hud_sprites[KF_HUD_TABLE_ROWS] = {
    {KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},
    {KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},
    {KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},
    {KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},
    {KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},
    {KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},
    {KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},
    {KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},
    {KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},
    {KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},
};

DATA(0x80055d20, 0x54)
KfNotificationSprite notification_sprites[KF_NOTIFICATION_SPRITE_COUNT] = {
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0, 0, 0x7f, 0x0f, 0xffc0, 0xffa0, 0x7f, 0x0f}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0, 0, 0x7f, 0x0f, 0xffc4, 0xffa0, 0x7f, 0x0f}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffc4, 0xffa3, 7, 0x0b}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffba, 0xffa3, 7, 0x0b}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffb0, 0xffa3, 7, 0x0b}},
    {KF_NOTIFICATION_SPRITE_HIDDEN, 0, {0xf0, 0, 7, 0x0b, 0xffa6, 0xffa3, 7, 0x0b}},
};

DATA(0x80055d74, 0x38)
KfEffectSprite effect_sprites[KF_EFFECT_SPRITE_TABLE_ROWS] = {
    {KF_EFFECT_SPRITE_ACTIVE, KF_ANIMATION_CLIP_FIRST, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},
    {KF_EFFECT_SPRITE_END},
};

enum {
    WEAPON_PROJECTED_DEPTH_SHIFT = 3,
    WEAPON_DEPTH_BIAS_SHIFT = 5,
    WEAPON_BASE_DEPTH_BIAS = 50
};

/*
 * Screen-space geometry emitters that run every frame from the top-level frame
 * renderer (render_frame).  Each walks a per-subsystem table of live entries,
 * feeds one through the GTE, and hands the projected model to the shared
 * display-list builder.
 */

/* Six light/color matrices fed to SetLightMatrix, one per render subsystem. */

/*
 * Draws the equipped weapon model held in the player's view.  Skips entirely
 * while no weapon swing is in progress (attack phase -1).  The weapon record
 * carries its own geometry-screen distance, an in-view translation, and a
 * rotation vector. The projected depth is biased by the record's Z translation
 * before the model is enqueued.
 */
ADDRESS(0x8001f798, 0x118)
void render_weapon(void)
{
    MATRIX model;
    KfWeaponRecord *weapon;
    KfTmdObject *object;
    s32 depth_bias;

    if (player_state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE) {
        return;
    }
    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_WEAPON]);
    SetGeomScreen(player_state.equipped_weapon_record->projection_distance);
    weapon = player_state.equipped_weapon_record;
    model.t[0] = weapon->render_translation.x;
    model.t[1] = weapon->render_translation.y;
    model.t[2] = weapon->render_translation.z;
    RotMatrix(&weapon->render_rotation, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    asset_registry_select(KF_ASSET_WEAPON);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &player_state.weapon_animation_cache, KF_ASSET_WEAPON, KF_ANIMATION_CLIP_FIRST,
            player_state.weapon_attack_phase,
            object->vertex_count) != 0) {
        tmd_project_vertices_shift(object->vertex_count, WEAPON_PROJECTED_DEPTH_SHIFT);
        depth_bias =
            player_state.equipped_weapon_record->render_translation.z >> WEAPON_DEPTH_BIAS_SHIFT;
        render_enqueue_tmd(0, -depth_bias + WEAPON_BASE_DEPTH_BIAS);
    }
}

/*
 * Draws the animated decal/sprite list.  The current color matrix is saved and
 * replaced with render_state's sprite color matrix for the whole pass and
 * restored afterwards.  Each live entry builds a rotated, uniformly scaled
 * model matrix with the translation folded into its t column, tests visibility
 * through render_bind_animated_instance, and, if visible, transforms and enqueues asset 0x15.
 */
ADDRESS(0x8001f8b0, 0x124)
void render_effect_sprites(void)
{
    MATRIX model;
    VECTOR scale;
    MATRIX saved_color_matrix;
    KfEffectSprite *entry;
    KfTmdObject *object;
    u16 scale_numerator;

    ReadColorMatrix(&saved_color_matrix);
    SetColorMatrix(&game_graphics_runtime.render_state.effect_color_matrix);
    scale.vz = KF_FIXED12_ONE;
    entry = effect_sprites;
    while (entry->state == KF_EFFECT_SPRITE_ACTIVE) {
        model.t[0] = entry->translation_x;
        model.t[1] = entry->translation_y;
        model.t[2] = entry->translation_z;
        RotMatrix(&entry->rotation, &model);
        scale_numerator = entry->scale;
        scale.vy = scale_numerator;
        scale.vx = scale_numerator;
        ScaleMatrix(&model, &scale);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        asset_registry_select(KF_ASSET_EFFECT_SPRITES);
        object = tmd_get_object(0);
        if (render_bind_animated_instance(
                &entry->animation_cache, KF_ASSET_EFFECT_SPRITES,
                entry->animation_clip, entry->asset_variant,
                object->vertex_count) != 0) {
            tmd_transform_vertices(object->vertex_count);
            render_enqueue_tmd(0, 0);
        }
        entry++;
    }
    SetColorMatrix(&saved_color_matrix);
}

/*
 * Walks a stride-14 table terminated by a 0xff type byte and dispatches every
 * active (type == 1) entry to render_screen_sprite, pointing it past the two-byte
 * header.  The table base is supplied by the caller.
 */
ADDRESS(0x8001f9d4, 0x70)
void render_hud_gauges(KfHudSprite *table)
{
    KfHudSprite *entry;

    entry = table;
    if (entry->state == KF_HUD_END) {
        return;
    }
    do {
        if (entry->state == KF_HUD_VISIBLE) {
            render_screen_sprite(&entry->sprite);
        }
        entry++;
    } while (entry->state != KF_HUD_END);
}

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

static inline void notify_dequeue_group(void)
{
    KfNotificationControl *control;
    KfNotificationId id;

    control = &game_graphics_runtime.notification_state.control;
    id = game_graphics_runtime.notification_message_ids[game_graphics_runtime.notification_state.control.queue_tail];
    do {
        game_graphics_runtime.notification_message_ids[control->queue_tail] = KF_NOTIFICATION_NONE;
        control->queue_tail = (control->queue_tail + 1) & (KF_NOTIFICATION_CAPACITY - 1);
    } while (id == game_graphics_runtime.notification_message_ids[control->queue_tail]
        && id != KF_NOTIFICATION_GOLD);
    control->effect_phase = KF_NOTIFICATION_IDLE;
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
                NOTIFICATION_GOLD_DIGITS, KF_FORMAT_PAD_SPACES, digits.formatted);
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
            game_graphics_runtime.notification_state.control.effect_angle_x = KF_ANGLE_EIGHTH_TURN;
            notification_sprites[KF_NOTIFICATION_THOUSANDS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_HUNDREDS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_TENS_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_ONES_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notification_sprites[KF_NOTIFICATION_TEXT_SPRITE].active = KF_NOTIFICATION_SPRITE_HIDDEN;
            notify_dequeue_group();
        }
        break;
    }
    }
}

ADDRESS(0x8001fdc8, 0x1c)
void display_flip_buffer_index(void)
{
    game_graphics_runtime.display_state.buffer_index = display_next_buffer(game_graphics_runtime.display_state.buffer_index);
}
