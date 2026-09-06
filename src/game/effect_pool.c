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

/* The whole shared state is cleared at startup. The selector reaches the magic
 * rows by a member-relative offset from the current-record pointer slot. */

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
    s32 *va = (s32 *)&direction;   /* variadic stack base: va[1]=arg6, va[2]=arg7, va[3]=arg8 */
    KfEffectRecord *record = effect_pool_find_free();
    KfMagicRecord *magic;

    if (record != 0) {
        record->type = type;
        record->kind = kind;
        record->position = *position;
        record->direction.vector = *direction;
        record->unknown_07 = 0;
        record->id = id;
        record->scale_z = 0x1000;
        record->scale_y = 0x1000;
        record->scale_x = 0x1000;
        record->unknown_08 = 0;
        record->unknown_05 = 0;

        magic = &magic_records[record->kind];

        switch (record->kind) {
        case 5:
            record->unknown_04 = 0xff;
            record->unknown_02 = 0;
            record->unknown_03 = 0;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, 0x7f);
            break;
        case 7:
            record->unknown_04 = 0xff;
            record->unknown_02 = 5;
            record->unknown_03 = 5;
            record->rotation.vx = 0x352;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, 0x7f);
            }
            break;
        case 0x17:
            record->unknown_02 = 0x11;
            record->unknown_03 = 0x11;
            record->kind = 4;
            goto initialize_kind_04;
        case 4:
            record->unknown_02 = 6;
            record->unknown_03 = 6;
        initialize_kind_04:
            record->unknown_04 = 0xff;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->unknown_38.value = *(u16 *)(va + 1);
            if (va[2] != 0) {
                audio_play_spatial_range(
                    &magic_records[4].sounds[0],
                    &record->position, 0x7f, 0x4e20, 0xea60);
            }
            break;
        case 0x29:
            record->unknown_02 = 0x13;
            record->unknown_03 = 0x13;
            record->kind = 0x20;
            goto initialize_kind_20;
        case 0x20:
            record->unknown_02 = 0xb;
            record->unknown_03 = 0xb;
        initialize_kind_20:
            record->unknown_04 = 0xff;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_range(
                &magic_records[4].sounds[1],
                &record->position, 0x7f, 0x4e20, 0xea60);
            break;
        case 0x2a:
            record->unknown_02 = 0xf;
            record->unknown_03 = 0xf;
            record->kind = 0x21;
            record->unknown_04 = 0;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            break;
        case 0x21:
            record->unknown_02 = 0;
            record->unknown_03 = 0;
            record->unknown_04 = 0;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            break;
        case 6:
            record->unknown_04 = 0;
            record->unknown_02 = 1;
            record->unknown_03 = 1;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_y = 0;
            {
                u16 argument = *(u16 *)(va + 1);
                record->unknown_38.value = 6;
                record->unknown_3a = argument;
            }
            if (record->unknown_3a != 0xff) {
                sound_ref_play(&magic->sounds[0], 0x78);
            }
            break;
        case 0x22:
            record->unknown_04 = 0;
            record->unknown_02 = 2;
            record->unknown_03 = 2;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_y = 0;
            break;
        case 9:
            record->unknown_04 = 0;
            record->unknown_02 = 3;
            record->unknown_03 = 3;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->scale_z = 0;
            record->scale_y = 0;
            record->scale_x = 0;
            record->unknown_38.value = *(u16 *)(va + 1);
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, 0x7f);
            break;
        case 0xa:
            record->unknown_04 = 0xff;
            record->unknown_02 = 9;
            record->unknown_03 = 9;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            record->unknown_3a = *(u16 *)(va + 1);
            record->unknown_38.value = *(u16 *)(va + 2);
            record->scale_x = record->scale_y = record->scale_z =
                record->unknown_08 = *(u16 *)(va + 3);
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, 0x7f);
            break;
        case 0xb:
            record->unknown_04 = 0xff;
            record->unknown_02 = 8;
            record->unknown_03 = 8;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, 0x7f);
            break;
        case 0xc:
            record->unknown_04 = 0xff;
            record->unknown_02 = 0xa;
            record->unknown_03 = 0xa;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
            audio_play_spatial_default_range(&magic->sounds[0],
                                             &record->position, 0x7f);
            break;
        case 0xd:
            record->unknown_04 = 0;
            record->unknown_02 = 9;
            record->unknown_03 = 9;
            record->rotation.vx = 0;
            record->rotation.vy = 0;
            record->rotation.vz = 0;
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
            record->rotation = *(const SVECTOR *)va[1];
            break;
        case 0x30:
            record->unknown_04 = 0;
            record->unknown_02 = 5;
            record->unknown_03 = 5;
            record->rotation = *(const SVECTOR *)va[1];
            break;
        case 8:
            record->unknown_04 = 0;
            record->unknown_02 = 6;
            record->unknown_03 = 6;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, 0x7f);
            }
            break;
        case 0x16:
            record->unknown_04 = 0;
            record->unknown_02 = 0xe;
            record->unknown_03 = 0xe;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            if (va[2] != 0) {
                audio_play_spatial_default_range(&magic->sounds[0],
                                                 &record->position, 0x7f);
            }
            break;
        case 0xf:
            record->unknown_04 = 0;
            record->unknown_02 = 7;
            record->unknown_03 = 7;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = 0x200;
            record->direction.words.z = 0;
            record->direction.words.y = 0;
            record->direction.words.x = 0;
            record->scale_z = 0xa28;
            record->scale_y = 0xa28;
            record->scale_x = 0xa28;
            break;
        case 0x10:
            record->unknown_04 = 0;
            record->unknown_02 = 7;
            record->unknown_03 = 7;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = 0x200;
            record->direction.words.z = 0;
            record->direction.words.y = 0;
            record->direction.words.x = 0;
            break;
        case 0x11:
            record->unknown_04 = 0;
            record->unknown_02 = 9;
            record->unknown_03 = 9;
            record->rotation.vz = 0;
            record->rotation.vy = 0;
            record->rotation.vx = 0;
            record->unknown_38.value = 0;
            record->scale_z = 0xaf0;
            record->scale_y = 0xaf0;
            record->scale_x = 0xaf0;
            record->direction.words.x = record->position.vx >> 8;
            record->direction.words.z = record->position.vz >> 8;
            record->direction.words.y = (u16)record->position.vy;
            break;
        case 0x24:
            record->unknown_04 = 0;
            record->unknown_02 = 0xa;
            record->unknown_03 = 0xa;
            record->rotation = *(const SVECTOR *)va[1];
            record->unknown_38.bytes.high = 0xff;
            record->unknown_38.bytes.low = 0xff;
            record->rotation.vx = -record->rotation.vx;
            audio_play_spatial_range(
                &magic_records[18].sounds[1],
                &record->position, 0x7f, 0x4e20, 0xea60);
            break;
        case 0x13:
            record->unknown_04 = 0xff;
            record->unknown_02 = 0xe;
            record->unknown_03 = 0xe;
            {
                u8 argument = *(u8 *)(va + 1);
                record->rotation.vz = 0;
                record->rotation.vy = 0;
                record->rotation.vx = 0;
                record->unknown_38.bytes.low = argument;
            }
            break;
        case 0x2c:
            record->unknown_02 = 0x11;
            record->unknown_03 = 0x11;
            record->kind = 0x12;
            record->unknown_04 = 0;
            if (va[1] != 0) {
                audio_play_spatial_range(
                    &magic_records[18].sounds[0],
                    &record->position, 0x7f, 0x4e20, 0xea60);
            }
            break;
        case 0x12:
            record->unknown_02 = 0xb;
            record->unknown_03 = 0xb;
            record->unknown_04 = 0;
            if (va[1] != 0) {
                audio_play_spatial_range(
                    &magic_records[18].sounds[0],
                    &record->position, 0x7f, 0x4e20, 0xea60);
            }
            break;
        case 0x18:
            record->unknown_02 = 0x10;
            record->unknown_03 = 0x10;
            record->kind = 0x14;
            record->unknown_04 = 0;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            record->unknown_38.bytes.low = *(u8 *)(va + 2);
            record->direction.vector = record->rotation;
            record->scale_y = 0x800;
            record->scale_x = 0x800;
            if (va[3] != 0) {
                audio_play_spatial_default_range(
                    &magic_records[20].sounds[0],
                    &record->position, 0x7f);
            }
            break;
        case 0x14:
            record->unknown_02 = 0xc;
            record->unknown_03 = 0xc;
            record->unknown_04 = 0;
            record->rotation = *(const SVECTOR *)va[1];
            record->rotation.vx = -record->rotation.vx;
            record->unknown_38.bytes.low = *(u8 *)(va + 2);
            record->direction.vector = record->rotation;
            record->scale_y = 0x800;
            record->scale_x = 0x800;
            if (va[3] != 0) {
                audio_play_spatial_default_range(
                    &magic_records[20].sounds[0],
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
            record->rotation.vx = 0;
            record->rotation.vy = player_state.camera_rotation.vy;
            record->rotation.vz = 0;
            break;
        default:
            record->type = 0xff;
            break;
        }
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
        record->rotation.vx = rotation_x;
        record->rotation.vy = rotation_y;
        record->rotation.vz = rotation_z;
        record->position.vx = position_x;
        record->position.vy = position_y;
        record->direction.words.y = direction_y;
        record->unknown_02 = 0xff;
        record->unknown_03 = 0xff;
        record->kind = 0x34;
        record->type = 0xf0;
        record->unknown_07 = 0;
        record->direction.words.x = position_x;
        record->direction.words.z = 0;
    }
    return record;
}

ADDRESS(0x8003781c, 0x34)
void effect_pool_set_current(KfEffectRecord *record)
{
    current_effect = record;
    current_effect_magic_record = &magic_records[record->kind];
}
