#ifndef KF_GAME_TYPES_H
#define KF_GAME_TYPES_H

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long s32;
typedef unsigned long u32;

struct KfMatrix {
    s16 m[3][3];
    s16 pad;
    s32 t[3];
};

struct KfPoolRecord {
    s16 state;
    s16 unknown_02;
    s16 value_04;
    s16 unknown_06;
    u32 unknown_08;
    void *allocation;
    u32 *backlink;
};

typedef char KfMatrix_size_is_32[(sizeof(struct KfMatrix) == 32) ? 1 : -1];
typedef char KfPoolRecord_size_is_20[
    (sizeof(struct KfPoolRecord) == 20) ? 1 : -1];

#endif
