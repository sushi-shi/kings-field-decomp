#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>


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
 * WIP: the ring, its payload table, and the head cursor are unresolved
 * address-only globals, so the payload table is reached as a byte view offset
 * from the head cursor (the retail shared-symbol addend the original struct
 * produced).  The trailing varargs slot carries the id 0x13 payload.
 */

ADDRESS(0x8001fa44, 0xa0)
void notify_enqueue(int id, ...)
{
    u8 *head;

    if (id == 0xff) {
        return;
    }
    head = &DAT_80095087;
    if (DAT_8009506e[*head] == 0xff) {
        DAT_8009506e[*head] = id;
        if (id == 0x13) {
            u16 *payload = (u16 *)(head - 17);
            payload[*head] = *(u16 *)(&id + 1);
        }
        *head = (*head + 1) & 7;
    }
}


ADDRESS(0x8001fae4, 0x18)
void func_8001fae4(unsigned char *object, int value)
{
    object[1] = value * 11;
}


/*
 * Notification effect state machine, stepped once per frame by the frame
 * renderer (render_frame).  It consumes the notification ring filled by
 * notify_enqueue and drives the six on-screen sprite records that
 * render_enqueue_sprite draws.
 *
 *   phase 0: a queued id starts an effect.  Id 0x13 is a numeric popup whose
 *            payload is split into four digit sprites; any other id shows one
 *            coloured sprite tinted from the id nibbles.
 *   phase 2: hold for fifteen frames.
 *   phase 3: slide out, then clear the records and dequeue the entry.
 *
 * WIP: the ring, its cursors, and the six sprite records are unresolved
 * address-only globals reached by their individual identities; record 0 also
 * anchors the payload sprite pointers handed to func_8001fae4.  The payload
 * table is a byte view offset from the phase cursor (the shared-symbol addend
 * the original struct produced).
 *
 * The menu_format_number digit scratch reserves 24 stack bytes (a fixed buffer
 * larger than the four digits used); the exact element count is unverified but
 * codegen-invariant across it.  With that 56-byte frame the phase/tail cursors
 * bind $a3/$t0 where retail binds $t0/$a3 -- an unattributed register-allocation
 * residue that leaves the body otherwise structurally exact.
 */

extern void menu_format_number(u16 value, s32 count, s32 base, u16 *out);

ADDRESS(0x8001fafc, 0x2cc)
void notify_effect_update(void)
{
    u8 *phase = &DAT_80095088;

    switch (*phase) {
    case 0: {
        u8 tail = DAT_80095086;
        u8 id = DAT_8009506e[tail];
        if (id == 0xff) {
            return;
        }
        *phase = 2;
        DAT_8009508a = 0;
        DAT_80095089 = 15;
        if (id == 0x13) {
            u16 digits[12];
            DAT_80055d20[0] = 0;
            DAT_80055d2e = 1;
            DAT_80055d30 = (id & 0xf0) << 3;
            DAT_80055d31 = (id & 0xf) << 4;
            menu_format_number(((u16 *)((char *)phase - 18))[tail], 4, 0, digits);
            DAT_80055d3c = 1;
            func_8001fae4(&DAT_80055d20[30], digits[3]);
            DAT_80055d4a = 1;
            func_8001fae4(&DAT_80055d20[44], digits[2]);
            DAT_80055d58 = 1;
            func_8001fae4(&DAT_80055d20[58], digits[1]);
            DAT_80055d66 = 1;
            func_8001fae4(&DAT_80055d20[72], digits[0]);
        } else {
            DAT_80055d20[0] = 1;
            DAT_80055d22 = (id & 0xf0) << 3;
            DAT_80055d23 = (id & 0xf) << 4;
            DAT_80055d66 = 0;
            DAT_80055d58 = 0;
            DAT_80055d4a = 0;
            DAT_80055d3c = 0;
            DAT_80055d2e = 0;
        }
        break;
    }
    case 2: {
        u8 counter = DAT_80095089 - 1;
        DAT_80095089 = counter;
        if (counter == 0) {
            *phase = 3;
        }
        break;
    }
    case 3: {
        s16 slide = DAT_8009508a + 128;
        DAT_8009508a = slide;
        if (slide >= 512) {
            u8 *tail = phase - 2;
            u8 id;
            DAT_8009508a = 512;
            DAT_80055d66 = 0;
            DAT_80055d58 = 0;
            DAT_80055d4a = 0;
            DAT_80055d3c = 0;
            DAT_80055d2e = 0;
            DAT_80055d20[0] = 0;
            id = DAT_8009506e[DAT_80095086];
            do {
                DAT_8009506e[*tail] = 0xff;
                *tail = (*tail + 1) & 7;
            } while (id == DAT_8009506e[*tail] && id != 0x13);
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
