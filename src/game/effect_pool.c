#include <kf/address.h>
#include <kf/game_effect.h>
#include <kf/game.h>

typedef char effect_state_size[sizeof(KfEffectState) == 0xd28 ? 1 : -1];
typedef char effect_state_alignment[__alignof__(KfEffectState) == 4 ? 1 : -1];
typedef char effect_state_records_offset[
    (u32)&((KfEffectState *)0)->records == 0x1e0 ? 1 : -1];
typedef char effect_state_current_magic_offset[
    (u32)&((KfEffectState *)0)->current_magic == 0xd20 ? 1 : -1];
typedef char effect_state_current_record_offset[
    (u32)&((KfEffectState *)0)->current_record == 0xd24 ? 1 : -1];

DATA(0x8009ce60, 0xd28)
KfEffectState effect_state;

/*
 * Effect-pool spawn band 0x80036f44..0x8003784f (GAME.EXE).
 *
 * effect_pool_construct is the general effect-pool constructor. It claims a free
 * KfEffectRecord from effect_pool_records (effect_pool_find_free), fixes the header
 * (type at 0x00, kind at 0x01, id at 0x06), copies the caller's VECTOR position
 * and SVECTOR direction, defaults the scale triple to 0x1000, then dispatches
 * on the effect kind (a magic_records index in 4..0x30) to seed the per-kind
 * parameters and play the effect's spatial sound. It is a K&R variadic entry:
 * the fifth argument (the direction pointer) is the base of the on-stack
 * variadic slots (arg6 at +4, arg7 at +8, arg8 at +12) that the kinds read.
 *
 * effect_pool_spawn_typed is a specialised constructor used by player_use_item; it seeds
 * the record directly without the kind dispatch.
 *
 * effect_pool_set_current publishes the "current" effect record and its magic_records row
 * through current_effect/current_effect_magic_record.
 *
 * The whole shared state is cleared at startup. The selector reaches the magic
 * rows by a member-relative offset from the current-record pointer slot.
 */

/* effect_pool_construct kind-dispatch jump table (kinds 0x04..0x30). */

ADDRESS(0x80036f00, 0x44)
KfEffectRecord *effect_pool_find_free(void)
{
    KfEffectRecord *record = effect_pool_records;
    u16 i = 48;

    do {
        if (record->type == 0xff) {
            return record;
        }
        record++;
    } while (--i != 0);
    return 0;
}

RODATA(0x80012c28, 0xb4)

