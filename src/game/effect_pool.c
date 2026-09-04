#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

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
 * effect_pool_spawn_typed is exact. effect_pool_construct carries the GCC 2.5.7 switch
 * cross-jumping / K&R stack-vararg codegen residue (a large jump table with
 * tail-merged cases and one fewer callee-saved register than retail).
 * effect_pool_set_current carries the shared-high-halfword data residue: the retail unit
 * reaches magic_records by offset from current_effect's base register because the
 * two globals are consecutive in the original translation unit, which a
 * reconstruction referencing magic_records as its own extern cannot reproduce.
 * See docs/patterns/source-shapes-gcc257.md.
 */

extern void audio_play_spatial_default_range(
    const SoundRef *sound, const VECTOR *position, s16 volume);
extern void audio_play_spatial_range(
    const SoundRef *sound, const VECTOR *position, s16 volume,
    s32 max_distance, s32 attenuation_distance);

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
    record->unknown_32 = direction->pad;
    record->unknown_07 = 0;
    record->id = id;
    record->scale_z = 0x1000;
    record->scale_y = 0x1000;
    record->scale_x = 0x1000;
    record->unknown_08 = 0;
    record->unknown_05 = 0;

    magic = &magic_records[kind];

    switch (kind) {
    case 5:
        record->unknown_04 = 0xff;
        record->unknown_02 = 0;
        record->unknown_03 = 0;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 7:
        record->unknown_04 = 0xff;
        record->unknown_02 = 5;
        record->unknown_03 = 5;
        record->rotation_x = 0x352;
        record->rotation_y = 0;
        record->rotation_z = 0;
        if (va[2] != 0) {
            audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                             &record->position, 0x7f);
        }
        break;
    case 4:
        record->unknown_02 = 6;
        record->unknown_03 = 6;
        record->unknown_04 = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->unknown_38 = *(u16 *)(va + 1);
        if (va[2] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[4] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x17:
        record->unknown_02 = 0x11;
        record->unknown_03 = 0x11;
        record->kind = 4;
        record->unknown_04 = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->unknown_38 = *(u16 *)(va + 1);
        if (va[2] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[4] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x29:
        record->unknown_02 = 0x13;
        record->unknown_03 = 0x13;
        record->kind = 0x20;
        record->unknown_04 = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_range(
            (const SoundRef *)((char *)&magic_records[4] + 5),
            &record->position, 0x7f, 0x4e20, 0xea60);
        break;
    case 0x20:
        record->unknown_02 = 0xb;
        record->unknown_03 = 0xb;
        record->unknown_04 = 0xff;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_range(
            (const SoundRef *)((char *)&magic_records[4] + 5),
            &record->position, 0x7f, 0x4e20, 0xea60);
        break;
    case 0x2a:
        record->unknown_02 = 0xf;
        record->unknown_03 = 0xf;
        record->kind = 0x21;
        record->unknown_04 = 0;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        break;
    case 0x21:
        record->unknown_02 = 0;
        record->unknown_03 = 0;
        record->unknown_04 = 0;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        break;
    case 0x22:
        record->unknown_04 = 0;
        record->unknown_02 = 2;
        record->unknown_03 = 2;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->scale_y = 0;
        break;
    case 6:
        record->unknown_04 = 0;
        record->unknown_02 = 1;
        record->unknown_03 = 1;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->scale_y = 0;
        record->unknown_38 = 6;
        record->unknown_3a = *(u16 *)(va + 1);
        if (record->unknown_3a != 0xff) {
            sound_ref_play((const SoundRef *)((char *)magic + 2), 0x78);
        }
        break;
    case 9:
        record->unknown_04 = 0;
        record->unknown_02 = 3;
        record->unknown_03 = 3;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->scale_z = 0;
        record->scale_y = 0;
        record->scale_x = 0;
        record->unknown_38 = *(u16 *)(va + 1);
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 0xa:
        record->unknown_04 = 0xff;
        record->unknown_02 = 9;
        record->unknown_03 = 9;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->unknown_3a = *(u16 *)(va + 1);
        record->unknown_38 = *(u16 *)(va + 2);
        record->unknown_08 = *(u16 *)(va + 3);
        record->scale_z = *(u16 *)(va + 3);
        record->scale_y = *(u16 *)(va + 3);
        record->scale_x = *(u16 *)(va + 3);
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 8:
        record->unknown_04 = 0;
        record->unknown_02 = 6;
        record->unknown_03 = 6;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        if (va[2] != 0) {
            audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                             &record->position, 0x7f);
        }
        break;
    case 0xb:
        record->unknown_04 = 0xff;
        record->unknown_02 = 8;
        record->unknown_03 = 8;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 0xc:
        record->unknown_04 = 0xff;
        record->unknown_02 = 0xa;
        record->unknown_03 = 0xa;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                         &record->position, 0x7f);
        break;
    case 0xd:
        record->unknown_04 = 0;
        record->unknown_02 = 9;
        record->unknown_03 = 9;
        record->rotation_x = 0;
        record->rotation_y = 0;
        record->rotation_z = 0;
        record->unknown_07 = 0x64;
        record->scale_z = 0x5dc;
        record->scale_y = 0x5dc;
        record->scale_x = 0x5dc;
        record->position.vy += 3500;
        break;
    case 0xe:
        record->unknown_04 = 0;
        record->unknown_02 = 4;
        record->unknown_03 = 4;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        break;
    case 0x30:
        record->unknown_04 = 0;
        record->unknown_02 = 5;
        record->unknown_03 = 5;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        break;
    case 0x16:
        record->unknown_04 = 0;
        record->unknown_02 = 0xe;
        record->unknown_03 = 0xe;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        if (va[2] != 0) {
            audio_play_spatial_default_range((const SoundRef *)((char *)magic + 2),
                                             &record->position, 0x7f);
        }
        break;
    case 0xf:
        record->unknown_04 = 0;
        record->unknown_02 = 7;
        record->unknown_03 = 7;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        record->rotation_x = 0x200;
        record->direction_z = 0;
        record->direction_y = 0;
        record->direction_x = 0;
        record->scale_z = 0xa28;
        record->scale_y = 0xa28;
        record->scale_x = 0xa28;
        break;
    case 0x10:
        record->unknown_04 = 0;
        record->unknown_02 = 7;
        record->unknown_03 = 7;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        record->rotation_x = 0x200;
        record->direction_z = 0;
        record->direction_y = 0;
        record->direction_x = 0;
        break;
    case 0x11:
        record->unknown_04 = 0;
        record->unknown_02 = 9;
        record->unknown_03 = 9;
        record->rotation_z = 0;
        record->rotation_y = 0;
        record->rotation_x = 0;
        record->unknown_38 = 0;
        record->scale_z = 0xaf0;
        record->scale_y = 0xaf0;
        record->scale_x = 0xaf0;
        record->direction_x = record->position.vx >> 8;
        record->direction_z = record->position.vz >> 8;
        record->direction_y = (u16)record->position.vy;
        break;
    case 0x24:
        record->unknown_04 = 0;
        record->unknown_02 = 0xa;
        record->unknown_03 = 0xa;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        record->unknown_3a = 0xff;
        record->unknown_38 = 0xff;
        record->rotation_x = -record->rotation_x;
        audio_play_spatial_range(
            (const SoundRef *)((char *)&magic_records[18] + 5),
            &record->position, 0x7f, 0x4e20, 0xea60);
        break;
    case 0x13:
        record->unknown_04 = 0xff;
        record->unknown_02 = 0xe;
        record->unknown_03 = 0xe;
        record->rotation_z = 0;
        record->rotation_y = 0;
        record->rotation_x = 0;
        record->unknown_38 = *(u8 *)(va + 1);
        break;
    case 0x2c:
        record->unknown_02 = 0x11;
        record->unknown_03 = 0x11;
        record->kind = 0x12;
        record->unknown_04 = 0;
        if (va[1] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[18] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x12:
        record->unknown_02 = 0xb;
        record->unknown_03 = 0xb;
        record->unknown_04 = 0;
        if (va[1] != 0) {
            audio_play_spatial_range(
                (const SoundRef *)((char *)&magic_records[18] + 2),
                &record->position, 0x7f, 0x4e20, 0xea60);
        }
        break;
    case 0x18:
        record->unknown_02 = 0x10;
        record->unknown_03 = 0x10;
        record->kind = 0x14;
        record->unknown_04 = 0;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        record->unknown_38 = *(u8 *)(va + 2);
        record->direction_x = record->rotation_x;
        record->direction_y = record->rotation_y;
        record->direction_z = record->rotation_z;
        record->unknown_32 = record->unknown_22;
        record->scale_y = 0x800;
        record->scale_x = 0x800;
        if (va[3] != 0) {
            audio_play_spatial_default_range(
                (const SoundRef *)((char *)&magic_records[20] + 2),
                &record->position, 0x7f);
        }
        break;
    case 0x14:
        record->unknown_02 = 0xc;
        record->unknown_03 = 0xc;
        record->unknown_04 = 0;
        record->rotation_x = ((SVECTOR *)va[1])->vx;
        record->rotation_y = ((SVECTOR *)va[1])->vy;
        record->rotation_z = ((SVECTOR *)va[1])->vz;
        record->unknown_22 = ((SVECTOR *)va[1])->pad;
        record->rotation_x = -record->rotation_x;
        record->unknown_38 = *(u8 *)(va + 2);
        record->direction_x = record->rotation_x;
        record->direction_y = record->rotation_y;
        record->direction_z = record->rotation_z;
        record->unknown_32 = record->unknown_22;
        record->scale_y = 0x800;
        record->scale_x = 0x800;
        if (va[3] != 0) {
            audio_play_spatial_default_range(
                (const SoundRef *)((char *)&magic_records[20] + 2),
                &record->position, 0x7f);
        }
        break;
    case 0x15:
        record->unknown_04 = 0;
        record->unknown_02 = 0xd;
        record->unknown_03 = 0xd;
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
        record->unknown_02 = 0xff;
        record->unknown_03 = 0xff;
        record->kind = 0x34;
        record->type = 0xf0;
        record->unknown_07 = 0;
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