ADDRESS(0x80036f44, 0x82c)
KfEffectRecord *effect_pool_construct(
    u8 id, u8 type, u8 kind, const VECTOR *position,
    const SVECTOR *direction, ...)
{
    KfEffectRecord *record = effect_pool_find_free();
    s32 *va = (s32 *)&direction;   /* variadic stack base: va[1]=arg6, va[2]=arg7, va[3]=arg8 */
    KfMagicRecord *magic;

    if (record == 0) {
        return record;
    }

    record->type = type;
    record->kind = kind;
    record->position = *position;
    record->direction_x = direction->vx;
    record->direction_y = direction->vy;
    record->direction_z = direction->vz;
    record->direction_pad = direction->pad;
    record->phase = 0;
    record->id = id;
    record->scale_z = 0x1000;
    record->scale_y = 0x1000;
    record->scale_x = 0x1000;
    record->visual.animation_phase = 0;
    record->sound_played = 0;

    magic = &magic_records[kind];

    switch (kind) {
    case 5:
        record->animation_clip = 0xff;
        record->base_render_id = 0;
        record->render_id = 0;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 7:
        record->animation_clip = 0xff;
        record->base_render_id = 5;
        record->render_id = 5;
        record->rotation_x = 0x352;
        record->rotation_y = 0;
        record->rotation_z = 0;
        if (va[2] != 0) {
            audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                             &record->position, 0x7f);
        }
        break;
    case 4:
        record->base_render_id = 6;
        record->render_id = 6;
        record->animation_clip = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->control.frames_remaining = *(u16 *)(va + 1);
        if (va[2] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[4] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x17:
        record->base_render_id = 0x11;
        record->render_id = 0x11;
        record->kind = 4;
        record->animation_clip = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->control.frames_remaining = *(u16 *)(va + 1);
        if (va[2] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[4] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x29:
        record->base_render_id = 0x13;
        record->render_id = 0x13;
        record->kind = 0x20;
        record->animation_clip = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_range(
            (const SoundRef *)((char *)&magic_records[4] + 5),
            &record->position, 0x7f, 0x4e20, 0xea60);
        break;
    case 0x20:
        record->base_render_id = 0xb;
        record->render_id = 0xb;
        record->animation_clip = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_range(
            (const SoundRef *)((char *)&magic_records[4] + 5),
            &record->position, 0x7f, 0x4e20, 0xea60);
        break;
    case 0x2a:
        record->base_render_id = 0xf;
        record->render_id = 0xf;
        record->kind = 0x21;
        record->animation_clip = 0;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        break;
    case 0x21:
        record->base_render_id = 0;
        record->render_id = 0;
        record->animation_clip = 0;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        break;
    case 0x22:
        record->animation_clip = 0;
        record->base_render_id = 2;
        record->render_id = 2;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->scale_y = 0;
        break;
    case 6:
        record->animation_clip = 0;
        record->base_render_id = 1;
        record->render_id = 1;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->scale_y = 0;
        record->control.frames_remaining = 6;
        record->propagation.branch = *(u16 *)(va + 1);
        if (record->propagation.branch != 0xff) {
            sound_ref_play((const SoundRef *)((char *)magic + 2), 0x78);
        }
        break;
    case 9:
        record->animation_clip = 0;
        record->base_render_id = 3;
        record->render_id = 3;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->scale_z = 0;
        record->scale_y = 0;
        record->scale_x = 0;
        record->control.frames_remaining = *(u16 *)(va + 1);
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 0xa:
        record->animation_clip = 0xff;
        record->base_render_id = 9;
        record->render_id = 9;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->propagation.generations_remaining = *(u16 *)(va + 1);
        record->control.frames_remaining = *(u16 *)(va + 2);
        record->visual.pulse_base_scale = *(u16 *)(va + 3);
        record->scale_z = *(u16 *)(va + 3);
        record->scale_y = *(u16 *)(va + 3);
        record->scale_x = *(u16 *)(va + 3);
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 8:
        record->animation_clip = 0;
        record->base_render_id = 6;
        record->render_id = 6;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        if (va[2] != 0) {
            audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                             &record->position, 0x7f);
        }
        break;
    case 0xb:
        record->animation_clip = 0xff;
        record->base_render_id = 8;
        record->render_id = 8;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 0xc:
        record->animation_clip = 0xff;
        record->base_render_id = 0xa;
        record->render_id = 0xa;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 0xd:
        record->animation_clip = 0;
        record->base_render_id = 9;
        record->render_id = 9;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->phase = 0x64;
        record->scale_z = 0x5dc;
        record->scale_y = 0x5dc;
        record->scale_x = 0x5dc;
        record->position.vy += 3500;
        break;
    case 0xe:
        record->animation_clip = 0;
        record->base_render_id = 4;
        record->render_id = 4;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        break;
    case 0x30:
        record->animation_clip = 0;
        record->base_render_id = 5;
        record->render_id = 5;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        break;
    case 0x16:
        record->animation_clip = 0;
        record->base_render_id = 0xe;
        record->render_id = 0xe;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        if (va[2] != 0) {
            audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                             &record->position, 0x7f);
        }
        break;
    case 0xf:
        record->animation_clip = 0;
        record->base_render_id = 7;
        record->render_id = 7;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        record->rotation_x = 0x200;
        record->direction_z = 0;
        record->direction_y = 0;
        record->direction_x = 0;
        record->scale_z = 0xa28;
        record->scale_y = 0xa28;
        record->scale_x = 0xa28;
        break;
    case 0x10:
        record->animation_clip = 0;
        record->base_render_id = 7;
        record->render_id = 7;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        record->rotation_x = 0x200;
        record->direction_z = 0;
        record->direction_y = 0;
        record->direction_x = 0;
        break;
    case 0x11:
        record->animation_clip = 0;
        record->base_render_id = 9;
        record->render_id = 9;
        record->rotation_z = 0;
        record->rotation_y = 0;
        record->rotation_x = 0;
        record->control.orbit_angle = 0;
        record->scale_z = 0xaf0;
        record->scale_y = 0xaf0;
        record->scale_x = 0xaf0;
        record->direction_x = record->position.vx >> 8;
        record->direction_z = record->position.vz >> 8;
        record->direction_y = (u16)record->position.vy;
        break;
    case 0x24:
        record->animation_clip = 0;
        record->base_render_id = 0xa;
        record->render_id = 0xa;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        record->propagation.branch = 0xff;
        record->control.frames_remaining = 0xff;
        record->rotation_x = -record->rotation_x;
        audio_play_spatial_range(
            (const SoundRef *)((char *)&magic_records[18] + 5),
            &record->position, 0x7f, 0x4e20, 0xea60);
        break;
    case 0x13:
        record->animation_clip = 0xff;
        record->base_render_id = 0xe;
        record->render_id = 0xe;
        record->rotation_z = 0;
        record->rotation_y = 0;
        record->rotation_x = 0;
        record->control.parent_effect_index = *(u8 *)(va + 1);
        break;
    case 0x2c:
        record->base_render_id = 0x11;
        record->render_id = 0x11;
        record->kind = 0x12;
        record->animation_clip = 0;
        if (va[1] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[18] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x12:
        record->base_render_id = 0xb;
        record->render_id = 0xb;
        record->animation_clip = 0;
        if (va[1] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[18] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x18:
        record->base_render_id = 0x10;
        record->render_id = 0x10;
        record->kind = 0x14;
        record->animation_clip = 0;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        record->control.target_mode = *(u8 *)(va + 2);
        record->direction_x = record->rotation_x;
        record->direction_y = record->rotation_y;
        record->direction_z = record->rotation_z;
        record->direction_pad = record->rotation_pad;
        record->scale_y = 0x800;
        record->scale_x = 0x800;
        if (va[3] != 0) {
            audio_play_spatial_default_range(
                (const SoundRef *)((char *)&magic_records[20] + 2),
                &record->position, 0x7f);
        }
        break;
    case 0x14:
        record->base_render_id = 0xc;
        record->render_id = 0xc;
        record->animation_clip = 0;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->rotation_pad = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        record->control.target_mode = *(u8 *)(va + 2);
        record->direction_x = record->rotation_x;
        record->direction_y = record->rotation_y;
        record->direction_z = record->rotation_z;
        record->direction_pad = record->rotation_pad;
        record->scale_y = 0x800;
        record->scale_x = 0x800;
        if (va[3] != 0) {
            audio_play_spatial_default_range(
                (const SoundRef *)((char *)&magic_records[20] + 2),
                &record->position, 0x7f);
        }
        break;
    case 0x15:
        record->animation_clip = 0;
        record->base_render_id = 0xd;
        record->render_id = 0xd;
        record->scale_y = 0;
        record->scale_z = 0x1800;
        record->scale_x = 0x1800;
        record->rotation_x = 0;
        record->rotation_z = 0;
        record->rotation_y = player_state.camera_rotation.vy;
        break;
    default:
        record->type = 0xff;
        break;
    }

    return record;
}

ADDRESS(0x80037770, 0xac)
KfEffectRecord *effect_pool_spawn_typed(
    u16 rotation_x, u16 rotation_y, u16 rotation_z, u16 direction_y,
    s32 position_x, s32 position_y)
{
    KfEffectRecord *record = effect_pool_find_free();
    if (record != 0) {
        record->rotation_x = rotation_x;
        record->rotation_y = rotation_y;
        record->rotation_z = rotation_z;
        record->position.vx = position_x;
        record->position.vy = position_y;
        record->direction_y = direction_y;
        record->base_render_id = 0xff;
        record->render_id = 0xff;
        record->kind = 0x34;
        record->type = 0xf0;
        record->phase = 0;
        record->direction_x = position_x;
        record->direction_z = 0;
    }
    return record;
}

ADDRESS(0x8003781c, 0x34)
void effect_pool_set_current(KfEffectRecord *record)
{
    current_effect = record;
    current_effect_magic_record = &magic_records[record->kind];
}
